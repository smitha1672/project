//************************************************************************************************
//! @file   si_hdcp_ddc.h
//! @brief  HDCP DDC definitions
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#ifndef SI_HDCP_DDC_H
#define SI_HDCP_DDC_H

#define REG_HDCP_DDC__BKSV                  0x00 // Bksv
#define LEN_HDCP_DDC__BKSV                  5

#define REG_HDCP_DDC__RI                    0x08 // Ri
#define LEN_HDCP_DDC__RI                    2

#define REG_HDCP_DDC__AKSV                  0x10 // Aksv
#define LEN_HDCP_DDC__AKSV                  5

#define REG_HDCP_DDC__AN                    0x18 // An
#define LEN_HDCP_DDC__AN                    8

#define REG_HDCP_DDC__BCAPS                 0x40 // B CAPS
#define BIT_HDCP_DDC__BCAPS__HDMI_C         0x80 // reserved for HDMI- 0 for DVI, ? for HDMI
#define BIT_HDCP_DDC__BCAPS__RPTR           0x40 // 1-repeater, 0- end point
#define BIT_HDCP_DDC__BCAPS__FIFO_RDY       0x20 // 1-FIFO ready, 0- FIFO not ready
#define BIT_HDCP_DDC__BCAPS__FAST           0x10 // 1-400kbps, 0-100kbps
#define BIT_HDCP_DDC__BCAPS__1_1_FEATURES   0x02 // HDCP 1.1 features
#define BIT_HDCP_DDC__BCAPS__FAST_RE_AUTH   0x01 // Fast Re-authentication

#define REG_HDCP_DDC__BSTATUS1              0x41 // B STATUS 1
#define BIT_HDCP_DDC__BSTATUS1__DEV_EXC     0x80 // device count exceeded
#define MSK_HDCP_DDC__BSTATUS1__DEV_COUNT   0x7F // device count

#define REG_HDCP_DDC__BSTATUS2              0x42 // B STATUS 2
#define BIT_HDCP_DDC__BSTATUS2__HDMI_MODE   0x10 // 1-device is in HDMI mode, 0-device is in DVI mode
#define BIT_HDCP_DDC__BSTATUS2__CAS_EXC     0x08 // cascade depth exceeded
#define MSK_HDCP_DDC__BSTATUS2__DEV_DEPTH   0x07 // device depth

#define REG_HDCP_DDC__DEVICE                0xFB // device ID
#define MSK_HDCP_DDC__DEVICE__DEV_ID        0xF8 // ID
#define MSK_HDCP_DDC__DEVICE__DEV_REV       0x07 // revision

#define REG_HDCP_DDC__V_H                   0x20 // V_H0...V_H4
#define LEN_HDCP_DDC__V_H                   20

#define REG_HDCP_DDC__KSV_FIFO              0x43 // KSV FIFO


#endif // SI_HDCP_DDC_H

