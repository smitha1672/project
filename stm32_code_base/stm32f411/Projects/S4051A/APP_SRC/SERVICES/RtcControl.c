
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_rtc.h"
#include "stm32fxxx_it.h"
#include "VirtualTimer.h"
#include "Debug.h"
#include "RtcControl.h"
#include "Command.h"
#include "UserParameterManager.h"
#include "GPIOMiddleLevel.h"


#define RTCClockOutput_Enable  /* RTC Clock/64 is output on tamper pin(PC.13) */

uint32_t TimeDisplay = 0;

void RTC_Configuration(void)
{
    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);

    /* Reset Backup Domain */
    BKP_DeInit();

    /* Enable LSE */
    RCC_LSEConfig(RCC_LSE_ON);
    /* Wait till LSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
    {}

    /* Select LSE as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Enable the RTC Second */
    RTC_ITConfig(RTC_IT_SEC, ENABLE);

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Set RTC prescaler: set RTC period to 1sec */
    RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
}

uint32_t Time_Regulate(void)
{
    uint32_t Tmp_HH = 0xFF, Tmp_MM = 0xFF, Tmp_SS = 0xFF;

    //TRACE_INFO((0,"\n==========Time Settings=========="));
    //TRACE_DEBUG((0,"\n  Please Set Hours"));

    while (Tmp_HH == 0xFF)
    {
        Tmp_HH = 0x00;//USART_Scanf(23);
    }
    //TRACE_DEBUG((0,":  %d", Tmp_HH));
    //TRACE_DEBUG((0,"\n  Please Set Minutes"));
    while (Tmp_MM == 0xFF)
    {
        Tmp_MM = 0x00;//USART_Scanf(59);
    }
    //TRACE_DEBUG((0,":  %d", Tmp_MM));
    //TRACE_DEBUG((0,"\n  Please Set Seconds"));
    while (Tmp_SS == 0xFF)
    {
        Tmp_SS = 0x01;//USART_Scanf(59);
    }
    //TRACE_DEBUG((0,":  %d", Tmp_SS));

    /* Return the value to store in RTC counter register */
    return((Tmp_HH * 3600 + Tmp_MM * 60 + Tmp_SS));
}

void Time_Adjust(void)
{
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Change the current time */
    RTC_SetCounter(Time_Regulate());
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
}

void Time_Display(uint32_t TimeVar)
{
    uint32_t THH = 0, TMM = 0, TSS = 0;

    /* Reset RTC Counter when Time is 23:59:59 */
    if (RTC_GetCounter() == 0x0001517F)
    {
        RTC_SetCounter(0x0);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
    }

    /* Compute  hours */
    THH = TimeVar / 3600;
    /* Compute minutes */
    TMM = (TimeVar % 3600) / 60;
    /* Compute seconds */
    TSS = (TimeVar % 3600) % 60;

    TRACE_DEBUG((0, "Time: %d:%d:%d\n", THH, TMM, TSS));
}

void RTCTime_Show(void)
{
    /* Infinite loop */
    if (TimeDisplay == 1)
    {
        TRACE_DEBUG((0, "RTC Time :"));
        /* Display current time */
        Time_Display(RTC_GetCounter());
        TimeDisplay = 0;
    }
}

void RTC_Control(bool RtcEn)
{
    if (RtcEn)
    {
        /* NVIC configuration */
        //NVIC_InitTypeDef NVIC_InitStructure;

#if 0	//Jerry mark for future new FW upgrade behavior
        /* Enable the RTC Interrupt */
        NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
#endif

        if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
        {
            /* Backup data register value is not correct or not yet programmed (when the first time the program is executed) */
            TRACE_DEBUG((0, "\r\n\n RTC not yet configured...."));
            /* RTC Configuration */
            RTC_Configuration();
            TRACE_DEBUG((0, "\r\n RTC configured...."));
            /* Adjust time by values entered by the user on the hyperterminal */
            Time_Adjust();
            BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
        }
        else
        {
            /* Check if the Power On Reset flag is set */
            if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
            {
                TRACE_DEBUG((0, "\r\n\n Power On Reset occurred...."));
            }
            /* Check if the Pin Reset flag is set */
            else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
            {
                TRACE_DEBUG((0, "\r\n\n External Reset occurred...."));
            }
            TRACE_DEBUG((0, "\r\n No need to configure RTC...."));
            /* Wait for RTC registers synchronization */
            RTC_WaitForSynchro();
            /* Enable the RTC Second */
            RTC_ITConfig(RTC_IT_SEC, ENABLE);
            /* Wait until last write operation on RTC registers has finished */
            RTC_WaitForLastTask();
        }
#ifdef RTCClockOutput_Enable
        /* Enable PWR and BKP clocks */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
        /* Allow access to BKP Domain */
        PWR_BackupAccessCmd(ENABLE);
        /* Disable the Tamper Pin */
        BKP_TamperPinCmd(DISABLE); /* To output RTCCLK/64 on Tamper pin, the tamper functionality must be disabled */
        /* Enable RTC Clock Output on Tamper Pin */
        BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
#endif
        /* Clear reset flags */
        RCC_ClearFlag();
    }
    else
    {
    }
}

//!_____________________________________Smith implemented @{

uint32 RealTimeCounter_reportCurrentTime(void)
{

#if defined ( EN_RTC )
    return RTC_GetCounter();
#else
	return VirtualTimer_getCurrentTime();
#endif 

}

