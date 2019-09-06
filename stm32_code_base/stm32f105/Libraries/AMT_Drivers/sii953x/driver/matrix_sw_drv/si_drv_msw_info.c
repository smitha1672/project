//**************************************************************************
//!file     si_drv_msw_info.c
//!brief    SiI9535 Switch Driver input video and Infoframe information API.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_drv_switch.h"
#include "si_drv_internal.h"
#include "si_drv_msw_internal.h"
#include "si_video_tables.h"


//-------------------------------------------------------------------------------------------------
//! @brief      Display the input information
//!
//! @param[in]  formatData - video format descriptor
//-------------------------------------------------------------------------------------------------
#if 0
static void VideoInputFormatDisplay (SiiInputformat_t *formatData)
{
    DEBUG_PRINT( MSG_STAT, "\n============================\n" );
    DEBUG_PRINT( MSG_STAT, "Video Input Information ( %d%s )\n",
        (int)formatData->vRes, (formatData->isInterlace) ? "i" : "p"
        );

    switch ( formatData->colorDepth )
    {
        case COLORDEPTH_10:
            DEBUG_PRINT( MSG_STAT, "Resolution:    %d x %d\n",
                    (int)((formatData->hRes << 2 ) / 5 ), (int)formatData->vRes );
            DEBUG_PRINT( MSG_STAT, "Color Depth:   10 bit \n" );
            break;
        case COLORDEPTH_12:
            DEBUG_PRINT( MSG_STAT, "Resolution:    %d x %d\n",
                    (int)((formatData->hRes << 1 ) / 3 ), (int)formatData->vRes );
            DEBUG_PRINT( MSG_STAT, "Color Depth:   12 bit \n" );
            break;
        case COLORDEPTH_16:
            DEBUG_PRINT( MSG_STAT, "Resolution:    %d x %d\n",
                    (int)(formatData->hRes >> 1 ), (int)formatData->vRes );
            DEBUG_PRINT( MSG_STAT, "Color Depth:   16 bit \n" );
            break;
        default:
            DEBUG_PRINT( MSG_STAT, "Resolution:    %d x %d\n", (int)formatData->hRes, (int)formatData->vRes );
            DEBUG_PRINT( MSG_STAT, "Color Depth:   8 bit \n" );
            break;

    }
    DEBUG_PRINT( MSG_STAT, "============================\n" );
}
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the resolution info for the specified CEA861 video mode
//! @param[in]  format
//! @param[out] pHoriz
//! @param[out] pVert
//! @param[out] pIsInterlaced
//!
//! @remark     Any output parameter set to NULL is skipped.
//-------------------------------------------------------------------------------------------------
void SiiDrvSwitchVideoFmtParse ( videoFormatId_t format, int_t *pHoriz, int_t *pVert, bool_t *pIsInterlaced )
{
    int_t   dummy;

    // Write locally if caller set a parameter to NULL
    pHoriz          = (pHoriz) ? pHoriz : &dummy;
    pVert           = (pVert) ? pVert : &dummy;
    pIsInterlaced   = (pIsInterlaced) ? pIsInterlaced : (bool_t *)&dummy;

    if ( format < vm_numVideoModes)
    {
        *pHoriz         = VideoModeTable[ format].Active.H;
        *pVert          = VideoModeTable[ format].Active.V;
        *pIsInterlaced  = VideoModeTable[ format].Interlaced;
    }
    else
    {
        *pHoriz         = 0;
        *pVert          = 0;
        *pIsInterlaced  = false;
    }
}
//-------------------------------------------------------------------------------------------------
//! @brief      Returns the resolution info for the requested port, if the port
//!             is connected to an active source.  For DVI modes, the alternate
//!             resolution detection hardware is used, for HDMI, the AVI VIC
//!             data is used.
//! @param[in]  port
//! @param[in]  isMainPipe
//! @param[in]  pTmdsClk     - video pixel clock (in 10kHz units)
//! @param[out] pHoriz
//! @param[out] pVert
//! @param[out] pIsInterlaced
//! @param[out] pIsHdmi
//!
//! @note       Any output parameter set to NULL is skipped.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvSwitchPortInfoGet ( uint8_t port, bool_t isMainPipe, int_t *pTmdsClk,
                                 int_t *pHoriz, int_t *pVert, bool_t *pIsInterlaced, bool_t *pIsHdmi )
{
    bool_t  interlaced, isHdmi, success;
    uint8_t portInfo[18];
    uint8_t vic, prep;
    int_t   horiz, vert, colorDepth, dummy, tmdsClock;

    if ( port >= SII_INPUT_PORT_COUNT )
    {
        pDrvSwitch->lastResultCode = SII_DRV_SWITCH_ERR_INVALID_PARAMETER;
        return( false );
    }
    pDrvSwitch->lastResultCode = SII_SUCCESS;

    // Write locally if caller set a parameter to NULL
    pTmdsClk        = (pTmdsClk) ? pTmdsClk : &dummy;
    pHoriz          = (pHoriz) ? pHoriz : &dummy;
    pVert           = (pVert) ? pVert : &dummy;
    pIsInterlaced   = (pIsInterlaced) ? pIsInterlaced : (bool_t *)&dummy;
    pIsHdmi         = (pIsHdmi) ? pIsHdmi : (bool_t *)&dummy;

    success     = false;
    horiz       = 0;
    vert        = 0;
    interlaced  = false;
    tmdsClock   = 0;

    // Get raw data for the port.
    SiiRegModify( REG_PORT_INFOFRAME_SELECT, VAL_RD_PORT_SEL_MASK, port );
    colorDepth  = SiiRegRead( isMainPipe ? REG_MP_COLOR_DEPTH : REG_RP_COLOR_DEPTH) & MSK_COLOR_DEPTH;

    isHdmi      = ((SiiRegRead( REG_PORT_ALTRES_MISC ) & BIT_IS_HDMI) != 0);

   

    if (isHdmi)
    {
        // For HDMI mode, we just extract the VIC information from the
        // AVI infoframe stored for the requested port.
        SiiRegReadBlock(REG_SP_AVI_HEADER, portInfo, 18);

        // Use the VIC value from the AVI to look up data in our video mode table
        if (((portInfo[REG_SP_AVI_VALID - REG_SP_AVI_HEADER] & BIT_SP_AVI_DATA_VALID) != 0))
        {
            vic = portInfo[REG_SP_AVI_PB4 - REG_SP_AVI_HEADER];
            prep = portInfo[REG_SP_AVI_PB5 - REG_SP_AVI_HEADER] + 1; // pixel repetition factor


            // Bug 31705 - Huawei Mobile is delivering 1080p/24Hz video but Melbourne is detecting as 480p resolution

            if ((vic > 0) && (vic <= LAST_KNOWN_CEA_VIC) && (prep > 1))
            {
                horiz       = VideoModeTable[ceaVicToVideoTableIndex[vic]].Active.H / prep;
                vert        = VideoModeTable[ceaVicToVideoTableIndex[vic]].Active.V;
                interlaced  = VideoModeTable[ceaVicToVideoTableIndex[vic]].Interlaced;
                tmdsClock   = VideoModeTable[ceaVicToVideoTableIndex[vic]].PixClk;

                // Convert pixel clock value to TMDS clock for deep color modes
                switch ( colorDepth )
                {
                    case COLORDEPTH_10:
                        tmdsClock = (tmdsClock * 5) / 4;
                        break;
                    case COLORDEPTH_12:
                        tmdsClock = (tmdsClock * 3) / 2;
                        break;
                    case COLORDEPTH_16:
                        tmdsClock *= 2;
                        break;
                }
                // protect against too high TMDS clock detection (> 300 MHz)
                success = (tmdsClock <= 30000);
            }
        }
    }

    // If we are in DVI mode or if AVI method has failed for HDMI mode
    // and the alternate resolution detection data is valid, return
    // that information to the caller.
    if ( !isHdmi || !success )
    {
        // Get the resolution info from the alternate resolution detection hardware.
        SiiRegReadBlock( isMainPipe ? REG_PORT_MP_ALTRES_DATA_0 : REG_PORT_SP_ALTRES_DATA_0, portInfo, 8 );
        if (((portInfo[7] & BIT_ALT_RES_DATA_VALID) != 0))
        {
            interlaced  = ((portInfo[7] & BIT_IS_INTERLACED) != 0);
            horiz       = portInfo[0] | ((int_t)portInfo[1] << 8);
            vert        = portInfo[4] | ((int_t)portInfo[5] << 8);
            success     = true;
        }
        tmdsClock = SiiDrvSwitchTmdsClockGet(isMainPipe);   // Read measured TMDS clock frequency



        // Horizontal size is counted using the TMDS clock, so if this is HDMI deep color
        // we need to adjust the values.
        if ( isHdmi )
        {
            switch ( colorDepth )
            {
                case COLORDEPTH_10:
                    horiz = (horiz * 4) / 5;
                    break;
                case COLORDEPTH_12:
                    horiz = (horiz * 2) / 3;
                    break;
                case COLORDEPTH_16:
                    horiz = horiz / 2;
                    break;
            }
        }
    }

    // If all attempts failed, return default values
    if ( !success )
    {
        horiz       = 0;
        vert        = 0;
        interlaced  = false;
        tmdsClock   = 0;
        DEBUG_PRINT( MSG_ERR, "VRES not detected in %s (port %d)\n", isMainPipe ? "MP" : "SP", port);
    }

    // Update caller data
    *pHoriz         = horiz;
    *pVert          = vert;
    *pIsInterlaced  = interlaced;
    *pTmdsClk       = tmdsClock;
    *pIsHdmi        = isHdmi;

    return( success );
}

//------------------------------------------------------------------------------
//! @brief  Returns the resolution info for the currently selected port.
//------------------------------------------------------------------------------
bool_t SiiDrvSwitchSelectedPortInfo ( bool_t isMainPipe, int_t *pHoriz, int_t *pVert, bool_t *pIsInterlaced, bool_t *pIsHdmi )
{
    int_t port, pclk;

    port = SiiDrvSwitchStateGet( isMainPipe ? SiiSwitch_SELECTED_PORT : SiiSwitch_SELECTED_PORT_SUB );
    return( SiiDrvSwitchPortInfoGet( port, isMainPipe, &pclk, pHoriz, pVert, pIsInterlaced, pIsHdmi ));
}

//------------------------------------------------------------------------------
// Function:    SwitchInfoframeProcessInterrupts
// Description: Processes InfoFrame change interrupts from the port attached to
//              the main and sub pipes.
//              Function checks only the main pipe interrupts so that real-time
//              notification of changes to the active port is available.
//              The current status of any of the background ports can be
//              read from the hardware as necessary.
// Returns:     true - New Info Frame
//              false- No Info Frame
//------------------------------------------------------------------------------

void SwitchInfoframeProcessInterrupts ( void )
{
    uint8_t intStatus;

    // Only check if the main pipe interrupts are enabled
    if ( SiiRegRead( REG_INT_ENABLE_P3 ) & MSK_P3_MP_IF_STATUS )
    {
        // Check main pipe infoframe status. If the status of an infoframe type
        // that we are interested in is set, go ahead and process it.
        intStatus = SiiRegRead( REG_INT_STATUS_P3 ) & MSK_P3_MP_IF_STATUS;
        SiiRegWrite( REG_INT_STATUS_P3, intStatus );        // Clear all status flags for MP.
        intStatus >>= 2;                                    // Main pipe bits are MSBs
        intStatus &= pDrvSwitch->mpInfoFrameMask;
        pDrvSwitch->mpInfoFrameInterruptStatus = intStatus;
        if ( intStatus )
        {
            pDrvSwitch->statusFlags |= SiiSWITCH_MP_INFOFRAME_INTR_VALID | SiiSWITCH_INT;
        }
    }
    // Only check if the sub pipe interrupts are enabled
    if ( SiiRegRead( REG_INT_ENABLE_P4 ) & MSK_P4_RP_IF_STATUS )
    {
        // Check sub pipe infoframe status. If the status of an infoframe type
        // that we are interested in is set, go ahead and process it.
        intStatus = SiiRegRead( REG_INT_STATUS_P4 ) & MSK_RP_IF_STATUS;
        SiiRegWrite( REG_INT_STATUS_P4, intStatus );        // Clear all status flags for RP.
        pDrvSwitch->spInfoFrameInterruptStatus = intStatus;
        if ( intStatus )
        {
            pDrvSwitch->statusFlags |= SiiSWITCH_SP_INFOFRAME_INTR_VALID | SiiSWITCH_INT;
        }
    }
}



//-------------------------------------------------------------------------------------------------
//! @brief      Return InfoFrame captured in an input port.
//!
//! @param[in]  isMainPipe  - pipe selection (true - main, false - sub)
//! @param[in]  ifType      - info frame type
//! @param[out] pInfoFrame  - pointer to the info frame data
//! @param[out] plen        - pointer to a length of info frame data in bytes
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvSwitchInfoFrameGet(bool_t isMainPipe, SiiSwitchInfoFrame_t ifType, uint8_t *pInfoFrame, uint8_t *pLen)
{
    uint8_t reg;
    uint8_t count;

    pDrvSwitch->lastResultCode = SII_SUCCESS;

    reg = SiiRegRead(REG_PORT_INFOFRAME_SELECT);

    // Select main or sub pipe to be source of the info frame data
    SiiRegModify( REG_PORT_INFOFRAME_SELECT, BIT_RD_PIPE_SEL | BIT_CS_PORT_PIPE_SEL | BIT_RD_CS_PIPE_SEL,
                  BIT_CS_PORT_PIPE_SEL | (isMainPipe ? 0 : BIT_RD_CS_PIPE_SEL));
    // Read requested IF data
    switch (ifType)
    {
        case IF_AVI:
            SiiRegReadBlock( REG_SP_AVI_HEADER, pInfoFrame, 18 );
            *pLen = pInfoFrame[2] + 3;
//            DEBUG_PRINT(MSG_DBG,"\n\n\r Audio Video(AVI) Info Frames : ");
//            for(count=0 ; count < 18 ; count++)
//            {
//            	DEBUG_PRINT(MSG_ALWAYS,"%x ",pInfoFrame[count]);
//            }
//            DEBUG_PRINT(MSG_DBG,"\n\n\r");
            break;
        case IF_AIF:
            *pLen = 31;
            SiiRegReadBlock( REG_SP_AIF_HEADER, pInfoFrame, 32 );
            *pLen = pInfoFrame[2] + 3;
            break;
        case IF_VSI:
            *pLen = 31;
            SiiRegReadBlock( REG_RP_VISF_CONTENT_HEADER, pInfoFrame, 32 );
            *pLen = pInfoFrame[2] + 3;
            DEBUG_PRINT(MSG_DBG,"\n\n\r Vendor Specific Info Frames : ");
            for(count=0 ; count < 32 ; count++)
            {
            	DEBUG_PRINT(MSG_ALWAYS,"%x ",pInfoFrame[count]);
            }
            DEBUG_PRINT(MSG_DBG,"\n\n\r");
         	break;
        default:
            pInfoFrame = 0;
            pDrvSwitch->lastResultCode = SII_DRV_SWITCH_ERR_INVALID_PARAMETER;
            return (false);
    }

    // Recover port selection settings
    SiiRegWrite(REG_PORT_INFOFRAME_SELECT, reg);

    return (true);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return AVI InfoFrame valid value captured in an input port.
//!
//! @param[in]  isMainPipe  - pipe selection (true - main, false - sub)
//-------------------------------------------------------------------------------------------------
uint8_t SiiDrvSwitchInfoFrameValidGet(bool_t isMainPipe)
{
	uint8_t reg;
	uint8_t valid = 0;

    pDrvSwitch->lastResultCode = SII_SUCCESS;

	reg = SiiRegRead(REG_PORT_INFOFRAME_SELECT);

	// Select main or sub pipe to be source of the info frame data
	SiiRegModify( REG_PORT_INFOFRAME_SELECT, BIT_RD_PIPE_SEL | BIT_CS_PORT_PIPE_SEL | BIT_RD_CS_PIPE_SEL,
	                  BIT_CS_PORT_PIPE_SEL | (isMainPipe ? 0 : BIT_RD_CS_PIPE_SEL));

	// Read requested AVI Valid bit
	 valid = SiiRegRead(REG_SP_AVI_VALID);

	// Recover port selection settings
	SiiRegWrite(REG_PORT_INFOFRAME_SELECT, reg);

	return (valid);
}
//-------------------------------------------------------------------------------------------------
//! @brief      Return InfoFrame captured in an input port.
//!
//! @param[in]  isMainPipe  - pipe selection (true - main, false - sub)
//! @param[in]  ifType      - info frame type
//! @param[out] pInfoFrame  - pointer to the info frame data
//! @param[out] plen        - pointer to a length of info frame data in bytes
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvSwitchPictAspectRatioGet(void)
{
    uint8_t reg;

    reg = SiiRegRead(REG_SP_AVI_PB2);
    DEBUG_PRINT(MSG_DBG,"\n\r SiiDrvSwitchPictAspectRatioGet %x  %x\n\r",reg,( (reg>>4) & 0x03 ));
    reg = ((reg >> 4) & 0x03);
    return(((reg==0x02)?true:0));
}
