#ifndef __GPIO_MIDDLE_LEVEL_H__
#define __GPIO_MIDDLE_LEVEL_H__

#include "Defs.h"

typedef enum { 
	__O_RST_HDMI_SIL953x		=0, 		 
	__I_FREE_02, 				 
	__O_MCU_EEPROM_WP, 	
	__O_DSP_RES,				 
	__O_MUTE_REAR,					 
	__O_SAMPLE_RATE,			 
	__O_RX_PD,   //(RF module RX power down)  ,					 
	__O_RST_RF,				 
	__O_RF_PAIRING, 			 
	__O_SPI1_NSS,				 
	__I_SCP1_IRQ,				 
	__O_DE_5V,					 
	__O_EN_24V,				 
	__I_SCP1_BUSY,				 
	__O_DE_1V8, 				 
	__I_BT_DET2,				 
	__O_EXT_FLASH_CS, 				 
	__O_CS8422_RST, 			 
	__I_BT_DET1,				 
	__O_CLK_SEL,					 
	 __O_BT_PAIRING,
	__O_HT68_RES,				 
	__I_USB_DET,				 
	__I_CEC_LINE,			 
	__I_AD_INT, 				 
	__I_AD_OVFL,				 
	__O_CS5346_RST, 
	__O_TAS5713_RST,
	__O_BT_REST,
	__O_BT_ON,
	__O_BT_MFB,
	__O_BT_FWD,
	__O_BT_REW,
	__O_BT_VOLUP,
	__O_BT_VOLDOWN,
	__O_USB_VBUS,
	__O_USB_VIN,
	__O_PAIRING_LED,    
	__I_BT_INF,
	__I_HDMI_WAKE,//39
	//__I_MHL_CD0,
	__O_HPD_TO_SII953X,//40
	__I_HDMI_MUTE_OUT,//41
	//__I_EN_MHL_VBUS,
	__I_HPD_FROM_TV,//42
	__I_HDMI_IN_HPD,	//43
	__I_CS8422_GPO_3, 
	__I_CS8422_GPO_2,
	__I_CS8422_GPO_1,
	__I_CS8422_GPO_0,
	__O_HPD_TO_DVD,
	__IO_MAX = 49  	

} VirtualPin;

typedef enum { 
	__I_DSP_RES =0,
	__I_SPI1_NSS,	
	__I_SPI_SCK, 
	__I_SPI1_MISO,
	__I_SPI1_MOSI,
	__SPI_IO_MAX = 5	
} SPIPin;

void GPIOMiddleLevel_Set( VirtualPin pin );

void GPIOMiddleLevel_Clr( VirtualPin pin );

uint8 GPIOMiddleLevel_Read_InBit( VirtualPin pin );

uint8 GPIOMiddleLevel_Read_OutBit( VirtualPin pin );


#endif /*__GPIO_MIDDLE_LEVEL_H__*/
