#ifndef __ULD_LOCATION_H__
#define __ULD_LOCATION_H__

#define DSPLoadSize 0x800//0x800   /* 2k */

#define NOISE_FILE_SIZE 0xc35A2
#define SIGNAL_DETECT_RTY 3


/*PCM base on flash_image.txt. uld_set array of size/address pairs*/  
#define PCM_INIT_ULD_LENGTH              0x92c
#define PCM_INIT_ULD_START_LOCATION     (NOISE_FILE_SIZE+0)/*OS*/

#define PCM_GAIN_ULD_LENGTH              0x7268
#define PCM_GAIN_ULD_START_LOCATION     (NOISE_FILE_SIZE+0x92c)/*GAIN*/

#define PCM_CROSSBAR_ULD_LENGTH          0x4970
#define PCM_CROSSBAR_ULD_START_LOCATION (NOISE_FILE_SIZE+0x7b94)/*tv_cs:srs circle surround*/

/*AC3 base on flash_image.txt*/  
#define AC3_INIT_ULD_LENGTH              0x220  
#define AC3_INIT_ULD_START_LOCATION     (NOISE_FILE_SIZE+0xc504)/*Decoder*/

#define AC3_GAIN_ULD_LENGTH              0x987c
#define AC3_GAIN_ULD_START_LOCATION     (NOISE_FILE_SIZE+0xcb20)/*GAIN*/

#define AC3_CROSSBAR_ULD_LENGTH          0x3FC
#define AC3_CROSSBAR_ULD_START_LOCATION (NOISE_FILE_SIZE+0xc724)/*crossbar*/

/*DTS base on flash_image.txt*/  
#define DTS_INIT_ULD_LENGTH              0x508
#define DTS_INIT_ULD_START_LOCATION     (NOISE_FILE_SIZE+0x1639c)/*Decoder*/

#define DTS_GAIN_ULD_LENGTH              0x987c
#define DTS_GAIN_ULD_START_LOCATION     (NOISE_FILE_SIZE+0x168a4)/*GAIN*/

#define DTS_CROSSBAR_ULD_LENGTH          0x3FC
#define DTS_CROSSBAR_ULD_START_LOCATION (NOISE_FILE_SIZE+0xc724)/*crossbar*/
#endif /*__ULD_LOCATION_H__*/
