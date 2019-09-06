//***************************************************************************
//!file     si_regs_osd.h
//!brief    SiI9535 Device Register Manifest Constants.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_REGS_OSD_H__
#define __SI_REGS_OSD_H__

#include "si_cra_cfg.h"

//------------------------------------------------------------------------------
// Registers in Page 14     (0xF0)
//------------------------------------------------------------------------------
#define REG_OSD_ACCESS_CTRL                 (PP_PAGE_OSD | 0x00)
#define REG_BIT_SPI_START						BIT0
#define REG_BIT_SPI_RELOAD						BIT1
#define REG_BIT_I2C_RAM_WRITE					BIT2
#define REG_BIT_I2C_RAM_RD						BIT3

#define REG_BIT_OSD_FORCE_TO_IDLE				BIT4
#define REG_BIT_INDEX_PATCH_EN					BIT5
#define REG_BIT_SPI_BYPASS_CHECKSUM				BIT6
#define REG_BIT_I2C_TO_RAM_EN					BIT7

#define REG_OSD_INDEX_CTRL                  (PP_PAGE_OSD | 0x01)
#define REG_BIT_ACTIVE_WND						BIT0
#define REG_BIT_INDEX_SWAP						BIT1
#define REG_BIT_INDEX0_ACTIVE                   BIT4
#define REG_BIT_INDEX1_ACTIVE                   BIT5
#define VAL_INDEX_MASK                          (BIT4 | BIT5)
#define REG_BIT_OSD_RESET                       BIT6

#define REG_OSD_RAM_ADDR_L                  (PP_PAGE_OSD | 0x02)
#define REG_OSD_RAM_ADDR_H                  (PP_PAGE_OSD | 0x03)

#define REG_OSD_RAM_WORD_COUNT_L            (PP_PAGE_OSD | 0x04)
#define REG_OSD_RAM_WORD_COUNT_H            (PP_PAGE_OSD | 0x05)

#define REG_OSD_STATE                       (PP_PAGE_OSD | 0x06)
#define MSK_OSD_FSM                             0x0F
#define VAL_OSD_IDLE                            0x00
#define VAL_OSD_OSD_ON                          0x01
#define VAL_OSD_RAM_LOAD                        0x02
#define VAL_OSD_RAM_LOAD_INT                    0x03
#define VAL_OSD_I2C_RAM_READ                    0x04
#define VAL_OSD_RAM_READ_INT                    0x05
#define VAL_OSD_BOOT                            0x06
#define MSK_RAM_FSM                             0x70
#define VAL_RAM_IDLE                            0x00
#define VAL_RAM_LOAD                            0x10
#define VAL_RAM_CHKSUM                          0x20
#define VAL_RAM_FETCH_OFF                       0x30
#define VAL_RAM_FETCH_ON                        0x40
#define VAL_RAM_I2C_RAM_READ                    0x50

#define REG_INT_STATUS_OSD                  (PP_PAGE_OSD | 0x07)
#define REG_BIT_RAM_SPI_CS_DONE					BIT0
#define REG_BIT_RAM_SPI_ERR						BIT1
#define BIT_RAM_I2C_WR_DONE					    BIT2
#define REG_BIT_RAM_I2C_RD_DONE					BIT3
#define REG_BIT_RAM_SPI_TRANSACTION_DONE		BIT4
#define REG_BIT_RAM_SPI_TRANSACTION_ERR			BIT5
#define REG_BIT_RAM_I2C_GRANTED					BIT6

#define REG_OSD_RAM_WR_DATA                 (PP_PAGE_OSD | 0x0A)

#define REG_OSD_RAM_RD_DATA0                (PP_PAGE_OSD | 0x0B)
#define REG_OSD_RAM_RD_DATA1                (PP_PAGE_OSD | 0x0C)
#define REG_OSD_RAM_RD_DATA2                (PP_PAGE_OSD | 0x0D)
#define REG_OSD_RAM_RD_DATA3                (PP_PAGE_OSD | 0x0E)
#define REG_OSD_RAM_RD_DATA4                (PP_PAGE_OSD | 0x0F)
#define REG_OSD_RAM_RD_DATA5                (PP_PAGE_OSD | 0x10)

#define REG_OSD_WIN_CTRL                    (PP_PAGE_OSD | 0x11)
#define BIT_OSD_ENABLE                          BIT0
#define BIT_OSD_INTERLACE_POL                   BIT1
#define VAL_OSD_FONT_SIZE_MASK					0x0C        
#define VAL_OSD_FONT_SIZE_SHIFT					0x02        
#define BIT_OSD_ENABLE_WINDOW_SHIFT				0x04        
#define BIT_OSD_ENABLE_WINDOW_0					BIT4        
#define BIT_OSD_ENABLE_WINDOW_1					BIT5        
#define BIT_OSD_ENABLE_WINDOW_2					BIT6        
#define BIT_OSD_ENABLE_WINDOW_3					BIT7        

#define REG_OSD_BITMAP_SEGMENT_L            (PP_PAGE_OSD | 0x12)
#define REG_OSD_BITMAP_SEGMENT_H            (PP_PAGE_OSD | 0x13)


#define REG_OSD_W0_CI0_L                    (PP_PAGE_OSD | 0x14)
#define REG_OSD_W0_CI0_H                    (PP_PAGE_OSD | 0x15)
#define REG_OSD_W1_CI0_L                    (PP_PAGE_OSD | 0x16)
#define REG_OSD_W1_CI0_H                    (PP_PAGE_OSD | 0x17)
#define REG_OSD_W2_CI0_L                    (PP_PAGE_OSD | 0x18)
#define REG_OSD_W2_CI0_H                    (PP_PAGE_OSD | 0x19)
#define REG_OSD_W3_CI0_L                    (PP_PAGE_OSD | 0x1A)
#define REG_OSD_W3_CI0_H                    (PP_PAGE_OSD | 0x1B)
#define REG_OSD_W0_CI1_L                    (PP_PAGE_OSD | 0x1C)
#define REG_OSD_W0_CI1_H                    (PP_PAGE_OSD | 0x1D)
#define REG_OSD_W1_CI1_L                    (PP_PAGE_OSD | 0x1E)
#define REG_OSD_W1_CI1_H                    (PP_PAGE_OSD | 0x1F)
#define REG_OSD_W2_CI1_L                    (PP_PAGE_OSD | 0x20)
#define REG_OSD_W2_CI1_H                    (PP_PAGE_OSD | 0x21)
#define REG_OSD_W3_CI1_L                    (PP_PAGE_OSD | 0x22)
#define REG_OSD_W3_CI1_H                    (PP_PAGE_OSD | 0x23)

#define REG_OSD_SET_ACTIVE_WIN              (PP_PAGE_OSD | 0x24)


#define REG_OSD_W0_W1_ROW_COUNT             (PP_PAGE_OSD | 0x26)           //total row w0 & w1
#define REG_OSD_W2_W3_ROW_COUNT             (PP_PAGE_OSD | 0x27)           //total row w2 & w3
#define MSK_ROW_MASK_LO                         0x0F
#define MSK_ROW_MASK_HI                         0xF0
#define VAL_ROW_SHIFT_LO                        0x00
#define VAL_ROW_SHIFT_HI                        0x04

#define REG_OSD_W0_ROW_LEN                  (PP_PAGE_OSD | 0x28)
#define REG_OSD_W1_ROW_LEN                  (PP_PAGE_OSD | 0x29)
#define REG_OSD_W2_ROW_LEN                  (PP_PAGE_OSD | 0x2A)
#define REG_OSD_W3_ROW_LEN                  (PP_PAGE_OSD | 0x2B)

#define REG_OSD_W0_X_L                      (PP_PAGE_OSD | 0x30)
#define REG_OSD_W0_X_H                      (PP_PAGE_OSD | 0x31)
#define REG_OSD_W0_Y_L                      (PP_PAGE_OSD | 0x32)
#define REG_OSD_W0_Y_H                      (PP_PAGE_OSD | 0x33)
#define REG_OSD_W0_ROW_CTRL                 (PP_PAGE_OSD | 0x34)
#define MSK_CURSOR_SELECTED                     0x0F
#define MSK_BORDER_WIDTH                        0x30
#define MSK_DIVIDER_WIDTH                       0xC0
#define REG_OSD_W0_DISABLED_ROW_L           (PP_PAGE_OSD | 0x35)
#define REG_OSD_W0_DISABLED_ROW_H           (PP_PAGE_OSD | 0x36)

#define REG_OSD_W0_BORDER                   (PP_PAGE_OSD | 0x37)
#define REG_OSD_W0_DIVIDER                  (PP_PAGE_OSD | 0x38)
#define REG_OSD_W0_SELECTED_FG              (PP_PAGE_OSD | 0x39)
#define REG_OSD_W0_SELECTED_BG              (PP_PAGE_OSD | 0x3A)
#define REG_OSD_W0_UNSEL_FG                 (PP_PAGE_OSD | 0x3B)
#define REG_OSD_W0_UNSEL_BG                 (PP_PAGE_OSD | 0x3C)
#define REG_OSD_W0_DISABLED_FG              (PP_PAGE_OSD | 0x3D)
#define REG_OSD_W0_DISABLED_BG              (PP_PAGE_OSD | 0x3E)

#define REG_OSD_W0_ALPHA                    (PP_PAGE_OSD | 0x70)
#define BIT_OSD_WIN_ALPHA_EN                    0x80
#define MSK_W0_ALPHA_BLENDING                       0x1F

#define REG_OSD_W1_ALPHA                    (PP_PAGE_OSD | 0x71)
#define REG_OSD_W2_ALPHA                    (PP_PAGE_OSD | 0x72)
#define REG_OSD_W3_ALPHA                    (PP_PAGE_OSD | 0x73)

#define REG_OSD_CHAR_PATCH_EN               (PP_PAGE_OSD | 0x74)
#define REG_OSD_C0_PATCH_IDX_L              (PP_PAGE_OSD | 0x75)
#define REG_OSD_C0_PATCH_COL                (PP_PAGE_OSD | 0x76)
#define REG_OSD_C0_PATCH_ROW                (PP_PAGE_OSD | 0x77)
#define REG_INDEX_PATCH_COL_SHIFT               (3)
#define REG_INDEX_PATCH_WND_SHIFT               (4)


#define REG_SPI_CTRL							(PP_PAGE_OSD | 0x8D)
#define REG_SPI_OP_CMD							(PP_PAGE_OSD | 0x8E)
#define BIT_SPI_ADDR_SEND						BIT0
#define BIT_SPI_READ							BIT4     //read data": yes read data, no write data, yes address
#define BIT_SPI_WRITE							BIT5     //" write data":  no read data, Yes write data, yes address
#define BIT_SPI_NONE							0x00	//for "write enable": only NO read data, no write data, no address 

#define REG_SPI_ADDR_LO							(PP_PAGE_OSD | 0x8F)
#define REG_SPI_ADDR_MID						(PP_PAGE_OSD | 0x90)
#define REG_SPI_ADDR_HI							(PP_PAGE_OSD | 0x91)

#define REG_SPI_LENGTH_LO						(PP_PAGE_OSD | 0x94)
#define REG_SPI_LENGTH_HI						(PP_PAGE_OSD | 0x95)

#define REG_SPI_WRITE_SIZE						(PP_PAGE_OSD | 0x96)

#define REG_SPI_CMD								(PP_PAGE_OSD | 0x92)
#define VAL_SPI_READ                            0x03    //Read data bytes
#define VAL_SPI_WREN							0x06	//Write enable
#define VAL_SPI_WRDI							0x04	//Write disable
#define VAL_SPI_RDID							0x9F	//Read identification
#define VAL_SPI_RDSR							0x05	//Read status register

#define BIT_RDSR_WIP							BIT0	// 1: Write in progress, 0: Not access
#define BIT_RDSR_WEL							BIT1	// 1: latch 0: no latch 											// 1 mean busy with write, program or erase
														
#define VAL_SPI_READ							0x03	//Read data bytes
#define VAL_SPI_FAST_READ						0x0B	//Fast Read
#define VAL_SPI_PW								0x0A	//Page Write
#define VAL_SPI_PP								0x02	//Page Program
#define VAL_SPI_PE								0xDB	//Page Erase
#define VAL_SPI_SE								0xD8	//Sector Erase
#define VAL_SPI_DP								0xB9	//Deep power-down
#define VAL_SPI_RDP								0xAB	//Release Deep power down


#define REG_SPI_TX_DATA							(PP_PAGE_OSD | 0x93)

#define REG_FL_STATUS_ID0						(PP_PAGE_OSD | 0x98)
#define REG_FL_STATUS_ID1						(PP_PAGE_OSD | 0x99)
#define REG_FL_STATUS_ID2						(PP_PAGE_OSD | 0x9A)
#define REG_FL_STATUS_ID3						(PP_PAGE_OSD | 0x9B)

#define REG_SPI_RX_DATA                         (PP_PAGE_OSD | 0x9C)

#endif  // __SI_REGS_OSD_H__

