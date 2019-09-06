#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "api_typedef.h"

#include "GPIOMiddleLevel.h"
#include "GPIOLowLevel.h"

#include "AudioDeviceManager.h"
#include "CS8422_ISR_Handler.h"
#include "CS8422.h"

#define CS8422_ISR_QUEUE_LENGTH 20
#define PS3_SILENT_AVAILABE 1000

//______________________________________________________________________________________________
static bool setDSP_PS3SilenceThresHold = FALSE; 
/* Struct for easy bit access */
typedef union CS8422_INT_BITS
{
    struct
    {
        unsigned src_unlock:1;
        unsigned fch:1;
        unsigned qch:1;
        unsigned rerr:1;
        unsigned cch:1;
        unsigned detc:1;
        unsigned oslip:1;
        unsigned pcch:1;
    } bits;
    unsigned char byte;
} CS8422_INT_STATUS;

/* Struct for easy bit access */
typedef union CS8422_FORMAT_BITS
{
    struct
    {
        unsigned reserved:2;
        unsigned dgtl_sil:1;
        unsigned hd_cd:1;
        unsigned dts_cd:1;
        unsigned dts_ld:1;
        unsigned iec61937:1;
        unsigned pcm:1;
    } bits;
    unsigned char byte;
} CS8422_FORMAT_STATUS;

/* Struct for easy bit access */
typedef union CS8422_RERR_BITS
{
    struct
    {
        unsigned par:1;
        unsigned bip:1;
        unsigned conf:1;
        unsigned c:1;
        unsigned unlock:1;
        unsigned ccrc:1;
        unsigned qcrc:1;
        unsigned reserved:1;
    } bits;
    unsigned char byte;
} CS8422_RERR_BITS;

/* Struct for easy bit access */
typedef union CS8422_PLL_STATUS_BITS
{
    struct
    {
        unsigned reserved:3;
        unsigned _192kHz:1;
        unsigned _96khz:1;
        unsigned pll_lock:1;
        unsigned isclock_active:1;
        unsigned rx_active:1;
    } bits;
    unsigned char byte;
} CS8422_PLL_STATUS_BITS;


typedef struct CS8422_ISR_HANDLE_PARAMETERS
{
    xTaskHandle Handle;
    TaskHandleState isr_state;
    xSemaphoreParameters SParams;
    xQueueParameters QParams;
    xSemaphoreHandle taskStateSema;
} xCS8422IsrHandleParams;

//______________________________________________________________________________________________
static void CS8422_ISR_QueueSendFromISR( uint8 *pIntType );

static void CS8422_ISR_IntConfig( void );

static void CS8455_CleanByte(void);

void CS8422_ISR_setState( TaskHandleState set_state );

extern GPIO_ISR_OBJECT *pGPIOIsr_ObjCtrl;
extern AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl;
extern AUDIO_DEVICE_QUEUE_OBJECT *pADM_QueueObj;

const SRC_ISR_OBJECT SRCIsr_ObjCtrl = 
{
    CS8422_ISR_IntConfig,
    CS8422_ISR_QueueSendFromISR
};

const SRC_ISR_OBJECT *pSRCIsr_ObjCtrl = &SRCIsr_ObjCtrl;

//______________________________________________________________________________________________
xCS8422IsrHandleParams mCS8422IsrParsms;

//______________________________________________________________________________________________
static void CS8422_ISR_QueueSendFromISR( uint8 *pIntType )
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if ( *pIntType == CS8422_ISR_FROM_EXTI )
    {
        //TRACE_DEBUG((0, "CS8422 ISR !! "));
        xQueueSendFromISR( mCS8422IsrParsms.QParams.xQueue, pIntType, &xHigherPriorityTaskWoken );
    }
    
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

static void _isr_state_lock(void)
{
    xSemaphoreTake(mCS8422IsrParsms.taskStateSema, portMAX_DELAY);
}

static void _isr_state_unlock(void)
{
    xSemaphoreGive(mCS8422IsrParsms.taskStateSema);
}

static TaskHandleState _get_isr_state(void)
{
    TaskHandleState state;
    _isr_state_lock();
    state = mCS8422IsrParsms.isr_state;
    _isr_state_unlock();

    return state;
}

static void _set_isr_state(TaskHandleState state)
{
    _isr_state_lock();
    mCS8422IsrParsms.isr_state = state;
    _isr_state_unlock();
}

void CS8422_ISR_Handler( void *pvParameters )
{
    uint8 ReceiverInt = 0x00;
    CS8422_INT_STATUS cs8422_int_status;
    CS8422_FORMAT_STATUS cs8422_fmt_status;
    CS8422_RERR_BITS cs8422_rerr_status;
    CS8422_PLL_STATUS_BITS cs8422_pll_status;
    static CS8422_FORMAT_STATUS preAudioFmt = {0};        
    static portTickType xTime1Silence = 0;
    static portTickType xTime2Silence = 0;
    portTickType SilenceAvailable = 0;
    AUDIO_EXCEPTION_QUEUE_TYPE tmp_QueueType;
    xADM_QUEUE_Events adm_q_event = ADM_QUEUE_EVENT_NULL;
    static TaskHandleState prevState = TASK_SUSPENDED;

    for( ;; )
    {
        if( xQueueReceive( mCS8422IsrParsms.QParams.xQueue, &ReceiverInt, mCS8422IsrParsms.QParams.xBlockTime ) == pdPASS)
        {
            TaskHandleState currState = _get_isr_state();
            if ( currState==TASK_RUNING && prevState == TASK_SUSPENDED )
                {
                    CS8455_CleanByte();
                    setDSP_PS3SilenceThresHold = FALSE;           
                    TRACE_DEBUG((0, "mCS8422IsrParsms.isr_state = TASK_RUNING "));
                }
            prevState = currState;
            
            switch ( currState )
            {
                case TASK_SUSPENDED: {}break;
                case TASK_RUNING:
                {
                    /*Got ISR*/
                    if ( ReceiverInt == CS8422_ISR_FROM_EXTI )
                    {
                        /* 
                        Tony140620:The following can avoid pop noise for the following cases:
                        1.Sony BD player : restart play the movie.
                        2.PS3 : play movie and music
                        3.TV : Switch PCM and DD 
                        */
                        
                        cs8422_int_status.byte = CS8422_ReadI2C_Byte(0x14); /*Get interrupt status*/
                        cs8422_pll_status.byte = CS8422_ReadI2C_Byte( 0x15 ); /*read pll status */
                        //TRACE_DEBUG((0, "~~~~ CS8422 interrupt status = 0x%X, pll status = 0x%X", cs8422_int_status.byte, cs8422_pll_status.byte));
                    
                        if ( cs8422_pll_status.bits.pll_lock )
                        {
                            adm_q_event = ADM_QUEUE_EVENT_FROM_ISR_DIGITAL_LOCK;
                            pADM_QueueObj->SignalDetector_sender( &adm_q_event );
                        }   
                        else
                        {
                            adm_q_event = ADM_QUEUE_EVENT_FROM_ISR_DIGITAL_UNLOCK;
                            pADM_QueueObj->SignalDetector_sender( &adm_q_event );
                        }

                        if ( cs8422_int_status.bits.fch )
                        {
                            cs8422_fmt_status.byte = CS8422_ReadI2C_Byte(0x12);
                            TRACE_DEBUG((0, "fmt = 0x%X", cs8422_fmt_status.byte ));

                            if( ( cs8422_fmt_status.bits.dgtl_sil == 1  ) && ( cs8422_fmt_status.bits.pcm == 1 ) ) 
                            {

                                //TRACE_DEBUG((0, " SilenceAvailable = %d", SilenceAvailable ));
                                
                                xTime2Silence = xTaskGetTickCount();
                                SilenceAvailable = (xTime2Silence - xTime1Silence);
                                
                                TRACE_DEBUG((0, " SilenceAvailable = %d", SilenceAvailable ));
                                
                                if(preAudioFmt.byte != 0x84)    /* PCM , Silence (0x84) */
                                {
                                    pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_FCH_VOLUME_DELAY_RELEASE);
                                }
                                else
                                {
                                    //Avoid PS3 click sound disappeared
                                    if(SilenceAvailable > PS3_SILENT_AVAILABE)
                                    {  
                                        pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_FCH_VOLUME_DELAY_RELEASE);
                                    }
                                }

                                if(setDSP_PS3SilenceThresHold == FALSE)
                                {
                                    setDSP_PS3SilenceThresHold = TRUE;
                                    tmp_QueueType.event = SET_DSP_PS3_SILEN_THRES;
                                    pAudLowLevel_ObjCtrl->ExceptionSender(&tmp_QueueType);
                                }
                                
                                xTime1Silence = xTime2Silence;
                            }
                            else if( ( cs8422_fmt_status.bits.pcm == 0 ) && ( cs8422_fmt_status.bits.dgtl_sil == 0 ) )/*TV from PCM to DTS or AC3*/
                            {
                                pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_FCH_VOLUME_DELAY_RELEASE);
                                if(setDSP_PS3SilenceThresHold)
                                {
                                    setDSP_PS3SilenceThresHold = FALSE;
                                    tmp_QueueType.event = SET_DSP_DEFAULT_SILEN_THRES;
                                    pAudLowLevel_ObjCtrl->ExceptionSender(&tmp_QueueType);
                                }
                            }
                            preAudioFmt = cs8422_fmt_status;
                            if ( cs8422_pll_status.bits.pll_lock )
                            {
                                if (  cs8422_pll_status.bits._96khz )
                                {
                                    adm_q_event = ADM_QUEUE_EVENT_FORM_ISR_DIGITAL_OVER_SAMPLE_RATE_48KHZ;
                                    pADM_QueueObj->SignalDetector_sender( &adm_q_event );
                                }
                                else
                                {
                                    adm_q_event = ADM_QUEUE_EVENT_FORM_ISR_DIGITAL_UNDER_SAMPLE_RATE_96KHZ;
                                    pADM_QueueObj->SignalDetector_sender( &adm_q_event );
                                }
                            }
                        }

                        
                        //if ( cs8422_int_status.bits.rerr )
                        {
                            cs8422_rerr_status.byte = CS8422_ReadI2C_Byte(0x13);
                            //TRACE_DEBUG((0, "cs8422 error bit =%X",cs8422_rerr_status.byte));
                            if (cs8422_rerr_status.bits.unlock)
                            {
                                pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_FCH_VOLUME_DELAY_RELEASE);

                                if(setDSP_PS3SilenceThresHold)
                                {
                                    setDSP_PS3SilenceThresHold = FALSE;
                                    tmp_QueueType.event = SET_DSP_DEFAULT_SILEN_THRES;
                                    pAudLowLevel_ObjCtrl->ExceptionSender(&tmp_QueueType);
                                }
                            }
                        }
                    }
                }
                    break;

                default:{}break;   
            }
        }
        #if !CS8422_NEW_TASK_HANDLE
        vTaskDelay(TASK_MSEC2TICKS(1));
        #endif
    }
}

static void CS8455_CleanByte(void)
{
    uint8 IntType = 0x00;
    uint8 IntCount = 0;
    
    /* Tony140630: Clean the bytes in 8422 when resume*/
    for (IntCount =0;IntCount<5;IntCount++)
    {
        IntType = CS8422_ReadI2C_Byte(0x14);
        CS8422_ReadI2C_Byte(0x13); 
        CS8422_ReadI2C_Byte(0x12); 
        if (IntType == 0)
            break;
    }
}

void CS8422_ISR_SuspendedCtrl( bool val )
{
    if ( val == BTASK_SUSPENDED )
    {
        CS8422_ISR_setState(TASK_SUSPENDED);
        TRACE_DEBUG((0, "CS8422_ISR_SuspendedCtrl suspend "));
    }
    else
    {
        CS8455_CleanByte();
        CS8422_ISR_setState(TASK_RUNING);
        TRACE_DEBUG((0, "CS8422_ISR_SuspendedCtrl resume "));
    }
}

void CS8422_ISR_setState( TaskHandleState set_state )
{
#if ( configCS8422_ISR == 1 )
#if 0
    uint8 newState;

    switch(set_state)
    {
    case TASK_SUSPENDED:
        newState = CS8422_TASK_CHANGE_STATE_SUSPEND;
        break;
    case TASK_RUNING:
        newState = CS8422_TASK_CHANGE_STATE_RUNNING;
        break;
    default:
        return;
    }
    // always the latest one is meaningful, kaomin
    //xQueueReset(mCS8422IsrParsms.QParams.xQueue);
    //xQueueSend( mCS8422IsrParsms.QParams.xQueue, &newState, BLOCK_TIME(0)); /*Tony150116 This will clear the interrupt queue from cs8422*/
    xQueueSend( mCS8422IsrParsms.QParams.xQueue, &newState, portMAX_DELAY); 
#else    
    _set_isr_state(set_state);
#endif    
#endif
}

static void CS8422_ISR_IntConfig( void )
{
#if ( configCS8422_ISR == 1 )
    pGPIOIsr_ObjCtrl->src_isr_configure( );
    
    mCS8422IsrParsms.QParams.xQueue = xQueueCreate( CS8422_ISR_QUEUE_LENGTH, FRTOS_SIZE(uint8) );
    mCS8422IsrParsms.QParams.xBlockTime = portMAX_DELAY;
    
    mCS8422IsrParsms.taskStateSema = xSemaphoreCreateMutex();

    if( mCS8422IsrParsms.QParams.xQueue == NULL )
    {
       TRACE_ERROR((0, "mCS8422IsrParsms queue creates failure " ));
    }

    if ( xTaskCreate( 
        CS8422_ISR_Handler, 
        ( portCHAR * ) "CS8422_ISR", 
        STACK_SIZE, 
        NULL, 
        tskCS8422_ISR_HANDLER_PRIORITY, 
        &mCS8422IsrParsms.Handle ) != pdPASS )
    {
        TRACE_ERROR((0, "CS8422_ISR task create failure " ));
    }
    else
    {
        CS8422_ISR_setState(TASK_SUSPENDED);
    }
#endif     
}


void CS8422_ISR_Control( bool bIsrEnable )
{
#if ( configCS8422_ISR == 1 )    
    if ( bIsrEnable == TRUE )
    {
        CS8422_ISR_SuspendedCtrl( BTASK_RESUME );
        pGPIOIsr_ObjCtrl->src_isr_control( bIsrEnable );
    }
    else if ( bIsrEnable == FALSE )
    {
        CS8422_ISR_SuspendedCtrl( BTASK_SUSPENDED );
        pGPIOIsr_ObjCtrl->src_isr_control( bIsrEnable );
    }
    else
    {
        return;
    }
#endif     
}
