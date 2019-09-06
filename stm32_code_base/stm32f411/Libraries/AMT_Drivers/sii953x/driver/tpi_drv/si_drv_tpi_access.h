//************************************************************************************************
//! @file   si_drv_tpi_access.h
//! @brief  TPI and Legacy registers access
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

// TPI register map

#ifndef SI_TX_TPI_ACCESS_H
#define SI_TX_TPI_ACCESS_H

#include "si_drv_internal.h"    // HAL, SI part level
#include "si_regs_tpi953x.h"
//#include "si_cra_cfg.h"



//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------
                                            //! TX Access instance record
typedef struct txAccessInstanceRecord
{
    bool_t   isTpiMode;                     //!< TPI or Legacy control mode
    uint8_t  previouslyAccessedLegacyPage;  //!< Page previously set for indirect access to legacy registers
    uint16_t previouslyAccessedLegacyOffset;//!< Offset previously set for indirect access to legacy registers

} txAccessInstanceRecord_t;


//-------------------------------------------------------------------------------------------------
//  TPI Access API functions
//-------------------------------------------------------------------------------------------------

#if SII_NUM_TX > 1
void    SiiDrvTpiAccessInstanceSet(uint8_t device_idx);
#endif


void    SiiDrvTpiAccessInit(void);
void    SiiDrvTpiAccessEnable(bool_t isTpi);
void    SiiDrvTpiIndirectWrite(uint16_t regAddr, uint8_t data);
uint8_t SiiDrvTpiIndirectRead(uint16_t regAddr);
void    SiiDrvTpiIndirectBlockWrite(uint16_t regAddr, uint8_t *pData, uint8_t length);
void    SiiDrvTpiIndirectBlockRead(uint16_t regAddr, uint8_t *pData, uint8_t length);

#endif // SI_TX_TPI_ACCESS_H

