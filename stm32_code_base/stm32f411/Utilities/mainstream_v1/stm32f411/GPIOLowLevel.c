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

#define RCC_PERIPH_GPIOA RCC_AHB1Periph_GPIOA
#define RCC_PERIPH_GPIOB RCC_AHB1Periph_GPIOB
#define RCC_PERIPH_GPIOC RCC_AHB1Periph_GPIOC
#define RCC_PERIPH_GPIOD RCC_AHB1Periph_GPIOD
#define RCC_PERIPH_GPIOE RCC_AHB1Periph_GPIOE
#define GPIO_SPEED GPIO_Speed_100MHz
#define GPIO_MODE_IN GPIO_Mode_IN
#define GPIO_MODE_OUT GPIO_Mode_OUT

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

#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F105 == 1 ) && ( configSIL953x_ISR == 1)
#define SIL953x_INT_LINE EXTI_Line3
#define SIL953x_IRQ EXTI3_IRQn
#define GPIOLowLevel_SIL9533_ISR EXTI3_IRQHandler
#endif 

#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F411 == 1 ) && ( configSIL953x_ISR == 1)
#define SIL953x_INT_LINE EXTI_Line1
#define SIL953x_IRQ EXTI1_IRQn
#define GPIOLowLevel_SIL9533_ISR EXTI1_IRQHandler
#endif 

const GPIO_PhysicalPin PhysicalPins[__IO_MAX] = 
{
    {GPIOE, GPIO_Pin_3, eOUT_PIN},      /* PE3  __O_DSP_RES, */
    {GPIOC, GPIO_Pin_0, eOUT_PIN},      /* PC0  __O_MCU_EEPROM_WP, */ 
    {GPIOC, GPIO_Pin_3, eOUT_PIN},      /* PC3  __O_RX_PD,  (RF module RX power down) */
    {GPIOA, GPIO_Pin_3, eOUT_PIN},      /* PA3  __O_RF_PAIRING,*/
    {GPIOA, GPIO_Pin_5, eIN_PIN },      /* PA5  __O_PAIRING_LED */
    {GPIOA, GPIO_Pin_6, eOUT_PIN},      /* PA6  __O_MUTE_REAR, */
    {GPIOA, GPIO_Pin_7, eOUT_PIN},      /* PA7  __O_USB_VIN */      
    {GPIOC, GPIO_Pin_4, eIN_PIN },      /* PC4  __I_HDMI_WAKE */
    {GPIOC, GPIO_Pin_5, eIN_PIN },      /* PC5  __I_MHL_CD0 */
    {GPIOE, GPIO_Pin_7, eOUT_PIN},      /* PE7  __O_HT68_RES, */
    {GPIOE, GPIO_Pin_8, eIN_PIN},       /* PE8  __I_BT_DET2, */ 
    {GPIOE, GPIO_Pin_10, eOUT_PIN},     /* PE10 __O_BT_PAIRING, */ 
    {GPIOE, GPIO_Pin_11, eOUT_PIN},     /* PE11 __O_EXT_FLASH_CS, */  
    {GPIOE, GPIO_Pin_12, eOUT_PIN},     /* PE12 __O_RST_AMP*/
    {GPIOE, GPIO_Pin_13, eIN_PIN},      /* PE13 __I_BT_DET1, */

    {GPIOE, GPIO_Pin_14, eOUT_PIN},     /* PE14 __O_CLK_SEL, */       

    {GPIOE, GPIO_Pin_15, eOUT_PIN},     /* PE15 __O_BT_ON */ 
    {GPIOB, GPIO_Pin_10, eIN_PIN},      /* PB10 __I_BT_DET3, */ 
    {GPIOB, GPIO_Pin_14, eOUT_PIN},     /* PB14 __O_RST_RF,*/
    {GPIOD, GPIO_Pin_8, eOUT_PIN},      /* PD8  __O_MUTE_AMP,*/
    {GPIOD, GPIO_Pin_9, eIN_PIN },      /* PD9  __I_BT_INF */ 
    {GPIOD, GPIO_Pin_10, eOUT_PIN},     /* PD10 __O_BT_REW */
    {GPIOD, GPIO_Pin_11, eOUT_PIN},     /* PD11 __O_BT_VOLUP */
    {GPIOD, GPIO_Pin_12, eOUT_PIN},     /* PD12 __O_BT_FWD */
    {GPIOD, GPIO_Pin_13, eOUT_PIN},     /* PD13 __O_BT_MFB */
    {GPIOD, GPIO_Pin_14, eOUT_PIN},     /* PD14 __O_BT_VOLDOWN */
    {GPIOD, GPIO_Pin_15, eOUT_PIN},     /* PD15 __O_BT_REST */

    {GPIOC, GPIO_Pin_6, eOUT_PIN},      /* PC6  __O_EN_1V, */
    {GPIOC, GPIO_Pin_8, eIN_PIN},       /* PC8  __I_CEC_LINE, */
    {GPIOC, GPIO_Pin_11, eOUT_PIN},     /* PC11 __O_CS5346_RST */

    {GPIOD, GPIO_Pin_0, eOUT_PIN},      /* PD0  __O_DE_5V, */
    {GPIOD, GPIO_Pin_1, eOUT_PIN},      /* PD1  __O_EN_24V, */

    {GPIOD, GPIO_Pin_2, eIN_PIN},       /* PD2  __I_AD_OVFL,*/ 
    {GPIOD, GPIO_Pin_3, eIN_PIN},       /* PD3  __I_AD_INT, */   

    {GPIOD, GPIO_Pin_4, eIN_PIN},       /* PD4  __I_CS8422_GPO_3 */
    {GPIOD, GPIO_Pin_5, eIN_PIN},       /* PD5  __I_CS8422_GPO_2 */
    {GPIOD, GPIO_Pin_6, eIN_PIN},       /* PD6  __I_CS8422_GPO_1 */
    {GPIOD, GPIO_Pin_7, eIN_PIN},       /* PD7  __I_CS8422_GPO_0 */

    {GPIOB, GPIO_Pin_5, eOUT_PIN},      /* PB5  __O_CS8422_RST, */
    {GPIOB, GPIO_Pin_9, eOUT_PIN},      /* PB9  __O_RST_HDMI_SIL953x */     
    {GPIOE, GPIO_Pin_10, eIN_PIN },     /* PE10 __I_HDMI_MUTE_OUT */

    {GPIOC, GPIO_Pin_1, eIN_PIN},       /* PC1  __I_SCP1_BUSY, */
    {GPIOC, GPIO_Pin_2, eIN_PIN},       /* PC2  __I_SCP1_IRQ,*/        

    {GPIOE, GPIO_Pin_4, eOUT_PIN},      /* PE4  __O_SPI1_NSS,*/   
    

    //{GPIOC, GPIO_Pin_2, eOUT_PIN},      /* PC2  __O_SAMPLE_RATE,*/
    
    //{GPIOA, GPIO_Pin_9, eOUT_PIN},      /* PA9  __O_USB_VBUS */
    //{GPIOD, GPIO_Pin_3, eIN_PIN },      /* PD3  __I_EN_MHL_VBUS */
    //{GPIOB, GPIO_Pin_6, eIN_PIN },      /* PB6  __I_HDMI_IN_HPD */
};

const GPIO_PhysicalPin SPIPins[__SPI_IO_MAX] = 
{
    {GPIOE, GPIO_Pin_2, eIN_PIN},       /* PE2  __O_SPI_SCK,   */
    {GPIOE, GPIO_Pin_3, eIN_PIN},       /* PE3  __O_DSP_RES,   */    
    {GPIOE, GPIO_Pin_4, eIN_PIN},       /* PE4  __O_SPI1_NSS,  */
    {GPIOE, GPIO_Pin_5, eIN_PIN},       /* PE5  __O_SPI1_MISO, */
    {GPIOE, GPIO_Pin_6, eIN_PIN},       /* PE6  __O_SPI1_MOSI, */
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
		GPIO_InitStructure.GPIO_Mode = GPIO_MODE_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_SPEED;
		GPIO_Init(SPIPins[i].PORTx, &GPIO_InitStructure);
	}
	
	RCC_AHB1PeriphClockCmd( (RCC_PERIPH_GPIOA| RCC_PERIPH_GPIOB| RCC_PERIPH_GPIOC| RCC_PERIPH_GPIOE), ENABLE);

}

void GPIOLowLevel_Configuration( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd( (RCC_PERIPH_GPIOA| RCC_PERIPH_GPIOB| RCC_PERIPH_GPIOC| RCC_PERIPH_GPIOD| RCC_PERIPH_GPIOE), ENABLE);


	for(uint8_t i=0; i < __IO_MAX; i++ )
	{
		switch( PhysicalPins[i].IO )
		{
			case eOUT_PIN:
			{
				GPIO_InitStructure.GPIO_Pin = PhysicalPins[i].PINx;
				GPIO_InitStructure.GPIO_Mode = GPIO_MODE_OUT; 
				GPIO_InitStructure.GPIO_Speed = GPIO_SPEED;
				GPIO_Init(PhysicalPins[i].PORTx, &GPIO_InitStructure);
			}
				break;

			case eIN_PIN:
			{
				GPIO_InitStructure.GPIO_Pin = PhysicalPins[i].PINx;
				GPIO_InitStructure.GPIO_Mode = GPIO_MODE_IN; 
				GPIO_InitStructure.GPIO_Speed = GPIO_SPEED;
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
		GPIO_InitStructure.GPIO_Mode = GPIO_MODE_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_SPEED;
		GPIO_Init(PhysicalPins[i].PORTx, &GPIO_InitStructure);
	}
	
	RCC_AHB1PeriphClockCmd( (RCC_PERIPH_GPIOA| RCC_PERIPH_GPIOB| RCC_PERIPH_GPIOC| RCC_PERIPH_GPIOE), ENABLE);

}


void GPIOLowLevel_Set( VirtualPin pin )
{
	PhysicalPins[pin].PORTx->BSRRL = PhysicalPins[pin].PINx;
};

void GPIOLowLevel_Clr( VirtualPin pin )
{
	PhysicalPins[pin].PORTx->BSRRH = PhysicalPins[pin].PINx;
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

//______________________________________________________________________________

/* F105 ***********************************************************************/
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F105 == 1 )
static void GPIOLowLevel_CS8422_ISR_Configure( void )
{
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F105 == 1 ) && ( configSIL953x_ISR == 1 ) && !defined ( STM32_IAP )
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Configure PB.09 pin as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* Connect EXTI5 Line to PD.05 pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource5);

    /* Configure EXTI5 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI9_5 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_CS8422_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif    

}

static void GPIOLowLevel_CS8422_ISR_Control( bool bIsrEnable )
{
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F105 == 1 ) && ( configSIL953x_ISR == 1 ) && !defined ( STM32_IAP )
    EXTI_InitTypeDef   EXTI_InitStructure;

    if ( bIsrEnable == TRUE )
    {
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    }
    else if ( bIsrEnable == FALSE )
    {
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;
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
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F105 == 1 ) && ( configSIL953x_ISR == 1 ) && !defined ( STM32_IAP )
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
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F105 == 1 ) && ( configSIL953x_ISR == 1 ) && !defined ( STM32_IAP )
    if ( EXTI_GetITStatus( SIL953x_INT_LINE ) != RESET )
    {
        SiiPlatformInterruptHandler();
        EXTI_ClearITPendingBit(SIL953x_INT_LINE);
    }
#endif 
}

void GPIOLowLevel_SIL9533_ISR_Configure( void )
{
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F105 == 1 ) && ( configSIL953x_ISR == 1) && !defined ( STM32_IAP ) 
	GPIO_InitTypeDef   GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	/* Configure PE.03 pin as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Enable AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);

	EXTI_InitStructure.EXTI_Line = SIL953x_INT_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = SIL953x_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_SIL9535_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
#endif 	
}
#endif


/* F411 ***********************************************************************/
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F411 == 1 )
static void GPIOLowLevel_CS8422_ISR_Configure( void )
{
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F411 == 0 ) && ( configSIL953x_ISR == 1 ) && !defined ( STM32_IAP )
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Enable GPIOD clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Configure PD.05 pin as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Connect EXTI Line5 to PD5 pin */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource5);

    /* Configure EXTI5 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI9_5 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_CS8422_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif    

}

static void GPIOLowLevel_CS8422_ISR_Control( bool bIsrEnable )
{
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F411 == 0 ) && ( configSIL953x_ISR == 1 ) && !defined ( STM32_IAP )
    EXTI_InitTypeDef   EXTI_InitStructure;

    if ( bIsrEnable == TRUE )
    {
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    }
    else if ( bIsrEnable == FALSE )
    {
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;
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
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F411 == 0 ) && ( configSIL953x_ISR == 1 ) && !defined ( STM32_IAP )
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
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F411 == 1 ) && ( configSIL953x_ISR == 1 ) && !defined ( STM32_IAP )
    if ( EXTI_GetITStatus( SIL953x_INT_LINE ) != RESET )
    {
        SiiPlatformInterruptHandler();
        EXTI_ClearITPendingBit(SIL953x_INT_LINE);
    }
#endif 
}

void GPIOLowLevel_SIL9533_ISR_Configure( void )
{
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F411 == 1 ) && ( configSIL953x_ISR == 1) && !defined ( STM32_IAP ) 
    GPIO_InitTypeDef   GPIO_InitStructure;
    EXTI_InitTypeDef   EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Configure PE.01 pin as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(GPIOE, &GPIO_InitStructure);


    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1);

    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = SIL953x_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_SIL9535_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
#endif 	
}
#endif

#endif //! (__ARM_CORTEX_MX__) @}

