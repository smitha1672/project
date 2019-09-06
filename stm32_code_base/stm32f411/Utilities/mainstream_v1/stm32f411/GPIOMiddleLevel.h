#ifndef __GPIO_MIDDLE_LEVEL_H__
#define __GPIO_MIDDLE_LEVEL_H__

#include "Defs.h"

typedef enum 
{ 
    __O_DSP_RES		=0,
    __O_MCU_EEPROM_WP,
    __O_RX_PD,
    __O_RF_PAIRING,
    __O_PAIRING_LED,
    __O_MUTE_REAR,
    __O_USB_VIN,
    __I_HDMI_WAKE,
    __I_MHL_CD0,
    __O_HT68_RES,
    __I_BT_DET2,
    __O_BT_PAIRING,
    __O_EXT_FLASH_CS,
    __O_RST_AMP,
    __I_BT_DET1,
    __O_CLK_SEL,
    __O_BT_ON,
    __I_BT_DET3,
    __O_RST_RF,
    __O_MUTE_AMP,
    __I_BT_INF,
    __O_BT_REW,
    __O_BT_VOLUP,
    __O_BT_FWD,
    __O_BT_MFB,
    __O_BT_VOLDOWN,
    __O_BT_REST,
    __O_EN_1V,
    __I_CEC_LINE,
    __O_CS5346_RST,
    __O_DE_5V,
    __O_EN_24V,
    __I_AD_OVFL,
    __I_AD_INT,
    __I_CS8422_GPO_3,
    __I_CS8422_GPO_2,
    __I_CS8422_GPO_1,
    __I_CS8422_GPO_0,
    __O_CS8422_RST,
    __O_RST_HDMI_SIL953x,
    __I_HDMI_MUTE_OUT,
    __I_SCP1_BUSY,
    __I_SCP1_IRQ,
    __O_SPI1_NSS,
	__IO_MAX,
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
