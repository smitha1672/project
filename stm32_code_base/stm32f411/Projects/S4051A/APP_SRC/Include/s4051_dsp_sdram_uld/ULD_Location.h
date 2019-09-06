#ifndef __ULD_LOCATION_H__
#define __ULD_LOCATION_H__

#define DSPLoadSize 0x800//0x800   /* 2k */

#define NOISE_FILE_SIZE 0xc35A2
#define SIGNAL_DETECT_RTY 3


/* ATMOS , We don't need noise because it just for CES demo */
#define ATM_APP_ULD_LENGTH              0x8b84
#define ATM_APP_ULD_START_LOCATION  (0)

#define ATM_DOLBY_ULD_LENGTH              0x2330c
#define ATM_DOLBY_ULD_START_LOCATION  (ATM_APP_ULD_START_LOCATION+ATM_APP_ULD_LENGTH)

#define ATM_OS_ULD_LENGTH              0x1d050
#define ATM_OS_ULD_START_LOCATION  (ATM_DOLBY_ULD_START_LOCATION+ATM_DOLBY_ULD_LENGTH)

#define ATM_HDMI_ULD_LENGTH              0x2a8c
#define ATM_HDMI_ULD_START_LOCATION  (ATM_OS_ULD_START_LOCATION+ATM_OS_ULD_LENGTH)

#define ATM_MAT_ULD_LENGTH              0xe1c4
#define ATM_MAT_ULD_START_LOCATION  (ATM_HDMI_ULD_START_LOCATION+ATM_HDMI_ULD_LENGTH)


/*PCM base on flash_image.txt. uld_set array of size/address pairs*/  
#define PCM_INIT_ULD_LENGTH              0x92C
#define PCM_INIT_ULD_START_LOCATION     (NOISE_FILE_SIZE+0)/*OS*/

#define PCM_GAIN_ULD_LENGTH              0x7284
#define PCM_GAIN_ULD_START_LOCATION     (NOISE_FILE_SIZE+0x92c)/*GAIN*/

#define PCM_CROSSBAR_ULD_LENGTH          0x4970
#define PCM_CROSSBAR_ULD_START_LOCATION (NOISE_FILE_SIZE+0x7bb0)/*tv_cs:srs circle surround*/

/*AC3 base on flash_image.txt*/  
#define AC3_INIT_ULD_LENGTH              0x220    
#define AC3_INIT_ULD_START_LOCATION     (NOISE_FILE_SIZE+0xc520)/*Decoder*/

#define AC3_GAIN_ULD_LENGTH              0x98a0
#define AC3_GAIN_ULD_START_LOCATION     (NOISE_FILE_SIZE+0xcb3c)/*GAIN*/

#define AC3_CROSSBAR_ULD_LENGTH          0x3FC
#define AC3_CROSSBAR_ULD_START_LOCATION (NOISE_FILE_SIZE+0xc740)/*crossbar*/

/*DTS base on flash_image.txt*/  
#define DTS_INIT_ULD_LENGTH              0x508
#define DTS_INIT_ULD_START_LOCATION     (NOISE_FILE_SIZE+0x163dc)/*Decoder*/

#define DTS_GAIN_ULD_LENGTH              0x98a0
#define DTS_GAIN_ULD_START_LOCATION     (NOISE_FILE_SIZE+0x168e4)/*GAIN*/

#define DTS_CROSSBAR_ULD_LENGTH          0x3FC
#define DTS_CROSSBAR_ULD_START_LOCATION (NOISE_FILE_SIZE+0xc740)/*crossbar*/

#endif /*__ULD_LOCATION_H__*/
