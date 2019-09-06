#include "Debug.h"
#include "freertos_conf.h"
#include "device_config.h"
#include "CardLowLevel_Platform.h"
#include "stm3210c_usb_audio_codec.h"
#include "EepromDriver.h"

#include "CS8422_ISR_Handler.h"

extern SRC_ISR_OBJECT *pSRCIsr_ObjCtrl;

//!	< External Application ___________________________________________________________________________________@{
//These applicatons is only used on here.
extern void CardLowLevel_initializeCard(void);

extern void SPILowLevel_initialize( void );

extern void SPILowLevel_Deinitialize( void );

extern void GPIOLowLevel_Configuration( );

extern void GPIOLowLevel_SIL9533_ISR_Configure( );

extern void IRLowLevel_initialize(void);

extern void IRLowLevelDecoder_CreateTask( void );

extern void CPU_resetMicroController( void );

extern void BackupAccessLowLevel_Configuration( void );
//!	@}


/*_______________________________________________________________________________________________*/
static void CardLowLevelPlatform_GPIOInitialize(void)
{
	GPIOLowLevel_Configuration( );
	
#ifdef BTMUTE_ENABLE      //Jerry add for BTmute    
	//BTMuteGpio_initialize();     //Jerry add for BT_INF detect mute function.
#endif

	GPIOMiddleLevel_Clr(__O_DE_5V);
	GPIOMiddleLevel_Clr(__O_EN_24V);
	GPIOMiddleLevel_Clr(__O_RX_PD);   //RF-RX Power down
	GPIOMiddleLevel_Clr(__O_BT_MFB);

	//! GPIO Default setting.	@{
    GPIOMiddleLevel_Set( __O_MCU_EEPROM_WP );  //High EEPROM Unprotect so set Low
    GPIOMiddleLevel_Clr( __O_RST_RF );
    GPIOMiddleLevel_Clr( __O_RF_PAIRING );
    GPIOMiddleLevel_Clr(__O_BT_ON);
	
	//! Device reset pin
	GPIOMiddleLevel_Clr( __O_CS8422_RST );
	GPIOMiddleLevel_Clr( __O_CS5346_RST );
	GPIOMiddleLevel_Clr( __O_DSP_RES );
	GPIOMiddleLevel_Clr( __O_TAS5713_RST );
	
	GPIOMiddleLevel_Set( __O_EXT_FLASH_CS );
	//! @}

	//! USB Swith Default is MCU path@{
	GPIOMiddleLevel_Set(__O_USB_VBUS);//vbus
	GPIOMiddleLevel_Clr( __O_USB_VIN );
	GPIOMiddleLevel_Clr( __O_DE_1V8 );
	//!}
        
	GPIOMiddleLevel_Clr( __O_MUTE_REAR);	
	//GPIOMiddleLevel_Set( __O_RST_HDMI_SIL953x );
}

#if ( configEEPROM == 1 ) 
static void CardlowLevelPlatform_mapEeproms()
{
    /* Initialize the EEPROM driver */
    EepromDriver_initialize();

    EepromDriver_mapLogicalToPhysical(LOGICAL_EEPROM_DEVICE_BLOCK0,
                            0xA0,
                            EEPROM_DEVICE_0_CAPACITY_BYTE,
                            EEPROM_DEVICE_0_PAGE_SIZE_BYTE,
                            EEPROM_DEVICE_0_CHUNK_SIZE_BYTE);

}
#endif 

void CardLowLevelPlatform_initializeCard(void)
{
	CardLowLevel_initializeCard();

    IRLowLevel_initialize();

	IRLowLevelDecoder_CreateTask();

	CardLowLevelPlatform_GPIOInitialize( );

#if ( configCS8422_ISR == 1 )    
    pSRCIsr_ObjCtrl->initialize( );
#endif 

#if ( configSIL953x_ISR == 1 )
	GPIOLowLevel_SIL9533_ISR_Configure( );
#endif

    TWI_Deinitialize( );
	TWI_initialize( );

    /*I2C-2 configure*/
    TWI2_Deinitialize( );
	TWI2_initialize( );

    /*SPI is for DSP and SPI flahs control*/
    SPILowLevel_Deinitialize( );
	SPILowLevel_initialize( );

#if ( configI2S_OUT == 1 )
	EVAL_AUDIO_Init(0, 65, 44100);
#endif 	

#if ( configEEPROM == 1 ) 
	/*initial eeprom address*/
	CardlowLevelPlatform_mapEeproms();	
#endif /*EEPROM*/

    BackupAccessLowLevel_Configuration( );
}

void CardLowLevelPlatform_resetSystem(void)
{
	CPU_resetMicroController();
}

