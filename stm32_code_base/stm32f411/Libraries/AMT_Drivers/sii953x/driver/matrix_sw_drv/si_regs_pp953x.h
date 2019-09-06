//***************************************************************************
//!file     si_regs_pp953x.h
//!brief    SiI9535 Device Register Manifest Constants.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_9535REGS_PP_H__
#define __SI_9535REGS_PP_H__

#include "si_cra_cfg.h"

//------------------------------------------------------------------------------
// Registers in Page 0  (0xB0/0xB2)
//------------------------------------------------------------------------------

#define REG_DEV_IDL_RX              (PP_PAGE | 0x02)
#define REG_DEV_IDH_RX              (PP_PAGE | 0x03)
#define REG_DEV_REV                 (PP_PAGE | 0x04)

// Software Reset Register
#define REG_SYS_RESET_1             (PP_PAGE | 0x05)
#define BIT_SWRST_AUTO                  0x10    // Auto SW Reset
#define BIT_SWRST                       0x01
#define BIT_SWRST_PA                    0x02

// System Status Register
#define REG_MP_STATE                (PP_PAGE | 0x06)
#define BIT_PWR5V                       0x08
#define BIT_SCDT                        0x01
#define BIT_CKDT                        0x02
#define BIT_DECRYPT_STATUS              0x04
#define BIT_AUTHENED_STATUS             0x10
#define BIT_PWRON_STANDBY               0x20    // Set if NOT in standby
#define BIT_NOT_STANDBY                 0x20    // Set if NOT in standby
#define BIT_MHL                         0x80

// System Control register

#define REG_SYS_RESET_3             (PP_PAGE | 0x07)
#define BIT_NO_USE_SCDT                 0x04
#define BIT_HDCP_RST                    0x01

// System Control Register #1
#define REG_SYS_CTRL_1              (PP_PAGE | 0x08)
#define BIT_PWD_CLKS                    0x01        // Power down PWD domain digital clocks
#define BIT_RX_OTP_SHARED               BIT2        // Share one BKSV for MP & SP

// Port Switch Register
#define REG_RX_HDCP_DDC_EN          (PP_PAGE | 0x09)
#define BIT_DDC0_EN                     0x10
#define BIT_DDC1_EN                     0x20
#define BIT_DDC2_EN                     0x40
#define BIT_DDC3_EN                     0x80
#define BIT_DDC4_EN                     0x00
#define BIT_DDC5_EN                     0x00
#define VAL_DDC_PORT_ALL_EN             0xF0

// Port Switch Register 2
#define REG_RX_PORT_SEL             (PP_PAGE | 0x0A)
#define MSK_MP_PORT_SEL                 0x07
#define MSK_RP_PORT_SEL                 0x38
#define VAL_MP_SHIFT                    0
#define VAL_RP_SHIFT                    3
#define BIT_20MHZ_PWRDN_EN              0x20
#define MASK_PORT_SEL                   0x07

#define REG_SYS_RESET_2             (PP_PAGE | 0x0B)
#define BIT_MHL_SRST                    0x10
#define BIT_NVM_SRST                    0x20
#define BIT_CEC_M_SRST                  0x01
#define BIT_TX0_SRST                    0x04
#define BIT_EDID_SRST                   0x80

#define VAL_SYS_SRST                     (BIT_MHL_SRST | BIT_NVM_SRST | BIT_TX0_SRST | BIT_CEC_M_SRST | BIT_EDID_SRST)

// HDCP DDC termination disable Register
#define REG_TERM_DISABLE            (PP_PAGE | 0x0C)
#define BIT_HDCP_AUTO_DISABLE           0x01
#define BIT_HDCP_TERM_DISABLE           0x02
#define VAL_HDCP_DISABLED               0x03

// HDCP KSV_FIFO_CTRL
#define REG_HDCP_KSV_FIFO_CTRL	    (PP_PAGE | 0x0D)
#define BIT_HDCP_FIFO_VALID				0x01
#define VAL_HDCP_FIFO_VALID				0x41


// System Subpipe Status Register
#define REG_RP_STATE                (PP_PAGE | 0x0E)


//Hot plug Control Register 2
#define REG_HP_CTRL2                (PP_PAGE | 0x0F)
#define VAL_HP_PORT4_MASK               0x03
#define VAL_HP_PORT5_MASK               0x0C
#define VAL_HP_PORT4_HI                 0x01
#define VAL_HP_PORT_5_HI                0x04
#define VAL_PULL_DOWN_RES               0x02



//Hot plug Control Register
#define REG_HP_CTRL1                (PP_PAGE | 0x10)
#define VAL_HP_PORT0_MASK               0x03
#define VAL_HP_PORT0_HI                 0x01
#define VAL_HP_PORT1_HI                 0x04
#define VAL_HP_PORT2_HI                 0x10
#define VAL_HP_PORT3_HI                 0x40
#define VAL_HP_PORT_ALL_HI              0x55
#define VAL_HP_PORT_ALL_LO              0x00
#define VAL_HP_PORT_ALL_MHL             0xAA

#define REG_SLAVE_ADDR0             (PP_PAGE | 0x16)
#define REG_SLAVE_ADDR1             (PP_PAGE | 0x17)
#define REG_SLAVE_ADDR2             (PP_PAGE | 0x18)
#define REG_SLAVE_ADDR3             (PP_PAGE | 0x19)
#define REG_SLAVE_ADDR4             (PP_PAGE | 0x15)
#define REG_SLAVE_ADDR5             (PP_PAGE | 0x14)
#define REG_SLAVE_ADDR6             (PP_PAGE | 0x11)
#define REG_SLAVE_ADDR7             (PP_PAGE | 0x12)
#define REG_SLAVE_ADDR8             (PP_PAGE | 0x13)

#define REG_SHD_BKSV_1_RX           (PP_PAGE | 0x1A)
#define REG_SHD_BKSV_2_RX           (PP_PAGE | 0x1B)
#define REG_SHD_BKSV_3_RX           (PP_PAGE | 0x1C)
#define REG_SHD_BKSV_4_RX           (PP_PAGE | 0x1D)
#define REG_SHD_BKSV_5_RX           (PP_PAGE | 0x1E)

#define REG_HDCP_SHD_RI1       	    (PP_PAGE | 0x1F)
#define REG_HDCP_SHD_RI2       	    (PP_PAGE | 0x20)

#define REG_HDCP_SHD_AKSV1     	    (PP_PAGE | 0x21)
#define REG_HDCP_SHD_AKSV2     	    (PP_PAGE | 0x22)
#define REG_HDCP_SHD_AKSV3     	    (PP_PAGE | 0x23)
#define REG_HDCP_SHD_AKSV4     	    (PP_PAGE | 0x24)

#define REG_HDCP_SHD_AN1            (PP_PAGE | 0x26)

#define REG_HDCP_BCAPS_SET     	    (PP_PAGE | 0x2E)
#define BIT_HDMI_CAPABLE                0x80
#define BIT_I2C_FAST                    0x10
#define BIT_P0_FIFO_RDY  				0x01
#define BIT_P0_REPEATER  				0x02
#define BIT_P1_FIFO_RDY  				0x04
#define BIT_P1_REPEATER  				0x08
#define BIT_P2_FIFO_RDY  				0x10
#define BIT_P2_REPEATER  				0x20
#define BIT_P3_FIFO_RDY  				0x40
#define BIT_P3_REPEATER  				0x80
#define VAL_P0_FIFO_RDY  				0x01
#define VAL_P0_REPEATER  				0x02
#define VAL_P1_FIFO_RDY  				0x04
#define VAL_P1_REPEATER  				0x08
#define VAL_P2_FIFO_RDY  				0x10
#define VAL_P2_REPEATER  				0x20
#define VAL_P3_FIFO_RDY  				0x40
#define VAL_P3_REPEATER  				0x80
#define VAL_P1_3_FIFO_RDY  				0x55

// HDCP Bstatus shadow registers
#define REG_HDCP_SHD_BSTAT1         (PP_PAGE | 0x2F)
#define BIT_DEV_EXCEEDED                 0x80
#define BIT_DEVICE_CNT                   0x7F
#define VAL_DEV_EXCEEDED                 0x80

// HDCP Debug Register
#define REG_BSTATUS2                (PP_PAGE | 0x30)
#define REG_HDCP_SHD_BSTAT2	        (PP_PAGE | 0x30)
#define BIT_HDMI_MODE                   0x10
#define BIT_DEPTH_EXCEEDED				0x08
#define VAL_DEPTH_EXCEEDED				0x08

#define REG_HDCPCTRL                (PP_PAGE | 0x31)
#define BIT_CLEAR_RI                    0x80

//HDCP Status
#define REG_HDCP_STAT               (PP_PAGE | 0x32)
#define BIT_SHA_BUSY  				    0x80

//HDCP Repeater KSV/SHA Start Register
#define REG_HDCP_KSV_SHA_START1     (PP_PAGE | 0x33)
#define REG_HDCP_KSV_SHA_START2     (PP_PAGE | 0x34)

#define REG_SHA_LEN1                (PP_PAGE | 0x35)
#define REG_SHA_LEN2                (PP_PAGE | 0x36)

//HDCP Repeater Control Register
#define REG_SHA_CTRL                (PP_PAGE | 0x37)
#define BIT_SHA_GO                      0x01
#define BIT_SHA_DS_MODE                 0x04
#define BIT_SHA_EN                      0x08
#define BIT_SHA_CTRL_MANUAL_US          0x20
#define BIT_SHA_CTRL_MANUAL_DS          0x40
#define VAL_SHA_US_HWMODE               0x38
#define VAL_SHA_US_SWMODE               0x28
#define VAL_SHA_DS_SWMODE               0x48

//HDCP Repeater KSV FIFO Register
#define REG_KSV_FIFO                (PP_PAGE | 0x38)

//HDCP Repeater KSV FIFO Register
#define REG_SHA_CTRL3                (PP_PAGE | 0x3D)
#define BIT_SHA_START_TX0				0x40
#define BIT_SHA_START_TX1				0x80

// HDCP Debug Register
#define REG_HDCP_SHD_BSTAT2	        (PP_PAGE | 0x30)
#define BIT_HDMI_MODE                   0x10
#define BIT_DEPTH_EXCEEDED				0x08
#define VAL_DEPTH_EXCEEDED				0x08

#define REG_SLAVE_ADDR11            (PP_PAGE | 0x42)
#define REG_SLAVE_ADDR12            (PP_PAGE | 0x43)
#define REG_SLAVE_ADDR13            (PP_PAGE | 0x44)
#define REG_SLAVE_ADDR_TX0_0        (PP_PAGE | 0x45)
#define REG_SLAVE_ADDR_TX0_1        (PP_PAGE | 0x46)

#define REG_TX_VIDEO_SRC            (PP_PAGE | 0x50)

#define VAL_MUX_SEL_MASK                (0x03)
#define VAL_MUX_SEL_HDMI_MP             (0x00)
#define VAL_MUX_SEL_HDMI_RP             (0x01)
#define VAL_MUX_SEL_INTERNAL            (0x03)

#define VAL_MUX_SEL_TX0_SHIFT           2
#define VAL_MUX_SEL_TX1_SHIFT           4

#define MSK_MUX_SEL_TX0                 (VAL_MUX_SEL_MASK << VAL_MUX_SEL_TX0_SHIFT)
#define VAL_MUX_MP_SOURCE_TX0           (VAL_MUX_SEL_HDMI_MP << VAL_MUX_SEL_TX0_SHIFT)
#define VAL_MUX_RP_SOURCE_TX0           (VAL_MUX_SEL_HDMI_RP << VAL_MUX_SEL_TX0_SHIFT)
#define VAL_MUX_PV_SOURCE_TX0           (VAL_MUX_SEL_HDMI_PARALLEL << VAL_MUX_SEL_TX0_SHIFT)
#define VAL_MUX_VP_SOURCE_TX0           (VAL_MUX_SEL_HDMI_INTERNAL << VAL_MUX_SEL_TX0_SHIFT)
#define MSK_MUX_SEL_TX1                 (VAL_MUX_SEL_MASK << VAL_MUX_SEL_TX1_SHIFT)
#define VAL_MUX_MP_SOURCE_TX1           (VAL_MUX_SEL_HDMI_MP << VAL_MUX_SEL_TX1_SHIFT)
#define VAL_MUX_RP_SOURCE_TX1           (VAL_MUX_SEL_HDMI_RP << VAL_MUX_SEL_TX1_SHIFT)
#define VAL_MUX_PV_SOURCE_TX1           (VAL_MUX_SEL_HDMI_PARALLEL << VAL_MUX_SEL_TX1_SHIFT)
#define VAL_MUX_VP_SOURCE_TX1           (VAL_MUX_SEL_HDMI_INTERNAL << VAL_MUX_SEL_TX1_SHIFT)

#define BIT_TX0_PIPE_EN                 BIT6
#define VAL_PIPE_EN_SHIFT               6

#define REG_TX_OTP_SEL_CTRL            (PP_PAGE | 0x53)
#define MSK_TX_OTP_POLICY              (BIT2 | BIT1)

#define REG_PACKET_MUX0_CTRL           (PP_PAGE | 0x55)
#define REG_PACKET_MUX1_CTRL           (PP_PAGE | 0x56)
#define BIT_PIPE_SUB_SELECT             BIT6
#define BIT_PIPE_SELECTION_ENABLE       BIT7


#define REG_ARC_CTRL7                 (PP_PAGE | 0x6E)
#define BIT_ARC_TX_SELECT             BIT6
#define BIT_ARC_RX_SELECT             BIT7
#define BIT_ARC_RX_TX_CLEAR           0x00

    /* Interrupt Control/Status */

#define REG_INT_GROUP_STATE_0       (PP_PAGE | 0x70)
#define BIT_INTR                        BIT0
#define BIT_INTR_GRP0                   BIT1    // INTR1, INTR2, INTR4
#define BIT_INTR_GRP1                   BIT2    // INTR5, INTR6, INTR8, INTR9, INTR10
#define BIT_INTR_GRP_PA                 BIT3    // PA_INTR1-PA_INTR10
#define BIT_INTR_TX0                    BIT4    // TX0 interrupts
#define BIT_INTR_TX1                    BIT5    // TX1 interrupts
#define BIT_INTR_GRP5                   BIT6    // CBUS interrupts
#define BIT_INTR_GRP6                   BIT7    // CEC interrupts

#define REG_INT_GROUP_STATE_1       (PP_PAGE | 0x73)
#define BIT_INTR_GRP7                   BIT0
#define BIT_INTR_GRP8                   BIT2
#define BIT_INTR_PV                     BIT3
#define BIT_INTR_PWD                    BIT4

#define REG_CH0_INT_CTRL            (PP_PAGE | 0x79)
#define BIT_PHYS_INT_POL                0x02
#define BIT_DO_SW_INT                   0x08

#define REG_INT_STATUS_1            (PP_PAGE | 0x71)
#define REG_INT_ENABLE_1            (PP_PAGE | 0x75)
#define BIT_PWR_STATUS_CHANGED          0x08

#define REG_INT_STATUS_2            (PP_PAGE | 0x72)
#define REG_INT_ENABLE_2            (PP_PAGE | 0x76)
#define BIT_RP_HDMI_DETECT_INT          0x01
#define BIT_RP_SCDT_CHG				    0x02
#define BIT_RP_CKDT_CHG                 0x04
#define BIT_SCDT_CHG                    0x08
#define BIT_CKDT_CHG                    0x10
#define BIT_SW_INT                      0x20
#define BIT_HDMI_DETECT_INT             0x80

#define REG_INT_CTRL        	    (PP_PAGE | 0x79)
#define BIT_PHYS_INT_POL                0x02
#define BIT_DO_SW_INT                   0x08

#define REG_INT_STATUS_5            (PP_PAGE | 0x7B)
#define REG_INT_ENABLE_5            (PP_PAGE | 0x7D)
#define BIT_5VPWR_CHG_MP                0x80    // Main pipe (selected port)
#define BIT_5VPWR_CHG_RP                0x40    // Roving pipe (selected port)
#define MSK_5VPWR_CHG_ALL_PORTS         0x0F

#define REG_RX_TMDS_TERM_2          (PP_PAGE | 0x81)

#define REG_RX_TMDS_TERM_0          (PP_PAGE | 0x82)
#define VAL_TERM_ALL_ON                 0x00
#define VAL_TERM_ON                     0x00
#define VAL_TERM_ALL_MHL                0x55
#define VAL_TERM_MHL                    0x01
#define VAL_TERM_ALL_SURGE              0xAA
#define VAL_TERM_SURGE                  0x02
#define VAL_TERM_ALL_OFF                0xFF
#define VAL_TERM_OFF                    0x03
#define MSK_TERM                        0x03


#define REG_TMDS_TERMCTRL1          (PP_PAGE | 0x83)
#define MSK_TMDS_PORT0                  (BIT7 | BIT6 | BIT5)
#define MSK_TMDS_PORT1                  (BIT2 | BIT1 | BIT0)

#define REG_TMDS_TERMCTRL3          (PP_PAGE | 0x80)
#define MSK_TMDS_PORT2                  (BIT7 | BIT6 | BIT5)
#define MSK_TMDS_PORT3                  (BIT2 | BIT1 | BIT0)

#define REG_PD_TOT                  (PP_PAGE | 0x84)
#define REG_PD_SYS2                 (PP_PAGE | 0x86)
#define REG_PD_SYS3                 (PP_PAGE | 0x87)
#define REG_PD_SYS4                 (PP_PAGE | 0x88)

#define REG_TMDS_TERMCTRL4          (PP_PAGE | 0x89)

#define SFT_TMDS_PORT_EVEN              0x5
#define VAL_TMDS_TERM_MHL               0x5
#define VAL_TMDS_TERM_HDMI              0x5

#define REG_PD_SYS_EXT2             (PP_PAGE | 0x8A)
#define REG_PD_SYS_EXT3             (PP_PAGE | 0x8B)
#define REG_PD_SYS_EXT4             (PP_PAGE | 0x8C)

#define REG_ESD_PROT_SEL_HWSW       (PP_PAGE | 0x8D)
#define BIT_ESD_MASK				    0x3F
#define BIT_ESD_MASK_P4				    0x10

#define REG_INT_STATUS_7            (PP_PAGE | 0x90)
#define REG_INT_ENABLE_7            (PP_PAGE | 0x92)
#define BIT_CEC_FIFO_FULL               0x10
#define BIT_BOOT_DONE_INT               0x40

#define REG_INT_STATUS_19           (PP_PAGE | 0xB2)
#define REG_INT_MASK_19             (PP_PAGE | 0xB4)
#define BIT_MP_SET_AVMUTE_RCVD          BIT0
#define BIT_MP_INTERLACED_VIDEO         BIT1
#define BIT_MP_HVSYNC_POLARITY_CHG      BIT2
#define BIT_MP_HRES_CHG                 BIT3
#define BIT_MP_VRES_CHG                 BIT4

#define REG_INT_STATUS_20           (PP_PAGE | 0xB3)
#define REG_INT_MASK_20             (PP_PAGE | 0xB5)
#define BIT_SP_SET_AVMUTE_RCVD          BIT0
#define BIT_SP_INTERLACED_VIDEO         BIT1
#define BIT_SP_HVSYNC_POLARITY_CHG      BIT2
#define BIT_SP_HRES_CHG                 BIT3
#define BIT_SP_VRES_CHG                 BIT4

#define REG_DS_BSTAT1			    (PP_PAGE | 0xD5)
#define BIT_DEVEXC          			0x80
#define BIT_DEVICECNT          			0x7F

#define REG_DS_BSTAT2			    (PP_PAGE | 0xD6)
#define BIT_HDMI_MODE          			0x10
#define BIT_CASEXC          			0x08
#define BIT_DEPTH          				0x07

#define REG_DS_M0				    (PP_PAGE | 0xD7)

#define REG_DS_VH0_0				(PP_PAGE | 0xDF)

//------------------------------------------------------------------------------
// Registers in Page 2      (0x66)
//------------------------------------------------------------------------------

#define REG_TMDS3_CCTRL1            (PP_PAGE_2 | 0x00)

#define REG_TMDS3_CTRL2             (PP_PAGE_2 | 0x02)
#define REG_TMDS3_CTRL3             (PP_PAGE_2 | 0x03)
#define REG_TMDS3_CLKDETECT_CTL		(PP_PAGE_2 | 0x04)
#define REG_PLL3_VCOCAL				(PP_PAGE_2 | 0x0A)
#define REG_XTAL_CAL_CONTRL  		(PP_PAGE_2 | 0x16)

#define REG_TMDS2_CCTRL1            (PP_PAGE_2 | 0x80)

#define REG_TMDS2_CTRL2             (PP_PAGE_2 | 0x82)
#define REG_TMDS2_CTRL3             (PP_PAGE_2 | 0x83)
#define REG_TMDS2_CLKDETECT_CTL     (PP_PAGE_2 | 0x84)

#define REG_PLL3_CALREFSEL          (PP_PAGE_2 | 0x07)
#define REG_PLL2_CALREFSEL          (PP_PAGE_2 | 0x87)
#define REG_PLL2_VCOCAL				(PP_PAGE_2 | 0x8A)
#define REG_APLL_POWER              (PP_PAGE_2 | 0xD0)
#define BIT_APLL0_PWR_ON                0x01
#define BIT_APLL1_PWR_ON                0x02

//------------------------------------------------------------------------------
// Registers in Page 4      (0x68)
//------------------------------------------------------------------------------
#define REG_TMDS4_CCTRL1            (PP_PAGE_4 | 0x00)
#define BIT_TMDS4_ESD_MASK			    0xC0
#define BIT_TMDS4_ESD_P4			    0x80
#define BIT_TMDS4_ESD_P4_DF			    0x40 	//Default value.

#define REG_TMDS4_CTRL2             (PP_PAGE_4 | 0x02)
#define REG_TMDS4_CTRL3             (PP_PAGE_4 | 0x03)
#define REG_TMDS4_CLKDETECT_CTL     (PP_PAGE_4 | 0x04)

#define REG_PLL4_CALREFSEL          (PP_PAGE_4 | 0x07)
#define REG_PLL5_CALREFSEL          (PP_PAGE_4 | 0x27)

//------------------------------------------------------------------------------
// Registers in Page 5     (0x50)
//------------------------------------------------------------------------------
#define REG_INT_STATUS_IP1          (PP_PAGE_5 | 0x01)
#define REG_INT_ENABLE_IP1          (PP_PAGE_5 | 0x15)
#define BIT_MP_CEC_HPE                  0x08
#define BIT_RP_CEC_HPE                  0x20

#define REG_INT_STATUS_P1           (PP_PAGE_5 | 0x02)
#define REG_INT_ENABLE_P1           (PP_PAGE_5 | 0x16)
#define BIT_MP_AV_MUTE                  0x10
#define BIT_MP_AV_UNMUTE                0x20
#define BIT_RP_AV_MUTE                  0x80
#define BIT_MP_AV_MUTE_MASK             BIT_MP_AV_MUTE
#define BIT_MP_AV_UNMUTE_MASK           BIT_MP_AV_UNMUTE
#define BIT_RP_AV_MUTE_MASK             BIT_RP_AV_MUTE

#define REG_INT_STATUS_P2           (PP_PAGE_5 | 0x03)
#define REG_INT_ENABLE_P2           (PP_PAGE_5 | 0x17)
#define BIT_RP_AV_UNMUTE                0x01
#define BIT_RP_AV_UNMUTE_MASK           BIT_RP_AV_UNMUTE

#define REG_INT_STATUS_IP2          (PP_PAGE_5 | 0x05)
#define REG_INT_ENABLE_IP2          (PP_PAGE_5 | 0x19)
#define BIT_HDCP_GOOD                   0x04
#define BIT_DECRYPT                     0x08
#define BIT_AUTH_CHG_P0                 0x10
#define BIT_AUTH_CHG_P1                 0x20
#define BIT_AUTH_CHG_P2                 0x40
#define BIT_AUTH_CHG_P3                 0x80
#define MSK_AUTH_CHG_P0_P3          ( BIT_AUTH_CHG_P0 | BIT_AUTH_CHG_P1 | BIT_AUTH_CHG_P2 | BIT_AUTH_CHG_P3 )

#define REG_INT_STATUS_P3           (PP_PAGE_5 | 0x06)
#define REG_INT_ENABLE_P3           (PP_PAGE_5 | 0x1A)
#define BIT_MP_NO_AIF 					0x04
#define BIT_MP_NO_AVI 					0x08
#define BIT_MP_NO_VSI 					0x10
#define BIT_MP_NEW_AIF 					0x20
#define BIT_MP_NEW_AVI 					0x40
#define BIT_MP_NEW_VSI 					0x80
#define BIT_P3_ENABLE_MP_NO_ALL         0x1C
#define BIT_P3_ENABLE_MP_NEW_ALL        0xE0
#define MSK_P3_MP_IF_STATUS             0xFC

#define REG_INT_STATUS_P4           (PP_PAGE_5 | 0x07)
#define REG_INT_ENABLE_P4           (PP_PAGE_5 | 0x1B)
#define BIT_RP_NO_AIF 					0x01
#define BIT_RP_NO_AVI 					0x02
#define BIT_RP_NO_VSI 					0x04
#define BIT_RP_NEW_AIF 					0x08
#define BIT_RP_NEW_AVI 					0x10
#define BIT_RP_NEW_VSI 					0x20
#define MSK_RP_IF_STATUS                0x3F
#define BIT_MP_IP_MUTE                  0x40
#define BIT_MP_IP_UNMUTE                0x80
#define MSK_P4_RP_IF_STATUS             0x3F

#define REG_INT_STATUS_IP3          (PP_PAGE_5 | 0x08)
#define REG_INT_ENABLE_IP3          (PP_PAGE_5 | 0x1C)
#define BIT_P0_ECC						0x04
#define BIT_P1_ECC 						0x08
#define BIT_P2_ECC 						0x10
#define BIT_P3_ECC 						0x20
#define BIT_P4_ECC 						0x40
#define BIT_MP_MUTE_STATUS 				0x80

#define REG_INT_STATUS_IP4          (PP_PAGE_5 | 0x09)
#define REG_INT_ENABLE_IP4          (PP_PAGE_5 | 0x1D)
#define BIT_RP_MUTE_STATUS              0x01
#define BIT_MP_RES_STABLE_CHG           0x08
#define BIT_MP_NEW_ACP                  0x40

#define REG_INT_STATUS_IP5          (PP_PAGE_5 | 0x0A)
#define REG_INT_ENABLE_IP5          (PP_PAGE_5 | 0x1E)
#define BIT_RP_NEW_ACP              	0x04


//Pre-authentication Control Register
#define REG_MISC_CTRL1              (PP_PAGE_5 | 0x3A)
#define BIT_PORT_CHG_ENABLE             0x01
#define BIT_SKIP_NON_HDCP               0x02

#define REG_SELECTED_PORT_0         (PP_PAGE_5 | 0x3B)
#define MAIN_PIPE_MASK                  0x0F
#define ROVE_PIPE_MASK                  0xF0

// Pre-authentication Status Register
#define REG_PAUTH_STAT1             (PP_PAGE_5 | 0x3C)
#define BIT_DECRYPT_MASK                0xF0
#define BIT_AUTH_MASK                   0x0F
#define BIT_P0_AUTH         			0x01
#define BIT_P1_AUTH         			0x02
#define BIT_P2_AUTH         			0x04
#define BIT_P3_AUTH         			0x08

// Pre-authentication Roving Pipe Overwrite Register
#define REG_PAUTH_MPOVR             (PP_PAGE_5 | 0x3E)
#define REG_PAUTH_RPOVR             (PP_PAGE_5 | 0x3F)
#define BIT_FORCE_RP_PORT               0x01
#define BIT_FORCE_RP_SELECTION          0x80

// InstaPort Reset Length Register
#define REG_PAUTH_RSTLEN0           (PP_PAGE_5 | 0x40)
#define REG_PAUTH_RSTLEN1           (PP_PAGE_5 | 0x41)
#define REG_PAUTH_RSTLEN2           (PP_PAGE_5 | 0x42)
#define REG_PAUTH_RSTLEN3           (PP_PAGE_5 | 0x43)

// InstaPort Reset On/Off Difference between Analog/Digital resets.
// This set of registers is for the leading edge of the resets.
#define REG_PAUTH_RST_ON_DIFF0      (PP_PAGE_5 | 0x44)
#define REG_PAUTH_RST_ON_DIFF1      (PP_PAGE_5 | 0x45)
#define REG_PAUTH_RST_ON_DIFF2      (PP_PAGE_5 | 0x46)
#define REG_PAUTH_RST_ON_DIFF3      (PP_PAGE_5 | 0x47)

// InstaPort Time Out Limit Register
#define REG_PAUTH_TMOUT0            (PP_PAGE_5 | 0x48)

// InstaPort Port Kept Authenticated Time Register
#define REG_PAUTH_KEEPLEN0          (PP_PAGE_5 | 0x4C)

// InstaPort Authenticated Port Time Registers
#define REG_IP_AUTH_PORT_TIME_7_0   (PP_PAGE_5 | 0x50)
#define REG_IP_AUTH_PORT_TIME_15_8  (PP_PAGE_5 | 0x51)
#define REG_IP_AUTH_PORT_TIME_23_16 (PP_PAGE_5 | 0x52)
#define REG_IP_AUTH_PORT_TIME_31_24 (PP_PAGE_5 | 0x53)

#define REG_PAUTH_HDMIM_CTRL        (PP_PAGE_5 | 0x54)
#define BIT_MP_HDMI_OVR    			    (BIT1)

#define REG_PAUTH_BCAPS_CTRL        (PP_PAGE_5 | 0x55)
#define BIT_P0_HDMI_CAPABLE    			0x02
#define BIT_P1_HDMI_CAPABLE    			0x08
#define BIT_P2_HDMI_CAPABLE    			0x20
#define BIT_P3_HDMI_CAPABLE    			0x80

#define PAUTH_HDMI_MODE             (PP_PAGE_5 | 0x56)
#define REG_HDMI_DETECTED               0x01

#define REG_PAUTH_ECC_CTRL          (PP_PAGE_5 | 0x57)
#define REG_EN_ECC                      0x10
#define BIT_EN_ECC                      0x10

#define REG_PAUTH_ECC_CHKTIME       (PP_PAGE_5 | 0x58)

#define REG_PAUTH_INV_CTRL          (PP_PAGE_5 | 0x59)
#define TX_BIT_INV                      0x80
#define MP_CLK_INV                      0x01

#define REG_PAUTH_MP_AOVR           (PP_PAGE_5 | 0x5A)
#define BIT_MP_OVR_P0                   (BIT0)

#define REG_PAUTH_RP_AOVR           (PP_PAGE_5 | 0x5B)

#define REG_MP_DLY_STAT             (PP_PAGE_5 | 0x5C)
#define REG_P0_DLY_STAT             (PP_PAGE_5 | 0x5D)
#define BIT_P5_AUTH                     0x10
#define BIT_P5_DECRYPT                  0x20

#define REG_P1_DLY_STAT             (PP_PAGE_5 | 0x5E)
#define REG_P2_DLY_STAT             (PP_PAGE_5 | 0x5F)
#define REG_P3_DLY_STAT             (PP_PAGE_5 | 0x60)

#define REG_PAUTH_ECC_THRES0        (PP_PAGE_5 | 0x61)
#define REG_PAUTH_ECC_THRES1        (PP_PAGE_5 | 0x62)

#define REG_MISC_CTRL0              (PP_PAGE_5 | 0x63)
#define BIT_CLEAR_ECC                   0x01
#define BIT_VIDEO_MUTE_SYNC             0x10
#define BIT_AUDIO_MUTE_SYNC             0x20
#define BIT_USE_FRAME_ECC               0x80

#define REG_FRAME_ECC_THR           (PP_PAGE_5 | 0x65)
#define REG_PAUTH_ECC_ERR0          (PP_PAGE_5 | 0x68)
#define REG_PAUTH_ECC_ERR1          (PP_PAGE_5 | 0x69)

#define REG_PAUTH_BCAPS_CTRL2       (PP_PAGE_5 | 0x6C)
#define BIT_P4_DECRYPT                  0x80
#define BIT_P4_DM_DONE                  0x40
#define BIT_P4_AUTH                     0x08
#define BIT_P4_AUTH_DONE                0x04
#define BIT_PAUTH_BCAPS_CTRL2_P4_AUTH	0x10

#define REG_SELECTED_PORT_1         (PP_PAGE_5 | 0x6D)
#define MAIN_PIPE_MASK2                 0x03
#define ROVE_PIPE_MASK2                 0x30

#define REG_PAUTH_RST_DIS_CKDT      (PP_PAGE_5 | 0x6E)
#define BIT_HDCP_RST_CKDT_EN            0x01
#define BIT_CKDT_FILTER_EN              0x02

#define REG_PAUTH_MODE			    (PP_PAGE_5 | 0x6F)
#define BIT_CLR_BOTH_RI			        0x20

#define REG_PA_CONFIG_1             (PP_PAGE_5 | 0x75)

#define REG_PORT_INFOFRAME_SELECT   (PP_PAGE_5 | 0x7C)
#define VAL_RD_PORT_SEL_MASK            0x07
#define BIT_VALID_CLEAR_NO_POWER        0x08
#define BIT_VALID_CLEAR_DVI_MODE        0x10
#define BIT_RD_PIPE_SEL                (BIT5)
#define BIT_CS_PORT_PIPE_SEL           (BIT6)
#define BIT_RD_CS_PIPE_SEL             (BIT7)

#define REG_3D_MARKER_CTRL          (PP_PAGE_5 | 0x7F)
#define BIT_EN_3D_MARKER_SUPPORT        0x08
#define BIT_EN_3D_VS_INSERT             0x10
#define BIT_SKIP_HDMI_3D_FORMAT_CHK     0x20
#define BIT_SKIP_IEEE_ID_CHK            0x40
#define BIT_SKIP_VSI_HEADER_CHK         0x80

#define REG_3D_1080P_MARKER_DLY_LO  (PP_PAGE_5 | 0x80)
#define REG_3D_1080P_MARKER_DLY_HI  (PP_PAGE_5 | 0x81)
#define BIT_SKIP_3D_STRUCT_CHK_MP       0x10
#define BIT_SKIP_3D_STRUCT_CHK_RP       0x20
#define BIT_ENABLE_300MHZ_3D_SUPPORT    0x40

#define REG_3D_MARKER_CTRL_RP       (PP_PAGE_5 | 0x83)

#define REG_3D_720P_MARKER_DLY_LO   (PP_PAGE_5 | 0x84)
#define REG_3D_720P_MARKER_DLY_HI   (PP_PAGE_5 | 0x85)

    // Auto (HW) Hot Plug Event control

#define REG_HPE_TRIGGER             (PP_PAGE_5 | 0x87)
#define BIT_OVR_MP                      0x01
#define BIT_TRIGGER_MP                  0x02
#define BIT_OVR_RP                      0x08
#define BIT_TRIGGER_RP                  0x10

#define REG_PHYS_HPD_DISABLE        (PP_PAGE_5 | 0x88)
#define VAL_FW_HPE_MASK                 0xFC

#define REG_IP_TIMER_GRANULARITY    (PP_PAGE_5 | 0x89)   // In ~ms (1.0235ms - 129.98ms)
#define REG_IP_HPE_HPD_START        (PP_PAGE_5 | 0x8A)   // In steps of REG_HPE_TIMER_GRAN
#define REG_IP_HPE_HPD_END          (PP_PAGE_5 | 0x8B)   // In steps of REG_HPE_TIMER_GRAN
#define REG_IP_HPE_RXTERM_START     (PP_PAGE_5 | 0x8C)   // In steps of REG_HPE_TIMER_GRAN
#define REG_IP_HPE_RXTERM_END       (PP_PAGE_5 | 0x8D)   // In steps of REG_HPE_TIMER_GRAN
#define REG_IP_HPE_HDCP_DDC_START   (PP_PAGE_5 | 0x8E)   // In steps of REG_HPE_TIMER_GRAN
#define REG_IP_HPE_HDCP_DDC_END     (PP_PAGE_5 | 0x8F)   // In steps of REG_HPE_TIMER_GRAN
#define REG_IP_HPE_EDID_DDC_START   (PP_PAGE_5 | 0x90)   // In steps of REG_HPE_TIMER_GRAN
#define REG_IP_HPE_EDID_DDC_END     (PP_PAGE_5 | 0x91)   // In steps of REG_HPE_TIMER_GRAN

#define REG_HPE_DRIVER_OVERRIDE_MP  (PP_PAGE_5 | 0x92)
#define REG_HPE_DRIVER_OVERRIDE_RP  (PP_PAGE_5 | 0x93)
#define BIT_HPD_DISABLE                 0x01
#define BIT_RXTERM_DISABLE              0x02
#define BIT_HDDC_DISABLE                0x04
#define BIT_EDDC_DISABLE                0x08
#define BIT_HPD_HW_OVR                  0x10
#define BIT_RXTERM_HW_OVR               0x20
#define BIT_HDDC_HW_OVR                 0x40
#define BIT_EDDC_HW_OVR                 0x80

#define REG_HDCP_STABLE_TIME0       (PP_PAGE_5 | 0x97)
#define REG_HDCP_STABLE_TIME1       (PP_PAGE_5 | 0x98)

#define REG_IP_CONFIG_43            (PP_PAGE_5 | 0x9F)
#define BGND_DISABLE                    0x80

#define REG_IP_CONFIG_47            (PP_PAGE_5 | 0xA3)
#define MSK_ALL_PORTS                   0x0F

#define PA_MHL1X_CONF_REG_1         (PP_PAGE_5 | 0xAF)
#define BIT_ALWAYS_1X_MHL1X             0x80
#define BIT_MHL_OPMODE_3X_DEF           0x01

#define REG_PORT_MP_ALTRES_DATA_0   (PP_PAGE_5 | 0xCC)       // First of 8 consecutive registers (main pipe)
#define REG_PORT_ALTRES_MISC        (PP_PAGE_5 | 0xD3)       // Last of 8 consecutive registers
#define BIT_IS_INTERLACED               0x01
#define BIT_IS_HDMI                     0x08
#define BIT_ALT_RES_DATA_VALID          0x10
#define REG_PORT_SP_ALTRES_DATA_0   (PP_PAGE_5 | 0xE9)       // First of 8 consecutive registers (sub pipe)

#define REG_MP_GCP_STATUS           (PP_PAGE_5 | 0xE8)
#define REG_RP_GCP_STATUS           (PP_PAGE_5 | 0xF0)
#define BIT_MP_RES_STABLE            0x40                   // Only in MP GCP status register
#define BIT_GCP_MUTE                 0x04
#define BIT_GCP_UNMUTE               0x08

#define REG_RP_COLOR_DEPTH          (PP_PAGE_5 | 0xF1)
#define MSK_COLOR_DEPTH              0x0F

#define REG_ECC_ERRCOUNT_LO_MP      (PP_PAGE_5 | 0xF2)
#define REG_ECC_ERRCOUNT_HI_MP      (PP_PAGE_5 | 0xF3)

//------------------------------------------------------------------------------
// Registers in Page 6      (0x52)
//------------------------------------------------------------------------------

#define REG_DCK_FIFO			    (PP_PAGE_6 | 0x64)
#define BIT_NRST_DCK_FIFO		        0x01
#define BIT_SCDT_RST_DCK_FIFO	        0x40

    /* Pre-Authentication Reset On/Off Difference between Analog/Digital resets.    */
    /* This set of registers is for the trailing edge of the resets.                */

#define REG_PAUTH_RST_OFF_DIFF0     (PP_PAGE_6 | 0x6D)
#define REG_PAUTH_RST_OFF_DIFF1     (PP_PAGE_6 | 0x6E)
#define REG_PAUTH_RST_OFF_DIFF2     (PP_PAGE_6 | 0x6F)
#define REG_PAUTH_RST_OFF_DIFF3     (PP_PAGE_6 | 0x70)
#define REG_AIF_CAPTURE_HEADER      (PP_PAGE_6 | 0x72)
#define REG_VSIF_CAPTURE_HEADER     (PP_PAGE_6 | 0x73)

#define REG_PAUTH_ALT_ZONE4_CFG     (PP_PAGE_6 | 0x7D)
#define BIT_PAUTH_AZ4_INT_DIV20         (BIT7)


#define REG_PLL0_ZONECTL            (PP_PAGE_6 | 0xC0)
#define REG_DPLL_MULTZONE_RDLY0     (PP_PAGE_6 | 0xC2)
#define REG_DPLL_MULTZONE_RDLY1     (PP_PAGE_6 | 0xC3)
#define REG_TMDST_TXDAT1            (PP_PAGE_6 | 0xC5)
#define REG_TMDS2_STAT6             (PP_PAGE_6 | 0xC6)

#define REG_TMDS0_BW_DATA0			(PP_PAGE_6 | 0xAE)
#define REG_TMDS0_BW_DATA1			(PP_PAGE_6 | 0xAF)
#define REG_TMDS0_BW_DATA2			(PP_PAGE_6 | 0xB0)
#define REG_TMDS0_BW_DATA3			(PP_PAGE_6 | 0xB1)
#define REG_TMDS0_CNTL          	(PP_PAGE_6 | 0xB3)
#define REG_TMDS0_CNTL2          	(PP_PAGE_6 | 0xB4)

#define REG_TMDS1_BW_DATA0			(PP_PAGE_6 | 0x90)
#define REG_TMDS1_BW_DATA2			(PP_PAGE_6 | 0x92)
#define REG_TMDS1_CNTL              (PP_PAGE_6 | 0x95)

#define REG_A0_EQ_I2C               (PP_PAGE_6 | 0xA3)
#define REG_A0_EQ_DATA0             (PP_PAGE_6 | 0xA4)
#define REG_A0_EQ_DATA1             (PP_PAGE_6 | 0xA5)
#define REG_A0_EQ_DATA2             (PP_PAGE_6 | 0xA6)
#define REG_A0_EQ_DATA3             (PP_PAGE_6 | 0xA7)
#define REG_A0_EQ_DATA4             (PP_PAGE_6 | 0xA8)
#define REG_A0_EQ_DATA5             (PP_PAGE_6 | 0xA9)
#define REG_A0_EQ_DATA6             (PP_PAGE_6 | 0xAA)
#define REG_A0_EQ_DATA7             (PP_PAGE_6 | 0xAB)

#define REG_A1_EQ_DATA0             (PP_PAGE_6 | 0x86)
#define REG_A1_EQ_DATA1             (PP_PAGE_6 | 0x87)
#define REG_A1_EQ_DATA2             (PP_PAGE_6 | 0x88)
#define REG_A1_EQ_DATA3             (PP_PAGE_6 | 0x89)
#define REG_A1_EQ_DATA4             (PP_PAGE_6 | 0x8A)
#define REG_A1_EQ_DATA5             (PP_PAGE_6 | 0x8B)
#define REG_A1_EQ_DATA6             (PP_PAGE_6 | 0x8C)
#define REG_A1_EQ_DATA7             (PP_PAGE_6 | 0x8D)

#define REG_PAUTH_NUM_SAMPS         (PP_PAGE_6 | 0xD0)
#define REG_PAUTH_SMP_INTV          (PP_PAGE_6 | 0xD1)
#define REG_PAUTH_ADD_CONFIG1       (PP_PAGE_6 | 0xD2)

#define REG_H_ACTIVE_L              (PP_PAGE_6 | 0xD9)
#define REG_H_ACTIVE_H              (PP_PAGE_6 | 0xDA)
#define MSK_H_ACTIVE_H                  0x1F

#define REG_V_ACTIVE_L              (PP_PAGE_6 | 0xDD)
#define REG_V_ACTIVE_H              (PP_PAGE_6 | 0xDE)
#define MSK_V_ACTIVE_H                  0x1F

#define REG_VFORMAT                 (PP_PAGE_6 | 0xE0)
#define MSK_VFORMAT_VID                 0x7F
#define BIT_IS_INTERLACE                0x80

#define REG_RO_MP_CONTENT           (PP_PAGE_6 | 0xE1)

#define REG_MP_COLOR_DEPTH          (PP_PAGE_6 | 0xE2)

#define REG_BCH_CORRECTED_THSHLD    (PP_PAGE_6 | 0xF3)

//------------------------------------------------------------------------------
// Registers in Page 7      (0x54)
//------------------------------------------------------------------------------
#define REG_PAUTH_MHL3D_CFG1            (PP_PAGE_PAUTH3 | 0x00)
#define REG_PAUTH_MHL3D_CFG2            (PP_PAGE_PAUTH3 | 0x01)

#define REG_PAUTH_MHL_IEEE_ID1          (PP_PAGE_PAUTH3 | 0x02)
#define REG_PAUTH_MHL_IEEE_ID2          (PP_PAGE_PAUTH3 | 0x03)
#define REG_PAUTH_MHL_IEEE_ID3          (PP_PAGE_PAUTH3 | 0x04)


#define REG_CLR_PACKET_BUFFER       (PP_PAGE_7 | 0x20)
#define BIT_VSI_CLR_EN                  0x01
#define BIT_AIF_CLR_EN                  0x02
#define BIT_VSI_ID_CHK_EN               0x08
#define BIT_USE_AIF_FOR_VSI             0x20

#define REG_SP_ACP_HEADER   	    (PP_PAGE_7 | 0x21)

#define REG_MP_AIF_CONTENT_HEADER   (PP_PAGE_7 | 0x90)

#define REG_SP_AIF_HEADER           (PP_PAGE_7 | 0x90)

#define REG_SP_AVI_HEADER           (PP_PAGE_7 | 0xB0)
#define REG_SP_AVI_PB2              (PP_PAGE_7 | 0xB5)       // Contains PREP data
#define REG_SP_AVI_PB4              (PP_PAGE_7 | 0xB7)       // Contains VIC data
#define REG_SP_AVI_PB5              (PP_PAGE_7 | 0xB8)       // Contains PREP data

#define REG_RP_VISF_CONTENT_HEADER      (PP_PAGE_7 | 0xD0)

#define REG_VISF_PB1      				(PP_PAGE_7 | 0xF0)
#define REG_VISF_PB2      				(PP_PAGE_7 | 0xF1)
#define REG_VISF_PB3      				(PP_PAGE_7 | 0xF2)
#define REG_VISF_PB4      				(PP_PAGE_7 | 0xF3)

#define REG_SP_AVI_VALID            (PP_PAGE_7 | 0xC1)
#define BIT_SP_AVI_DATA_VALID           0x01

//------------------------------------------------------------------------------
// Registers in Page 9      (0xE0)
//------------------------------------------------------------------------------

#define RX_EDID_DDC_EN             (PP_PAGE_EDID | 0x01)
#define BIT_EDDC_EN0                    0x01
#define BIT_EDDC_EN1                    0x02
#define BIT_EDDC_EN2                    0x04
#define BIT_EDDC_EN3                    0x08
#define VAL_EN_DDC_NONE                 0x00
#define VAL_EN_DDC_ALL                  0x0F
#define MSK_EN_DDC_ALL                  0x0F

#define NVM_HPD_HW_CTRL				(PP_PAGE_9 | 0x14)

// Newly added Registers in Melbourne for NVRAM Bypass
#define REG_NVM_REPLACE_BYTE0				(PP_PAGE_9 | 0x32)
#define REG_NVM_REPLACE_BYTE1				(PP_PAGE_9 | 0x33)
#define REG_NVM_REPLACE_BYTE2				(PP_PAGE_9 | 0x34)
#define REG_NVM_REPLACE_BYTE3				(PP_PAGE_9 | 0x35)
#define REG_NVM_BSM_REPLACE  				(PP_PAGE_9 | 0x36)


#define REG_DDC_FILTER_SEL			(PP_PAGE_9 | 0x70)
#define MSK_FILTER_SETTING              0x03
#define VAL_FILTER_SETTING              0x02


#define REG_POR_CTRL                (PP_PAGE_9 | 0x75)
#define MSK_PDD_THRESHOLD               0xF0
#define MSK_AON_THRESHOLD               0x0F

#define REG_REGUL_PWR_ENABLE        (PP_PAGE_9 | 0x78)
#define BIT_PEN_RX12V                   0x01
#define BIT_NEN_RX12V                   0x02
#define BIT_PEN_TX12V                   0x04
#define BIT_NEN_TX12V                   0x08
#define BIT_PEN_LOGIC12V                0x10
#define BIT_NEN_LOGIC12V                0x20

#define REG_REGUL_VOLT_CTRL         (PP_PAGE_9 | 0x7A)
#define MSK_VOLT_CTRLB                  0x38
#define MSK_VOLT_CTRL                   0x07

#define REG_REGUL_BIAS_CTRL         (PP_PAGE_9 | 0x7B)
#define MSK_BIAS_CTRL                   0x07

#define REG_CLKDETECT_STATUS        (PP_PAGE_9 | 0xD0)
#define BIT_CKDT_0                      0x01
#define BIT_CKDT_1                      0x02
#define BIT_CKDT_2                      0x04
#define BIT_CKDT_3                      0x08
#define MSK_CKDT_ALL                    0x0F

#define REG_PWR5V_STATUS            (PP_PAGE_9 | 0xD1)
#define BIT_PWR5V_0                     0x01
#define BIT_PWR5V_1                     0x02
#define BIT_PWR5V_2                     0x04
#define BIT_PWR5V_3                     0x08
#define MSK_PWR5V_ALL                   0x0F

#define REG_MHL_WKPWIDTH_MIN1L            (PP_PAGE_9 | 0xA8)
#define REG_MHL_WKPWIDTH_MIN1M            (PP_PAGE_9 | 0xA9)
#define REG_MHL_WKPWIDTH_MIN1H            (PP_PAGE_9 | 0xAA)
#define REG_MHL_WKPWIDTH_MAX1L            (PP_PAGE_9 | 0xAB)
#define REG_MHL_WKPWIDTH_MAX1M            (PP_PAGE_9 | 0xAC)
#define REG_MHL_WKPWIDTH_MAX1H            (PP_PAGE_9 | 0xAD)
#define REG_MHL_WKPWIDTH_MIN2L            (PP_PAGE_9 | 0xAE)
#define REG_MHL_WKPWIDTH_MIN2M            (PP_PAGE_9 | 0xAF)
#define REG_MHL_WKPWIDTH_MIN2H            (PP_PAGE_9 | 0xB0)
#define REG_MHL_WKPWIDTH_MAX2L            (PP_PAGE_9 | 0xB1)
#define REG_MHL_WKPWIDTH_MAX2M            (PP_PAGE_9 | 0xB2)
#define REG_MHL_WKPWIDTH_MAX2H            (PP_PAGE_9 | 0xB3)

#define REG_SPECIAL_PURPOSE         (PP_PAGE_9 | 0xFF)
#define BIT_HARDRESET                   0x80

//------------------------------------------------------------------------------
// Registers in Page A      (0x64)
//------------------------------------------------------------------------------

#define REG_TMDS1_CTRL2             (PP_PAGE_A | 0x02)
#define REG_HDMI_RX1_MHL_TEST_3X          0x00        // use 1/3x clock input in MHL mode0: 1x input (default)
#define REG_HDMI_RX1_MHL_TEST_1X          0x08        // use 1/3x clock input in MHL mode0: 1/3x input clock in MHL

#define REG_TMDS1_CTRL3             (PP_PAGE_A | 0x03)
#define REG_HDMI_RX1_PLL_CONFIG           0x00

#define REG_TMDS1_CLKDETECT_CTL		(PP_PAGE_A | 0x04)  //kamal

#define REG_PLL1_CALREFSEL          (PP_PAGE_A | 0x07)

#define REG_PLL1_VCOCAL             (PP_PAGE_A | 0x0A) //kamal

#define REG_TMDS0_CTRL2             (PP_PAGE_A | 0x82)
#define REG_MHL_TEST_SOFT_CTRL_3X         0x20        // use 1/3x clock input in MHL mode0: 1x input (default)
#define REG_MHL_TEST_SOFT_CTRL_1X         0x28        // use 1/3x clock input in MHL mode0: 1/3x input clock in MHL

// PLL configuration
#define REG_TMDS0_CTRL3             (PP_PAGE_A | 0x83)

#define REG_TMDS0_CLKDETECT_CTL		(PP_PAGE_A | 0x84)

#define REG_PLL0_CALREFSEL          (PP_PAGE_A | 0x87)

#define REG_PLL0_VCOCAL             (PP_PAGE_A | 0x8A)

//------------------------------------------------------------------------------
// Registers in Page 11     (0x90)
//------------------------------------------------------------------------------

#define REG_TMDST_CTRL1             (PP_PAGE_B | 0x00)
#define MSK_TMDS_EN                     0x01
#define MSK_TMDS_OE_ALL                 0xF0

#define REG_TMDST_CTRL2             (PP_PAGE_B | 0x01)

#define REG_TMDST_CTRL3             (PP_PAGE_B | 0x02)
#define MSK_TMDS_TERM_EN_ALL            0x0F

#define REG_SYS_CTRL2               (PP_PAGE_B | 0x03)
#define BIT_CHEAP_HPD_ALT               0x80
#define BIT_PWR_AS_USEL                 0x40
#define BIT_DLY_CHG_RST_EN              0x20
#define BIT_DLY_CHG_INTR_CLRB           0x10
#define MSK_OVR_LOW_BW_EN               0x0C
#define MSK_OVR_LOW_BW_VAL              0x03

#define REG_SYS_CTRL3               (PP_PAGE_B | 0x04)
#define BIT_RST_N_DCKFIFO               0x02
#define BIT_CLR_ONLY_IN_DVI             0x01

#define REG_HDMI_TX_BW_CTRL         (PP_PAGE_B | 0x11)

#define REG_HDMI_SWING_CTRL         (PP_PAGE_B | 0x12)
#define REG_HDMI_SWING_CTRL_TX1     (PP_PAGE_B | 0x15)

//------------------------------------------------------------------------------
// Registers in Page 15     (0xF8)
//------------------------------------------------------------------------------

#define REG_XPCLK_ENABLE            (PP_PAGE_AUDIO | 0x62)
#define BIT_XPCLK_ENABLE                BIT0

#define REG_XPCLK_CNT_0             (PP_PAGE_AUDIO | 0x66)
#define REG_XPCLK_CNT_1             (PP_PAGE_AUDIO | 0x67)


#endif  // __SI_9535REGS_PP_H__

