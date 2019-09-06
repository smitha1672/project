#ifndef __DEVICE_CONFIG_H__
#define __DEVICE_CONFIG_H__

#if defined( S4051A )|| defined ( S4551A )
/*application config*/
#define configSTM32F411_PORTING 0

#define configRTOS_DEBUG 0
#define configAPP_UART_CTRL 0
#define configAPP_ASH 1
#define configAPP_USB_WAV_PLAYER 1 /*TODO*/
#define configAPP_USB_VCP 1     /*TODO*/
#define configAPP_CCK 1     /*TODO: Channel check*/
#define configAPP_UDM 1
#define configAPP_APD 1
#define configAPP_AID 1
#define configAPP_IR_REMOTE 1 
#define configAPP_BTN_CTRL 1
#define configAPP_DEV_SII9533 1
#define configAPP_INTERNAL_DSP_ULD 0
#define configAPP_AUDIO_321 1
#define configAPP_AUDIO_200 0
#define configAPP_AUDIO_201 0
#define configAPP_AUDIO_301 0
#define configAPP_STORAGE_MANAGER 1 /*for EEPROM */

#if ( configAPP_INTERNAL_DSP_ULD == 0 )
#define configAPP_SPI_FLASH_DSP_ULD 1 
#endif

#if ( configAPP_SPI_FLASH_DSP_ULD == 1 ) || ( configAPP_CCK == 1 )
#define configSPI_FLASH 1
#endif

#if ( configAPP_USB_WAV_PLAYER == 1 )
#define configUSB_WAV_PLAYER 1
#else
#define configUSB_WAV_PLAYER 0
#endif 

#if ( configAPP_IR_REMOTE == 1 )
#define configIR 1
#endif 

#define configDSP_SRAM 1
#define configI2S_OUT  1 /*for STM32 I2S output*/     
#define configDAP_TAS5727 1 /*for LR channel */
#define configDAP_TAS5711 0 /*for center*/
#define configDAP_TAS5707 0 /*for LR channel*/
#define configDAP_TAS5713 1 /*for center*/
#define configCS4953x 1
#define configCS8422 1
#define configEEPROM 1
#define configSII_DEV_9535 0    //SII_DEV_9535

#if ( configEEPROM ==  1 )
#define configEEPROM_512 1
#else
#define configEEPROM_512 0
#endif 

#if ( configCS8422 == 1 )
#define configCS8422_ISR 1
#endif 

#define errSII_DEV_953x_PORTING_BUILD_ERROR 0
#define configSII_DEV_953x_PORTING 1
#define configSII_DEV_953x_PORTING_CBUS 0 /*Smith modify: We could not support CBUS (MHL) */
#define configRTOS_TIMER 1

#if ( configSII_DEV_953x_PORTING == 1 )
#define configSIL953x_ISR 1
#else
#error "sil953x driver has not been enabled"	
#endif

#if ( configSII_DEV_9535 == 1 )
#warning "hdmi driver is for SIL9535"
#else
//#warning "hdmi driver is for SIL9533"
#endif 


//SII_DEV_953x_PORTING_PLATFORM_LM3S5956
#define configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 0
#if( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
#error "The CPU is for STM32"
#endif 

#define configSII_DEV953x_PORTING_PLATFORM_STM32F105 0
#define configSII_DEV953x_PORTING_PLATFORM_STM32F411 1
#endif 


#endif /*__DEVICE_CONFIG_H__*/
