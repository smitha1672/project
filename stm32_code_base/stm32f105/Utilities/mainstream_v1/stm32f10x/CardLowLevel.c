#include "CardLowLevel.h"

//_____________________________________________________________________________
#if defined ( FREE_RTOS )
/*NVIC_PriorityGroup_4: 4 bits for pre-emption priority,0 bits for subpriority*/
#define PRIORITY_GROP 	NVIC_PriorityGroup_4
#else
#define PRIORITY_GROP 	NVIC_PriorityGroup_1
#endif 

//_____________________________________________________________________________
/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;

	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);

	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
	
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1); 

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* ADCCLK = PCLK2/4 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div4); 
  
#ifndef STM32F10X_CL  
    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

#else
    /* Configure PLLs *********************************************************/
    /* PLL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz */
    RCC_PREDIV2Config(RCC_PREDIV2_Div5);
    RCC_PLL2Config(RCC_PLL2Mul_8);

    /* Enable PLL2 */
    RCC_PLL2Cmd(ENABLE);

    /* Wait till PLL2 is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET)
    {}

    /* PLL configuration: PLLCLK = (PLL2 / 5) * 9 = 72 MHz */ 
    RCC_PREDIV1Config(RCC_PREDIV1_Source_PLL2, RCC_PREDIV1_Div5);
    RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);

    /* PPL3 configuration: PLL3CLK = (HSE / 5) * 11 = PLL3_VCO = 110 MHz */
    RCC_PLL3Config(RCC_PLL3Mul_9);
    /* Enable PLL3 */
    RCC_PLL3Cmd(ENABLE);    
    /* Wait till PLL3 is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET)
    {}

    /* Configure I2S clock source: On Connectivity Line Devices, the I2S can be 
        clocked by PLL3 VCO instead of SYS_CLK in order to guarantee higher 
        precision */
    RCC_I2S3CLKConfig(RCC_I2S3CLKSource_PLL3_VCO);
    RCC_I2S2CLKConfig(RCC_I2S2CLKSource_PLL3_VCO);  
#endif

    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }  

	/* Set the Vector Table base location at 0x8000; Don't remove */ 
	NVIC_VECTOR_TABLE();

	NVIC_PriorityGroupConfig( PRIORITY_GROP );

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

}

static void CPU_initializeClocks(void)
{
	//SystemInit();
	RCC_Configuration();
}

void CPU_initializeClocks_24Mhz(void)
{
	//SystemInit_24Mhz();
}

void CPU_initializeClocks_72Mhz(void)
{
	//SystemInit_72Mhz();
}

void CardLowLevel_initializeCard(void)
{
	CPU_initializeClocks(); 

	/*Virtual Timer enable*/
	VirtualTimer_init();
}

void CPU_resetMicroController()
{
	NVIC_SystemReset();
}
