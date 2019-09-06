//***************************************************************************
//!file     si_regs_audio9535.h
//!brief    SiI9535 Device Register Manifest Constants.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_9535REGS_AUDIO_H__
#define __SI_9535REGS_AUDIO_H__

#include "si_cra_cfg.h"

//------------------------------------------------------------------------------
// PP_PAGE_AUDIO    - Page 15       0xF8
//------------------------------------------------------------------------------

// Software Reset Register
#define REG_AUD_RST                                 (PP_PAGE_AUDIO | 0x00)
#define BIT_FIFORST                                 BIT0    //BIT1
#define BIT_ACRRST                                  BIT1    //BIT2
#define BIT_ALLRST                                  BIT4    //BIT1
#define BIT_AOUTRST                                 BIT3    //BIT1

// ACR Configuration Registers
#define REG_APLL_POLE                               (PP_PAGE_AUDIO | 0x02	) //0x088
#define MSK_APLL_MANUAL_N                               (BIT7 | BIT6)
#define VAL_APLL_MANUAL_N_11                            (0x1 << 7)
#define VAL_AACR_CFG1_VALUE                             (0x88)      // recommended value

#define REG_APLL_CLIP                               (PP_PAGE_AUDIO | 0x03	) //0x089
#define VAL_AACR_CFG2_VALUE           					(0x16)      // recommended value
//#define VAL_LKTHRESH1_VALUE           				(0x20)      // recommended value
//#define VAL_CTS_THRESH_VALUE             				(4)      // recommended value

#define REG_APLL_OVR_CTRL                            (PP_PAGE_AUDIO | 0x06)
#define BIT_APLL_MANUAL_M_4                             (BIT2)
#define BIT_APLL_OVR_N                                  (BIT6)
#define BIT_APLL_OVR_M                                  (BIT7)

// Auto Exception Control 3
#define REG_AUDIOAEC3_CTRL                  			(PP_PAGE_AUDIO | 0x10) 	//0x0B2		
#define BIT_AAC_UNMUTE_GOT_CTS         			BIT0
#define BIT_AAC_UNMUTE_GOT_NEW_AUDIO   			BIT1


// Auto Exception Control 2
#define REG_AUDIOAEC2_CTRL                      		(PP_PAGE_AUDIO | 0x11) 	//0x0B3		
#define BIT_AAC_UNMUTE_GOT_HDMI            		BIT0
#define BIT_AAC_UNMUTE_GOT_PLL             		BIT1
#define BIT_AAC_UNMUTE_GOT_FIFO_UNDER      		BIT2
#define BIT_AAC_UNMUTE_GOT_FIFO_OVER       		BIT3
#define BIT_AAC_UNMUTE_GOT_CTS_CHANGED     		BIT4
#define BIT_AAC_UNMUTE_GOT_N_CHANGED       		BIT5
#define BIT_AAC_UNMUTE_GOT_CTS_RESUED      		BIT6
#define BIT_AAC_UNMUTE_GOT_CTS_DROPPED     		BIT7

#define REG_AUDIOAEC1_CTRL              				(PP_PAGE_AUDIO | 0x12)		//0x0B4

// Auto Exception Control 1
#define REG_AUDIOAEC_CTRL               				(PP_PAGE_AUDIO | 0x13)		//0x0B5
#define BIT_AAC_EN                 				BIT0        // Enable hardware auto mute
#define BIT_AAC_ALL                				BIT1        // Enable hardware auto un-mute
#define BIT_AVC_EN                 				BIT2        // Enable Auto Video Configuration (AVC)
#define BIT_AAC_OE                 				BIT5        // Let AAC to turn off the Sd0, sd1, sd2, sd3, sclk and Spdif
#define BIT_AAC_EXP_SEL            				BIT6        // Let AAC Control audio stability
#define BIT_AAC_ACR_EN             				BIT7        // Let AAC Control ACR: Mclk and Fs

// AEC Exception Enable Registers (3 bytes total)
#define REG_AUDIOAEC_EN1                				(PP_PAGE_AUDIO | 0x14) 		//0x0B6
#define BIT_CABLE_UNPLUG           				BIT0
#define BIT_CKDT_DETECT            				BIT7
#define BIT_SYNC_DETECT            				BIT6
#define BIT_HDMI_MODE_CHANGED      				BIT0
#define BIT_AUDIO_FIFO_UNDERUN     				BIT1
#define BIT_AUDIO_FIFO_OVERRUN     				BIT2
#define BIT_FS_CHANGED             				BIT4
#define BIT_H_RES_CHANGED          				BIT7
#define BIT_V_RES_CHANGED          				BIT0

#define REG_AEC_EN3			    				(PP_PAGE_AUDIO | 0x16)
#define BIT_FN_CLK_CHG							BIT2

#define REG_AUDIOECC_CTRL               				0x0BB
#define BIT_CAPCNT                 					BIT0

#define REG_AUDIOECC_HDCP_THRES         				0x0C5
#define CONF__HDCPTHRESH           					(0x0B40)      // recommended value

// ACR Control Register #1
#define REG_AUDIOACR_CTRL1              				(PP_PAGE_AUDIO | 0x22)  //0x100
#define BIT_ACR_INIT               					BIT0
#define BIT_FS_SEL		    						BIT1

#define REG_AUDIOAAC_MCLK_SEL              			(PP_PAGE_AUDIO | 0x23)  //0x101
#define MSK_MCLK_DSD_SHIFT              			0x00	
#define MSK_MCLK_HBRA_SHIFT              			0x02	
#define MSK_MCLK_PCM_SHIFT              			0x04	
#define MSK_MCLK              						0x03	
#define MSK_MCLK_4DAC             					0x30	
#define MSK_MCLK_4HBRA             					0x0C
#define MSK_MCLK_4DSD            					0x03	

// ACR Audio Frequency Register
#define REG_AUDIOFREQ_SVAL              				(PP_PAGE_AUDIO | 0x24) 	//0x102
#define VAL_SWMCLK_128     						0x00
#define VAL_SWMCLK_256     						0x50
#define VAL_SWMCLK_384     						0xA0
#define VAL_SWMCLK_512     						0xF0
#define MSK_SWMCLK         						0xF0
#define MSK_SWFS           						0x0F

#define VAL_FS_48K     							0x02
#define VAL_FS_192K     							0x0E
#define VAL_FS_768K     							0x09


#define  REG_N_HVAL1_P0         (PP_PAGE_AUDIO | 0x28)
#define  REG_N_HVAL2_P0         (PP_PAGE_AUDIO | 0x29)
#define  REG_N_HVAL3_P0         (PP_PAGE_AUDIO | 0x2A)
#define  REG_CTS_HVAL1_P0       (PP_PAGE_AUDIO | 0x2E)
#define  REG_CTS_HVAL2_P0       (PP_PAGE_AUDIO | 0x2F)
#define  REG_CTS_HVAL3_P0       (PP_PAGE_AUDIO | 0x30)


// ACR PLL Lock Value Registers
#define REG_AUDIOLKTHRESH1             					(PP_PAGE_AUDIO | 0x36) 	//0x114

#define REG_AUDIOTCLK_FS          						(PP_PAGE_AUDIO | 0x39) 	//0x117
#define MSK__TCLKFS           						0x0F

// ACR Control #3 Register
#define REG_AUDIOACR_CTRL3              				(PP_PAGE_AUDIO | 0x3A) 	//0x118
#define VAL_CTS_THRESH_MASK          				0x78  // CTS change threshold
#define VAL_CTS_THRESH_SHIFT          				0x03  // CTS change threshold

// Audio Output Formatting Registers
#define REG_AUDIO_I2S_CTRL1              			(PP_PAGE_AUDIO | 0x45) 	//0x126
#define BIT_SCK                    					BIT6

// Audio Out I2S Control Register #2
#define REG_AUDIO_I2S_CTRL2              			(PP_PAGE_AUDIO | 0x46)		//0x127
#define BIT_SD3                    					BIT7    	// I2S output enables
#define BIT_SD2                    					BIT6
#define BIT_SD1                    					BIT5
#define BIT_SD0                    					BIT4
#define BIT_MCLKEN                 					BIT3    	// MCLK enable
#define BIT_TWO_CHANNELS                 			BIT_SD0
#define MSK_MULTI_CHANNELS                 		    (BIT_SD0 | BIT_SD1 | BIT_SD2 | BIT_SD3)
#define MSK_TWO_CHANNELS                            (BIT_SD0)
#define MSK_DSD_CHANNELS                 			(BIT_SD0 | BIT_SD1 | BIT_SD2)

// Audio Out Control Register
#define REG_AUDIOAUD_CTRL               	(PP_PAGE_AUDIO | 0x48)
#define BIT_MUTE_MODE              					BIT5    // Soft mute enable
#define BIT_PSERR                  					BIT4    // Pass S/PDIF error
#define BIT_PAERR                  					BIT3    // Pass Audio error
#define BIT_I2SMODE                					BIT2
#define BIT_SPEN                   					BIT0    // S/PDIF enable
#define VAL_AUD_CTRL                 	  	(BIT_MUTE_MODE | BIT_PSERR | BIT_PAERR | BIT_I2SMODE)  //use soft mute - pass errors



#define  REG_CHST1_P0                       (PP_PAGE_AUDIO | 0x49)
#define  REG_CHST2_P0                       (PP_PAGE_AUDIO | 0x4A)
#define  REG_CHST3_P0                       (PP_PAGE_AUDIO | 0x4B)
#define  REG_CHST4_P0                       (PP_PAGE_AUDIO | 0x4F)

// Audio Out Control Register
#define REG_AUDIO_FS               					(PP_PAGE_AUDIO | 0x4F)
#define MSK_AUD_FS           						0x0F

#define REG_CHST5_P0                        (PP_PAGE_AUDIO | 0x50)

// Audio FIFO redirection register
#define REG_AUDP_CTRL                       (PP_PAGE_AUDIO | 0x52)
#define BIT_REDIR_AUD_FIFO                          BIT2
#define VAL_AUD_FIFO_RX_EXTR                        0
#define VAL_AUD_FIFO_TX_INSERT                      4

// HDMI Audio Status Register
#define REG_AUDIOAUDP_STAT             	    (PP_PAGE_AUDIO | 0x53)	//0x134
#define BIT_HBRA_ON                					BIT6
#define BIT_DSD_STATUS             					BIT5
#define BIT_AUDIO_LAYOUT            				BIT3  // Audio Layout - ignore unused MSB
#define BIT_HDMI_DET               					BIT0
#define BIT_HDMI_ON									BIT1  //Bug 31751 - Audio extraction is disabled and not resumed after receiving glitch in the audio

// HDMI Mute Register
#define REG_AUDIOHDMI_MUTE             	    (PP_PAGE_AUDIO | 0x56)	//0x137
#define BIT_POL_MOUT               					BIT2    // MUTE_OUT
#define BIT_AUDM                   					BIT1
#define BIT_VIDM                   					BIT0

#define REG_AUDP0_FIFO                              (PP_PAGE_AUDIO | 0x58)

#define REG_AUDP0_TEST0_STAT                (PP_PAGE_AUDIO | 0x5A)
#define BIT_AUDIO_P0_TST_XCLK                       BIT7

#define REG_AU_PD_SYS                       (PP_PAGE_AUDIO | 0x5E)
#define BIT_AUDIO_XTAL_EN                           BIT0
#define BIT_AUDIO_OUT_EN                            BIT7


//------------------------------------------------------------------------------
// PP_PAGE_AUDIO_INTS   - Page 0    0xB0
//------------------------------------------------------------------------------

// Interrupt Status #1 Register
//#define REG_AUDIOINTR1                                0x071
#define REG_AUDIO_INTR11                        (PP_PAGE_AUDIO_INTS |   0x0A2)      //0x071
#define BIT_AAC_UNMUTE                          BIT2
#define BIT_GOTAUD                              BIT0
#define BIT_GOTCTS                              BIT1

// Interrupt Status #5 Register
//#define REG_AUDIOINTR5                                0x07B
#define REG_AUDIOAUD_INTR12                     (PP_PAGE_AUDIO_INTS |   0x0A3)
#define BIT_FIFO_UNDER                          BIT0
#define BIT_FIFO_OVER                           BIT1
#define BIT_CTS_REUSE                           BIT2
#define BIT_CTS_DROP                            BIT3
#define BIT_AAC_DONE                            BIT6
#define MSK_CTS_ERROR                           (BIT_CTS_DROP  | BIT_CTS_REUSE)
#define MSK_FIFO_ERROR                          (BIT_FIFO_OVER | BIT_FIFO_UNDER)

#define BIT_AULERR                              BIT5
#define BIT_AACDONE                             BIT6
#define BIT_FNCHG                               BIT7
#define MSK_AUDIO_INTR                          (BIT_FNCHG | BIT_AACDONE | BIT_AULERR)

#define REG_AUDIOAUD_INTR13                     (PP_PAGE_AUDIO_INTS |   0x0A4)      //0x07B
#define BIT_FSCHG                               BIT0
#define BIT_CHST_RDY                            BIT5

#define BIT_VIDEO_READY                         BIT7
#define BIT_INTR7_CEC_FIFO                      BIT4
#define BIT_INTR7_PWR_CHNG                      BIT5

#define REG_AUDIO_INTR11_MASK                   (PP_PAGE_AUDIO_INTS |   0x0A5)      //0x071

#endif  // __SI_9535REGS_AUDIO_H__

