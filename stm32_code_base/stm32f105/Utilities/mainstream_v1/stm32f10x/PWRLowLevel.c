#include "PWRLowLevel.h"
#include "config.h"

void PWRLowLevel_initialize(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOB clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	/* Configure PB.09 pin as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Enable AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* Connect EXTI8 Line to PE.08 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource8);

	/* Configure EXTI8 line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI9_5 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PWR_STOP_TRIG_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = PWR_STOP_TRIG_SUBPRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

void PWRLowLevel_EnterStopMode( void )
{
	/* Request to enter STOP mode with regulator in low power mode*/
    PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
}


void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		/* Clear the  EXTI line 8 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line8);
		
	}

}



