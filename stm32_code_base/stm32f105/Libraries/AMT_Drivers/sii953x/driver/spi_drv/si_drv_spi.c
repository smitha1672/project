//***************************************************************************
//!file     si_drv_spi.c
//!brief    Silicon Image SPI Driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_drv_gpio_internal.h"
#include "si_drv_spi_internal.h"
#include "si_drv_internal.h"
#if INC_OSD
#include "si_drv_osd_internal.h"
#include "si_regs_osd.h"
#endif
#define SPI_MAX_PAGE_SIZE   256

//------------------------------------------------------------------------------
// Function: SpiReadStatus
// Description:  
// Parameters: 
// Return: none
//------------------------------------------------------------------------------
static uint8_t SpiReadStatus ()
{
	SiiRegWrite(REG_SPI_OP_CMD, BIT_SPI_READ);
	SiiRegWrite(REG_SPI_CMD, VAL_SPI_RDSR);
	return (SiiRegRead(REG_FL_STATUS_ID0));
}

//------------------------------------------------------------------------------
// Function: SpiWREN
// Description:  Check for WREN to go hi. if it does not go hi for speicific time then 
//				force to exit.
// Parameters: 
// Return: none
//------------------------------------------------------------------------------
static uint8_t SpiWREN ()
{
	uint8_t temp;
	clock_time_t start = SiiPlatformTimerSysTicksGet();
	uint8_t cnt=5;
	
	SiiRegWrite(REG_SPI_OP_CMD, BIT_SPI_NONE); //Command only
	SiiRegWrite(REG_SPI_CMD, VAL_SPI_WREN); //Write Enable
	while (cnt)
	{
		temp = SpiReadStatus();
		if (temp & BIT_RDSR_WEL)
			return false;
		//Wait for 5 system ticks
		while ((SiiPlatformTimerSysTicksGet() - start ) < 5);
		start = SiiPlatformTimerSysTicksGet();
		cnt--;
	}
	return true;
}

//------------------------------------------------------------------------------
// Function: SpiWaitWIP
// Description:  Wait for WIP to go low and prevent lock up issue.
// Parameters:
// Return: return true if WIP has never been set. false has been set
//------------------------------------------------------------------------------
static bool_t SpiWaitWIP ()
{
	bool_t results = true;
	while (1)
	{
		//waiting until for finishing writing the data
		if (SpiReadStatus() & BIT_RDSR_WIP)
		{
			results= false;
		}
		else
		{
			break;
		}
	}
	return results;
}

//------------------------------------------------------------------------------
// Function: SiiDrvSpiErase   
// Description:  
// Parameters: 
// Return: none
//------------------------------------------------------------------------------
bool_t SiiDrvSpiErase (uint8_t ctrl, uint32_t addr)
{
	uint32_t addrSector;
	uint8_t i;
	
	if (SpiWREN())
		return false;
	
	//SPI start address
	
	switch (ctrl)
	{
		case SPI_PAGE_ERASE:
			SiiRegWrite(REG_SPI_OP_CMD, BIT_SPI_ADDR_SEND);	//Issue address only
			SiiRegWrite(REG_SPI_CMD, VAL_SPI_PE); 			//Page Erase
			break;
		case SPI_SECTOR_ERASE:
			for (i=0; i < 4; i++)
			{
				addrSector = i * 0x10000;
				SiiRegWrite(REG_SPI_ADDR_LO, addrSector & 0xFF);
				SiiRegWrite(REG_SPI_ADDR_MID, (addrSector >> 8) & 0xFF);
				SiiRegWrite(REG_SPI_ADDR_HI, (addrSector >> 16) & 0xFF);
				SiiRegWrite(REG_SPI_OP_CMD, BIT_SPI_ADDR_SEND); //Issue address only
				SiiRegWrite(REG_SPI_CMD, VAL_SPI_SE); 			//Sector Erase
				SpiWaitWIP();
			}
			break;
	}

	DEBUG_PRINT(MSG_DBG, "\n Write Done");

	return true;
}

//------------------------------------------------------------------------------
// Function: SiiDrvSpiWrite
// Description:  
// Parameters: 
// Return: none
//------------------------------------------------------------------------------
bool_t SiiDrvSpiWrite (uint32_t addr, uint8_t *pBuf, uint16_t size)
{
	uint16_t i;
	uint16_t  pageSize;
	uint16_t  cnt;

	//size = SiiRegRead(REG_SCRATCH0);

	pageSize = size / SPI_MAX_PAGE_SIZE;
	DEBUG_PRINT(MSG_DBG, "\nAddr: 0x%x, size: %d (0x%x)", addr, size, size); 
	DEBUG_PRINT(MSG_DBG, "\npageSize : %d (0x%x)", addr, pageSize , pageSize ); 

	for(;;)
	{
	
		if (SpiWREN())
		{
			return false;
		}

		//Wrte send Address, send Data
		SiiRegWrite(REG_SPI_OP_CMD, BIT_SPI_ADDR_SEND | BIT_SPI_WRITE);	

		cnt = pageSize ? SPI_MAX_PAGE_SIZE : (size & 0xFF);

		//# of bytes to write
		SiiRegWrite(REG_SPI_WRITE_SIZE, cnt - 1);

		DEBUG_PRINT(MSG_DBG, "\nAddr: 0x%x, size: %d (0x%x)", addr, cnt, cnt); 

		//SPI start address
		SiiRegWrite(REG_SPI_ADDR_LO, addr & 0xFF);
		SiiRegWrite(REG_SPI_ADDR_MID, (addr >> 8) & 0xFF);
		SiiRegWrite(REG_SPI_ADDR_HI, (addr >> 16) & 0xFF);

		//Page Write
		SiiRegWrite(REG_SPI_CMD, VAL_SPI_PW);

		addr += SPI_MAX_PAGE_SIZE;
		
		//Write Bytes.
		for (i=0; i < cnt; i++)
		{
			SiiRegWrite(REG_SPI_TX_DATA, *pBuf++);	
		}

		if (SpiWaitWIP())
		{
			return false;
		}

		if (!pageSize)
		{
			break;
		}
		pageSize--;
		
	}

	DEBUG_PRINT(MSG_DBG, "\n Write is done.B");
	
	return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Read from SPI flash into the specified byte buffer in local memory
//! @param[in]  spiAddr     - Address with SPI flash memory of target data
//! @param[in]  length      - Number of bytes to read from flash
//! @param[in]  pDestData   - Pointer to target buffer.
//! @return                 - true if successful, false if a failure occurred.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvSpiRead(  uint32_t spiAddr, int length, uint8_t *pDestData )
{
    int         i;
    uint8_t     osdStatus;
    uint32_t    addr, oldAddr;

//    pDrvSpi->lastResultCode = SII_OSD_ERR_NOT_ENABLED;

    SiiRegWrite( REG_INT_STATUS_OSD, REG_BIT_RAM_SPI_TRANSACTION_DONE );

    // Set up for reading one byte at a time
    SiiRegWrite( REG_SPI_OP_CMD, BIT_SPI_ADDR_SEND | BIT_SPI_READ);
    SiiRegWrite( REG_SPI_CMD, VAL_SPI_READ);
    SiiRegWrite( REG_SPI_LENGTH_LO, 0x00);  // Read 1 byte.
    SiiRegWrite( REG_SPI_LENGTH_HI, 0x00);

    addr = spiAddr;
    oldAddr = spiAddr;
    SiiRegWrite( REG_SPI_ADDR_LO, (uint8_t)addr);
    SiiRegWrite( REG_SPI_ADDR_MID, (uint8_t)(addr >> 8));
    SiiRegWrite( REG_SPI_ADDR_HI, (uint8_t)(addr >> 16));
    for (i = 0; i < length; i++)
    {
        SiiRegWrite( REG_SPI_ADDR_LO, (uint8_t)addr);
        if (( addr & 0x0100 ) != (oldAddr & 0x0100))
        {
            SiiRegWrite( REG_SPI_ADDR_MID, (uint8_t)(addr >> 8));
        }
        if (( addr & 0x010000 ) != (oldAddr & 0x010000))
        {
            SiiRegWrite( REG_SPI_ADDR_HI, (uint8_t)(addr >> 16));
        }
        SiiRegWrite( REG_SPI_CMD, VAL_SPI_READ );
        oldAddr = addr;
        addr++;

        pDestData[i] = SiiRegRead( REG_SPI_RX_DATA );

        osdStatus = SiiRegRead(REG_INT_STATUS_OSD);
    }

    // Wait for transaction to complete (up to 100ms)
    clock_time_t start = SiiPlatformTimerSysTicksGet();
    while (SkTimeDiffMs(start, SiiPlatformTimerSysTicksGet()) < 100)
    {
        if (SiiRegRead(REG_INT_STATUS_OSD) & REG_BIT_RAM_SPI_TRANSACTION_DONE)
        {
            SiiRegWrite( REG_INT_STATUS_OSD, REG_BIT_RAM_SPI_TRANSACTION_DONE );
            return( true );
        }
    }

    return( false );
}

//-------------------------------------------------------------------------------------------------
//! @brief      SPI driver initialization
//-------------------------------------------------------------------------------------------------
void SiiDrvSpiInitialize(void)
{
    //Setup GPIO for SPI
    SiiDrvGpioPinType(SII_GPIO_PIN_8, SII_GPIO_ALT_SPI_SS);
    SiiDrvGpioPinType(SII_GPIO_PIN_9, SII_GPIO_ALT_SPI_SCLK);
    SiiDrvGpioPinType(SII_GPIO_PIN_10, SII_GPIO_ALT_SPI_SDO);
    SiiDrvGpioPinType(SII_GPIO_PIN_11, SII_GPIO_ALT_SPI_SDI);
    SiiRegWrite(REG_SPI_CTRL, 0x02);    //data from flash is driving on the falling edge of external SPI_CLK 1:
                                        //data from flash is driving on the rising edge of external SPI_CLK


    SiiRegBitsSet( REG_GPIO_DIR, BIT4, false );
    SiiRegBitsSet( REG_GPIO_ALTEN_DIR, BIT1, true );


}
