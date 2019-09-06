//***************************************************************************
//!file     si_tx_config.h
//!brief    System-specific configuration information for TX
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SI_CONFIG_TX_H__
#define __SI_CONFIG_TX_H__

//------------------------------------------------------------------------------
//  TX Component SiI953x-specific configuration constants
//------------------------------------------------------------------------------

#define SII_NUM_TX                          1       // Number of TX instances

#define SI_TX_TMDS_TOGGLE_FOR_HDCP_RESET    DISABLE
#define SI_TX_ALLOW_YCBCR422_12BIT          ENABLE

#define SI_TX_COMP_REVISION                 0x01    //!< Revision of TX component firmware
#define SI_TX_INIT_TO_STANDBY               ENABLE  //!< Keep Tx in Standby mode after initialization

#define SI_TX_VIDEO_CONVERSION              DISABLE  //!< Enable video conversion functions
#define SI_TX_AUDIO_I2S_IN_BUS              ENABLE  //!< Enable I2S audio configuration functions

#define SI_TX_COMP_UNIT_TESTS               DISABLE //!< Enable internal unit test functions

#endif  //__SI_CONFIG_TX_H__
