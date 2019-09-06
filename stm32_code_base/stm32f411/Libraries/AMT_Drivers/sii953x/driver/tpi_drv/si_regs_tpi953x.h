//***************************************************************************
//!file     si_regs_tpi953x.h
//!brief    SiI9535 Device Transmitter Programming Interface (TPI) registers.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_9535REGS_TPI_H__
#define __SI_9535REGS_TPI_H__

//#include "si_cra_cfg.h"
#include "si_drv_internal.h"

//
#define REG_TPI__ACLK_DIV                 (TX_PAGE_TPI | 0x00)
#define BIT_TPI__ACLK_DIV_FACTOR                  (BIT0)

// Pixel Repetition Data ---

#define REG_TPI__PRD                      (TX_PAGE_TPI | 0x08)
#define MSK_TPI__PRD__ICLK                        (BIT0 | BIT1)
#define VAL_TPI__PRD__ICLK_1                      (0x00)
#define VAL_TPI__PRD__ICLK_2                      (0x01)
#define VAL_TPI__PRD__ICLK_4                      (0x03)
#define BIT_TPI__PRD__EMB_SYNC_MODE               (BIT2)
#define BIT_TPI__PRD__DDR_444                     (BIT3)
#define BIT_TPI__PRD__EDGE                        (BIT4)
#define BIT_TPI__PRD__BSEL                        (BIT5)
#define BIT_TPI__PRD__YC_SWAP_MODE                (BIT6)
#define BIT_TPI__PRD__DATA_R_ALIGN                (BIT7)


// TPI AVI Input and Output Format Data ---

#define REG_TPI__INP_FMT                  (TX_PAGE_TPI | 0x09)
#define MSK_TPI__INP_FMT__COLOR_SPACE             (BIT0 | BIT1)
#define VAL_TPI__INP_FMT__COLOR_SPACE__RGB        (0x00)
#define VAL_TPI__INP_FMT__COLOR_SPACE__YCBCR_444  (0x01)
#define VAL_TPI__INP_FMT__COLOR_SPACE__YCBCR_422  (0x02)
#define VAL_TPI__INP_FMT__COLOR_SPACE__BLACK      (0x03)
#define MSK_TPI__INP_FMT__RANGE                   (BIT2 | BIT3)
#define VAL_TPI__INP_FMT__RANGE__AUTO             (0x00)
#define VAL_TPI__INP_FMT__RANGE__EXPAND           (0x04)
#define VAL_TPI__INP_FMT__RANGE__NO_EXPAND        (0x08)
#define MSK_TPI__INP_FMT__TCLK                    (BIT5 | BIT4)
#define VAL_TPI__INP_FMT__TCLK_HALF               (0x00)
#define VAL_TPI__INP_FMT__TCLK_X1                 (0x10)
#define MSK_TPI__INP_FMT__DEPTH                   (BIT6 | BIT7)
#define VAL_TPI__INP_FMT__DEPTH__8                (0x00)
#define VAL_TPI__INP_FMT__DEPTH__10               (0x80)
#define VAL_TPI__INP_FMT__DEPTH__12               (0xC0)
#define VAL_TPI__INP_FMT__DEPTH__16               (0x40)

#define REG_TPI__OUTP_FMT                (TX_PAGE_TPI | 0x0A)
#define MSK_TPI__OUTP_FMT__COLOR_SPACE            (BIT0 | BIT1)
#define VAL_TPI__OUTP_FMT__COLOR_SPACE__RGB       (0x00)
#define VAL_TPI__OUTP_FMT__COLOR_SPACE__YCBCR_444 (0x01)
#define VAL_TPI__OUTP_FMT__COLOR_SPACE__YCBCR_422 (0x02)
#define VAL_TPI__OUTP_FMT__COLOR_SPACE__RGB_DVI   (0x03)
#define MSK_TPI__OUTP_FMT__RANGE                  (BIT2 | BIT3)
#define VAL_TPI__OUTP_FMT__RANGE__AUTO            (0x00)
#define VAL_TPI__OUTP_FMT__RANGE__NO_COMPRESS     (0x04)
#define VAL_TPI__OUTP_FMT__RANGE__COMPRESS        (0x08)
#define BIT_TPI__OUTP_FMT__601_709                (BIT4)
#define VAL_TPI__OUTP_FMT__601                    (0x00)
#define VAL_TPI__OUTP_FMT__709                    (0x10)
#define BIT_TPI__OUTP_FMT__DITHER                 (BIT5)
#define VAL_TPI__OUTP_FMT__DITHER__OFF            (0x00)
#define VAL_TPI__OUTP_FMT__DITHER__ON             (0x20)
#define MSK_TPI__OUTP_FMT__DEPTH                  (BIT6 | BIT7)
#define VAL_TPI__OUTP_FMT__DEPTH__8               (0x00)
#define VAL_TPI__OUTP_FMT__DEPTH__10              (0x80)
#define VAL_TPI__OUTP_FMT__DEPTH__12              (0xC0)
#define VAL_TPI__OUTP_FMT__DEPTH__16              (0x40)

#define REG_TPI__VID_IN_MODE              (TX_PAGE_TPI | 0x0B)
#define BIT_TPI__VID_IN_MODE                      (BIT0)
#define VAL_TPI__VID_IN_MODE_RX_MAIN              (0x00)
#define VAL_TPI__VID_IN_MODE_TPG                  (0x01)
#define BIT_TPI__VID_IN_MODE_OUT_CLK_DIV          (BIT1)
#define VAL_TPI__VID_IN_MODE_OUT_CLK_DIV_DISABLE  (0x00)
#define VAL_TPI__VID_IN_MODE_OUT_CLK_DIV_ENABLE   (0x02)
#define MSK_TPI__VID_IN_MODE_OUT_CLK_DIV_VAL      (BIT2 | BIT3 | BIT4 | BIT 5 | BIT6)
#define BIT_TPI__VID_IN_MODE__SWAP_BITS           (BIT7) // swap bits: LSB becomes MSB and wise versa


// TPI AVI InfoFrame Data ---

#define REG_TPI__AVI_BYTE_0               (TX_PAGE_TPI | 0x0C)
#define REG_TPI__AVI_BYTE_1               (TX_PAGE_TPI | 0x0D)
#define REG_TPI__AVI_BYTE_2               (TX_PAGE_TPI | 0x0E)
#define REG_TPI__AVI_BYTE_3               (TX_PAGE_TPI | 0x0F)
#define REG_TPI__AVI_BYTE_4               (TX_PAGE_TPI | 0x10)
#define REG_TPI__AVI_BYTE_5               (TX_PAGE_TPI | 0x11)

#define REG_TPI__END_TOP_BAR_LSB          (TX_PAGE_TPI | 0x12)
#define REG_TPI__END_TOP_BAR_MSB          (TX_PAGE_TPI | 0x13)

#define REG_TPI__START_BTM_BAR_LSB        (TX_PAGE_TPI | 0x14)
#define REG_TPI__START_BTM_BAR_MSB        (TX_PAGE_TPI | 0x15)

#define REG_TPI__END_LEFT_BAR_LSB         (TX_PAGE_TPI | 0x16)
#define REG_TPI__END_LEFT_BAR_MSB         (TX_PAGE_TPI | 0x17)

#define REG_TPI__END_RIGHT_BAR_LSB        (TX_PAGE_TPI | 0x18)
#define REG_TPI__END_RIGHT_BAR_MSB        (TX_PAGE_TPI | 0x19)

// TPI System Control Data ---

#define REG_TPI__SYSTEM_CONTROL           (TX_PAGE_TPI | 0x1A)
#define BIT_TPI__SYSTEM_CONTROL__HDMI             (BIT0)
#define BIT_TPI__SYSTEM_CONTROL__DDC_GRANT        (BIT1)
#define BIT_TPI__SYSTEM_CONTROL__DDC_REQUEST      (BIT2)
#define BIT_TPI__SYSTEM_CONTROL__AVMUTE           (BIT3)
#define BIT_TPI__SYSTEM_CONTROL__PD               (BIT4)
#define BIT_TPI__SYSTEM_CONTROL__REAUTH_EN        (BIT6)

// TPI Identification Registers ---

#define REG_TPI__DEVICE_ID                (TX_PAGE_TPI | 0x1B)
#define REG_TPI__DEVICE_REV_ID            (TX_PAGE_TPI | 0x1C)
#define REG_TPI__TPI_REV_ID               (TX_PAGE_TPI | 0x1D)

// TPI Device Power State Control Data ---

#define REG_TPI__DEVICE_POWER_STATE_CTRL  (TX_PAGE_TPI | 0x1E)
#define MSK_TPI__POWER_STATE_MASK                 (BIT0 | BIT1)
#define VAL_TPI__POWER_STATE_D0                   (0x00)
#define VAL_TPI__POWER_STATE_D1                   (0x01)
#define VAL_TPI__POWER_STATE_D2                   (0x02)
#define VAL_TPI__POWER_STATE_D3                   (0x03)

// Configuration of I2S Interface ---

#define REG_TPI__I2S_MAP                  (TX_PAGE_TPI | 0x1F)
#define MSK_TPI__I2S_MAP__FIFO                    (BIT0 | BIT1)
#define VAL_TPI__I2S_MAP__FIFO_0                  0x00
#define VAL_TPI__I2S_MAP__FIFO_1                  0x01
#define VAL_TPI__I2S_MAP__FIFO_2                  0x02
#define VAL_TPI__I2S_MAP__FIFO_3                  0x03
#define BIT_TPI__I2S_MAP__SWAP_L_R                (BIT2)
#define BIT_TPI__I2S_MAP__AUTO_DOWN_SAMPLE        (BIT3)
#define MSK_TPI__I2S_MAP__SD_MAP                  (BIT4 | BIT5)
#define VAL_TPI__I2S_MAP__SD_0                    0x00
#define VAL_TPI__I2S_MAP__SD_1                    0x10
#define VAL_TPI__I2S_MAP__SD_2                    0x20
#define VAL_TPI__I2S_MAP__SD_3                    0x30
#define BIT_TPI__I2S_MAP__SD_ENABLE               (BIT7)


#define REG_TPI__I2S_IN_CFG               (TX_PAGE_TPI | 0x20)
#define BIT_TPI__I2S_IN_CFG__1ST_BIT              (BIT0)
#define BIT_TPI__I2S_IN_CFG__DATA_DIR             (BIT1)
#define BIT_TPI__I2S_IN_CFG__JUSTIFY              (BIT2)
#define BIT_TPI__I2S_IN_CFG__WS                   (BIT3)
#define MSK_TPI__FREQ_SVAL                        (BIT4 | BIT5 | BIT6)
#define SFT_TPI__FREQ_SVAL                        4
#define BIT_TPI__I2S_IN_CFG__SCK_EDGE             (BIT7)

// Available only when TPI 0x26[7:6]=10 to select I2S input ---
#define REG_TPI__I2S_CHST_1               (TX_PAGE_TPI | 0x21)
#define REG_TPI__I2S_CHST_2               (TX_PAGE_TPI | 0x22)
#define REG_TPI__I2S_CHST_3               (TX_PAGE_TPI | 0x23)
#define REG_TPI__I2S_CHST_4               (TX_PAGE_TPI | 0x24)
#define REG_TPI__I2S_CHST_5               (TX_PAGE_TPI | 0x25)


// Available only when 0x26[7:6]=01 ---
#define REG_TPI__SPDIF_HEADER             (TX_PAGE_TPI | 0x24)
#define REG_TPI__SPDIF_HANDLING           (TX_PAGE_TPI | 0x25)
#define MSK_TPI__SPDIF_HANDLING                   (BIT1 | BIT0)
#define VAL_TPI__SPDIF_HANDLING_PASS_BASIC_AUDIO  0x00
#define VAL_TPI__SPDIF_HANDLING_PASS_ALL_AUDIO    0x01
#define VAL_TPI__SPDIF_HANDLING_DOWNSAMPLE        0x02
#define VAL_TPI__SPDIF_HANDLING_NO_CHECK_AUDIO    0x03
#define BIT_TPI__SPDIF_HANDLING_GCPP_SRC_TPI      (BIT7)


// Audio Configuration Registers ---
#define REG_TPI__AUDIO_CFG                (TX_PAGE_TPI | 0x26)
#define BIT_TPI__AUDIO_CFG__MUTE                  (BIT4)
#define BIT_TPI__AUDIO_CFG__LAYOUT1               (BIT5)
#define MSK_TPI__AUDIO_CFG__INTERFACE             (BIT6 | BIT7)
#define VAL_TPI__AUDIO_CFG__DISABLE               0x00
#define VAL_TPI__AUDIO_CFG__SPDIF                 0x40
#define VAL_TPI__AUDIO_CFG__I2S                   0x80
#define VAL_TPI__AUDIO_CFG__DSD                   0xC0  // reserved in 9535

#define REG_TPI__AUDIO_SAMPLE             (TX_PAGE_TPI | 0x27)
#define BIT_TPI__AUDIO_SAMPLE__HBRA               (BIT2)
#define MSK_TPI__AUDIO_SAMPLE__FREQUENCY          (BIT3 | BIT4 | BIT5)
#define SFT_TPI__AUDIO_SAMPLE__FR                 3
#define VAL_TPI__AUDIO_SAMPLE__FR_STREAM          0x00 // sample frequency from audio stream header
#define VAL_TPI__AUDIO_SAMPLE__32_KHZ             0x08
#define VAL_TPI__AUDIO_SAMPLE__44_KHZ             0x10
#define VAL_TPI__AUDIO_SAMPLE__48_KHZ             0x18
#define VAL_TPI__AUDIO_SAMPLE__88_KHZ             0x20
#define VAL_TPI__AUDIO_SAMPLE__96_KHZ             0x28
#define VAL_TPI__AUDIO_SAMPLE__176_KHZ            0x30
#define VAL_TPI__AUDIO_SAMPLE__192_KHZ            0x38
#define MSK_TPI__AUDIO_SAMPLE__WIDTH              (BIT6 | BIT7)
#define SFT_TPI__AUDIO_SAMPLE__WIDTH              6
#define VAL_TPI__AUDIO_SAMPLE__WD_STREAM          0x00 // sample width from audio stream header
#define VAL_TPI__AUDIO_SAMPLE__16_BIT             0x40
#define VAL_TPI__AUDIO_SAMPLE__20_BIT             0x80
#define VAL_TPI__AUDIO_SAMPLE__24_BIT             0xC0

#define REG_TPI__AUDIO_PKT_SRC_CFG        (TX_PAGE_TPI | 0x28)
#define BIT_TPI__AUDIO_SMP_PKT_SRC_TPI            (BIT3)
#define BIT_TPI__AUDIO_CTS_PKT_SRC_TPI            (BIT6)
#define BIT_TPI__AUDIO_GCP_PKT_SRC_TPI            (BIT7)

// HDCP Registers ---

#define REG_TPI__HDCP_QUERY               (TX_PAGE_TPI | 0x29)
#define BIT_TPI__HDCP_QUERY__EXTENDED_PROTECT     (BIT7)
#define BIT_TPI__HDCP_QUERY__LOCAL_PROTECT        (BIT6)
#define MSK_TPI__HDCP_QUERY__STATUS               (BIT5 | BIT4)
#define VAL_TPI__HDCP_QUERY__STATUS_NORMAL        (0x00)
#define VAL_TPI__HDCP_QUERY__STATUS_LOST          (0x10)
#define VAL_TPI__HDCP_QUERY__STATUS_FAILED        (0x20)
#define VAL_TPI__HDCP_QUERY__STATUS_SUSPENDED     (0x30)  // not used in 9535
#define BIT_TPI__HDCP_QUERY__DS_REPEATER          (BIT3)
#define MSK_TPI__HDCP_QUERY__CONNECTR             (BIT2 | BIT0)
#define VAL_TPI__HDCP_QUERY__CONNECTR_DVI         (0x00)
#define VAL_TPI__HDCP_QUERY__CONNECTR_HDMI        (0x04)

#define BIT_TPI__HDCP_QUERY__SINK_CAPABLE         (BIT1)

#define REG_TPI__HDCP_CTRL                (TX_PAGE_TPI | 0x2A)
#define BIT_TPI__HDCP_CTRL__PROTECTION            (BIT0)
#define BIT_TPI__HDCP_CTRL__SHORT_RI              (BIT1)
#define BIT_TPI__HDCP_CTRL__DOUBLE_RI_CHECK       (BIT2)
#define BIT_TPI__HDCP_CTRL__INTERMEDIATE_RI_CHECK (BIT3)
#define BIT_TPI__HDCP_CTRL__KSV_FORWARDING        (BIT4) // forwarding of KSV list enabled
#define BIT_TPI__HDCP_CTRL__ENCRYPT_DISABLE       (BIT5)
#define BIT_TPI__HDCP_CTRL__PROT_REQUEST_CANCEL   (BIT6)
#define BIT_TPI__HDCP_CTRL__DDC_MASTER_ABORT      (BIT7)


#define REG_TPI__BKSV_1                   (TX_PAGE_TPI | 0x2B)
#define REG_TPI__BKSV_2                   (TX_PAGE_TPI | 0x2C)
#define REG_TPI__BKSV_3                   (TX_PAGE_TPI | 0x2D)
#define REG_TPI__BKSV_4                   (TX_PAGE_TPI | 0x2E)
#define REG_TPI__BKSV_5                   (TX_PAGE_TPI | 0x2F)

#define REG_TPI__HDCP_REVISION            (TX_PAGE_TPI | 0x30)
#define REG_TPI__V_PRIME_SELECTOR         (TX_PAGE_TPI | 0x31)

// V' Value Read Back Registers ---

#define REG_TPI__V_PRIME_7_0              (TX_PAGE_TPI | 0x32)
#define REG_TPI__V_PRIME_15_9             (TX_PAGE_TPI | 0x33)
#define REG_TPI__V_PRIME_23_16            (TX_PAGE_TPI | 0x34)
#define REG_TPI__V_PRIME_31_24            (TX_PAGE_TPI | 0x35)

// Aksv Value Read Back Registers ---

#define REG_TPI__AKSV_1                   (TX_PAGE_TPI | 0x36)
#define REG_TPI__AKSV_2                   (TX_PAGE_TPI | 0x37)
#define REG_TPI__AKSV_3                   (TX_PAGE_TPI | 0x38)
#define REG_TPI__AKSV_4                   (TX_PAGE_TPI | 0x39)
#define REG_TPI__AKSV_5                   (TX_PAGE_TPI | 0x3A)

// Interrupt Service Registers ---
#define REG_TPI__INT_ENABLE1              (TX_PAGE_TPI | 0x3C)
#define REG_TPI__INT_STATUS1              (TX_PAGE_TPI | 0x3D)
#define BIT_TPI__INT__HPD                         (BIT0) // HPD Interrupt
#define BIT_TPI__INT__RX_SENSE                    (BIT1) // RxSense Interrupt
#define BIT_TPI__INT__HPD_STATE                   (BIT2) // HPD State
#define BIT_TPI__INT__RX_SENSE_STATE              (BIT3) // RxSense State
#define BIT_TPI__INT__AUDIO_ERROR                 (BIT4) // Audio Error
#define BIT_TPI__INT__SECURITY_CHNG               (BIT5) // Security Change
#define BIT_TPI__INT__V_READY                     (BIT6) // V Ready
#define BIT_TPI__INT__HDCP_CHNG                   (BIT7) // HDCP Status Change

#define REG_TPI__INT_STATUS2              (TX_PAGE_TPI | 0x3E)
#define REG_TPI__INT_ENABLE2              (TX_PAGE_TPI | 0x3F)
#define BIT_TPI__INT__P_STABLE                    (BIT0) // P_STABLE (PLL locked, HDMI Link stable)
#define BIT_TPI__INT__KSV_READY                   (BIT1) // first KSV is available

#define REG_TPI__RAR                      (TX_PAGE_TPI | 0x40)
#define BIT_TPI__RAR__SW_RST                      (BIT0)
#define BIT_TPI__RAR__AUDIO_MUTE                  (BIT1)
#define BIT_TPI__RAR__DC_PKT_EN                   (BIT2) // GCP DeepColor enabled
#define BIT_TPI__RAR__CEC_SW_RST                  (BIT3)
#define BIT_TPI__RAR__IFR_RST_DISABLE             (BIT4) // if set, toggling BIT0 won't reset InfoFrame sending enable flags
#define BIT_TPI__RAR__PCLK_TCLK_FIFO_SW_RST       (BIT5)
#define BIT_TPI__RAR__P_STABLE                    (BIT6)
#define BIT_TPI__RAR__PA_SSC_FIFO_ENABLE          (BIT7)

#define REG_TPI__HDCP_FIFO_LEVEL          (TX_PAGE_TPI | 0x41) // KSV FIFO fill level
#define MSK_TPI__HDCP_FIFO_LEVEL                  (BIT0 | BIT1 | BIT2 | BIT3 | BIT4)
#define VAL_TPI__HDCP_FIFO_BUFFER_LENGTH          (16)
#define BIT_TPI__HDCP_FIFO_LEVEL__LAST            (BIT7) // last portion

// KSV FIFO (single and burst reads are possible)
#define REG_TPI__HDCP_FIFO_DATA           (TX_PAGE_TPI | 0x42) // KSV FIFO data port
#define REG_TPI__HDCP_BCAPS               (TX_PAGE_TPI | 0x44) // BCAPS of downstream device
#define REG_TPI__HDCP_BSTATUS1            (TX_PAGE_TPI | 0x45) // TPI BSTATUS1 of downstream device
#define REG_TPI__HDCP_BSTATUS2_SHADOW     (TX_PAGE_TPI | 0x46) // TPI BSTATUS2 of downstream device

// Blank color registers (MSB aligned)
#define REG_TPI__INP_BLANK0               (TX_PAGE_TPI | 0x4A) // B/Cb LSB
#define REG_TPI__INP_BLANK1               (TX_PAGE_TPI | 0x4B) // B/Cb MSB
#define REG_TPI__INP_BLANK2               (TX_PAGE_TPI | 0x4C) // G/Y LSB
#define REG_TPI__INP_BLANK3               (TX_PAGE_TPI | 0x4D) // G/Y MSB
#define REG_TPI__INP_BLANK4               (TX_PAGE_TPI | 0x4E) // R/Cr/YC LSB
#define REG_TPI__INP_BLANK5               (TX_PAGE_TPI | 0x4F) // R/Cr/YC MSB


// Sync Register Configuration and Sync Monitoring Registers ---

#define REG_TPI__SYNC_GEN_CTRL            (TX_PAGE_TPI | 0x60)
#define BIT_TPI__SYNC_GEN_CTRL__FIELD_2_VBLANK_OFFSET (BIT0)
#define BIT_TPI__SYNC_GEN_CTRL__FIELD_2_VBLANK_ADJ    (BIT1)
#define BIT_TPI__SYNC_GEN_CTRL__DE_NO_VSYNC_ADJ       (BIT2)
#define BIT_TPI__SYNC_GEN_CTRL__INV_FIELD_POL         (BIT4)
#define BIT_TPI__SYNC_GEN_CTRL__EN_DE_MUX             (BIT5)
#define BIT_TPI__SYNC_GEN_CTRL__SYNC_EXTRACT_LAYOUT   (BIT7)

#define REG_TPI__SYNC_POLAR_DETECT        (TX_PAGE_TPI | 0x61)
#define BIT_TPI__SYNC_POLAR_DETECT_H_LOW          (BIT0)
#define BIT_TPI__SYNC_POLAR_DETECT_V_LOW          (BIT1)
#define BIT_TPI__SYNC_POLAR_DETECT_INTERLACED     (BIT2)


// Explicit Sync DE Generator Registers (TPI 0x60[7]=0) ---

#define REG_TPI__DE_DLY                   (TX_PAGE_TPI | 0x62)

#define REG_TPI__DE_CTRL                  (TX_PAGE_TPI | 0x63)
#define MSK_TPI__DE_CTRL__DE_DLY_H                (BIT0 | BIT1)
#define BIT_TPI__DE_CTRL__HSYNC_POL               (BIT4)
#define BIT_TPI__DE_CTRL__VSYNC_POL               (BIT5)
#define BIT_TPI__DE_CTRL__EN_VID_DE_GEN           (BIT6)

#define REG_TPI__DE_TOP                   (TX_PAGE_TPI | 0x64)

#define REG_TPI__DE_CNT_LSB               (TX_PAGE_TPI | 0x66)
#define REG_TPI__DE_CNT_MSB               (TX_PAGE_TPI | 0x67)

#define REG_TPI__DE_LIN_LSB               (TX_PAGE_TPI | 0x68)
#define REG_TPI__DE_LIN_MSB               (TX_PAGE_TPI | 0x69)

#define REG_TPI__DE_H_RES_LSB             (TX_PAGE_TPI | 0x6A)
#define REG_TPI__DE_H_RES_MSB             (TX_PAGE_TPI | 0x6B)

#define REG_TPI__DE_V_RES_LSB             (TX_PAGE_TPI | 0x6C)
#define REG_TPI__DE_V_RES_MSB             (TX_PAGE_TPI | 0x6D)

// Embedded Sync Register Set (TPI 0x60[7]=1) ---

#define REG_TPI__HBIT_TO_HSYNC_LSB        (TX_PAGE_TPI | 0x62)
#define REG_TPI__HBIT_TO_HSYNC_MSB        (TX_PAGE_TPI | 0x63)
#define MSK_TPI__HBIT_TO_HSYNC_MSB                 (BIT0 | BIT1)
#define BIT_TPI__HBIT_TO_HSYNC_MSB__HSYNC_POL      (BIT4)
#define BIT_TPI__HBIT_TO_HSYNC_MSB__VSYNC_POL      (BIT5)
#define BIT_TPI__HBIT_TO_HSYNC_MSB__EN_EMBED_SYNC  (BIT6)

#define REG_TPI__FIELD_2_OFFSET_LSB       (TX_PAGE_TPI | 0x64)
#define REG_TPI__FIELD_2_OFFSET_MSB       (TX_PAGE_TPI | 0x65)
#define REG_TPI__HWIDTH_LSB               (TX_PAGE_TPI | 0x66)
#define REG_TPI__HWIDTH_MSB               (TX_PAGE_TPI | 0x67)
#define REG_TPI__VBIT_TO_VSYNC            (TX_PAGE_TPI | 0x68)
#define REG_TPI__VBIT_TO_VSYNC            (TX_PAGE_TPI | 0x68)
#define REG_TPI__VWIDTH                   (TX_PAGE_TPI | 0x69)

// Repeater HDCP state machine state (DBG5)
#define REG_TPI__RPT_SM                   (TX_PAGE_TPI | 0x7D)

// Pipe (Main, Sub/Roving) selection as a source for audio insertion ---
#define REG_TPI__AUDIO_SRC_CTRL           (TX_PAGE_TPI | 0x80)
#define BIT_TPI__AUDIO_RX_RP_SELECT               (BIT0)
#define BIT_TPI__AUDIO_ENABLE                     (BIT1)
#define BIT_TPI__AUDIO_RX_LAYOUT_USE              (BIT2)

// "Backdoor" access to Legacy space in TPI mode ---
#define REG_TPI__REG_SEL                  (TX_PAGE_TPI | 0xBC)
#define MSK_TPI__REG_SEL__PAGE                    (BIT1 | BIT0)
#define VAL_TPI__REG_SEL__PAGE_L0                 0x01
#define VAL_TPI__REG_SEL__PAGE_L1                 0x02
#define VAL_TPI__REG_SEL__PAGE_L2                 0x03
#define BIT_TPI__REG_SEL__LEGACY_ENABLE           (BIT7)

#define REG_TPI__REG_OFFSET               (TX_PAGE_TPI | 0xBD)
#define REG_TPI__REG_DATA                 (TX_PAGE_TPI | 0xBE)
#define REG_TPI__HW_OPT	                 (TX_PAGE_TPI | 0xB9)
#define VAL_TPI__R0_CALC_OSC			(0xb0)
#define VAL_TPI__R0_CALC_VSYNC			(0x3F)
// InfoFrame generic interface ---
#define REG_TPI__IF_SELECT                (TX_PAGE_TPI | 0xBF)
#define MSK_TPI__IF_SELECT__TITLE                 (BIT0 | BIT1 |BIT2)
#define VAL_TPI__IF_SELECT__AVI                   0x00 // do not use for packet starting
#define VAL_TPI__IF_SELECT__SPD                   0x01
#define VAL_TPI__IF_SELECT__AUDIO                 0x02
#define VAL_TPI__IF_SELECT__MPEG                  0x03
#define VAL_TPI__IF_SELECT__GEN1                  0x04
#define VAL_TPI__IF_SELECT__GEN2                  0x05
#define VAL_TPI__IF_SELECT__VSIF                  0x06
#define VAL_TPI__IF_SELECT__GBD                   0x07
#define BIT_TPI__IF_SELECT__OVRD_PACKET           (BIT4)
#define BIT_TPI__IF_SELECT__NO_MODIFY             (BIT5)
#define BIT_TPI__IF_SELECT__REPEAT                (BIT6)
#define BIT_TPI__IF_SELECT__ENABLE                (BIT7)


#define REG_TPI__IF_BUFFER_START          (TX_PAGE_TPI | 0xC0)
#define LEN_TPI__IF_BUFFER_LENGTH                 0x31

#define REG_TPI__IF_TYPE                  (TX_PAGE_TPI | 0xC0)
#define REG_TPI__IF_VERSION               (TX_PAGE_TPI | 0xC1)
#define REG_TPI__IF_LENGTH                (TX_PAGE_TPI | 0xC2)
#define REG_TPI__IF_DBYTE0                (TX_PAGE_TPI | 0xC3) // check sum / first data byte
#define REG_TPI__IF_DBYTE1                (TX_PAGE_TPI | 0xC4)
#define REG_TPI__IF_DBYTE2                (TX_PAGE_TPI | 0xC5)
#define REG_TPI__IF_DBYTE3                (TX_PAGE_TPI | 0xC6)
#define REG_TPI__IF_DBYTE4                (TX_PAGE_TPI | 0xC7)
#define REG_TPI__IF_DBYTE5                (TX_PAGE_TPI | 0xC8)
#define REG_TPI__IF_DBYTE6                (TX_PAGE_TPI | 0xC9)
#define REG_TPI__IF_DBYTE7                (TX_PAGE_TPI | 0xCA)
#define REG_TPI__IF_DBYTE8                (TX_PAGE_TPI | 0xCB)
#define REG_TPI__IF_DBYTE9                (TX_PAGE_TPI | 0xCC)
#define REG_TPI__IF_DBYTE10               (TX_PAGE_TPI | 0xCD) // last location for Audio Info Frame
#define REG_TPI__IF_DBYTE11               (TX_PAGE_TPI | 0xCE)
#define REG_TPI__IF_DBYTE12               (TX_PAGE_TPI | 0xCF)
#define REG_TPI__IF_DBYTE13               (TX_PAGE_TPI | 0xD0)
#define REG_TPI__IF_DBYTE14               (TX_PAGE_TPI | 0xD1) // must be 0 for AVI
#define REG_TPI__IF_DBYTE15               (TX_PAGE_TPI | 0xD2) // last location for AVI Info frame (must be 0)
#define REG_TPI__IF_DBYTE16               (TX_PAGE_TPI | 0xD3)
#define REG_TPI__IF_DBYTE17               (TX_PAGE_TPI | 0xD4)
#define REG_TPI__IF_DBYTE18               (TX_PAGE_TPI | 0xD5)
#define REG_TPI__IF_DBYTE19               (TX_PAGE_TPI | 0xD6)
#define REG_TPI__IF_DBYTE20               (TX_PAGE_TPI | 0xD7)
#define REG_TPI__IF_DBYTE21               (TX_PAGE_TPI | 0xD8)
#define REG_TPI__IF_DBYTE22               (TX_PAGE_TPI | 0xD9)
#define REG_TPI__IF_DBYTE23               (TX_PAGE_TPI | 0xDA)
#define REG_TPI__IF_DBYTE24               (TX_PAGE_TPI | 0xDB)
#define REG_TPI__IF_DBYTE25               (TX_PAGE_TPI | 0xDC)
#define REG_TPI__IF_DBYTE26               (TX_PAGE_TPI | 0xDD)
#define REG_TPI__IF_DBYTE27               (TX_PAGE_TPI | 0xDE) // Last Info Frame byte


// DDC I2C Master Control registers
// DDC I2C Manual Register
#define REG_TPI__DDC_MANUAL               (TX_PAGE_TPI | 0xF0)
#define MSK_TPI__DDC_MANUAL__IO_DSCL              0x01    // r
// DSCL input status.  This bit reflects the live status of the DSCL pin.
#define MSK_TPI__DDC_MANUAL__IO_DSDA              0x02    // r
// DSDA input status.  This bit reflects the live status of the DSDA pin.
#define MSK_TPI__DDC_MANUAL__DSCL                 0x10    // r/w
// DSCL manual output value.  Only enabled onto the DSCL output if reg_man_ddc = 1.
#define MSK_TPI__DDC_MANUAL__DSDA                 0x20    // r/w
// DSDA manual output value.  Only enabled onto the DSDA output if reg_man_ddc = 1.
#define MSK_TPI__DDC_MANUAL__MAN_DDC              0x80    // r/w
// manual override of scl and sda output

// DDC I2C Target Slave Address Register
#define REG_TPI__DDC_ADDR                 (TX_PAGE_TPI | 0xF1)
#define MSK_TPI__DDC_ADDR                         0xFE    // r/w
// DDC target Slave address

// DDC I2C Target Segment Address Register
#define REG_TPI__DDC_SEGM                 (TX_PAGE_TPI | 0xF2)  // r/w

// DDC I2C Target Offset Address Register
#define REG_TPI__DDC_OFFSET               (TX_PAGE_TPI | 0xF3)  // r/w

// DDC I2C Data In count #1 Register
#define REG_TPI__DDC_DIN_CNT1             (TX_PAGE_TPI | 0xF5)  // r/w
// Bits [7:0] of the number of bytes to transfer in

// DDC I2C Data In count #2 Register
#define REG_TPI__DDC_DIN_CNT2             (TX_PAGE_TPI | 0xF6)
#define MSK_TPI__DDC_DIN_CNT2                     0x03    // r/w
// Bits [9:8] of the number of bytes to transfer in

// DDC I2C Status Register
#define REG_TPI__DDC_STATUS               (TX_PAGE_TPI | 0xF7)
#define MSK_TPI__DDC_STATUS__DDC_FIFO_WRITE_IN_USE 0x01    // r
// DDC fifo write in use
#define MSK_TPI__DDC_STATUS__DDC_FIFO_READ_IN_SUE  0x02    // r
// DDC fifo read in use
#define MSK_TPI__DDC_STATUS__DDC_FIFO_EMPTY        0x04    // r
// DDC FIFO is empty
#define MSK_TPI__DDC_STATUS__DDC_FIFO_FULL         0x08    // r
// DDC FIFO full
#define MSK_TPI__DDC_STATUS__DDC_I2C_IN_PROG       0x10    // r
// DDC in programming mode
#define MSK_TPI__DDC_STATUS__DDC_I2C_NO_ACK        0x20    // r/w
// When read get the status from hardware: to clear write 0
#define MSK_TPI__DDC_STATUS__DDC_I2C_BUS_LOW       0x40    // r/w
// When read get the status from hardware: to clear write 0

// DDC I2C Command Register
#define REG_TPI__DDC_CMD                  (TX_PAGE_TPI | 0xF8)
#define MSK_TPI__DDC_CMD                          0x0F    // r/w
// DDC Command[3:0]:
// 1111 - Abort transaction
// 1001 - Clear FIFO
// 1010 - Clock SCL
// 0000 - Current address read with no ACK on last byte
// 0010 - Sequential read with no ACK on last byte
// 0100 - Enhanced DDC read with no ACK on last byte
// 0110 - Sequential write ignoring ACK on last byte
// 0111 - Sequential write requiring ACK on last byte
// write to this register generates 1 clock strobe; which will force decoding of the ddc command
#define CMD_TPI__DDC_ABORT_TRANSACTION                               0x0F
#define CMD_TPI__DDC_CLEAR_FIFO                                      0x09
#define CMD_TPI__DDC_CLOCK_SCL                                       0x0A
#define CMD_TPI__DDC_CURRENT_ADDRESS_READ_WITH_NO_ACK_ON_LAST_BYTE   0x00
#define CMD_TPI__DDC_SEQUENTIAL_READ_WITH_NO_ACK_ON_LAST_BYTE        0x02
#define CMD_TPI__DDC_ENHANCED_DDC_READ_WITH_NO_ACK_ON_LAST_BYTE      0x04
#define CMD_TPI__DDC_SEQUENTIAL_WRITE_IGNORING_ACK_ON_LAST_BYTE      0x06
#define CMD_TPI__DDC_SEQUENTIAL_WRITE_REQUIRING_ACK_ON_LAST_BYTE     0x07
// Enable HW TPI DDC master
#define MSK_TPI__DDC_CMD__ENABLE                  (BIT7)

// DDC I2C FIFO Data In/Out Register
#define REG_TPI__DDC_DATA                 (TX_PAGE_TPI | 0xF4)  // r/w
// DDC FIFO register.
// During write to FIFO write data into this register
// During the read from FIFO this register will have the FIFO data

// DDC I2C Data Out Counter Register
#define REG_TPI__DDC_DOUT_CNT             (TX_PAGE_TPI | 0xF9)
#define MSK_TPI__DDC_DOUT_CNT                       0x01F   // r

#define LEN_TPI__DDC_FIFO_SIZE                      16

#define REG_TPI__DDC_DELAY_CNT            (TX_PAGE_TPI | 0xFA)
#define REG_TPI__DDC_MIN_IDLE             (TX_PAGE_TPI | 0xFB)
#define REG_TPI__DDC_STALL_TIME           (TX_PAGE_TPI | 0xFC)
#define REG_TPI__DDC_SETUP_DELAY          (TX_PAGE_TPI | 0xFD)



// ***************************************************************************************************
// Registers located in legacy pages extending TPI functions ---
// ***************************************************************************************************

// TPI Enable Register (in Legacy space) ---
#define REG_TX_TPI__ENABLE                (TX_PAGE_L0 | 0xC7)
#define BIT_TX_TPI__ENABLE                        (0x00)

// HDCP Ri 128 Compare Value Register
#define REG_TX__Ri_128_COMP                (TX_PAGE_L0 | 0x24)

// Output video blanking registers
#define REG_TX__VID_BLANK1                (TX_PAGE_L0 | 0x4B)
#define REG_TX__VID_BLANK2                (TX_PAGE_L0 | 0x4C)
#define REG_TX__VID_BLANK3                (TX_PAGE_L0 | 0x4D)

// Audio In I2S Control Register
#define REG_TX__I2S_IN_CTRL               (TX_PAGE_L1 | 0x1D)

// WS to SD: 1st-bit shift
// 0 - 1st-bit shift; Philips spec.
// 1 - no shift
#define BIT_TX__I2S_IN_CTRL__1ST_BIT              (BIT0)

// SD: MSb/LSb first
// 0 - MSb first
// 1 - LSb first
#define BIT_TX__I2S_IN_CTRL__DATA_DIR             (BIT1)

// SD: Left-; Right-justified
// 0 - data is Left justified (default)
// 1 - data is right justified
#define BIT_TX__I2S_IN_CTRL__JUSTIFY              (BIT2)

// WS: Left/Right polarity
// 0 - Left polarity when Word Select is low (default)
// 1 - Left polarity when Word Select is high
#define BIT_TX__I2S_IN_CTRL__WS                   (BIT3)

// V bit value
// 0 = PCM
// 1 = Compressed
#define BIT_TX__I2S_IN_CTRL__VBIT                 (BIT4)

// SCK: Sample edge rising/falling
// 0 - Sample edge is falling: SD3-SD0 and WS source should change state on the rising edge of the SCK
// 1 - Sample edge is rising (default): SD3-SD0 and WS source should change state on the falling edge of SCK
#define BIT_TX__I2S_IN_CTRL__SCK_EDGE             (BIT6)

// High Bit Rate Audio On
// 0 = Input stream is not high bit rate
// 1 = Input stream is high bit rate. All of the I2S control bits will apply to the control of the High Bit Rate Audio.
#define BIT_TX__I2S_IN_CTRL__HBRA_ON              (BIT7)

#define REG_TX__TMDS_CTRL1               (TX_PAGE_L0 | 0x80)
#define REG_TX__TMDS_CTRL2               (TX_PAGE_L0 | 0x81)
#define BIT_TX__TMDS_CTRL2__SRC_TERM_EN           (BIT5)
#define REG_TX__TMDS_CTRL3               (TX_PAGE_L0 | 0x82)

// TMDS termination
//#define REG_TX__TMDS_TERM_SELECT          (TX_PAGE_L0 | 0x82)
#define VAL_TX__TMDS_TERM__NONE                  0x00
#define VAL_TX__TMDS_TERM__DIFF_300_OHM          0x01
#define VAL_TX__TMDS_TERM__DIFF_150_OHM          0x02

#define REG_TX__AUDIO_CLK_DIV             (TX_PAGE_L0 | 0x83)
#define VAL_TX__AUDIO_CLK_RATIO_1                0x10
#define VAL_TX__AUDIO_CLK_RATIO_2                0x18
#define VAL_TX__AUDIO_CLK_RATIO_3                0x14
#define VAL_TX__AUDIO_CLK_RATIO_5                0x16
#define VAL_TX__AUDIO_CLK_RATIO_4                0x1A
#define VAL_TX__AUDIO_CLK_RATIO_6                0x1C
#define VAL_TX__AUDIO_CLK_RATIO_10               0x1E

// TX PLL bandwidth
//#define REG_TX__TMDS_PLL_BW               (TX_PAGE_L0 | 0x84)

// TMDS Swing control register
#define REG_TX__TMDS_SWING_CTRL           (TX_PAGE_L0 | 0x85)

#define REG_TX__TMDS_ZONE_CTRL1           (TX_PAGE_L0 | 0x8C)
// TMDS control register
#define REG_TX__TMDS_ZONE_CTRL2           (TX_PAGE_L0 | 0x8D)

// SPDIF MCLK generation control
#define REG_TX__OTP_CLK                   (TX_PAGE_L0 | 0xFB)
#define BIT_TX__OTP_CLK_CTS_GEN_NO_MCLK         (BIT4)

#define REG_TX__AUDIO_IN_CTRL               (TX_PAGE_L1 | 0x14)

#define BIT_TX__AUDIO_IN_EN                       (BIT0)


#endif  // __SI_9535REGS_TPI_H__

