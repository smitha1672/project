//***************************************************************************
//!file     si_drv_osd_ram.c
//!brief    Silicon Image OSD Driver OSD RAM read/write.
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
#include "si_drv_tpg.h"
#include "si_regs_tpg953x.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Wait for OSD write operation to complete
//! @return     true - completed successfully, false: timed out @ 100ms
//-------------------------------------------------------------------------------------------------
bool_t OsdWaitForWriteIntDone(void)
{
    clock_time_t start = SiiPlatformTimerSysTicksGet();
    while (SkTimeDiffMs(start, SiiPlatformTimerSysTicksGet()) < 200)
    {
        if ( SiiRegRead( REG_INT_STATUS_OSD) & BIT_RAM_I2C_WR_DONE)
        {
            SiiRegWrite( REG_INT_STATUS_OSD, BIT_RAM_I2C_WR_DONE ); // Clear the flag
            return true;
        }
    }
    DEBUG_PRINT( MSG_DBG, "FP1519: -------------------------->>> OsdWaitForWriteIntDone: TIME OUT\n" );
    return false;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Determines if it is OK to read/write OSDRAM, and if not, disables OSD to allow
//!             read/write to occur.
//-------------------------------------------------------------------------------------------------
static void OsdRamEnableReadWrite ( void )
{
    if (!( SiiRegRead( REG_OSD_WIN_CTRL )   & BIT_OSD_ENABLE )    ||                        // OSD is disabled (OSDRAM is using XCLK)
        ( SiiRegRead( REG_MP_GCP_STATUS )   & BIT_MP_RES_STABLE ) ||                        // TMDS Clock source is stable
       (( SiiRegRead( REG_VPG_CTRL_3 )      & MSK_VPG_CLK_SEL) == (SI_TPG_CLK_XCLK << 1)) ) // TPG Clock source is present
    {
        return;
    }
    DEBUG_PRINT( MSG_DBG, "*");
    DrvOsdDisableInternal();
}


//-------------------------------------------------------------------------------------------------
//! @brief      Write passed byte data to the OSD RAM via I2C
//! @param[in]  osdWordAddr     - Starting OSD WORD address within OSD RAM
//! @param[in]  osdWordCount    - Length in RAM words (6 bytes/word).
//! @param[in]  pByteData       - pointer to byte array to be written to OSD RAM.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdRamI2cWrite ( int osdWordAddr,  int osdWordCount, uint8_t *pByteData )
{
	int         i, count;
	bool_t      success = false;
	uint8_t     *pData;
	
    for ( ;; )
    {
        pData = pByteData;
        count = osdWordCount;

        OsdRamEnableReadWrite();    // Force correct conditions to allow read/write

        SiiRegModify(REG_OSD_ACCESS_CTRL, ~REG_BIT_INDEX_PATCH_EN, CLEAR_BITS); // Clear all but patch index enable

        // Set up write address and word count
        SiiRegWrite(REG_OSD_RAM_ADDR_L, osdWordAddr & 0xFF);
        SiiRegWrite(REG_OSD_RAM_ADDR_H, (osdWordAddr >> 0x08) & 0x07);
        SiiRegWrite(REG_OSD_RAM_WORD_COUNT_L, osdWordCount & 0xFF);
        SiiRegWrite(REG_OSD_RAM_WORD_COUNT_H, (osdWordCount >> 0x08) & 0x0F);

        //Strobe to start OSD RAM Write
        SiiRegModify(REG_OSD_ACCESS_CTRL, REG_BIT_I2C_TO_RAM_EN | REG_BIT_I2C_RAM_WRITE, SET_BITS);

        //Write data to OSD RAM
        while ( count--)
        {
            for ( i = 0; i < OSD_RAM_BURST_SIZE; i++)
            {
                SiiRegWrite( REG_OSD_RAM_WR_DATA, *pData++);
            }
        }

        if ( SiiRegRead( REG_INT_STATUS_OSD) & BIT_RAM_I2C_WR_DONE)
        {
            success = true;
            SiiRegWrite( REG_INT_STATUS_OSD, BIT_RAM_I2C_WR_DONE ); // Clear the flag
            break;
        }
        else
        {
            // Are we in OSD_EN state or internal clock?
            if ( SiiRegRead( REG_OSD_WIN_CTRL ) & BIT_OSD_ENABLE )
            {
                // Force switch to XCLK.  The write will be tried again.
                DrvOsdDisableInternal();
            }
            else
            {
                // We are already using internal clock, so break off the attempt.
                break;
            }
        }
    }

    pDrvOsd->lastResultCode = success ? SII_OSDDRV_SUCCESS : SII_OSDDRV_OSDRAM_ACCESS_ERR;
	return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Read OSD RAM data into passed byte array via I2C
//! @param[in]  osdWordAddr     - Starting OSD WORD address within OSD RAM
//! @param[in]  osdWordCount    - Length in RAM words (6 bytes/word).
//! @param[in]  pByteData       - pointer to byte array to be written from OSD RAM.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdRamI2cRead ( int osdWordAddr,  int osdWordCount, uint8_t *pByteData )
{
    int             i, count;
    uint8_t         *pData;
	int             outIndex;
	bool_t          success = false;

    for ( ;; )
    {
        pData = pByteData;
        count = osdWordCount;

        OsdRamEnableReadWrite();    // Force correct conditions to allow read/write

        // Set up read address and word count
        SiiRegWrite( REG_OSD_RAM_ADDR_L, osdWordAddr & 0xFF);
        SiiRegWrite( REG_OSD_RAM_ADDR_H, (osdWordAddr >> 0x08) & 0x0F);
        SiiRegWrite( REG_OSD_RAM_WORD_COUNT_L, osdWordCount & 0xFF);
        SiiRegWrite( REG_OSD_RAM_WORD_COUNT_H, (osdWordCount >> 0x08) & 0x0F);

        // Strobe to start read.
        SiiRegModify( REG_OSD_ACCESS_CTRL, REG_BIT_I2C_RAM_RD, SET_BITS);

        // Read requested OSDRAM data.
        outIndex = 0;
        while ( count--  )
        {
            //Store the data to the output array
            for ( i=0; i < OSD_RAM_BURST_SIZE; i ++)
            {
                pData[ outIndex++] =  SiiRegRead( REG_OSD_RAM_RD_DATA0 + i );
            }
        }

        // Report done flag state
        success = ((SiiRegRead(REG_INT_STATUS_OSD) & REG_BIT_RAM_I2C_RD_DONE) != 0);
        if ( success )
        {
            SiiRegWrite( REG_INT_STATUS_OSD, REG_BIT_RAM_I2C_RD_DONE ); // Clear the flag
            break;
        }
        else
        {
            // Are we in OSD_EN state or internal clock?
            if ( SiiRegRead( REG_OSD_WIN_CTRL ) & BIT_OSD_ENABLE )
            {
                // Force switch to XCLK.  The read will be tried again.
                DrvOsdDisableInternal();

                // Attempt to flush any remaining read data
                count = osdWordCount;
                while ((count != 0 ) && ((SiiRegRead(REG_INT_STATUS_OSD) & REG_BIT_RAM_I2C_RD_DONE) == 0))
                {
                    count--;
                    // send the data to a bit bucket
                    for ( i = 0; i < OSD_RAM_BURST_SIZE ; i++ )
                    {
                        SiiRegRead( REG_OSD_RAM_RD_DATA0 + i );
                    }
                }
                if ( ((SiiRegRead(REG_INT_STATUS_OSD) & REG_BIT_RAM_I2C_RD_DONE) != 0) )
                {
                    SiiRegWrite( REG_INT_STATUS_OSD, REG_BIT_RAM_I2C_RD_DONE ); // Clear the flag
                }
            }
            else
            {
                // We are already using internal clock, so break off the attempt.
                break;
            }
        }
    }

    pDrvOsd->lastResultCode = success ? SII_OSDDRV_SUCCESS : SII_OSDDRV_OSDRAM_ACCESS_ERR;
	return( success );
}

