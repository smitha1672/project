#include "CardLowLevel.h"

#define TRACE_CLK 0
#define TRACE_HSE_CLK 0
#define TRACE_PLL_CLK 1

#if defined (__ARM_CORTEX_MX__)
#define NVIC_VECTOR_TABLE()	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000)
#else
#define NVIC_VECTOR_TABLE()
#endif 

//_____________________________________________________________________________
#if defined ( FREE_RTOS )
/*NVIC_PriorityGroup_4: 4 bits for pre-emption priority,0 bits for subpriority*/
#define PRIORITY_GROP 	NVIC_PriorityGroup_4
#else
#define PRIORITY_GROP 	NVIC_PriorityGroup_1
#endif 

//_____________________________________________________________________________
void RCC_ClockMonitor(void)
{
#if ( TRACE_CLK == 1 )
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Output HSE clock on MCO1 pin(PA8) ****************************************/ 
	/* Enable the GPIOA peripheral */ 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Configure MCO1 pin(PA8) in alternate function */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);

#if ( TRACE_PLL_CLK == 1 ) /*PLL clock is 84MHZ*/
	/* PLL clock selected to output on MCO1 pin(PA8)*/
	RCC_MCO1Config(RCC_MCO1Source_PLLCLK, RCC_MCO1Div_1);
#endif

#if ( TRACE_HSE_CLK == 1 )/*HSE is related crystal frequency(25MHZ)*/
    /* HSE clock selected to output on MCO1 pin(PA8)*/
	RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1);
#endif

#endif	
}

static void CPU_initializeClocks(void)
{
	RCC_ClockMonitor();
}

void CardLowLevel_initializeCard(void)
{
	CPU_initializeClocks();

#if !defined ( STM32_IAP )        
    NVIC_VECTOR_TABLE( );
	NVIC_PriorityGroupConfig(PRIORITY_GROP);

	/*Virtual Timer enable*/
	VirtualTimer_init();
#endif        
}

void CPU_resetMicroController()
{
	NVIC_SystemReset();
}
