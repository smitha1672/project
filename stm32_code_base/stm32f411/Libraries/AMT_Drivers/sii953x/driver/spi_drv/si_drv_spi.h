//***************************************************************************
//!file     si_drv_spi.h
//!brief    Silicon Image SPI driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_SPI_DRV_H__
#define __SI_SPI_DRV_H__
#include "si_common.h"


enum
{
	SPI_PAGE_WRITE,
	SPI_PAGE_PROGRAM,
	SPI_PAGE_ERASE,
	SPI_SECTOR_ERASE,
};

bool_t  SiiDrvSpiErase( uint8_t ctrl, uint32_t addr );
bool_t  SiiDrvSpiWrite( uint32_t addr, uint8_t *pBuf, uint16_t size );
bool_t  SiiDrvSpiRead( uint32_t spiAddr, int length, uint8_t *pDestData );

void    SiiDrvSpiInitialize(void);


#endif // __SI_SPI_DRV_H__
