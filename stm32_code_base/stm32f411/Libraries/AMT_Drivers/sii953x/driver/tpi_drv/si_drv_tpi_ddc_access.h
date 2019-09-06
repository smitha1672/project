//************************************************************************************************
//! @file   si_drv_tpi_ddc_access.h
//! @brief  Access to downstream device over DDC line
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#ifndef SI_TX_TPI_DDC_ACCESS_H
#define SI_TX_TPI_DDC_ACCESS_H

#include "si_common.h"

//-------------------------------------------------------------------------------------------------
//  Module types definitions
//-------------------------------------------------------------------------------------------------
                                //! DDC bus access error codes
typedef enum ddcComErr
{
    SI_TX_DDC_ERROR_CODE_NO_ERROR    = 0x00, //!< Success
    SI_TX_DDC_ERROR_CODE_TIMEOUT     = 0x01, //!< DDC bus is not granted within timeout
    SI_TX_DDC_ERROR_CODE_NO_ACK      = 0x02, //!< No ACK from DDC device
    SI_TX_DDC_ERROR_CODE_BUSY        = 0x03, //!< DDC bus is busy
    SI_TX_DDC_ERROR_CODE_TX_HW       = 0x04,
    SI_TX_DDC_ERROR_CODE_LIM_EXCEED  = 0x05

} ddcComErr_t;


//-------------------------------------------------------------------------------------------------
//  TPI DDC Access API functions
//-------------------------------------------------------------------------------------------------

void        SiiDrvTpiDdcInit(void);
void        SiiDrvTpiDdcMasterReset(void);
ddcComErr_t SiiDrvTpiDdcGetLastError(void);
void        SiiDrvTpiDdcErrorsPrint(void);

ddcComErr_t SiiDrvTpiDdcReadBlock(uint8_t segmentIndex,  uint8_t regAddr, uint8_t *pBuf, uint16_t length);
ddcComErr_t SiiDrvTpiDdcWriteBlock(uint8_t regAddr, const uint8_t *pBuf, uint16_t length);


#endif // SI_TX_TPI_DDC_ACCESS_H
