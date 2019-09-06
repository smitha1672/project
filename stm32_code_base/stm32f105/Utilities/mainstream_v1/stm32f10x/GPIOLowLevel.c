//!	< Only for STM series	@{
#if !defined (__ARM_CORTEX_MX__)
#error	The file is only used to STM32
#else

#include "device_config.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"

#include "GPIOLowLevel.h"
#include "GPIOMiddleLevel.h"

#if !defined ( STM32_IAP )
#include "CS8422_ISR_Handler.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "si_platform.h"
#endif

#endif


//_______________________________________________________________________________
#if ( configCS8422_ISR == 1 )
#define GPIOLowLevel_CS8422_ISR EXTI9_5_IRQHandler
#endif

#if ( configSII_DEV953x_PORTING_PLATFORM_STM32 == 1 ) && ( configSIL953x_ISR == 1)
#define SIL953x_INT_LINE EXTI_Line3
#define SIL953x_IRQ EXTI3_IRQn
#define GPIOLowLevel_SIL9533_ISR EXTI3_IRQHandler
#endif


//_______________________________________________________________________________
const GPIO_PhysicalPin PhysicalPins[__IO_MAX] =
{
	{GPIOE, GPIO_Pin_2, eOUT_PIN},	  /*PE2 	  __O_RST_HDMI_SIL953x 	  */
	{GPIOE, GPIO_Pin_4, eIN_PIN},	  /*PE4 	  __I_FREE_02,		 */
	{GPIOE, GPIO_Pin_5, eOUT_PIN},	  /*PE5 	  __O_MCU_EEPROM_WP,		 */
	{GPIOC, GPIO_Pin_0, eOUT_PIN},	  /*PC0 	  __O_DSP_RES,		 */
	{GPIOC, GPIO_Pin_1, eOUT_PIN},	  /*PC1 	  __O_MUTE_REAR, 		 */
	{GPIOC, GPIO_Pin_2, eOUT_PIN},	  /*PC2 	  __O_SAMPLE_RATE,	 */
	{GPIOC, GPIO_Pin_3, eOUT_PIN}, 	  /*PC3 	  __O_RX_PD,  (RF module RX power down) */
	{GPIOA, GPIO_Pin_2, eOUT_PIN},	  /*PA2	  __O_RST_RF,	 */
	{GPIOA, GPIO_Pin_3, eOUT_PIN},	  /*PA3	  __O_RF_PAIRING,	 */
	{GPIOA, GPIO_Pin_4, eOUT_PIN},	  /*PA4		  __O_SPI1_NSS, 	 */
	{GPIOC, GPIO_Pin_4, eIN_PIN},     /*PC4	  __I_SCP1_IRQ, 	 */
	{GPIOC, GPIO_Pin_5, eOUT_PIN},	  /*PC5		  __O_DE_5V,		 */
	{GPIOB, GPIO_Pin_1, eOUT_PIN},	  /*PB1	  __O_EN_24V, 	 */
	{GPIOB, GPIO_Pin_2, eIN_PIN},     /*PB2 	  __I_SCP1_BUSY,	 */
	{GPIOE, GPIO_Pin_7, eOUT_PIN},    /*PE7	  __O_DE_1V8,		 */
	{GPIOE, GPIO_Pin_8, eIN_PIN},	  /*PE8	  __I_BT_DET2, 	 */
	{GPIOE, GPIO_Pin_11, eOUT_PIN},	  /*PE11  __O_EXT_FLASH_CS,		 */
	{GPIOE, GPIO_Pin_12, eOUT_PIN},	  /*PE12  __O_CS8422_RST,	 */
	{GPIOE, GPIO_Pin_13, eIN_PIN},	  /*PE13  __I_BT_DET1,	 */
	{GPIOE, GPIO_Pin_14, eOUT_PIN},	  /*PE14 __O_CLK_SEL,		 */
	{GPIOE, GPIO_Pin_10, eOUT_PIN},    /*PE10 __O_BT_PAIRING,	 */
	{GPIOD, GPIO_Pin_8, eOUT_PIN},	  /*PD8	  __O_HT68_RES, 	 */
	{GPIOD, GPIO_Pin_9, eIN_PIN},	  /*PD9 	  __I_USB_DET,		 */
	{GPIOB, GPIO_Pin_8, eIN_PIN},	  /*PB8	  __I_CEC_LINE, */
	{GPIOB, GPIO_Pin_9, eIN_PIN},	  /*PB9 	  __I_AD_INT,		 */
	{GPIOE, GPIO_Pin_0, eIN_PIN},	  /*PE0 	  __I_AD_OVFL,*/
	{GPIOC, GPIO_Pin_8, eOUT_PIN},	  /*PC8	__O_CS5346_RST */
	{GPIOC, GPIO_Pin_7, eOUT_PIN},	  /*PC7	__O_TAS5713_RST */
	{GPIOD, GPIO_Pin_15, eOUT_PIN},	  /*PD15 __O_BT_REST */ //Jerry Add for BT Use
	{GPIOE, GPIO_Pin_15, eOUT_PIN},   /*PE15 __O_BT_ON */
	{GPIOD, GPIO_Pin_13, eOUT_PIN},	  /*PD13 __O_BT_MFB */
	{GPIOD, GPIO_Pin_12, eOUT_PIN},	  /*PD12 __O_BT_FWD */
	{GPIOD, GPIO_Pin_10, eOUT_PIN},	  /*PD10 __O_BT_REW */
	{GPIOD, GPIO_Pin_11, eOUT_PIN},	  /*PD11 __O_BT_VOLUP */
	{GPIOD, GPIO_Pin_14, eOUT_PIN},	  /*PD14 __O_BT_VOLDOWN */
	{GPIOC, GPIO_Pin_9, eOUT_PIN},	  /*PC9   __O_USB_VBUS */
	{GPIOE, GPIO_Pin_6, eOUT_PIN},	  /*PE6   __O_USB_VIN */
	{GPIOB, GPIO_Pin_0, eIN_PIN },	  /*PB0 	__O_PAIRING_LED */
	{GPIOD, GPIO_Pin_9, eIN_PIN },	  /*PD9 	__I_BT_INF */	//Angus Modify
	{GPIOD, GPIO_Pin_0, eIN_PIN }, 	  /*PD0  __I_HDMI_WAKE*/
	//{GPIOD, GPIO_Pin_1, eIN_PIN },    /*PD1 __I_MHL_CD0 */
	{GPIOD, GPIO_Pin_1, eOUT_PIN },    /*PD1 40 __O_HPD_TO_SII953X */
	{GPIOD, GPIO_Pin_2, eIN_PIN },    /*PD2 __I_HDMI_MUTE_OUT */
	//{GPIOD, GPIO_Pin_3, eIN_PIN },    /*PD3 __I_EN_MHL_VBUS */
	{GPIOD, GPIO_Pin_3, eIN_PIN},	  /*PD3 	42  __I_HPD_FROM_TV 	  */
	{GPIOB, GPIO_Pin_6, eIN_PIN },	  /*PB6  43 __I_HDMI_IN_HPD	*/
	{GPIOD, GPIO_Pin_4, eIN_PIN}, 	  /* PD4 __I_CS8422_GPO_3 */
	{GPIOD, GPIO_Pin_5, eIN_PIN},	  /* PD5  __I_CS8422_GPO_2 */
	{GPIOD, GPIO_Pin_6, eIN_PIN},	  /* PD6  __I_CS8422_GPO_1 */
	{GPIOD, GPIO_Pin_7, eIN_PIN},	  /* PD7  __I_CS8422_GPO_0 */
	{GPIOC, GPIO_Pin_12, eOUT_PIN}	  /* PC12  __O_HPD_TO_DVD */
};

const GPIO_PhysicalPin SPIPins[__SPI_IO_MAX] =
{
	{GPIOC, GPIO_Pin_0, eIN_PIN},	 	 /*PC0 	  __O_DSP_RES,	 */
	{GPIOA, GPIO_Pin_4, eIN_PIN},		 /*PA4	 __O_SPI1_NSS, 	 */
	{GPIOA, GPIO_Pin_5, eIN_PIN},	 	/*PA5	 __O_SPI_SCK, 	 */
	{GPIOA, GPIO_Pin_6, eIN_PIN},		 /*PA6	__O_SPI1_MISO, 	 */
	{GPIOA, GPIO_Pin_7, eIN_PIN},		 /*PA7	__O_SPI1_MOSI, 	 */
};

/*_______________________________________________________________________________________________________________*/
static void GPIOLowLevel_CS8422_ISR_Configure( void );

static void GPIOLowLevel_CS8422_ISR_Control( bool bIsrEnable );

#if !defined ( STM32_IAP )
extern SRC_ISR_OBJECT *pSRCIsr_ObjCtrl;
#endif

const GPIO_ISR_OBJECT GPIOLOWLevelIsr_ObjCtrl =
{
    GPIOLowLevel_CS8422_ISR_Configure,
    GPIOLowLevel_CS8422_ISR_Control
};

const GPIO_ISR_OBJECT *pGPIOIsr_ObjCtrl = &GPIOLOWLevelIsr_ObjCtrl;

/*_______________________________________________________________________________________________________________*/
void SPI_GPIOLowLevel_DeConfiguration( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	for(uint8_t i=0; i < __SPI_IO_MAX; i++ )
	{
		GPIO_InitStructure.GPIO_Pin |= SPIPins[i].PINx;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(SPIPins[i].PORTx, &GPIO_InitStructure);
	}

	RCC_APB2PeriphClockCmd( (RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOC| RCC_APB2Periph_GPIOE), (FunctionalState)ENABLE);

}

void GPIOLowLevel_Configuration( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( (RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOC| RCC_APB2Periph_GPIOD| RCC_APB2Periph_GPIOE), (FunctionalState)ENABLE);


	for(uint8_t i=0; i < __IO_MAX; i++ )
	{
		switch( PhysicalPins[i].IO )
		{
			case eOUT_PIN:
			{
				GPIO_InitStructure.GPIO_Pin = PhysicalPins[i].PINx;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_Init(PhysicalPins[i].PORTx, &GPIO_InitStructure);
			}
				break;

			case eIN_PIN:
			{
				GPIO_InitStructure.GPIO_Pin = PhysicalPins[i].PINx;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_Init(PhysicalPins[i].PORTx, &GPIO_InitStructure);
			}
				break;
		}

	}

}

void GPIOLowLevel_DeConfiguration( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	for(uint8_t i=0; i < __IO_MAX; i++ )
	{
		GPIO_InitStructure.GPIO_Pin |= PhysicalPins[i].PINx;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(PhysicalPins[i].PORTx, &GPIO_InitStructure);
	}

	RCC_APB2PeriphClockCmd( (RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOC| RCC_APB2Periph_GPIOE), (FunctionalState)ENABLE);

}


void GPIOLowLevel_Set( VirtualPin pin )
{
	PhysicalPins[pin].PORTx->BSRR = PhysicalPins[pin].PINx;
};

void GPIOLowLevel_Clr( VirtualPin pin )
{
	PhysicalPins[pin].PORTx->BRR = PhysicalPins[pin].PINx;
}

uint8_t	GPIOLowLevel_RinputPinBit( VirtualPin pin )
{
	uint8_t bitstatus = 0x00;

	if( (PhysicalPins[pin].PORTx->IDR & PhysicalPins[pin].PINx) != (uint32_t)Bit_RESET )
	{
		bitstatus = (uint8_t)Bit_SET;
	}
	else
	{
		bitstatus = (uint8_t)Bit_RESET;
	}

	return bitstatus;
}

uint8_t GPIOLowLevel_RoutputPinBit( VirtualPin pin )
{
	uint8_t bitstatus = 0x00;

	if ((PhysicalPins[pin].PORTx->ODR & PhysicalPins[pin].PINx) != (uint32_t)Bit_RESET)
	{
		bitstatus = (uint8_t)Bit_SET;
	}
	else
	{
		bitstatus = (uint8_t)Bit_RESET;
	}

	return bitstatus;
}

//_________________________________________________________________________________________
static void GPIOLowLevel_CS8422_ISR_Configure( void )
{
#if ( configCS8422_ISR == 1 ) && !defined ( STM32_IAP )
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Configure PB.09 pin as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, (FunctionalState)ENABLE);

    /* Connect EXTI5 Line to PD.05 pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource5);

    /* Configure EXTI5 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = (FunctionalState)DISABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI9_5 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_CS8422_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = (FunctionalState)ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

}

static void GPIOLowLevel_CS8422_ISR_Control( bool bIsrEnable )
{
#if ( configCS8422_ISR == 1 ) && !defined ( STM32_IAP )
    EXTI_InitTypeDef   EXTI_InitStructure;

    if ( bIsrEnable == TRUE )
    {
        EXTI_InitStructure.EXTI_LineCmd = (FunctionalState)ENABLE;
    }
    else if ( bIsrEnable == FALSE )
    {
        EXTI_InitStructure.EXTI_LineCmd = (FunctionalState)DISABLE;
    }
    else
    {
        return;
    }

    /* Configure EXTI5 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure);
#endif
}

void GPIOLowLevel_CS8422_ISR( void )
{
#if ( configCS8422_ISR == 1 ) && !defined ( STM32_IAP )
    uint8 IntType = 0x00;

    if ( EXTI_GetITStatus( EXTI_Line5 ) != RESET )
    {
        IntType = 0x01;

        pSRCIsr_ObjCtrl->queue_send_from_isr( &IntType );

        EXTI_ClearITPendingBit(EXTI_Line5);
    }
#endif
}

void GPIOLowLevel_SIL9533_ISR ( void )
{
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32 == 1 ) && ( configSIL953x_ISR == 1 ) && !defined ( STM32_IAP )
    if ( EXTI_GetITStatus( SIL953x_INT_LINE ) != RESET )
    {
        SiiPlatformInterruptHandler();
        EXTI_ClearITPendingBit(SIL953x_INT_LINE);
    }
#endif
}

void GPIOLowLevel_SIL9533_ISR_Configure( void )
{
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32 == 1 ) && ( configSIL953x_ISR == 1) && !defined ( STM32_IAP )
	GPIO_InitTypeDef   GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	/* Configure PE.03 pin as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Enable AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, (FunctionalState)ENABLE);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);

	EXTI_InitStructure.EXTI_Line = SIL953x_INT_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = (FunctionalState)ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = SIL953x_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_SIL9535_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = (FunctionalState)ENABLE;

	NVIC_Init(&NVIC_InitStructure);
#endif
}


#endif //! (__ARM_CORTEX_MX__) @}

