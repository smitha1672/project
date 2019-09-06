#ifndef __DEVICE_CONFIG_H__
#define __DEVICE_CONFIG_H__

/*application config*/
#define configRTOS_DEBUG 0
#define configAPP_ASH 1
#define configAPP_INTERNAL_DSP_ULD 1
#define configAPP_EXT_FLASH_FW_UPDATE 0
#define configLipSync_12steps 1 /* 1 is 20ms*12 steps,0 is 10ms*24steps*/

#if ( configAPP_EXT_FLASH_FW_UPDATE == 1 ) 
#define configSPI_FLASH 1
#endif

#define configDSP_SRAM 0
#define configSII_DEV_953x_PORTING 0 

#endif /*__DEVICE_CONFIG_H__*/
