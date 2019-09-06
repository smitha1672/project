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

#define CS8422_ISR_QUEUE_LENGTH 3
#define PS3_SILENT_AVAILABE 1000

//______________________________________________________________________________________________
/* Struct for easy bit access */
typedef union CS8422_INT_BITS {
    struct {
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
typedef union CS8422_FORMAT_BITS {
    struct {
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
typedef union CS8422_RERR_BITS {
    struct {
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

typedef struct CS8422_ISR_HANDLE_PARAMETERS
{
    xTaskHandle Handle;
    TaskHandleState taskState;
	xSemaphoreParameters SParams;
    xQueueParameters QParams;
} xCS8422IsrHandleParams;

//______________________________________________________________________________________________
static void CS8422_ISR_QueueSendFromISR( uint8 *pIntType );

static void CS8422_ISR_IntConfig( void );

extern GPIO_ISR_OBJECT *pGPIOIsr_ObjCtrl;
extern AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl;

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

void CS8422_ISR_Handler( void *pvParameters )
{
    for( ;; )
    {
         uint8 ReceiverInt = 0x00;
         CS8422_INT_STATUS cs8422_int_status;
         CS8422_FORMAT_STATUS cs8422_fmt_status;
         CS8422_RERR_BITS cs8422_rerr_status;
         static CS8422_FORMAT_STATUS LastFmt;
         static portTickType xTime1Silence = 0;
         static portTickType xTime2Silence = 0;
         portTickType SilenceAvailable = 0;
         AUDIO_EXCEPTION_QUEUE_TYPE tmp_QueueType;
       
        /*Got ISR*/
        if( xQueueReceive( mCS8422IsrParsms.QParams.xQueue, &ReceiverInt, mCS8422IsrParsms.QParams.xBlockTime ) )
        {
            if ( ReceiverInt == CS8422_ISR_FROM_EXTI )
            {
                /* Tony140620:The following can avoid pop noise for the following cases:
                1.Sony BD player : restart play the movie.
                2.PS3 : play movie and music
                3.TV : Switch PCM and DD */
                
                cs8422_int_status.byte = CS8422_ReadI2C_Byte(0x14); /*Get interrupt status*/
                //TRACE_DEBUG((0, "Int = 0x%X", cs8422_int_status.byte ));
                if ( cs8422_int_status.bits.fch )
                {
                    cs8422_fmt_status.byte = CS8422_ReadI2C_Byte(0x12);
                    //TRACE_DEBUG((0, "fmt = 0x%X", cs8422_fmt_status.byte ));
                    if( ( cs8422_fmt_status.bits.dgtl_sil == 1  ) && ( cs8422_fmt_status.bits.pcm == 1 ) ) 
                    {
                        xTime2Silence = xTaskGetTickCount();

                        SilenceAvailable = (xTime2Silence - xTime1Silence);
                        //TRACE_DEBUG((0, " SilenceAvailable = %d", SilenceAvailable ));
                        tmp_QueueType.event = SRC_PCM_SILENCE;
                        pAudLowLevel_ObjCtrl->ExceptionSender(&tmp_QueueType);
                        if ( SilenceAvailable > PS3_SILENT_AVAILABE )
                        {
                            //pAudLowLevel_ObjCtrl->SetMute( SOUND_MUTE );
                        }
                        else
                        {
                            //pAudLowLevel_ObjCtrl->SetMute( SOUND_DEMUTE ); /*PS3 click sound release*/
                        }
                      
                        xTime1Silence = xTime2Silence;
                    }
                    else if( ( cs8422_fmt_status.bits.pcm == 0 ) && ( cs8422_fmt_status.bits.dgtl_sil == 0 ) )/*TV from PCM to DTS or AC3*/
                    {
                        //pAudLowLevel_ObjCtrl->SetMute( SOUND_MUTE );
                        
                        if( ( LastFmt.bits.pcm == 1 ) && ( LastFmt.bits.dgtl_sil == 1 )) 
                        {
                            //pAudLowLevel->LowLevelSetMute( SOUND_MUTE );
                        }
                        
                        LastFmt.byte = cs8422_fmt_status.byte;
                    }
                        
                }

                if ( cs8422_int_status.bits.rerr )
                {
                    cs8422_rerr_status.byte = CS8422_ReadI2C_Byte(0x13);
                    //TRACE_DEBUG((0, "cs8422 error bit =%X",cs8422_rerr_status.byte));
                    if (cs8422_rerr_status.bits.unlock)
                    {
                        pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_SET_MUTE);
                        tmp_QueueType.event = SRC_RECEIVER_ERROR;
                        pAudLowLevel_ObjCtrl->ExceptionSender(&tmp_QueueType);
                    }
                }
            }            
        }
		vTaskDelay(TASK_MSEC2TICKS(1));
    }
}

void CS8422_ISR_SuspendedCtrl( bool val )
{
    uint8 IntType = 0x00;
    uint8 IntCount = 0;

    if ( val == BTASK_SUSPENDED )
    {
         if ( xTaskIsTaskSuspended( mCS8422IsrParsms.Handle ) != pdPASS ) /*task is working*/
         {
            vTaskSuspend( mCS8422IsrParsms.Handle );
            //TRACE_DEBUG((0, "CS8422_ISR_SuspendedCtrl suspend "));
         }
    }
    else
    {
        if ( xTaskIsTaskSuspended( mCS8422IsrParsms.Handle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( mCS8422IsrParsms.Handle );
            //TRACE_DEBUG((0, "CS8422_ISR_SuspendedCtrl resume "));
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
    }

}

static void CS8422_ISR_IntConfig( void )
{
#if ( configCS8422_ISR == 1 )
    pGPIOIsr_ObjCtrl->src_isr_configure( );
    
    mCS8422IsrParsms.QParams.xQueue = xQueueCreate( CS8422_ISR_QUEUE_LENGTH, FRTOS_SIZE(uint8) );
    mCS8422IsrParsms.QParams.xBlockTime = BLOCK_TIME(0);
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
        vTaskSuspend( mCS8422IsrParsms.Handle );
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





