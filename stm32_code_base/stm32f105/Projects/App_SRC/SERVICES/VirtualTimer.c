#include "FreeRTOSConfig.h"
#include "freertos_conf.h"
#include "VirtualTimer.h"
#include "stm32f10x.h"
//_________________________________________________________________________

#define TIME_BASE	TIM7
#define TIME_BASE_IRQ	TIM7_IRQn
#define TIME_BASE_CLK	RCC_APB1Periph_TIM7
#define VirtualTimer_ISP	TIM7_IRQHandler

/*----------------------- Private Member Definitions ------------------------ */

#if ( configRTOS_DEBUG == 1 )
xTaskHandle current_task;
char taskname[100];
#endif 


/*@{*/



#if defined( VIRTUAL_TIMER_PROC )
/**
 *  timer event queue
 */
static struct
{
    uint32                  deadline; /**< timer value event is called at */
    uint32                  period;   /**< period at which the event will */
    /**< be repeated                    */
    VirtualTimer_callback   callback; /**< pointer to callback function   */
    void                   *data;     /**< pointer to data to pass        */
    /**< callback                       */
    VirtualTimer_flags      flags;    /**< flags                          */

} m_eventQueue[VIRTUAL_TIMER_MAX_EVENTS];
#endif 

/**
 *  the system timer variable
 */
static volatile uint32 m_now = 0;

/**
 *  sleep timer counter
 */
static volatile uint32 m_sleep = 0;


/**
 *  sleep timer counter
 */
static volatile uint32 m_virtual_sec = 0;

/*@}*/

/*------------------------ Private Member Definitions ---------------------- */

/**
 *  timer ISP that gets called each tick and scans the event list for checking
 *  events to execute
 */
void VirtualTimer_ISP(void)
{

#if defined ( VIRTUAL_TIMER_PROC )
	uint8 i;
#endif 

#if ( configRTOS_DEBUG == 1 )
	current_task = xTaskGetCurrentTaskHandle();
	sprintf( taskname, "%s", pcTaskGetTaskName( current_task ));
#endif 	


	if (TIM_GetITStatus(TIME_BASE, TIM_IT_Update) != RESET)
	{
		/* Increment time */
		m_now++;

		/*Increment time for one second */
		if ( (m_now % VIRTUAL_TIMER_MSEC2TICKS(1000)) == 0 )
		{
			m_virtual_sec++;
			Watchdog_check();
		}

		/* Decrement sleep counter if required */
		if (m_sleep)
		{
			m_sleep--;
		}

#if defined ( VIRTUAL_TIMER_PROC )
	    /* Scan timer event queue to check which events need to be serviced */
	    for (i = 0; i < VIRTUAL_TIMER_MAX_EVENTS; i++)
	    {
	        if (m_eventQueue[i].flags.bits.valid &&
	                m_eventQueue[i].flags.bits.active &&
	                m_eventQueue[i].deadline == m_now)
	        {
	            /* if callback is available... */
	            if (m_eventQueue[i].callback)
	            {
	                if (m_eventQueue[i].flags.bits.realtime)
	                {
	                    /* if it's realtime call it and pass its data */
	                    m_eventQueue[i].callback(m_eventQueue[i].data);
	                }
	                else
	                {
	                    /* otherwise flag it for execution */
	                    m_eventQueue[i].flags.bits.execute = 1;
	                }
	            }
	            if (m_eventQueue[i].flags.bits.periodic == 0)
	            {
	                /* if event is not periodic cancel it */
	                m_eventQueue[i].flags.bits.active = 0;
	            }
	            else
	            {
	                /* if event is periodic update deadline */
	                m_eventQueue[i].deadline = m_now + m_eventQueue[i].period;
	            }
	        }
	    }
#endif 
		/* Pin PC.06 toggling with frequency = 10K Hz */
		//GPIO_WriteBit(GPIOC, GPIO_Pin_6, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_6)));

		TIM_ClearITPendingBit(TIME_BASE, TIM_IT_Update);
	}

}

/*------------------------ Public Member Definitions ------------------------ */

void VirtualTimer_init(void)
{
#if defined ( VIRTUAL_TIMER_PROC )
    /* initialize event queue so that everything is clear                     */
    MEMSET(((char *)m_eventQueue), 0, sizeof(m_eventQueue));
#endif 

    /*Enable TIM7 interrupt in every 50us*/
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;
	uint16_t PrescalerValue = 0;

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(TIME_BASE_CLK, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIME_BASE_IRQ;

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_VIRUTAL_TIMER_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
	//!	@}

	/* ---------------------------------------------------------------
	TIME_BASE Configuration(Timer 7 is for basic timer):
	--------------------------------------------------------------- */
	/* Compute the prescaler value; Setup time base as 10Khz; Timer interrupt is in every 50us */
	PrescalerValue = (SystemCoreClock / 20000 ) - 1;
	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIME_BASE, &TIM_TimeBaseStructure);

	/* TIM IT enable */
	TIM_ITConfig(TIME_BASE, TIM_IT_Update, ENABLE);

	/* TIME_BASE enable counter */
	TIM_Cmd(TIME_BASE, ENABLE);
}

void VirtualTimer_poll(void)
{
#if defined (VIRTUAL_TIMER_PROC)
    uint8 i;

    /* traverse timer event list */
    for (i = 0; i < VIRTUAL_TIMER_MAX_EVENTS; i++)
    {
        if (m_eventQueue[i].flags.bits.execute)
        {
            /* clear execution flag  */
            m_eventQueue[i].flags.bits.execute = 0;

            /* if callback is available execute it */
            if (m_eventQueue[i].callback)
            {
                /* execute callback */
                m_eventQueue[i].callback(m_eventQueue[i].data);
            }
        }
    }
#endif 
	
}

int VirtualTimer_register(
    uint32 time,
    VirtualTimer_callback callback,
    void *data,
    uint8 flags
)
{
    int i = 0;

#if defined ( VIRTUAL_TIMER_PROC )	
    /* Look for an empty entry */
    for (i = 0; i < VIRTUAL_TIMER_MAX_EVENTS; i++)
    {
        if (!m_eventQueue[i].flags.bits.valid)
        {
            break;
        }
    }

    if (i < VIRTUAL_TIMER_MAX_EVENTS)
    {
        /* if we found an empty entry to use let's fill it with event data  */
        m_eventQueue[i].callback          = callback;
        m_eventQueue[i].data              = data;
        m_eventQueue[i].period            = time;
        m_eventQueue[i].deadline          = m_now + time;
        m_eventQueue[i].flags.data        = flags;
        m_eventQueue[i].flags.bits.valid  = 1;
    }
    else
    {
        /* otherwise return -1 to flag no entry was available */
        i = -1;
    }
#endif 

    return i;
}

bool VirtualTimer_cancel(uint8 event)
{
    bool ret = FALSE;

#if defined ( VIRTUAL_TIMER_PROC )
    /* if event valid and was active */
    if ((event < VIRTUAL_TIMER_MAX_EVENTS) &&
            m_eventQueue[event].flags.bits.valid)
    {
        /* disable it */
        m_eventQueue[event].flags.bits.active = 0;

        /* and return TRUE */
        ret = TRUE;
    }
#endif 

    return ret;
}

bool VirtualTimer_remove(uint8 event)
{
    bool ret = FALSE;

#if defined ( VIRTUAL_TIMER_PROC )
    /* if event valid and was active */
    if (event < VIRTUAL_TIMER_MAX_EVENTS      &&
            m_eventQueue[event].flags.bits.valid)
    {
        /* disable and invalidate it */
        m_eventQueue[event].flags.bits.active = 0;
        m_eventQueue[event].flags.bits.valid  = 0;

        /* and return TRUE */
        ret = TRUE;
    }
#endif 	

    return ret;
}

bool VirtualTimer_reschedule(uint8 event, uint32 timeMsec)
{
    bool ret = FALSE;

#if defined ( VIRTUAL_TIMER_PROC )
    /* If event valid and was active */
    if (event < VIRTUAL_TIMER_MAX_EVENTS      &&
            m_eventQueue[event].flags.bits.valid)
    {
        /* Update deadline to new value and activate */
        m_eventQueue[event].deadline =
            m_now + VIRTUAL_TIMER_MSEC2TICKS(timeMsec);
        m_eventQueue[event].flags.bits.active = 1;

        /* and return TRUE */
        ret = TRUE;
    }
#endif 

    return ret;
}

uint32 VirtualTimer_now(void)
{
    return m_now;
}

uint32 VirtualTimer_nowMs(void)
{
    /* Returns the time in MS */
    return VIRTUAL_TIMER_TICKS2MSEC(m_now);
}

void VirtualTimer_sleep(uint32 timeMsec)
{
#if defined ( VIRTUAL_TIMER_PROC )

    m_sleep = VIRTUAL_TIMER_MSEC2TICKS(timeMsec);

    while (m_sleep)
    {
        /* VirtualTimer_poll(); */
    };
#else
    vTaskDelay(TASK_MSEC2TICKS(timeMsec));
#endif 
	
}

void VirtualTimer_minisleep(uint32 time50usec)
{
#if defined ( VIRTUAL_TIMER_PROC )
    m_sleep = VIRTUAL_TIMER_50USEC2TICKS(time50usec);

    while (m_sleep)
    {
        /* VirtualTimer_poll(); */
    };
#endif 	
}

bool VirtualTimer_isActive(uint8 event)
{
#if defined ( VIRTUAL_TIMER_PROC )

    if (event < VIRTUAL_TIMER_MAX_EVENTS)
    {
        return m_eventQueue[event].flags.bits.valid &&
               m_eventQueue[event].flags.bits.active;
    }
#endif 

    return FALSE;
}

bool VirtualTimer_isTimeout(uint32 *pTimeStamp, uint32 time)
{
    if ((VirtualTimer_now() - (*pTimeStamp)) <= time)
    {
        return TRUE;
    }

    *pTimeStamp = VirtualTimer_now();

    return FALSE;
}

/**
 * Get current system time in second. The function converts system tick count
 * to ms.
 */
uint32 VirtualTimer_getCurrentTime()
{
    return ( m_virtual_sec );
}

