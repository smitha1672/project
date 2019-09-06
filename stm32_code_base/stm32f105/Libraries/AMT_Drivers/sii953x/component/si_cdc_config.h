//***************************************************************************
//!file     si_cdc_config.h
//!brief    System-specific configuration information for CDC
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SI_CONFIG_CDC_H__
#define __SI_CONFIG_CDC_H__

//------------------------------------------------------------------------------
//  CDC Component SiI953x-specific
//------------------------------------------------------------------------------

#define SII_NUM_CDC                 1

typedef enum _SiiCdcInstanceTypes_t
{
    CDC_INSTANCE_RX     = 0,
} SiiCdcInstanceTypes_t;


#endif  //__SI_CONFIG_CDC_H__
