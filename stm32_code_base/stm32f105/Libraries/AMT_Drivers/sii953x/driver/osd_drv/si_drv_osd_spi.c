//***************************************************************************
//!file     si_drv_osd_spi.c
//!brief    Silicon Image OSD Driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_drv_osd_internal.h"
#include "si_drv_internal.h"
#include "si_regs_osd.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Load OSD RAM from SPI directly.
//! @param[in]  spiByteAddr     - Starting byte address within SPI flash memory
//! @param[in]  osdWordAddr     - Starting OSD WORD address within OSD RAM
//! @param[in]  spiByteLength   - Length in bytes of SPI data.  Padded to a multiple of OSD
//!                               RAM words (6 bytes/word).
//! @param[in]  showOptions
//! @return     true - success, false - fail
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdSpiOsdRamLoad ( uint32_t spiByteAddr,  uint32_t osdWordAddr, uint16_t spiByteLength)
{	
    SiiTimer_t  timerInfo;
	uint16_t    osdWordLength;
	int         mod;
	bool_t      success;

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

//    DEBUG_PRINT(MSG_DBG, "OSD RAM LOAD: SPI: %04X, OSD RAM start: %04X, SPI byte length: %d status = %02X\n", spiByteAddr, osdWordAddr, spiByteLength, mod );
    SiiOsTimerSet( &timerInfo, 0 );

	// Load SPI start address
	SiiRegWrite(REG_SPI_ADDR_LO, spiByteAddr & 0xFF);
	SiiRegWrite(REG_SPI_ADDR_MID, (spiByteAddr >> 8) & 0xFF);
	SiiRegWrite(REG_SPI_ADDR_HI, (spiByteAddr >> 16) & 0xFF);

	// Determine padding for non-multiple of OSD_RAM_BURST_SIZE
	mod = spiByteLength % OSD_RAM_BURST_SIZE;
	if (mod)
	{
	    spiByteLength += (OSD_RAM_BURST_SIZE - mod);
	}
	osdWordLength = (spiByteLength / OSD_RAM_BURST_SIZE);

	// Load SPI length - 1
	spiByteLength -= 1;
	SiiRegWrite(REG_SPI_LENGTH_LO, (spiByteLength) & 0xFF);
	SiiRegWrite(REG_SPI_LENGTH_HI, (spiByteLength >> 0x08) & 0x3F);

	// Load OSD start address and length
	SiiRegWrite(REG_OSD_RAM_ADDR_L, osdWordAddr & 0xFF);
	SiiRegWrite(REG_OSD_RAM_ADDR_H, (osdWordAddr >> 0x08) & 0x07);
	SiiRegWrite(REG_OSD_RAM_WORD_COUNT_L, osdWordLength & 0xFF);
	SiiRegWrite(REG_OSD_RAM_WORD_COUNT_H, (osdWordLength >> 0x08) & 0x0F);

	// Start OSD read from SPI operation
	SiiRegModify(REG_OSD_ACCESS_CTRL, REG_BIT_SPI_BYPASS_CHECKSUM | REG_BIT_SPI_RELOAD, SET_BITS);

	// Start SPI read operation
	SiiRegModify(REG_SPI_OP_CMD, BIT_SPI_ADDR_SEND | BIT_SPI_READ, SET_BITS);
	SiiRegWrite(REG_SPI_CMD, VAL_SPI_READ);

	success = OsdWaitForWriteIntDone();
//    DEBUG_PRINT( MSG_DBG, "SiiDrvOsdSpiOsdRamLoad: %d ms success = %d\n", SiiOsTimerElapsed( timerInfo ), success );
	return( success );
}

