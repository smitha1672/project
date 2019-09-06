#include "IRLowLevel.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"

#include "Debug.h"
#include "VirtualTimer.h"

//____________________________________________________________________________________________________________
//! IR configuration	@{
#define IR_RCV_RCC			RCC_APB2Periph_TIM1
#define IR_RCV_PORT_RCC		RCC_AHB1Periph_GPIOE
#define IR_RCV_NVIC_ISR		TIM1_CC_IRQn
#define IR_RCV_IN_CH 		TIM_Channel_1
#define IR_RCV_TRIG_EDGE	TIM_ICPolarity_Falling
#define IR_RCV				TIM1
#define IR_RCV_IN			GPIO_Pin_9
#define IR_RCV_PORT			GPIOE
//!	@}


#define INFRARED_ISR_QUEUE_LENGTH IR_ROW_DATA_MAX
#define BITS_32_MAX 0xffffffff
#define IRLowLevel_ISR  TIM1_CC_IRQHandler //Interrupt process function.

//____________________________________________________________________________________________________________
typedef struct IR_ISR_HANDLE_PARAMETERS
{
    xQueueParameters queue;
    xOS_TaskErrIndicator xOS_ErrId;
} xOS_InfraredIsrParams;

//____________________________________________________________________________________________________________
xOS_InfraredIsrParams xOS_InfraredIsr;

uint16 pluse_duration = 0;
uint32 ir_interrupt_time_tick;

static uint16 IRLowLevel_getQueueNumber( void );

static uint16 IRLowLevel_getRowData( uint16 *pData );

static void IRLowLevel_clrRowData( void );



const IR_LOWLEVEL_OBJECT IR_ObjCtrl = 
{
    IRLowLevel_getRowData,
    IRLowLevel_getQueueNumber,
    IRLowLevel_clrRowData,
};

const IR_LOWLEVEL_OBJECT *pIR_LowLevel = &IR_ObjCtrl;

//____________________________________________________________________________________________________________
static void IRLowLevel_QueueCreate( void )
{
    xOS_InfraredIsr.queue.xQueue = xQueueCreate( INFRARED_ISR_QUEUE_LENGTH, FRTOS_SIZE(uint16) );
    xOS_InfraredIsr.queue.xBlockTime = BLOCK_TIME(0);
    if( xOS_InfraredIsr.queue.xQueue == NULL )
    {
       xOS_InfraredIsr.xOS_ErrId = xOS_TASK_QUEUE_CREATE_FAIL;
    }
}

static uint16 IRLowLevel_getQueueNumber( void )
{
    if( xOS_InfraredIsr.queue.xQueue != NULL )
    {
        return (uint16)uxQueueMessagesWaiting( xOS_InfraredIsr.queue.xQueue );
    }

    return 0;
}

static void IRLowLevel_clrRowData( void )
{
    if ( xOS_InfraredIsr.queue.xQueue != NULL )    
    {
         xQueueReset( xOS_InfraredIsr.queue.xQueue );
    }
}


static uint16 IRLowLevel_getRowData( uint16 *pData )
{
    uint16 buff_number = 0;
    uint16 i = 0;

    if ( pData == NULL )
        return 0;

    if ( xOS_InfraredIsr.queue.xQueue == NULL )
    {
        return 0;
    }    

    buff_number = IRLowLevel_getQueueNumber();
    for( i = 0; i < buff_number; i++ )
    {
        if ( xQueueReceive( xOS_InfraredIsr.queue.xQueue, pData+i , xOS_InfraredIsr.queue.xBlockTime ) != pdPASS )
        {
            xOS_InfraredIsr.xOS_ErrId = xOS_TASK_QUEUE_GET_FAIL;
            break;
        }    
    
    }
    xQueueReset( xOS_InfraredIsr.queue.xQueue );
    return i;

}


void IRLowLevel_initialize( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;

    /* TIM1 clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    /* GPIOA clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    /* Enable the TIM3 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = IR_RCV_NVIC_ISR;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_IR_DECODE_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* TIM1 channel 2 pin (PE.9) configuration */
    GPIO_InitStructure.GPIO_Pin =  IR_RCV_IN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* Connect TIM pins to AF2 */
    GPIO_PinAFConfig(IR_RCV_PORT, GPIO_PinSource9, GPIO_AF_TIM1);
    //! @}

    TIM_ICInitStructure.TIM_Channel = IR_RCV_IN_CH;
    TIM_ICInitStructure.TIM_ICPolarity = IR_RCV_TRIG_EDGE;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;

    TIM_ICInit(IR_RCV, &TIM_ICInitStructure);

    /* TIM enable counter */
    TIM_Cmd(IR_RCV, ENABLE);

    /* Enable the CC1 Interrupt Request */
    TIM_ITConfig(IR_RCV, TIM_IT_CC1, ENABLE);

    IRLowLevel_QueueCreate( );
}

static uint32 IRLowLevel_getTrigDuration(uint32 CurTime)
{
    uint32 duration = 0;
    static uint32 T0 = 0;

    if (CurTime > T0)
    {
        duration = CurTime - T0;
    }
    else
    {
        duration = (BITS_32_MAX - T0) + CurTime;
    }

    T0 = CurTime;
    return (duration);
}

void IRLowLevel_ISR(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken;

     xHigherPriorityTaskWoken = pdFALSE;
    if (TIM_GetITStatus(IR_RCV, TIM_IT_CC1) == SET)
    {
        /* Clear TIM1 Capture compare interrupt pending bit */
        TIM_ClearITPendingBit(IR_RCV, TIM_IT_CC1);
        
        if ( IR_RCV->CCER & 0x0002 ) /*falling edge is done*/
        {
            IR_RCV->CCER = (IR_RCV->CCER & 0xFFFFD); /*configure next edge as rising edge*/
        }
        else if ( !( IR_RCV->CCER & 0x0002 ) ) /*rising edge is done*/
        {
            IR_RCV->CCER = (IR_RCV->CCER | 0x0002); /*configure next edge as rising edge*/
        }

        ir_interrupt_time_tick = VirtualTimer_now( );
        pluse_duration = (uint16)IRLowLevel_getTrigDuration(ir_interrupt_time_tick); 
        if ( pluse_duration < (110000/IR_TIME_BASE) ) 
        {
            if (GPIO_ReadInputDataBit(IR_RCV_PORT, IR_RCV_IN ) == Bit_SET )
            {
                pluse_duration |=0x8000; /*MSB bit is 1 that indicates low pluse*/
            }
            else
            {
                pluse_duration &=0x7FFF; /*MSB bit is 0 that indicates high pluse*/
            }

            /*insert data in QUEUE*/
            if( xOS_InfraredIsr.queue.xQueue != NULL )
            {
                if (xQueueSendFromISR( xOS_InfraredIsr.queue.xQueue, &pluse_duration, &xHigherPriorityTaskWoken )!= pdPASS )
                {
                    xOS_InfraredIsr.xOS_ErrId = xOS_TASK_QUEUE_SET_FAIL;
					pluse_duration = 0;
                }
            }    
        }
        else
        {
            pluse_duration = 0;
            IRLowLevel_clrRowData();
        }
        
    }

    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

