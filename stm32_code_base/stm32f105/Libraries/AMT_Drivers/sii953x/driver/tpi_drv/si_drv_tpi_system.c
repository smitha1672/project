//************************************************************************************************
//! @file   si_drv_tpi_system.c
//! @brief  Low level TPI operations such as interrupt and power controls (SiI9535 version)
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#include "si_common.h"
#include "si_tx_component_internal.h"
#include "si_drv_tpi_access.h"
#include "si_drv_tpi_ddc_access.h"
#include "si_hdcp_ddc.h"
#include "si_drv_tpi_system.h"

#include "si_drv_tpi_access.h"
#include "PowerHandler.h"
extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;

//! Mask selecting events that trigger an interrupt signal
//! Byte 0
#define TX_INTERRUPT_MASK0 \
    ( BIT_TPI__INT__HPD | BIT_TPI__INT__RX_SENSE | BIT_TPI__INT__HDCP_CHNG )

//! Byte 1
#define TX_INTERRUPT_MASK1 \
    ( BIT_TPI__INT__P_STABLE | BIT_TPI__INT__KSV_READY )


//! HDCP control flags
#define HDCP_CTRL_MODE ( 0 \
    | BIT_TPI__HDCP_CTRL__PROTECTION \
    | BIT_TPI__HDCP_CTRL__SHORT_RI \
    | BIT_TPI__HDCP_CTRL__DOUBLE_RI_CHECK \
    | BIT_TPI__HDCP_CTRL__INTERMEDIATE_RI_CHECK \
    | BIT_TPI__HDCP_CTRL__KSV_FORWARDING \
    )


#define PART2_DONE (BIT_TPI__HDCP_QUERY__EXTENDED_PROTECT | BIT_TPI__HDCP_QUERY__LOCAL_PROTECT)

// Control over Override bits in REG_TPI__AUDIO_PKT_SRC_CFG by TX (ENABLE) or Application (DISABLE)
// Application should be the owner of these bits as it has complete knowledge of the passthrough
// pipe status and mute/unmute requests possibly coming from RX (in the repeater application).
#define SI_TX_OVRD_TPI_CONTROL   DISABLE


//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------

txTpiInstanceRecord_t txTpiInstRec[SII_NUM_TX];
txTpiInstanceRecord_t *pTpi = &txTpiInstRec[0];

// SWWA_FP1554
#if INC_OSD
#include "si_drv_osd_internal.h"
#endif
// SWWA_FP1554

//-------------------------------------------------------------------------------------------------
//  Local service functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//! @brief      Switches either Embedded Sync control register set or DE generation
//!             control set into current context.
//!
//! @param[in]  isEnabled - true enables Embedded Sync control set,
//!                         false enables DE generation set.
//-------------------------------------------------------------------------------------------------

static void EmbeddedSyncControlPageEnable(bool_t isEnabled)
{

    SiiRegModify(REG_TPI__SYNC_GEN_CTRL, BIT_TPI__SYNC_GEN_CTRL__SYNC_EXTRACT_LAYOUT,
                            isEnabled ? SET_BITS : CLEAR_BITS);
}

//------------------------------------------------------------------------------------------------
//! @brief      audio input enable
//
//---------------------------------------------------------------------------------------------------
void SiiDrvTpiAudioInEnable(void)
{
	SiiDrvTpiIndirectWrite(REG_TX__AUDIO_IN_CTRL, 0x03);
}

//------------------------------------------------------------------------------------------------
//! @brief      Source Termination Enable
//
//---------------------------------------------------------------------------------------------------
void SiiDrvTpiSourceTerminationEnable(bool_t isEnabled)
{
    uint8_t regVal;

    regVal = SiiDrvTpiIndirectRead(REG_TX__TMDS_CTRL2);
    regVal = isEnabled ? (regVal | BIT_TX__TMDS_CTRL2__SRC_TERM_EN) : (regVal & ~BIT_TX__TMDS_CTRL2__SRC_TERM_EN);
    SiiDrvTpiIndirectWrite(REG_TX__TMDS_CTRL2, regVal);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Convert Info Frame ID into TPI selector code for the info frame type.
//!
//! @param[in]  ifType - info frame type ID.
//-------------------------------------------------------------------------------------------------
static uint8_t InfoFrameSelectCodeGet(infoFrameType_t ifType)
{
    uint8_t selector;

    switch(ifType)
    {
        default:
        case SI_INFO_FRAME_AVI:
            selector = VAL_TPI__IF_SELECT__AVI;
            break;
        case SI_INFO_FRAME_AUDIO:
            selector = VAL_TPI__IF_SELECT__AUDIO;
            break;
        case SI_INFO_FRAME_SPD:
            selector = VAL_TPI__IF_SELECT__SPD;
            break;
        case SI_INFO_FRAME_MPEG: // or HDMI Vendor Specific data
            selector = VAL_TPI__IF_SELECT__MPEG;
            break;
        case SI_INFO_FRAME_ISRC1:
            selector = VAL_TPI__IF_SELECT__GEN1;
            break;
        case SI_INFO_FRAME_ISRC2:
            selector = VAL_TPI__IF_SELECT__GEN2;
            break;
        case SI_INFO_FRAME_ACP:
            selector = VAL_TPI__IF_SELECT__MPEG; // ACP uses MPEG buffer
            break;
        case SI_INFO_FRAME_GBD:
            selector = VAL_TPI__IF_SELECT__GBD;
            break;
        case SI_INFO_FRAME_VSIF:
            selector = VAL_TPI__IF_SELECT__VSIF;
            break;
    }

    return selector;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable/Disable InfoFrame transmission.
//!
//! @param[in]  isEnabled - true, if InfoFrame transmission is to be enabled,
//! @param[in]  selector  - TPI InfoFrame index (0-7). @see InfoFrameSelectCodeGet().
//-------------------------------------------------------------------------------------------------

static void InfoFrameEnable(bool_t isEnabled, uint8_t selector)
{
    if (isEnabled)
    {
        // select the buffer and prepare for new packet update
        SiiRegWrite(REG_TPI__IF_SELECT, selector | BIT_TPI__IF_SELECT__ENABLE |
                                                 BIT_TPI__IF_SELECT__REPEAT |
                                                 BIT_TPI__IF_SELECT__OVRD_PACKET);

        // Initiate transmission by writing to the last register of the IF group
        if (selector == InfoFrameSelectCodeGet(SI_INFO_FRAME_AUDIO)) // Audio IF
        {
            SiiRegWrite(REG_TPI__IF_DBYTE10, SiiRegRead(REG_TPI__IF_DBYTE10));
        }
        else if (selector == InfoFrameSelectCodeGet(SI_INFO_FRAME_AVI)) // AVI IF
        {
            SiiRegWrite(REG_TPI__IF_DBYTE15, SiiRegRead(REG_TPI__IF_DBYTE15));
        }
        else // All other IFs
        {
            SiiRegWrite(REG_TPI__IF_DBYTE27, SiiRegRead(REG_TPI__IF_DBYTE27));
        }
    }
    else
    {
        SiiRegWrite(REG_TPI__IF_SELECT, selector);
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Select a source of control packets (CTS,GCP,Audio) when external
//!             SPDIF source is used along with the HDMI Rx source. One can select either
//!             TPI or Rx source. The Rx source selection only makes sense when the SPDIF
//!             audio is extracted from HDMI Rx data without change.
//!
//!             @note Controls SiI9535 specific TPI register!
//!
//! @param[in]  isAudioSampleTpi - Audio Sample packet info is taken from TPI,
//! @param[in]  isCtsTpi         - CTS packet info is taken from TPI,
//! @param[in]  isGcpTpi         - GCP info is taken from TPI.
//-------------------------------------------------------------------------------------------------
#if 0
static void ControlPacketsSourceSet(bool_t isAudioSampleTpi, bool_t isCtsTpi, bool_t isGcpTpi)
{
    SiiRegWrite(REG_TPI__AUDIO_PKT_SRC_CFG,
                            (isAudioSampleTpi ? BIT_TPI__AUDIO_SMP_PKT_SRC_TPI : 0) |
                            (isCtsTpi ? BIT_TPI__AUDIO_CTS_PKT_SRC_TPI : 0) |
                            (isGcpTpi ? BIT_TPI__AUDIO_GCP_PKT_SRC_TPI : 0));
}
#endif


//-------------------------------------------------------------------------------------------------
//! @brief      The function is used for TMDS swing & termination adjustments.
//!
//! @param[in]  pixelClk   - Pixel Clock frequency (* 10000 Hz),
//-------------------------------------------------------------------------------------------------

static void TmdsSwingTermSet(uint16_t pixelClk )
{
    uint8_t zoneVal = 0x8;

    if (pixelClk < 16500)
    {
        // up to 165Mhz
        if (pixelClk < 3000)
        {
            //SWWA:22208
            zoneVal = 0x9;
        }
    }

    // TMDS termination resistor to support Vclk up to 300MHz
 //  SiiDrvTpiIndirectWrite(REG_TX__TMDS_TERM_SELECT, term | 0x20);
    // TMDS Swing adjustment per chip characterization
   // SiiDrvTpiIndirectWrite(REG_TX__TMDS_SWING_CTRL, swing);
    // TMDS zone adjustment for low frequency inputs
   // SiiDrvTpiIndirectWrite(REG_TX__TMDS_ZONE_CTRL2, zoneVal);
}



//-------------------------------------------------------------------------------------------------
//  TPI System level API functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Select Tx device instance in multi-transmitter system.
//!
//! @param[in]  device_idx - zero based index of an instance.
//-------------------------------------------------------------------------------------------------

#if SII_NUM_TX > 1
void SiiDrvTpiInstanceSet(uint8_t deviceIndex)
{
    if(deviceIndex < SII_NUM_TX)
    {
        pTpi = &txTpiInstRec[deviceIndex];

        SiiDrvTpiAccessInstanceSet(deviceIndex);

        // Set instance for CRA pages used.
        SiiRegInstanceSet( TX_PAGE_TPI, deviceIndex );
        SiiRegInstanceSet( TX_PAGE_L0,  deviceIndex );
        SiiRegInstanceSet( TX_PAGE_L1,  deviceIndex );
    }
}

int SiiDrvTpiInstanceGet( void )
{

    return( pTpi - &txTpiInstRec[0] );
}
#endif // SII_NUM_TX > 1

void SiiDrvTpiHdcpR0CalcConfigSet( int isVsyncBase )
{
	if(isVsyncBase)
	{
		SiiRegWrite(REG_TPI__HW_OPT, VAL_TPI__R0_CALC_VSYNC);
	}
	else
	{
		SiiRegWrite(REG_TPI__HW_OPT, VAL_TPI__R0_CALC_OSC);
	}
}
//-------------------------------------------------------------------------------------------------
//! @brief      Initialize TPI module
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiInit(void)
{
    // Clear interrupt flags
    pTpi->txInterruptWord = 0;

    SiiDrvTpiDdcInit();


    // ---------------- Inits & SWWAs -------------------

    // TMDS termination resistor to support Vclk up to 165MHz
    // The differential termination
    //SiiDrvTpiIndirectWrite(REG_TX__TMDS_TERM_SELECT, VAL_TX__TMDS_TERM__NONE);
    // TMDS Swing adjustment per chip characterization
    //SiiDrvTpiIndirectWrite(REG_TX__TMDS_SWING_CTRL, 0xDB);
    //SiiDrvTpiIndirectWrite(REG_TX__TMDS_PLL_BW, 0x04);

    // No HDCP protection on the link
    // Enable HDCP cancellation whenever HDCP engine stopped
    SiiRegWrite(REG_TPI__HDCP_CTRL, BIT_TPI__HDCP_CTRL__PROT_REQUEST_CANCEL);

    // TMDS output power down, output mode - DVI
    SiiRegWrite(REG_TPI__SYSTEM_CONTROL, BIT_TPI__SYSTEM_CONTROL__PD);

    // ------------------------------------------------

    // Check if DS device is connected, then simulate HPD interrupt catch
    if ((SiiRegRead(REG_TPI__INT_STATUS1) & BIT_TPI__INT__HPD_STATE) != 0)
    {
        pTpi->isWakeUpRequired = true;
        pTpi->txInterruptWord = BIT_TPI__INT__HPD;
    }
	// Initial value for R0 calculation time.
	// Will be updated based on the incoming infoframe
	SiiDrvTpiHdcpR0CalcConfigSet(false);
}


//-------------------------------------------------------------------------------------------------
//! @brief      TPI software reset
//!
//!             Reset all sections, including the audio FIFO, except registers that are user
//!             configurable.
//!             Caution (applicable for Evita only): CEC operation is not guaranteed after a
//!             Software Reset because the software reset clears only write pointer while keeping
//!             the read pointer.
//!             Therefore, there will be mismatches in write/read pointers, the amount of
//!             mismatch will depend on when software reset is issued.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiSwReset(void)
{
    uint8_t regVal;

    regVal = SiiRegRead(REG_TPI__RAR);
    regVal |= BIT_TPI__RAR__SW_RST;
    SiiRegWrite(REG_TPI__RAR, regVal);
    regVal &= ~BIT_TPI__RAR__SW_RST;
    SiiRegWrite(REG_TPI__RAR, regVal);

    SiiOsTimerWait(20);       // Wait 20 ms to settle down after software reset

   // DEBUG_PRINT( TX_MSG_DBG, "TPI_SW_RESET\n");
}



//-------------------------------------------------------------------------------------------------
//! @brief      Enable or Disable TPI interrupts.
//!
//!             Controls only TPI interrupts selected by TX_INTERRUPT_MASK macro.
//!
//! @param[in]  isInterruptsEnabled - true for enable
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiInterruptsEnable(const bool_t isInterruptsEnabled)
{

    SiiRegBitsSet(REG_TPI__INT_ENABLE1, TX_INTERRUPT_MASK0, isInterruptsEnabled );
    SiiRegBitsSet(REG_TPI__INT_ENABLE2, TX_INTERRUPT_MASK1, isInterruptsEnabled );

}


//-------------------------------------------------------------------------------------------------
//! @brief      Turn power on.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiPowerUp(void)
{
    SiiRegWrite(REG_TPI__DEVICE_POWER_STATE_CTRL, VAL_TPI__POWER_STATE_D0);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Standby mode.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiStandby(void)
{
#if INC_OSD // SWWA: FP1554 - Make sure OSD_EN is 0 before setting Tx0 power state to other than D0
#if SII_NUM_TX > 1
    if ( SiiDrvTpiInstanceGet() == 0 )
#endif
    {
        DrvOsdDisableInternal();
    }
#endif

    // Disable interrupts for the TPI driver
    SiiDrvTpiInterruptsEnable( false );

    // Clear outstanding interrupts
    SiiRegWrite( REG_TPI__INT_ENABLE1, TX_INTERRUPT_MASK0 );
    SiiRegWrite( REG_TPI__INT_ENABLE2, TX_INTERRUPT_MASK1 );

    // Go into low power state (D2)
    SiiRegWrite(REG_TPI__DEVICE_POWER_STATE_CTRL, VAL_TPI__POWER_STATE_D2);
}

#if ( configSII_DEV_953x_PORTING == 1 )
void SiiDrcTpiDisableInterruptStandby( void )
{
    uint8_t intstatus = 0;

    intstatus = SiiRegRead(REG_TPI__INT_STATUS1 );

    /*Write 1 to the interrupt bits to clear the pending status*/
    SiiRegWrite( REG_TPI__INT_STATUS1, intstatus );

    /*Disable interrupt event for standy */
    SiiRegModify(REG_TPI__INT_ENABLE1,( BIT_TPI__INT__HPD | BIT_TPI__INT__RX_SENSE | BIT_TPI__INT__HDCP_CHNG ), CLEAR_BITS );
}
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Deep sleep mode. Waking up requires hardware reset.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiPowerDown(void)
{
#if INC_OSD // SWWA: FP1554 - Make sure OSD_EN is 0 before setting power state to other than D0
#if SII_NUM_TX > 1
    if ( SiiDrvTpiInstanceGet() == 0 )
#endif
    {
        DrvOsdDisableInternal();
    }
#endif
    SiiRegWrite(REG_TPI__DEVICE_POWER_STATE_CTRL, VAL_TPI__POWER_STATE_D3);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return true, if TX is in full power mode (not standby or power down)
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTpiIsPoweredUp(void)
{
    uint8_t powerMode = SiiRegRead(REG_TPI__DEVICE_POWER_STATE_CTRL);
    return ((powerMode & MSK_TPI__POWER_STATE_MASK) == VAL_TPI__POWER_STATE_D0);
}
//-------------------------------------------------------------------------------------------------
//! @brief      Return true, if TMDS is in active state
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTmdsIsPoweredUp(void)
{
    uint8_t tmdsControl = SiiRegRead(REG_TPI__SYSTEM_CONTROL);
    return ((tmdsControl & BIT_TPI__SYSTEM_CONTROL__PD) == BIT_TPI__SYSTEM_CONTROL__PD);
}
//-------------------------------------------------------------------------------------------------
//! @brief      Enable or Disable TMDS output signals.
//!
//! @param[in]  isTmdsEnabled - true enables TMDS, false disables TMDS
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiTmdsOutputEnable(bool_t isEnabled)
{
    // if no 5V from Rx, force Tx TMDS power down, kaomin
    if(!pPowerHandle_ObjCtrl->power_check_5v_from_dvd())
    {
        isEnabled = FALSE;
    }

	if(isEnabled)
	{
		// Fix to RTL BUG (26110): clear "bgr_ext_res_en" bit-1 of 0x83 regester in Legacy Tx page 0
		SiiDrvTpiIndirectWrite(REG_TX__AUDIO_CLK_DIV, 0x88);
	}

    SiiRegModify(REG_TPI__SYSTEM_CONTROL, BIT_TPI__SYSTEM_CONTROL__PD,
                            isEnabled ? CLEAR_BITS : SET_BITS);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set an output mode (DVI or HDMI).
//!
//! @param[in]  isHdmi - true for HDMI, false for DVI
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiHdmiOutputModeSet(bool_t isHdmi)
{
    SiiRegModify(REG_TPI__SYSTEM_CONTROL, BIT_TPI__SYSTEM_CONTROL__HDMI,
                            isHdmi ? SET_BITS : CLEAR_BITS);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return transmitter device ID.
//!
//! @return     MSByte - device ID code, LSByte - device revision code.
//-------------------------------------------------------------------------------------------------

uint16_t SiiDrvTpiDeviceIdGet(void)
{
    uint8_t deviceId;
    uint8_t deviceRevision;

    deviceId = SiiRegRead(REG_TPI__DEVICE_ID);
    deviceRevision = SiiRegRead(REG_TPI__DEVICE_REV_ID);

    return ((deviceId << 8) | deviceRevision);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return TPI revision ID.
//!
//! @return     TPI revision code.
//-------------------------------------------------------------------------------------------------

uint8_t SiiDrvTpiRevisionGet(void)
{
    return (SiiRegRead(REG_TPI__TPI_REV_ID) & 0x7F) ;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Check for TX related interrupts. If found, clear pending hardware interrupt
//!             bits and change the status to indicate pending interrupt.
//!
//!             This function is to be called from the Device Interrupt manager upon receiving
//!             a hardware interrupt from TX.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiProcessInterrupts(void)
{
    uint8_t txInterrupts[2];
    uint8_t plugStatus;

    txInterrupts[0] = SiiRegRead(REG_TPI__INT_STATUS1);
    txInterrupts[1] = SiiRegRead(REG_TPI__INT_STATUS2);

    // Update TX connection status (HPD & RSEN)
    plugStatus = 0;
    if ((txInterrupts[0] & BIT_TPI__INT__HPD_STATE) != 0)
    {
        plugStatus |= BIT1;
#if (FPGA_BUILD == ENABLE)
        plugStatus |= BIT0; // FPGA model doesn't support RSEN. Make RSEN = HPD status.
#endif
    }

#if (FPGA_BUILD == DISABLE)
    if ((txInterrupts[0] & BIT_TPI__INT__RX_SENSE_STATE) != 0)
    {
        plugStatus |= BIT0;
        //plugStatus |= BIT1; // RxSense should come with HPD, kaomin
    }
#endif
    // to recover RxSense interrupt not triggered with VIZIO 8220 TV (no audio issue after TV power off on), kaomin
    if((plugStatus&BIT0)!=(pTpi->plugStatus&BIT0))
    {
        txInterrupts[0] |= BIT_TPI__INT__RX_SENSE;
        TRACE_DEBUG((0, "recover RxSense interrupt 0x%X", plugStatus));
    }
    if((plugStatus&BIT1)!=(pTpi->plugStatus&BIT1))
    {
        txInterrupts[0] |= BIT_TPI__INT__HPD;
        TRACE_DEBUG((0, "recover HPD interrupt 0x%X", plugStatus));
    }
    pTpi->plugStatus = plugStatus;
    TRACE_DEBUG((0, "pTpi->plugStatus = 0x%X", pTpi->plugStatus));
    
    // Mask in interrupt related bits only (ignore status bits)
    txInterrupts[0] &= TX_INTERRUPT_MASK0;
    txInterrupts[1] &= TX_INTERRUPT_MASK1;

    if (SiiDrvTpiIsPoweredUp())
    {

        pTpi->txInterruptWord = txInterrupts[0] | (((uint16_t) txInterrupts[1]) << 8);

        // Check for pending hardware interrupts
        if (pTpi->txInterruptWord)
        {
            DEBUG_PRINT(TX_MSG_DBG, "TPI INTERRUPT CAUGHT: %04X\n", pTpi->txInterruptWord);
            if (( txInterrupts[1] != 0 ) && (( txInterrupts[1] & ~BIT_TPI__INT__P_STABLE) != 0))  // Skip PSTABLE interrupts
            {
                DEBUG_PRINT( TX_MSG_DBG, "Interrupt - INT0: %02X  INT1: %02X\n", txInterrupts[0], txInterrupts[1] );
            }
            
		    // Clear all pending hardware TX interrupts
		    if (txInterrupts[0])
		    {
		        SiiRegWrite(REG_TPI__INT_STATUS1, txInterrupts[0]);
		    }
		    if (txInterrupts[1])
		    {
		        SiiRegWrite(REG_TPI__INT_STATUS2, txInterrupts[1]);
		    }            
        }
    }
    else
    {
        // Low power mode requires wake-up to process the interrupts
        if (!pTpi->isWakeUpRequired)
        {
            // Only print the message for a new event
            DEBUG_PRINT( TX_MSG_DBG, "Interrupt - WAKEUP NEEDED\n");
        }
        pTpi->isWakeUpRequired = true;
    }

}

//-------------------------------------------------------------------------------------------------
//! @brief      Determine current state of TX HPD pin.
//! @return     boolean as follows
//! @retval     true    - HPD is HIGH on current TX
//! @retval     false   - HPD is LOW on current TX
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvTpiGetHpdState(void)
{

    return(( SiiRegRead( REG_TPI__INT_STATUS1 ) & BIT_TPI__INT__HPD_STATE) != 0);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return pending interrupt flags.
//-------------------------------------------------------------------------------------------------

uint16_t SiiDrvTpiPendingInterruptsGet(void)
{
    return pTpi->txInterruptWord;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Clean pending interrupt flags.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiPendingInterruptsClear(void)
{
    pTpi->txInterruptWord = 0;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return Wake up require flag.
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTpiIsWakeUpRequired(void)
{
    return pTpi->isWakeUpRequired;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Clear Wake up request.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiWakeUpRequestClear(void)
{
    pTpi->isWakeUpRequired = false;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Check if HDMI Link is stable and PLL is locked.
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTpiIsHdmiLinkStable(void)
{
    uint8_t clkStatus = SiiRegRead(REG_TPI__RAR);
    return ((clkStatus & BIT_TPI__RAR__P_STABLE) != 0);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Select a source of audio sample packets.
//!
//! @param[in]  isTpi - Audio Sample packet info is taken from TPI.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAudioSamplePacketSourceSet(bool_t isTpi)
{
    SiiRegModify(REG_TPI__AUDIO_PKT_SRC_CFG, BIT_TPI__AUDIO_SMP_PKT_SRC_TPI,
                                   (isTpi ? BIT_TPI__AUDIO_SMP_PKT_SRC_TPI : 0));
}


//-------------------------------------------------------------------------------------------------
//! @brief      Select a source of CTS packets.
//!
//! @param[in]  isTpi - CTS packet info is taken from TPI.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiCtsPacketSourceSet(bool_t isTpi)
{
    SiiRegModify(REG_TPI__AUDIO_PKT_SRC_CFG, BIT_TPI__AUDIO_CTS_PKT_SRC_TPI,
                                   (isTpi ? BIT_TPI__AUDIO_CTS_PKT_SRC_TPI : 0));
}


//-------------------------------------------------------------------------------------------------
//! @brief      Select a source of AV Mute/Unmute bits of the GCP packets.
//!
//! @param[in]  isTpi - AVMute/Unmute bits are taken from TPI.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAvMuteSourceSet(bool_t isTpi)
{
    SiiRegModify(REG_TPI__AUDIO_PKT_SRC_CFG, BIT_TPI__AUDIO_GCP_PKT_SRC_TPI,
                                   (isTpi ? BIT_TPI__AUDIO_GCP_PKT_SRC_TPI : 0));
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return current HPD and RxSense status. The status gets updated by h/w interrupt.
//!
//! @return     Bitfield with the following bits:
//! @retval     BIT0  - Rx Sense detected,
//! @retval     BIT1  - hot plug detected.
//-------------------------------------------------------------------------------------------------

uint8_t SiiDrvTpiPlugStatusGet(void)
{
    return pTpi->plugStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Select video source type.
//!
//!             This function implementation highly depends on particular TX chip.
//!             Current implementation is for SiI953x (SiI953x)
//!
//! @param[in]  source - video source type.
//!
//! @return     Success flag.
//! @retval     true  - video source type is supported and set successfully,
//! @retval     false - video source type is not supported.
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTpiVideoSourceSelect(txVideoSource_t source)
{
    bool_t  isSuccess = true;

    switch (source)
    {
        case SI_TX_VIDEO_SOURCE_HDMI:   // RX_main - TX pipe
            SiiDrvTpiInfoFrameEnable(false, true, 0); // Disable all InfoFrames from TPI
            // Bug 33551 - Snow/green video appears on TV while port switching 
            //SiiDrvTpiAvMuteSourceSet(false);          // AV Mute from RX
            SiiDrvTpiDeepColorGcpEnable(true);        // Deep Color data from RX
            SiiRegBitsSet(REG_TPI__VID_IN_MODE, BIT_TPI__VID_IN_MODE, VAL_TPI__VID_IN_MODE_RX_MAIN );
            break;

        case SI_TX_VIDEO_SOURCE_PGEN:   // Internal Pattern Generator
            SiiDrvTpiAvMuteSourceSet(true);
            break;

        // Other source types are not supported
        default:
            isSuccess = false;
            break;
    }


    return isSuccess;
}
//-------------------------------------------------------------------------------------------------
//! @brief      Set blanking levels for all 3 color channels: [R,G,B] or [Y,Cb,Cr].
//!
//!             Legacy access is used as TPI doesn't support this feature.
//!
//! @param[in]  levels - array of 3 numbers having values in the range 0-255.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiBlankingLevelsSet(uint8_t aLevels[3])
{
    SiiDrvTpiIndirectBlockWrite(REG_TX__VID_BLANK1, aLevels, 3);
    // SWWA:16600
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set TPI video mode data.
//!
//!             In SiI9535 this interface is deprecated. Used to adjust TMDS swing and termination.
//!
//! @param[in]  pixelClk   - Pixel Clock frequency (* 10000 Hz),
//! @param[in]  vertFreqHz - Vertical frequency (Hz),
//! @param[in]  totalPixelsPerLine - Total number of pixels per line,
//! @param[in]  totalLines - Total number of lines.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiVideoModeDataSet(uint16_t pixelClk, uint16_t vertFreqHz, uint16_t totalPixelsPerLine,
                               uint16_t totalLines)
{
    TmdsSwingTermSet(pixelClk);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set Audio audio clock divider.
//!
//!
//! @param[in]  tmdsClock   - TMDS Clock frequency (* 10000 Hz),
//! @param[in]  isSrcSpdif - if the audio source is spdif
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAudioClkDividerSet(uint16_t tmdsClock, bool_t isSrcSpdif)
{
    static uint16_t tmdsClk = 0;
    uint8_t clkVal = VAL_TX__AUDIO_CLK_RATIO_1;

    if(tmdsClock == 0)
    {
        tmdsClock = tmdsClk;
    }
    else
    {
        tmdsClk = tmdsClock;
    }

    if ((tmdsClock < 6000)||(!isSrcSpdif))
    {
        clkVal = VAL_TX__AUDIO_CLK_RATIO_1;
    }
    else if (tmdsClock < 10000)
    {
        clkVal = VAL_TX__AUDIO_CLK_RATIO_2;
    }
    else if (tmdsClock < 12000)
    {
        clkVal = VAL_TX__AUDIO_CLK_RATIO_3;
    }
    else if (tmdsClock < 16000)
    {
        clkVal = VAL_TX__AUDIO_CLK_RATIO_4;
    }
    else if (tmdsClock < 20000)
    {
        clkVal = VAL_TX__AUDIO_CLK_RATIO_5;
    }
    else
    {
        clkVal = VAL_TX__AUDIO_CLK_RATIO_6;
    }

    SiiDrvTpiIndirectWrite(REG_TX__AUDIO_CLK_DIV, clkVal);

}

//-------------------------------------------------------------------------------------------------
//! @brief      toggle Audio audio clock divider.
//!
//-------------------------------------------------------------------------------------------------
void SiiDrvTpiAudioClkToggle(void)
{
    SiiRegModify(REG_TPI__ACLK_DIV, BIT_TPI__ACLK_DIV_FACTOR, SET_BITS);
    SiiOsTimerWait(100);
    SiiRegModify(REG_TPI__ACLK_DIV, BIT_TPI__ACLK_DIV_FACTOR, CLEAR_BITS);

}
//-------------------------------------------------------------------------------------------------
//! @brief      Set input video pixel capturing parameters in the chip.
//!
//! @param[in]  clockMode             - input clock usage fashion,
//! @param[in]  isLatchOnRisingEdge   - defines latching/first edge of a clock pulse,
//! @param[in]  repeatedPixelsNumber  - a number of replicated pixels (total pixels in a bunch - 1),
//! @param[in]  isInputReplicated     - true, if input video has replicated pixels.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiPixelCapturingConfig(txVbusClockMode_t clockMode, bool_t isLatchOnRisingEdge,
                                   uint8_t repeatedPixelsNumber, bool_t isInputReplicated)
{
    uint8_t regPrdVal = 0;

    if (clockMode != SI_TX_VBUS_CLOCK_LATCH_BOTH_EDGES)
    {
        // latching by a single edge (rising or falling)
        regPrdVal |= BIT_TPI__PRD__BSEL;
    }

    if (isLatchOnRisingEdge)
    {
        regPrdVal |= BIT_TPI__PRD__EDGE;
    }

    if (isInputReplicated)
    {
        // every input pixel has already been repeated
        // as many times as needed at the output

        switch (repeatedPixelsNumber)
        {
            default: // no repetition
                //case 0:
                regPrdVal |= VAL_TPI__PRD__ICLK_1; // every incoming pixel is unique
                break;
            case 1: // every pixel sent twice
                regPrdVal |= VAL_TPI__PRD__ICLK_2; // every input pixel is repeated once
                break;
            case 3: // 4 times repetition
                regPrdVal |= VAL_TPI__PRD__ICLK_4; // every input pixel is repeated 4 times
                break;
        }
    }

    SiiRegWrite(REG_TPI__PRD, regPrdVal);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set input video color parameters.
//!
//! @param[in]  colorSpace                    - input video color space,
//! @param[in]  bitsPerColor                  - color depth per channel (R,G or B),
//! @param[in]  isDataRangeExpansionRequired  - true, if input data range is limited and require
//!                                             expansion at the output.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiInputVideoColorSet(txVbusColSpace_t colorSpace, txVbusColDepth_t bitsPerColor,
                                 bool_t isDataRangeExpansionRequired)
{
    uint8_t regInpFmt = 0;

    switch (colorSpace)
    {
        default: // RGB
            regInpFmt |= VAL_TPI__INP_FMT__COLOR_SPACE__RGB;
            break;
        case SI_TX_VBUS_COLOR_SPACE_YCBCR444:
            regInpFmt |= VAL_TPI__INP_FMT__COLOR_SPACE__YCBCR_444;
            break;
        case SI_TX_VBUS_COLOR_SPACE_YCBCR422:
            regInpFmt |= VAL_TPI__INP_FMT__COLOR_SPACE__YCBCR_422;
            break;
    }

    // Video data range expansion (auto format option is not used)
    if (isDataRangeExpansionRequired)
    {
        regInpFmt |= VAL_TPI__INP_FMT__RANGE__EXPAND;
    }
    else
    {
        regInpFmt |= VAL_TPI__INP_FMT__RANGE__NO_EXPAND;
    }

    switch (bitsPerColor)
    {
        default:
            regInpFmt |= VAL_TPI__INP_FMT__DEPTH__8;
            break;
        case SI_TX_VBUS_BITS_PER_COLOR_10:
            regInpFmt |= VAL_TPI__INP_FMT__DEPTH__10;
            break;
        case SI_TX_VBUS_BITS_PER_COLOR_12:
            regInpFmt |= VAL_TPI__INP_FMT__DEPTH__12;
            break;
        case SI_TX_VBUS_BITS_PER_COLOR_16:
            switch (colorSpace)
            {
                case SI_TX_VBUS_COLOR_SPACE_RGB:
                case SI_TX_VBUS_COLOR_SPACE_YCBCR444:
                    // input pixel comes partly on rising, partly on falling edge
                    regInpFmt |= VAL_TPI__INP_FMT__DEPTH__16;
                    break;
                default:
                    // Latching by a single edge (rising or falling).
                    // There are only 36 input pins, so it is not possible
                    // to have 16*3=48 bits at 36 pins;
                    // correcting input bus with with a proper value 3*12=36 bits.
                    regInpFmt |= VAL_TPI__INP_FMT__DEPTH__12;
            }
            break;
    }

    regInpFmt |= VAL_TPI__INP_FMT__TCLK_X1; // default TCLK multiplier 1.0

    SiiRegWrite(REG_TPI__INP_FMT, regInpFmt);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set output video color parameters.
//!
//! @param[in]  colorSpace                      - output video color space,
//! @param[in]  bitsPerColor                    - color depth per channel (R,G or B),
//! @param[in]  colorimetry                     - colorimetry standard,
//! @param[in]  isDataRangeCompressionRequired  - true, if input data range is full and require
//!                                               contraction.
//! @param[in]  isDitheringEnabled              - true, if color dithering function shall be used
//!                                               to smooth color gradation steps.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiOutputVideoColorSet(txVbusColSpace_t colorSpace,  txVbusColDepth_t bitsPerColor,
                                  txVbusColorimetry_t colorimetry, bool_t isDataRangeCompressionRequired,
                                  bool_t isDitheringEnabled)
{
    uint8_t regOutFmt = 0;

    // Video data range compression (auto format option is not used)
    if (isDataRangeCompressionRequired)
    {
        regOutFmt |= VAL_TPI__OUTP_FMT__RANGE__COMPRESS;
    }
    else
    {
        regOutFmt |= VAL_TPI__OUTP_FMT__RANGE__NO_COMPRESS;
    }

    switch (bitsPerColor)
    {
        default:
            regOutFmt |= VAL_TPI__OUTP_FMT__DEPTH__8;
            break;
        case SI_TX_VBUS_BITS_PER_COLOR_10:
            regOutFmt |= VAL_TPI__OUTP_FMT__DEPTH__10;
            break;
        case SI_TX_VBUS_BITS_PER_COLOR_12:
            regOutFmt |= VAL_TPI__OUTP_FMT__DEPTH__12;
            break;
        case SI_TX_VBUS_BITS_PER_COLOR_16:
            regOutFmt |= VAL_TPI__OUTP_FMT__DEPTH__16;
            break;
    }

    switch (colorSpace)
    {
        case SI_TX_VBUS_COLOR_SPACE_YCBCR422:
            regOutFmt |= VAL_TPI__OUTP_FMT__COLOR_SPACE__YCBCR_422;
            break;
        case SI_TX_VBUS_COLOR_SPACE_YCBCR444:
            regOutFmt |= VAL_TPI__OUTP_FMT__COLOR_SPACE__YCBCR_444;
            break;
        default: // RGB
            regOutFmt |= VAL_TPI__OUTP_FMT__COLOR_SPACE__RGB;
            break;
    }

    switch (colorimetry)
    {
        case SI_TX_VIDEO_COLORIMETRY_ITU709:
        case SI_TX_VIDEO_COLORIMETRY_XV709:
            regOutFmt |= VAL_TPI__OUTP_FMT__709;
            break;
        default:
            regOutFmt |= VAL_TPI__OUTP_FMT__601;
    }

    if (isDitheringEnabled)
    {
        regOutFmt |= VAL_TPI__OUTP_FMT__DITHER__ON;
    }

    SiiRegWrite(REG_TPI__OUTP_FMT, regOutFmt);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable or Disable Deep Color GCP Packet transmission.
//!
//!             GCP (General Control Packet)
//!
//! @param[in]  isEnabled - true for enabling, false for disabling.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiDeepColorGcpEnable(bool_t isEnabled)
{
    SiiRegModify(REG_TPI__RAR, BIT_TPI__RAR__DC_PKT_EN, isEnabled ? SET_BITS : CLEAR_BITS );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set various configuration parameters of the Sync signals generation
//!             and extraction system.
//!
//!             This function preserves current page of the Sync/DE register set.
//!             It also keeps YC Mux Mode bit intact.
//!
//! @param[in]  pSynRg - pointer to a structure containing the parameters.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiSyncRegenConfigurationSet(syncRegen_t *pSynRg)
{
    // Get REG_TPI__SYNC_GEN_CTRL register;
    // keep SYNC_EXTRACT_LAYOUT and EN_DE_MUX bits,
    // clear the rest as they are going to be replaced
    // according to configuration.
    uint8_t syncRegConfig = SiiRegRead(REG_TPI__SYNC_GEN_CTRL) &
        (BIT_TPI__SYNC_GEN_CTRL__SYNC_EXTRACT_LAYOUT | BIT_TPI__SYNC_GEN_CTRL__EN_DE_MUX);

    uint8_t options = 0;

    // Pack user defined options
    switch (pSynRg->adjustVbitToVsync)
    {
        default:
        case 0: // no adjustment
            options = 0 | 0;
            break;
        case 1: // increment by 1
            options = BIT_TPI__SYNC_GEN_CTRL__FIELD_2_VBLANK_ADJ | BIT_TPI__SYNC_GEN_CTRL__FIELD_2_VBLANK_OFFSET;
            break;
        case (-1): // decrement by 1
            options = BIT_TPI__SYNC_GEN_CTRL__FIELD_2_VBLANK_ADJ | 0;
            break;
    }

    options |= pSynRg->isVsyncAdjusted ? 0 : BIT_TPI__SYNC_GEN_CTRL__DE_NO_VSYNC_ADJ;
    options |= pSynRg->isFieldPolarityInverted ? BIT_TPI__SYNC_GEN_CTRL__INV_FIELD_POL : 0;

    syncRegConfig |= options; // insert options into register keeping 2 stored bits intact
    SiiRegWrite(REG_TPI__SYNC_GEN_CTRL, syncRegConfig);

}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable or Disable DE signal generation. In case of enabling, an additional
//!             argument provides parameters to set in the GE generator register page.
//!
//! @param[in]  isEnabled - true for enabling, false for disabling,
//! @param[in]  pSynRg    - pointer to a structure containing video parameters.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiDeGenerationEnable(bool_t isEnabled, syncRegen_t *pSynRg)
{
    uint8_t buf[4];  // for data packing

    // Switch to DE control page
    EmbeddedSyncControlPageEnable(false);

    if (isEnabled)
    {
        // Set parameters of DE generator
        buf[0] = (uint8_t) pSynRg->deDelayPix;
        buf[1] = (uint8_t) (((pSynRg->deDelayPix >> 8) & MSK_TPI__DE_CTRL__DE_DLY_H)
                           | (pSynRg->isHsyncPolarityNeg ? BIT_TPI__DE_CTRL__HSYNC_POL : 0)
                           | (pSynRg->isVsyncPolarityNeg ? BIT_TPI__DE_CTRL__VSYNC_POL : 0)
                           |  BIT_TPI__DE_CTRL__EN_VID_DE_GEN ); // Enable DE generator
        buf[2] = pSynRg->deTopLn;

        SiiRegWriteBlock(REG_TPI__DE_DLY, buf, 3);

        buf[0] = (uint8_t)  pSynRg->deWidthPix;
        buf[1] = (uint8_t) (pSynRg->deWidthPix >> 8);
        buf[2] = (uint8_t)  pSynRg->deHeightLn;
        buf[3] = (uint8_t) (pSynRg->deHeightLn >> 8);

        SiiRegWriteBlock(REG_TPI__DE_CNT_LSB, buf, 4);

    }
    else
    {
        // Disable the GE generation if requested
        SiiRegModify(REG_TPI__DE_CTRL, BIT_TPI__DE_CTRL__EN_VID_DE_GEN, CLEAR_BITS);
    }


}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable or Disable HV Sync signal extraction from data embedded into video stream.
//!             In case of enabling, an additional argument provides parameters to set
//!             in the Embedded Sync Extraction register page.
//!
//! @param[in]  isEnabled - true for enabling, false for disabling,
//! @param[in]  pSynRg    - pointer to a structure containing video parameters.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiEmbSyncExtractionEnable(bool_t isEnabled, syncRegen_t *pSynRg)
{
    uint8_t buf[8];  // for data packing

    // Switch to Sync Extraction control page
    EmbeddedSyncControlPageEnable(true);

    if (isEnabled)
    {
        // Set parameters of HV Sync Extraction system
        buf[0] = (uint8_t)  pSynRg->hbitToHsyncDelayPix;
        buf[1] = (uint8_t) (((pSynRg->hbitToHsyncDelayPix >> 8) & MSK_TPI__HBIT_TO_HSYNC_MSB)
                           | (pSynRg->isHsyncPolarityNeg ? BIT_TPI__HBIT_TO_HSYNC_MSB__HSYNC_POL : 0)
                           | (pSynRg->isVsyncPolarityNeg ? BIT_TPI__HBIT_TO_HSYNC_MSB__VSYNC_POL : 0)
                           |  BIT_TPI__HBIT_TO_HSYNC_MSB__EN_EMBED_SYNC ); // Enable Sync Extraction
        buf[2] = (uint8_t)  pSynRg->oddFieldVsyncOffsetPix;
        buf[3] = (uint8_t) (pSynRg->oddFieldVsyncOffsetPix >> 8);
        buf[4] = (uint8_t)  pSynRg->hsyncWidthPix;
        buf[5] = (uint8_t) (pSynRg->hsyncWidthPix >> 8);
        buf[6] = (uint8_t)  pSynRg->vbitToVsyncDelayLn;
        buf[7] = (uint8_t)  pSynRg->vsyncWidthLn;

        SiiRegWriteBlock(REG_TPI__HBIT_TO_HSYNC_LSB, buf, 8);
    }
    else
    {
        // Disable HV Sync extraction
        SiiRegModify(REG_TPI__HBIT_TO_HSYNC_MSB, BIT_TPI__HBIT_TO_HSYNC_MSB__EN_EMBED_SYNC, CLEAR_BITS);
    }

}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable YC demuxing for YCbCr422 video where Y and C data buses are muxed
//!             and being transmitted at double rate.
//!
//! @param[in]  isEnabled - true, if demuxing is to be enabled.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiYcDemuxEnable(bool_t isEnabled)
{
    SiiRegModify(REG_TPI__SYNC_GEN_CTRL, BIT_TPI__SYNC_GEN_CTRL__EN_DE_MUX, isEnabled ? SET_BITS : CLEAR_BITS);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set the parameters of I2S interface.
//!
//! @param[in]  pI2sSettings - pointer to parameters of I2S receiver.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAudioI2sConfigSet(txAudioSrcConfig_t *pI2sSettings, bool_t isHbr, bool_t isAudioEncoded)
{
    uint8_t regVal = 0;

    //-----------------------------------------------------------------------
    // SWWA: 18393, 18394
    //-----------------------------------------------------------------------
    if(isHbr)
    {
        regVal |= BIT_TX__I2S_IN_CTRL__HBRA_ON | BIT_TX__I2S_IN_CTRL__VBIT | 0x20;
    }
    else if(isAudioEncoded)
    {
        regVal |= BIT_TX__I2S_IN_CTRL__VBIT;
    }

    SiiDrvTpiIndirectWrite(REG_TX__I2S_IN_CTRL, regVal);
    //-----------------------------------------------------------------------


    regVal = ((pI2sSettings->sampleEdge == SI_TX_AUDIO_I2S_SCK_EDGE_RISING) ? BIT_TPI__I2S_IN_CFG__SCK_EDGE : 0)
          | ((pI2sSettings->mclkMult << SFT_TPI__FREQ_SVAL) & MSK_TPI__FREQ_SVAL)
          | ((pI2sSettings->wsPolarity == SI_TX_AUDIO_I2S_WS_HIGH) ? BIT_TPI__I2S_IN_CFG__WS : 0)
          | ((pI2sSettings->sdJustify == SI_TX_AUDIO_I2S_SD_JUSTIF_RIGHT) ? BIT_TPI__I2S_IN_CFG__JUSTIFY : 0)
          | ((pI2sSettings->sdDirection == SI_TX_AUDIO_I2S_SD_DIR_LSB) ? BIT_TPI__I2S_IN_CFG__DATA_DIR : 0)
          | ((pI2sSettings->sdFirstBitShifted == SI_TX_AUDIO_I2S_SD_BIT0_NOT_SHIFTED) ? BIT_TPI__I2S_IN_CFG__1ST_BIT : 0);

    SiiRegWrite(REG_TPI__I2S_IN_CFG, regVal);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set the parameters of audio samples.
//!
//! @param[in]  smpSize - sample size of input audio,
//! @param[in]  smpRate - sample rate of input audio,
//! @param[in]  isHbr   - true, if input audio is HBR.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAudioSampleConfigSet(audioSampleSize_t smpSize, audioSampleRate_t smpRate, bool_t isHbr)
{
    uint8_t regVal;

    // Ignore incoming audio status channel information
    // and always retransmit all 24 bits that covers all cases anyway
    smpSize = SI_TX_AUDIO_SMP_SIZE_24_BIT;


    regVal = (isHbr ? BIT_TPI__AUDIO_SAMPLE__HBRA : 0)
          | ((smpRate << SFT_TPI__AUDIO_SAMPLE__FR) & MSK_TPI__AUDIO_SAMPLE__FREQUENCY)
          | ((smpSize << SFT_TPI__AUDIO_SAMPLE__WIDTH) & MSK_TPI__AUDIO_SAMPLE__WIDTH);

    SiiRegWrite(REG_TPI__AUDIO_SAMPLE, regVal);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set the audio interface type.
//!
//! @param[in]  source - audio source mode.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAudioInterfaceSet(txAudioSrc_t source)
{
    // Do not modify "Audio Mute" bit
    uint8_t regVal = (SiiRegRead(REG_TPI__AUDIO_CFG) & BIT_TPI__AUDIO_CFG__MUTE);

    switch (source)
    {
        default:
        case SI_TX_AUDIO_SRC_NONE:
            // Disabled audio shall be muted as well
            regVal |= BIT_TPI__AUDIO_CFG__MUTE;
            // Set Rx (not TPI) source for the audio information in packets
            SiiDrvTpiAudioSamplePacketSourceSet(false);
            SiiDrvTpiCtsPacketSourceSet(false);
            break;

        case SI_TX_AUDIO_SRC_SPDIF:
            // SPDIF input (I2S is disabled)
            regVal |= VAL_TPI__AUDIO_CFG__SPDIF;
            // Set TPI (not Rx) source for the audio information in packets
            SiiDrvTpiAudioSamplePacketSourceSet(true);
            SiiDrvTpiCtsPacketSourceSet(true);

            break;

        case SI_TX_AUDIO_SRC_DSD_L1:
            regVal |= BIT_TPI__AUDIO_CFG__LAYOUT1;
            // proceed without break
        case SI_TX_AUDIO_SRC_DSD_L0:
            regVal |= VAL_TPI__AUDIO_CFG__DSD;
            break;

        case SI_TX_AUDIO_SRC_I2S_L1:
            regVal |= BIT_TPI__AUDIO_CFG__LAYOUT1;
            // proceed without break
        case SI_TX_AUDIO_SRC_I2S_L0:
        case SI_TX_AUDIO_SRC_HBRA:        
            SiiDrvTpiAudioSamplePacketSourceSet(true);
            SiiDrvTpiCtsPacketSourceSet(true);
            // I2S input, including HBRA
            regVal |= VAL_TPI__AUDIO_CFG__I2S;
            break;
    }

    SiiRegWrite(REG_TPI__AUDIO_CFG, regVal); // note: audio could be off at this moment
}


//-------------------------------------------------------------------------------------------------
//! @brief      Disables audio input.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAudioDisable(void)
{
    SiiDrvTpiAudioInterfaceSet(SI_TX_AUDIO_SRC_NONE);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Mute/Unmute audio.
//!
//! @param[in]  isMuted - true, if audio is to be muted.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAudioMuteSet(bool_t isMuted)
{
    // Do not modify everything but "Audio Mute" bit
    uint8_t regVal = (SiiRegRead(REG_TPI__AUDIO_CFG) & ~BIT_TPI__AUDIO_CFG__MUTE);

    if (isMuted)
    {
        regVal |= BIT_TPI__AUDIO_CFG__MUTE;
    }

    // Allow Audio control from TPI, if muting in direct mode
#if (SI_TX_OVRD_TPI_CONTROL == ENABLE)
    if (pTpi->isPassThroughMode)
    {
        ControlPacketsSourceSet(isMuted, false, false);
    }
#endif

    SiiRegWrite(REG_TPI__AUDIO_CFG, regVal);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set Audio Status Channel data.
//!
//! @param[in]  statusChannel - status channel frame.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAudioStatusChannelSet(uint8_t aStatusChannel[5])
{
    SiiRegWriteBlock(REG_TPI__I2S_CHST_1, aStatusChannel, 5);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Assign I2S pins to internal FIFO pipes based on provided channel
//!             allocation status.
//!
//! @param[in]  chanAlloc     - channel allocation code (as defined in CEA-861-D).
//! @param[in]  isAutoDownsmp - enable automatic downsampling to basic audio for stereo audio input
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAudioI2sPinsToFifoAssign(audioSpkAlloc_t chanAlloc, bool_t isAutoDownsmp)
{
    int i;
    uint8_t regVal;
    uint8_t channelPipeMask = 0x1; // bits [3,2,1,0] enable corresponding channel pipes

    // Set pipe enabling mask based on the channel allocation
    if (chanAlloc == SI_TX_AUDIO_SPK_FL_FR)
    {
        channelPipeMask = 0x1; // enable pipe 0
    }
    else if (chanAlloc <= SI_TX_AUDIO_SPK_FL_FR_LFE_FC)
    {
        channelPipeMask = 0x3; // enable pipes 0,1
    }
    else if (chanAlloc <= SI_TX_AUDIO_SPK_FL_FR_LFE_FC_RL_RR)
    {
        channelPipeMask = 0x7; // enable pipes 0,1,2
    }
    else if ((chanAlloc >= SI_TX_AUDIO_SPK_FL_FR_FLC_FRC) && (chanAlloc <= SI_TX_AUDIO_SPK_FL_FR_LFE_FC_FLC_FRC))
    {
        channelPipeMask = 0xB; // enable pipes 0,1,3
    }
    else
    {
        channelPipeMask = 0xF; // enable pipes 0,1,2,3
    }

    // Connect and enable up to 4 pipes depending on the pipe enabling mask
    for (i = 3; i >= 0; i--)
    {
        regVal = i | (i << 4); // will connect DCx pin to FIFO#x (no permutations)
        if (channelPipeMask & (BIT0 << i)) // pick one bit of channelPipeMask starting from bit0
        {
            regVal |= BIT_TPI__I2S_MAP__SD_ENABLE;
        }
        SiiRegWrite(REG_TPI__I2S_MAP, regVal);
    }

    // Auto downsampling
    if ((chanAlloc == SI_TX_AUDIO_SPK_FL_FR) && isAutoDownsmp)
    {
		//DEBUG_PRINT(MSG_ALWAYS,"AUTO SAMPLING CONFIGURED \n");
        SiiRegModify(REG_TPI__I2S_MAP, BIT_TPI__I2S_MAP__AUTO_DOWN_SAMPLE, BIT_TPI__I2S_MAP__AUTO_DOWN_SAMPLE);
    }
//    else
//		DEBUG_PRINT(MSG_ALWAYS,"AUTO SAMPLING NOT CONFIGURED \n");
}


//-------------------------------------------------------------------------------------------------
//! @brief      Configure inserting audio stream extracted internally in the Rx path (SiI9535 specific).
//!
//! @param[in]  isEnabled  - audio insertion source is internal (true), or external (false & default)
//! @param[in]  isMainPipe - Main or Sub pipes is selected as a source of audio
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAudioRxSource(bool_t isEnabled, bool_t isMainPipe)
{
    uint8_t regVal;

    SiiRegModify(REG_TPI__AUDIO_SRC_CTRL, BIT_TPI__AUDIO_RX_RP_SELECT | BIT_TPI__AUDIO_ENABLE,
                (isEnabled ? BIT_TPI__AUDIO_ENABLE : 0) | (isMainPipe ? 0 : BIT_TPI__AUDIO_RX_RP_SELECT));

    // Generate CTS with mclk
    regVal = SiiDrvTpiIndirectRead(REG_TX__OTP_CLK);
    if (isEnabled)
    {
        regVal &= ~BIT_TX__OTP_CLK_CTS_GEN_NO_MCLK; // clear bit
    }
    else
    {
        regVal |= BIT_TX__OTP_CLK_CTS_GEN_NO_MCLK; // set bit
    }
    SiiDrvTpiIndirectWrite(REG_TX__OTP_CLK, regVal);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Initiate sending AVI frame to the sink.
//!
//! @param[in]  pAviFrame - pointer to the frame data,
//! @param[in]  length    - data size in bytes,
//! @param[in]  checksum  - checksum byte.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAviFrameStart(uint8_t *pAviFrame, uint8_t length, uint8_t checksum)
{
    SiiRegWrite(REG_TPI__AVI_BYTE_0, checksum);
    SiiRegWriteBlock(REG_TPI__AVI_BYTE_1, pAviFrame, length);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Reads and Writes back AVI info frame. This operation triggers AVI frame
//!             update in the chip, as well as color space conversion, if available.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAviFrameUpdate(void)
{
    uint8_t regVal = SiiRegRead(REG_TPI__END_RIGHT_BAR_MSB);
    SiiRegWrite(REG_TPI__END_RIGHT_BAR_MSB, regVal);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Initiate sending AV Mute command to the sink.
//!
//!             This feature is useful for switching video modes in order to prevent flicker.
//!
//! @param[in]  isMuted - true, if AV Mute shall be sent.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAvMuteSendingEnable(bool_t isMuted)
{
    // Allow GCP control from TPI, if muting in direct mode
#if (SI_TX_OVRD_TPI_CONTROL == ENABLE)
    if (pTpi->isPassThroughMode)
    {
        ControlPacketsSourceSet(false, false, isMuted);
    }
#endif

    SiiRegModify(REG_TPI__SYSTEM_CONTROL, BIT_TPI__SYSTEM_CONTROL__AVMUTE, isMuted ? SET_BITS : CLEAR_BITS);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable/Disable sending info frame of a given type.
//!
//! @param[in]  isEnabled  - true for enable, false for disable sending,
//! @param[in]  isAll      - true, if all info frames have to be disabled,
//! @param[in]  ifType     - info frame type ID (ignored, if isAll==true).
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiInfoFrameEnable(bool_t isEnabled, bool_t isAll, infoFrameType_t ifType)
{
    uint8_t selector;

    if (isAll)
    {
        for (selector = 0; selector <= 7; selector++)
        {
            InfoFrameEnable(isEnabled, selector);
        }
    }
    else
    {
        selector = InfoFrameSelectCodeGet(ifType);
        InfoFrameEnable(isEnabled, selector);
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Check if an InfoFrame is enabled (transmitted) or not.
//!
//! @param[in]  ifType - info frame type ID
//!
//! @retval     true   - enabled and currently transmitting (periodically),
//! @retval     false  - InfoFrame disabled.
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTpiInfoFrameIsEnabled(infoFrameType_t ifType)
{
    uint8_t selector = InfoFrameSelectCodeGet(ifType);

    // find out if the buffer is currently transmitting
    SiiRegWrite(REG_TPI__IF_SELECT, selector | BIT_TPI__IF_SELECT__NO_MODIFY | BIT_TPI__IF_SELECT__OVRD_PACKET);
    return ((SiiRegRead(REG_TPI__IF_SELECT) & BIT_TPI__IF_SELECT__ENABLE) != 0);

}


//-------------------------------------------------------------------------------------------------
//! @brief      Load InfoFrame data into the chip.
//!
//!             The function has synchronization mechanism that ensures flawless update
//!             of enabled frames. Enabled InfoFrames are being sent periodically, so a proper
//!             synchronization is required to prevent transmission of a data block in the middle
//!             of the update transaction.
//!
//! @param[in]  ifType             - info frame type ID,
//! @param[in]  pFrame             - pointer to InfoFrame data (with a packet header),
//! @param[in]  length             - size in bytes of the InfoFrame data,
//! @param[in]  isChecksumIncluded - true, if the InfoFrame
//!
//! @retval     true   - enabled and currently transmitting (periodically),
//! @retval     false  - InfoFrame disabled.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiInfoFrameLoad(infoFrameType_t ifType, uint8_t *pFrame, uint8_t length,
                            bool_t isChecksumIncluded)
{
    bool_t isHotSwap;

    // find out if the buffer is currently transmitting
    isHotSwap = SiiDrvTpiInfoFrameIsEnabled(ifType);
    // true: the buffer is in use and there is risk of sending out
    // partially modified packet
    // false: the buffer is released and ready for new data

    if (isHotSwap)
    {
        // Modify packet ID (title) to non-existing one.
        // This is to ensure that if partially modified packet is sent out,
        // it will be ignored by the downstream device,
        // Alternatively, the firmware should wait until the buffer is released by polling
        // BIT_TPI__IF_SELECT__ENABLE, but it could take up to one video frame length time
        // and we do not want waiting that long.
        SiiRegWrite(REG_TPI__IF_TYPE, 0x55);

        // Fill out packet version and length fields
        SiiRegWriteBlock(REG_TPI__IF_VERSION, &pFrame[1], REG_TPI__IF_DBYTE0 - REG_TPI__IF_VERSION);
    }
    else
    {
        // Fill out packet type, version and length fields
        SiiRegWriteBlock(REG_TPI__IF_TYPE, pFrame, REG_TPI__IF_DBYTE0 - REG_TPI__IF_TYPE);
    }

    if (isChecksumIncluded)
    {
        // update check sum field
        SiiRegWrite(REG_TPI__IF_DBYTE0, pFrame[IF_CHECKSUM_INDEX]);

        // check if the packet was too short for the write strobe
        if (ifType == SI_INFO_FRAME_AUDIO)
        {
            // audio packet strobe is in REG_TPI__IF_DBYTE10 register;
            // update REG_TPI__IF_DBYTE1 to REG_TPI__IF_DBYTE10
            // (including the strobe)
            SiiRegWriteBlock(REG_TPI__IF_DBYTE1, &pFrame[IF_HEADER_LENGTH], REG_TPI__IF_DBYTE10 - REG_TPI__IF_DBYTE1 + 1);
        }
        else
        {
            // packet strobe is in REG_TPI__IF_DBYTE27
            if (length < (REG_TPI__IF_DBYTE27 - REG_TPI__IF_DBYTE1 + 1))
            {
                // update only registers present in the packet
                SiiRegWriteBlock(REG_TPI__IF_DBYTE1, &pFrame[IF_HEADER_LENGTH], length);
                // update the strobe
                SiiRegWrite(REG_TPI__IF_DBYTE27, 0);
            }
            else
            {
                // update all packet registers including the strobe at once
                SiiRegWriteBlock(REG_TPI__IF_DBYTE1, &pFrame[IF_HEADER_LENGTH], REG_TPI__IF_DBYTE27 - REG_TPI__IF_DBYTE1 + 1);
            }
        }
    }
    else
    {
        SiiRegWriteBlock(REG_TPI__IF_DBYTE0, &pFrame[IF_CHECKSUM_INDEX], REG_TPI__IF_DBYTE27 - REG_TPI__IF_DBYTE0 + 1);
    }

    if (isHotSwap)
    {
        // Set the real packet ID (title)
        SiiRegWrite(REG_TPI__IF_TYPE, pFrame[0]);
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable/Disable HDCP protection.
//!
//! @param[in]  isEnabled - true, if HDCP protection has to be enabled.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiHdcpProtectionEnable(bool_t isEnabled)
{
    SiiRegModify(REG_TPI__HDCP_CTRL, HDCP_CTRL_MODE, isEnabled ? SET_BITS : CLEAR_BITS);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return HDCP Status byte.
//-------------------------------------------------------------------------------------------------

uint8_t SiiDrvTpiHdcpStatusGet(void)
{
    uint8_t hdcpStatus = SiiRegRead(REG_TPI__HDCP_QUERY);
    return hdcpStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Check if HDCP authentication Part 2 is done.
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTpiHdcpIsPart2Done(void)
{
    uint8_t hdcp_status = SiiDrvTpiHdcpStatusGet();
    return (PART2_DONE == (hdcp_status & PART2_DONE));
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return true if the link HDCP authentication status is good.
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTpiHdcpIsAuthenticationGood(void)
{
    uint8_t hdcpStatus = SiiRegRead(REG_TPI__HDCP_QUERY);
    bool_t isSuccess = true;

    switch (hdcpStatus & MSK_TPI__HDCP_QUERY__STATUS)
    {
        case VAL_TPI__HDCP_QUERY__STATUS_LOST:
        case VAL_TPI__HDCP_QUERY__STATUS_FAILED:
            isSuccess = false;
    }
    return isSuccess;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set dynamic or static Link Integrity mode.
//!
//!             If downstream repeater is discovered, the dynamic mode shall be enabled.
//!             It forces the chip logic to re-authenticate any time the incoming clock
//!             resolution changes. Dynamic mode works for non-repeater sink. However
//!             the authentication may take a little longer time.
//!
//! @param[in]  isEnabled - true for dynamic, false for static mode.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiHdcpDynamicAuthenticationEnable(bool_t isEnabled)
{
    SiiRegModify(REG_TPI__SYSTEM_CONTROL, BIT_TPI__SYSTEM_CONTROL__REAUTH_EN, isEnabled ? SET_BITS : CLEAR_BITS);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable/Disable HDCP encryption of HDMI data.
//!
//! @param[in]  isEnabled - true: to enable encryption if authentication succeeded,
//!                         false: to disable encryption (even after successful authentication).
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiHdcpEncriptionEnable(bool_t isEnable)
{
    SiiRegModify(REG_TPI__HDCP_CTRL, BIT_TPI__HDCP_CTRL__ENCRYPT_DISABLE, isEnable ? CLEAR_BITS : SET_BITS);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return true if Sink HDCP is available, false otherwise.
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTpiIsDownstreamHdcpAvailable(void)
{
    return ((SiiRegRead(REG_TPI__HDCP_QUERY) & BIT_TPI__HDCP_QUERY__SINK_CAPABLE) != 0);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Read BKSV that is 8*5 = 40 bits.
//!
//! @param[in]  pBksv - pointer to an array to store the BKSV.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiBksvGet(uint8_t aBksv[5])
{
    SiiRegReadBlock(REG_TPI__BKSV_1, aBksv, 5);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return a number of bytes in the KSV list FIFO ready to be read and Done flag.
//!
//!             To be used in repeater operation when DS KSV list is copied from DS to US.
//!
//! @param[out] pBytesToRead - pointer to a variable holding the number of bytes to read,
//!
//! @return     KSV List reading done flag.
//! @retval     true - if a final portion of the KSV list is awaiting to be read.
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTpiKsvListPortionSizeGet(uint8_t *pBytesToRead)
{
    bool_t isDone;
    uint8_t fifoStatus = SiiRegRead(REG_TPI__HDCP_FIFO_LEVEL);

    *pBytesToRead = fifoStatus & MSK_TPI__HDCP_FIFO_LEVEL;
    isDone = ((fifoStatus & BIT_TPI__HDCP_FIFO_LEVEL__LAST) != 0);

    return isDone;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get DS BKSV list.
//!
//! @param[in]  pBuffer - pointer to a buffer for the KSV list storage,
//! @param[in]  length  - number of bytes to read.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiKsvListGet(uint8_t *pBuffer, uint8_t length)
{
    // Note: this FIFO register is a special case. While reading from it in burst mode
    // the slave I2C interface don't increment the offset after every single reading.
    // Content of the register gets immediately updated by data waiting in the FIFO
    // after every reading.
    SiiRegReadBlock(REG_TPI__HDCP_FIFO_DATA, pBuffer, length);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Checks if zero-length KSV list has been "provided" from downstream.
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvTpiIsZeroKsvListReady(void)
{
    uint8_t devCount;
    uint8_t IsRepStateMachineDone;

    // Repeater state machine is done the KSV list processing if status is set to a magic word 0x66
    // SWWA: 21471
    IsRepStateMachineDone = (SiiRegRead(REG_TPI__RPT_SM) == 0x66);

    if (IsRepStateMachineDone)
    {
        // Get number of downstream devices included into KSV List
        devCount = SiiRegRead(REG_TPI__HDCP_BSTATUS1) & MSK_HDCP_DDC__BSTATUS1__DEV_COUNT;
        if (devCount == 0)
        {
            return true;
        }
    }

    return false;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get DS BSTATUS information.
//!
//! @param[out] pDsBStatus - pointer to a buffer of length 2
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiBStatusGet(uint8_t *pDsBStatus)
{
    SiiRegReadBlock(REG_TPI__HDCP_BSTATUS1, pDsBStatus, 2);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Get DS BCAPS information.
//!
//! @return     BCAPS status flags.
//-------------------------------------------------------------------------------------------------

uint8_t  SiiDrvTpiBcapsGet(void)
{
    return SiiRegRead(REG_TPI__HDCP_BCAPS);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set register value of HDCP Ri_128_COMP
//!
//! @param[in]  value - need to set to register Ri_128_COMP
//!
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiHdcpRi128Comp(uint8_t value)
{
	SiiDrvTpiIndirectWrite(REG_TX__Ri_128_COMP, value);
}

void SiiDrvTpiSetPlugStatus(bool_t hpd, bool_t rxsense)
{
    pTpi->plugStatus = 0;
    if(hpd)
    {
        pTpi->plugStatus |= BIT1;
    }
    if(rxsense)
    {
        pTpi->plugStatus |= BIT0;
    }
}
