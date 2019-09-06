//************************************************************************************************
//! @file   si_edid_tx_component.c
//! @brief  Silicon Image EDID Tx Component
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/

#include "string.h"
#include <stdio.h>
#include "si_edid_tx_component.h"
#include "si_edid_tx_internal.h"
#include "si_tx_config.h"
#include "si_scaler_tables.h"
#include "si_video_tables.h"
#include "si_cbus_internal.h"
#include "si_cbus_config.h"
#include "si_tx_component.h"

#if (MHL_20 == ENABLE)
extern MHL3DSupportData_t gMHL3D_VIC[SII_NUM_EDID_TX];
extern MHL3DSupportData_t gMHL3D_DTD[SII_NUM_EDID_TX];
extern MHL3DSupportData_t  *pMHL3D_VIC;
extern MHL3DSupportData_t  *pMHL3D_DTD;
#endif


//------------------------------------------------------------------------------
//  EdidTx Component Instance Data
//------------------------------------------------------------------------------

EdidTxDecodData_t    edidDecodData[SII_NUM_TX];
EdidTxInstanceData_t edidTxInstance[SII_NUM_EDID_TX];
EdidTxInstanceData_t *pEdidTx = &edidTxInstance[0];


//-------------------------------------------------------------------------------------------------
//  Local service functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Print EDID TX related error messages.
//!
//! @param[in]  errors - error mask. @see TX_EDID_ERROR_MASKS
//-------------------------------------------------------------------------------------------------

static void PrintErrors(uint32_t errors)
{
    if (errors)
    {
        DEBUG_PRINT(EDID_TX_MSG_DBG, ("EDID Errors: "));
        if (errors & EDID_ERROR__INCORRECT_VERSION)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("INCORRECT VERSION "));
        if (errors & EDID_ERROR__CHECK_SUM_MISMATCH)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("CHECK SUM MISMATCH "));
        if (errors & EDID_ERROR__INCORRECT_HEADER)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("INCORRECT HEADER "));
        if (errors & EDID_ERROR__DEFAULT_EDID_USED)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("DEFAULT EDID USED "));
        if (errors & EDID_ERROR__CANNOT_READ_BL0)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("CANNOT READ BL0 "));
        if (errors & EDID_ERROR__CANNOT_READ_EXT)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("CANNOT READ EXT "));
        if (errors & EDID_ERROR__INCORRECT_MAP_TAG)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("INCORRECT MAP TAG "));
        if (errors & EDID_ERROR__TOO_MANY_EXTENSIONS)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("TOO MANY EXTENSIONS "));
        if (errors & EDID_ERROR__EXT_BLOCK_VIOLATION)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("EXT BLOCK VIOLATION "));
        if (errors & EDID_ERROR__BLOCK_0_VIOLATION)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("BLOCK 0 VIOLATION "));
        if (errors & EDID_ERROR__DATA_COLLECT_VIOLATION)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("DATA COLLECTION VIOLATION "));
        if (errors & EDID_ERROR__NOT_ENOUGH_ROOM)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("NOT ENOUGH ROOM "));
        if (errors & EDID_ERROR__FOUND_UNSUPPORTED_DTD)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("FOUND UNSUPPORTED DTD "));
        if (errors & EDID_ERROR__DTD_VIOLATION)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("DTD VIOLATION "));
        if (errors & EDID_ERROR__INTERNAL_ERROR)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("INTERNAL ERROR "));
        if (errors & EDID_ERROR__CANNOT_WRITE)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("CANNOT WRITE "));
        if (errors & EDID_ERROR__RANGE_LIMITATION)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("RANGE LIMITATION "));
        if (errors & EDID_ERROR__NO_SPACE_FOR_DBC)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("NO SPACE FOR DBC "));
        if (errors & EDID_ERROR__INCORRECT_AR_IN_DTD_FIX)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("INCORRECT AR IN DTD FIX "));
        if (errors & EDID_ERROR__RANGE_LIMITS_CORRECTED)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("RANGE LIMITS CORRECTED "));
        if (errors & EDID_ERROR__INCORRECT_PICTURE_SIZE)
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("INCORRECT PICTURE SIZE "));
        DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("\n"));
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Print a list of features supported by DS.
//!
//-------------------------------------------------------------------------------------------------

static void PrintFeatures(void)
{
    DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("DS Device Supports: "));
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_HDMI))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("HDMI "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_AI))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("AI "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_ENCODED_AUDIO))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("Encoded_Audio "));}
    else if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_AUDIO))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("PCM_Audio "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_1BIT_AUDIO))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("DSD "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_HBR_AUDIO))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("HBRA "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_YCbCr))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("YCbCr "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_NTSC))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("NTSC "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_PAL))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("PAL "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_AR_16x9))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("16x9 "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_AR_4x3))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("4x3 "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_DEEP_COLOR_30_BIT))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("DC30 "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_DEEP_COLOR_36_BIT))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("DC36 "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_DEEP_COLOR_48_BIT))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("DC48 "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_DEEP_COLOR_YCbCr_444))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("DC_Y444 "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_XVYCC_P0))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("xvYCC "));}
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_3D))
        {DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("3D "));}
    DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, ("\n"));

    if (SiiEdidTxDsDeviceRemedyMaskGet())
        DEBUG_PRINT(EDID_TX_MSG_DBG, "DS Remedy Mask: 0x%04X\n", (int) SiiEdidTxDsDeviceRemedyMaskGet());

}



//------------------------------------------------------------------------------
// Function:    EdidFixDecodedEdid
// Description: Modifies decoded EDID information in the parsing structure filled
//              by the EdidTxDecodeEdid() function.  Fixes some common
//              problems found when reading downstream EDIDs.
//------------------------------------------------------------------------------

static void EdidFixDecodedEdid ( void )
{
    bool_t use_default_cec_pa = false;

    // Attempt to recover CEC PA when DS EDID is not readable

    if ( pEdidTx->pEdidDecodData->edidInfo.ErrorCode & EDID_ERROR__DEFAULT_EDID_USED )
    {
#if (SI_USE_CEC == ENABLE)
        uint16_t old_pa = SiiCecGetDevicePA();

        if ((0 != old_pa) && (0xFFFF != old_pa))
        {
            DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, "Attempt to recover CEC PA- old CEC PA is used\n" );
            parsing->HDMI_VSDB.cec.a = (old_pa >> 12) & 0x0F;
            parsing->HDMI_VSDB.cec.b = (old_pa >> 8) & 0x0F;
            parsing->HDMI_VSDB.cec.c = (old_pa >> 4) & 0x0F;
            parsing->HDMI_VSDB.cec.d = old_pa & 0x0F;
        }
        else
#endif // SI_USE_CEC
        {
            use_default_cec_pa = true;
        }
    }

    // 080403
    // A fix for downstream DVI devices.
    if (0 == pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.cec.a)
        use_default_cec_pa = true;

    if (use_default_cec_pa)
    {
        DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, "CEC PA is defaulted to 1000\n" );
        pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.cec.a = 1;
        pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.cec.b = 0;
        pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.cec.c = 0;
        pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.cec.d = 0;
    }

    // 080403
    // A fix for DVI devices declaring audio or other HDMI features.
    // Example: Dell W1900 DVI Monitor declares audio support.
    if ( !pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.HdmiMode )
    {
        pEdidTx->pEdidDecodData->edidInfo.SupportAudio = false;
        pEdidTx->pEdidDecodData->edidInfo.SupportYCbCr = false;
        //memset(pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.Extras, 0, sizeof(pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.Extras));
        pEdidTx->pEdidDecodData->edidInfo.SadCount = 0;
    }
}





//-------------------------------------------------------------------------------------------------
//  EDID TX API functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Returns index of an instance selected in context
//!
//! @return     Index of an instance record
//-------------------------------------------------------------------------------------------------

uint8_t SiiEdidTxInstanceGet(void)
{
    return (pEdidTx - &edidTxInstance[0]);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Brings an instance referred by given index into current context.
//!
//! @param[in]  instance - index of an instance record
//-------------------------------------------------------------------------------------------------

void SiiEdidTxInstanceSet(uint8_t instance)
{
#if (SII_NUM_EDID_TX > 1)
    if (instance < SII_NUM_EDID_TX)
    {
        pEdidTx = &edidTxInstance[instance];
        pEdidTx->instanceIndex = instance;
#if (MHL_20 == ENABLE)
        pMHL3D_VIC = &gMHL3D_VIC[instance];
        pMHL3D_DTD = &gMHL3D_DTD[instance];
#endif
    }
#endif
}


//-------------------------------------------------------------------------------------------------
//! @brief      Switch EDID input to come from hooked TX component.
//!
//! @param[in]  input - EDID TX input accepting the EDID data from TX. Should be different
//!                     for different TX instances in multi-input model.
//! @return     success flag.
//-------------------------------------------------------------------------------------------------

bool_t SiiEdidTxInputSet(uint8_t input)
{
    bool_t isSuccess = true;

    pEdidTx->lastResultCode = SI_TX_EDID_ERROR_CODE_NO_ERROR;

    if(input < pEdidTx->numOfTx)
    {
        pEdidTx->pEdidDecodData = &edidDecodData[input];
    }
    else
    {
        pEdidTx->lastResultCode = SI_TX_EDID_ERROR_CODE_BAD_INPUT_INDEX;
        isSuccess = false;
    }

    return isSuccess;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Configures the EDID TX component to operate with the feature set
//!             of the system video and audio, as well as sets system EDID options.
//!
//! @param[in]  numOfTx          - Number of transmitters providing EDID per the component instance
//! @param[in]  videoFeatureMask - Video option mask. @see TX_EDID_VIDEO_CFG_MASKS
//! @param[in]  audioFeatureMask - Audio option mask. @see TX_EDID_AUDIO_CFG_MASKS
//! @param[in]  miscOptionsMask  - Miscellaneous EDID option mask. @see TX_EDID_MISC_CFG_MASKS
//!
//! @return     true if the configuration was successful, or false if some
//!             failure occurred
//-------------------------------------------------------------------------------------------------

bool_t SiiEdidTxConfigure ( uint8_t  numOfTx, uint16_t videoFeatureMask,
                            uint16_t audioFeatureMask, uint16_t miscOptionsMask)
{
    pEdidTx->numOfTx = numOfTx;

    // Limits of this device
    if (videoFeatureMask & SI_TX_EDID_VIDEO_CFG_ENABLE_MHL_LIMITS)
    {
        pEdidTx->edidConfig.maxHorizKhz = MHL__MAX_H_KHZ, // Max Horiz. Freq
        pEdidTx->edidConfig.minHorizKhz = MHL__MIN_H_KHZ, // Min Horiz. Freq
        pEdidTx->edidConfig.maxVertHz = MHL__MAX_V_HZ, // Max Vertical Freq
        pEdidTx->edidConfig.minVertHz = MHL__MIN_V_HZ, // Min Vertical Freq
        pEdidTx->edidConfig.maxPixClock10Mhz = MHL__MAX_PIX_CLK_10MHZ; // Pixel Clock in 10MHz
    }
    else
    {
        pEdidTx->edidConfig.maxHorizKhz = VIDEO__MAX_H_KHZ, // Max Horiz. Freq
        pEdidTx->edidConfig.minHorizKhz = VIDEO__MIN_H_KHZ, // Min Horiz. Freq
        pEdidTx->edidConfig.maxVertHz = VIDEO__MAX_V_HZ, // Max Vertical Freq
        pEdidTx->edidConfig.minVertHz = VIDEO__MIN_V_HZ, // Min Vertical Freq
        pEdidTx->edidConfig.maxPixClock10Mhz = VIDEO__MAX_PIX_CLK_10MHZ; // Pixel Clock in 10MHz
    }


    pEdidTx->usEdidBlockCount   = 2;

    pEdidTx->isSoundBarMode     = (0 != (miscOptionsMask & SI_TX_EDID_CFG_OPTIONS_SOUNDBAR_MODE));
    pEdidTx->isHdmi3DEnabled    = (0 != (videoFeatureMask & SI_TX_EDID_VIDEO_CFG_ENABLE_3D));
    pEdidTx->isXvYccProfile0    = (0 != (videoFeatureMask & SI_TX_EDID_VIDEO_CFG_ENABLE_XVYCC));

    pEdidTx->parseOptions.AllowCheckSumMismatchForBlock0 = (0 != (videoFeatureMask & SI_TX_EDID_VIDEO_CFG_ALLOW_BAD_CSUM_FOR_BLOCK0));
    pEdidTx->parseOptions.AllowPcModesOnScalerBypass = (0 != (videoFeatureMask & SI_TX_EDID_VIDEO_CFG_ALLOW_PC_MODES_NO_SCALER));
    pEdidTx->parseOptions.BypassScaler  = (0 != (videoFeatureMask & SI_TX_EDID_VIDEO_CFG_BYPASS_SCALER ));
    pEdidTx->parseOptions.OutIdx        = SII_PREFERRED_OUT_IDX;
    pEdidTx->parseOptions.AudioBypass   = (0 != (audioFeatureMask & SI_TX_EDID_AUDIO_CFG_BYPASS));
    pEdidTx->parseOptions.DviInput      = (0 != (miscOptionsMask & SI_TX_EDID_CFG_OPTIONS_FORCE_DVI_INPUT));
    pEdidTx->parseOptions.enableRemedies    = (0 != (videoFeatureMask & SI_TX_EDID_VIDEO_CFG_ENABLE_REMEDIES));

    pEdidTx->isDeepColor3036      = (0 != (videoFeatureMask & SI_TX_EDID_VIDEO_CFG_DC30_36));
    pEdidTx->isDeepColorOvrd      = (0 != (videoFeatureMask & SI_TX_EDID_VIDEO_CFG_DC_OVRD));
    pEdidTx->isSpdifOnly          = (0 != (audioFeatureMask & SI_TX_EDID_AUDIO_CFG_SPDIF_INPUT));

    return( true );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Returns a status flag word containing EdidTx-specific
//!             information about the state of the device.
//! @return     Sixteen-bit status flags word for the EdidTx Component
//-------------------------------------------------------------------------------------------------

uint16_t SiiEdidTxStatusGet ( void )
{

    return( pEdidTx->statusFlags );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Places the EdidTx component into standby mode if available
//!
//!             This is a dummy call. EDID component doesn't control any hardware
//!             directly.
//!
//! @return     Success flag.
//-------------------------------------------------------------------------------------------------

bool_t SiiEdidTxStandby ( void )
{

    return( true );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Causes operation of the EdidTx component to return to the state
//!             it was prior to the previous call to SiiEdidTxStandby.
//!             Dummy call.
//-------------------------------------------------------------------------------------------------

bool_t SiiEdidTxResume ( void )
{

    return( true );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Returns the result of the last SiiEdidTxxxx function called.
//!
//-------------------------------------------------------------------------------------------------

txEdidErrCode_t SiiEdidTxGetLastResult ( void )
{

    return( pEdidTx->lastResultCode );
}


//-------------------------------------------------------------------------------------------------
//! @brief      EDID Tx component initialization.
//!
//! @return     Success flag.
//-------------------------------------------------------------------------------------------------

bool_t SiiEdidTxInitialize ( void )
{
    memset(pEdidTx, 0, sizeof(EdidTxInstanceData_t));

    // By default one-TX configuration is set
    pEdidTx->numOfTx = 1;
    pEdidTx->pEdidDecodData = &edidDecodData[0];
    memset(pEdidTx->pEdidDecodData, 0, sizeof(EdidTxDecodData_t));
    pEdidTx->pEdidDecodData->isActive = false;

    // Credentials of this device
    pEdidTx->edidConfig.mfgId = EDID__MANUFACTURER_ID; // Two byte manufacturer ID
    pEdidTx->edidConfig.productId = EDID__PRODUCT_ID; // Two byte product ID
    pEdidTx->edidConfig.serialNumber = 0; // Four byte serial number (may be last 4 digits)
    pEdidTx->edidConfig.mfgWeek = SET_MANUFACTURER_WEEK; // week manufactured
    pEdidTx->edidConfig.mfgYear = SET_MANUFACTURER_YEAR; // year manufactured

    // Default limits of this device
    pEdidTx->edidConfig.maxHorizKhz = DEFAULT__MAX_H_KHZ, // Max Horiz. Freq
    pEdidTx->edidConfig.minHorizKhz = DEFAULT__MIN_H_KHZ, // Min Horiz. Freq
    pEdidTx->edidConfig.maxVertHz = DEFAULT__MAX_V_HZ, // Max Vertical Freq
    pEdidTx->edidConfig.minVertHz = DEFAULT__MIN_V_HZ, // Min Vertical Freq
    pEdidTx->edidConfig.maxPixClock10Mhz = DEFAULT__MAX_PIX_CLK_10MHZ; // Pixel Clock in 10MHz
    pEdidTx->edidConfig.maxTmdsClock5Mhz = DEFAULT__MAX_TMDS_CLK_5MHZ; // maxTmdsClock in 5Mhz units

    return (true);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Initialize the EDID decode info structure
//!
//!             Call before starting SiiEdidTxProcessEdid().
//!
//-------------------------------------------------------------------------------------------------

static void EdidTxInitProcessing ( void )
{
    memset( &pEdidTx->pEdidDecodData->edidInfo, 0, sizeof( decodedEdid_t ));
    pEdidTx->pEdidDecodData->SVD_idx    = 0;
    pEdidTx->pEdidDecodData->nmbOfDecodedExtBlocks = 0;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Creates new EDID data for upstream based on all active TX EDIDs.
//!             Call upon TX connection or disconnection.
//-------------------------------------------------------------------------------------------------

static void EdidTxCreateUsEdid(void)
{
    uint8_t i;
    bool_t  isFirstActiveTx = true;
    EdidTxDecodData_t *pEdidDecodData;

    DEBUG_PRINT(EDID_TX_MSG_DBG_PLAIN, "DS EDIDs Attached: %d\n", pEdidTx->numOfActiveTx);

    // If more than one parsed EDID input exists, merge them into a "greatest common denominator" data
    // in order to leave only those capability declarations that are supported by all
    // active transmitters
    for (i = 0; i < pEdidTx->numOfTx; i++)
    {
        pEdidDecodData = &edidDecodData[i];
        if (pEdidDecodData->isActive)
        {
            if (!isFirstActiveTx)
            {
                // Apply the Feature Meet Filter (FMF)
                EdidTxFeatureMeetFilter(&(pEdidDecodData->edidInfo));
            }
            else
            {
                // Copy first active edidInfo into output edidInfo
                memcpy(&pEdidTx->edidInfo, &(pEdidDecodData->edidInfo), sizeof(decodedEdid_t));
                // Make sure other active TXs added through the FMF
                isFirstActiveTx = false;
            }
        }
    }

    EdidTxConvert();
    PrintErrors( pEdidTx->edidInfo.ErrorCode );

    EdidTxCompose();
    PrintErrors( pEdidTx->edidInfo.ErrorCode );
}

#if (MHL_20 == ENABLE)

extern  uint8_t g3DEdidTableVIC [ EDID_3D_VIC_TABLE_LEN ];
extern  uint8_t g3DEdidTableDTD [ EDID_3D_DTD_TABLE_LEN ];

void CreateMHL3DData(bool_t isDismissEDID)
{
	uint8_t tot;
	uint8_t i, j;
	uint8_t mhl_3d_all;
	uint8_t unsupported_vic[2] = {16, 31};
	uint16_t hdmi_3d_structure_all = pEdidTx->edidInfo.HDMI_VSDB.hdmi_3d_structure_all;

	memset( pMHL3D_VIC, 0, sizeof( MHL3DSupportData_t ));
	memset( pMHL3D_DTD, 0, sizeof( MHL3DSupportData_t ));

	//DEBUG_PRINT(MSG_ALWAYS, "\n\nCreateMHL3DData:: \n\n");

	// DTD
	// Nothing to do at EDID process

    if(isDismissEDID)
    {
        uint8_t *pg3DEdidTableVIC;
        uint8_t *pg3DEdidTableDTD;
        uint16_t idx = 0;

        pg3DEdidTableVIC = g3DEdidTableVIC;
        pMHL3D_VIC->tot = pg3DEdidTableVIC[3];

        for (i=0; i<5; i++)
        {
            pg3DEdidTableVIC += 5;  // move to number of 2 byte entries in a WRITE_BURST
            tot = *pg3DEdidTableVIC++; // Read number of 2 byte entries in this WRITE_BURST
            for (j=0; j<tot; j++)
            {
                pMHL3D_VIC->vdi[idx].vdi_h = *pg3DEdidTableVIC++;
                pMHL3D_VIC->vdi[idx++].vdi_l = *pg3DEdidTableVIC++;
            }
        }

        pg3DEdidTableDTD = g3DEdidTableDTD;
        pMHL3D_DTD->tot = pg3DEdidTableDTD[3];

        for (i=0; i<2; i++)
        {
            *pg3DEdidTableDTD += 5;  // move to number of 2 byte entries in a WRITE_BURST
            tot = *pg3DEdidTableDTD++; // Read number of 2 byte entries in this WRITE_BURST
            for (j=0; j<tot; j++)
            {
                pMHL3D_DTD->vdi[idx].vdi_h = *pg3DEdidTableDTD++;
                pMHL3D_DTD->vdi[idx++].vdi_l = *pg3DEdidTableDTD++;
            }
        }

        return;
    }

	// VIC
	pMHL3D_VIC->tot = pEdidTx->edidInfo.numOfVic;
	tot = pMHL3D_VIC->tot;
	DEBUG_PRINT(MSG_ALWAYS, "pMHL3D_VIC->tot: %d %x\n", pMHL3D_VIC->tot ,hdmi_3d_structure_all);

	// Add 3D structure and Masked mode
	if (hdmi_3d_structure_all)
	{
        mhl_3d_all = (hdmi_3d_structure_all&BIT8)>>6 |(hdmi_3d_structure_all&BIT6)>>5 | (hdmi_3d_structure_all&BIT0);
        //mhl_3d_all = (hdmi_3d_structure_all&BIT8)>>6 |(hdmi_3d_structure_all&BIT6)>>5;  // Frame Sequential is not supported

		if (pEdidTx->edidInfo.HDMI_VSDB.mask_3d_support){
			for (i=0; i<tot; i++){
				if (pEdidTx->edidInfo.SDTD[i].Valid){
					if (pEdidTx->edidInfo.SDTD[i].masked_3d){
						pMHL3D_VIC->vdi[i].vdi_l= mhl_3d_all;
					}
					else	{
						pMHL3D_VIC->vdi[i].vdi_l= 0;
					}
				}
			}
		}
		else{
			for (i=0; i<tot; i++)
				pMHL3D_VIC->vdi[i].vdi_l= mhl_3d_all;
		}
	}

	// Add individual 3D mode
	for (i=0; i<tot; i++)
	{
		uint8_t indivLength = pEdidTx->edidInfo.SDTD[i].indiv3dModeLength;

		if (pEdidTx->edidInfo.SDTD[i].Valid && (indivLength != 0))
		{
			uint8_t j;
			Indiv3dMode_t *pIndiv3d = pEdidTx->edidInfo.SDTD[i].indiv3dModes;
			for (j=0; j<indivLength; j++){
				if (!pIndiv3d[j].struct_3d_present) continue;
				switch (pIndiv3d[j].struct_3d)
				{
					case 0:  // Frame packing
						pMHL3D_VIC->vdi[i].vdi_l |= BIT_FS_SUPP;  // Frame Sequential is not supported
						break;
					case 6:  // Top-and-Bottom
						pMHL3D_VIC->vdi[i].vdi_l |= BIT_TB_SUPP;
						break;
					case 8:  // Side-by-Side(half)
						pMHL3D_VIC->vdi[i].vdi_l |= BIT_LR_SUPP;
						break;
				}
			}
		}
	}

	// Remove a bit of unsupported format
	for (i=0; i<tot; i++)
	{
		for (j=0; j<sizeof(unsupported_vic); j++)
			if (pEdidTx->edidInfo.SDTD[i].Vic == unsupported_vic[j]) 
				pMHL3D_VIC->vdi[i].vdi_l = 0;
	}
}
#endif


//-------------------------------------------------------------------------------------------------
//! @brief      Decode, analyze, convert input EDID, and compose a new EDID.
//!
//!             This function shall be called upon downstream HPD event.
//!
//! @param[in]  input - EDID TX input accepting the EDID data from TX. Should be different
//!                     for different TX instances in multi-input model.
//-------------------------------------------------------------------------------------------------

void SiiEdidTxProcessEdid ( uint8_t input )
{
    if (SiiEdidTxInputSet(input))
    {
        if (!pEdidTx->pEdidDecodData->isActive)
        {
            pEdidTx->numOfActiveTx++;
            // Activate instance when EDID processing is requested
            memset(pEdidTx->pEdidDecodData, 0, sizeof(EdidTxDecodData_t));
            pEdidTx->pEdidDecodData->isActive = true;
        }

        EdidTxInitProcessing();
        if(EdidTxDecodeEdid())
        {
            SiiTxSetBadEdidTrig(false);
        }
        else
        {
            SiiTxSetBadEdidTrig(true);
        }
        SiiDrvTpiDdcErrorsPrint();
        EdidFixDecodedEdid();

        EdidTxAnalyze();
        PrintErrors(pEdidTx->pEdidDecodData->edidInfo.ErrorCode);

        // Print a list of supported features
        PrintFeatures();

        // Make US EDID based on already parsed data, if any, and the newly attached DS EDID
        EdidTxCreateUsEdid();

#if (MHL_20 == ENABLE)
        CreateMHL3DData(false);    // BUGID 30677 - Melbourne protocol-systems testing is failing
#endif
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Remove one of the input EDIDs and re-compose US EDID.
//!
//!             This function shall be called upon downstream disconnection event.
//!
//! @param[in]  input - EDID TX input accepting the EDID data from TX. Should be different
//!                     for different TX instances in multi-input model.
//-------------------------------------------------------------------------------------------------

void SiiEdidTxDismissEdid( uint8_t input )
{
    if (SiiEdidTxInputSet(input))
    {
        pEdidTx->lastResultCode = SI_TX_EDID_ERROR_CODE_NO_US_EDID_CREATED;

#if ( MHL_20 == ENABLE ) /*Smith modifies*/
        CreateMHL3DData(true);
#endif         

        if (pEdidTx->pEdidDecodData->isActive)
        {
////            if (pEdidTx->numOfActiveTx > 0)// for melbourne we have only one TX.
////            {
//                pEdidTx->numOfActiveTx--;
//            //}
            // Deactivate instance when TX is disconnected
            pEdidTx->pEdidDecodData->isActive = false;


            // Re-make US EDID based on already parsed data if at least one active DS EDID is left
//            if (pEdidTx->numOfActiveTx > 0)
//            {
//                EdidTxCreateUsEdid();
//                pEdidTx->lastResultCode = SI_TX_EDID_ERROR_CODE_NO_ERROR;
//                return;
//            }
        }

    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return CEC physical address of downstream device.
//!
//-------------------------------------------------------------------------------------------------

uint16_t SiiEdidTxCecPhysAddrGet(void)
{

    return  ((((uint16_t) pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.cec.a) << 12) |
             (((uint16_t) pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.cec.b) << 8)  |
             (((uint16_t) pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.cec.c) << 4)  |
             ((uint16_t)  pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.cec.d));

}


//-------------------------------------------------------------------------------------------------
//! @brief      Read the specified single block from the specified EDID source.
//!
//! @param[in]  blockIndex - zero based EDID block number,
//! @param[in]  pEdidBlock - pointer to a buffer for the EDID block data
//!
//! @return     Success flag.
//-------------------------------------------------------------------------------------------------

bool_t EdidTxReadBlock ( int blockIndex, uint8_t *pEdidBlock )
{
    bool_t success = false;

    // NOTE: correct TX instance must be selected for the read to happen from the right DDC bus
    success = (SI_TX_DDC_ERROR_CODE_NO_ERROR == SiiDrvTpiDdcReadBlock( blockIndex / 2,
                                                        (blockIndex & 1) ? 128 : 0,
                                                        pEdidBlock, EDID_BLOCK_SIZE ));

    if ( success )
    {
        // Verify checksum
        if ( !EdidTxCheckBlockCheckSum( pEdidBlock ))
        {
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__CHECK_SUM_MISMATCH;
        }
    }

    return( success );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Write an EDID block prepared by the EDID Composer to Rx EDID memory that
//!             is exposed to the upstream.
//!
//!             IMPORTANT: This function requires pEdidBlock to point to 128 byte array (1 EDID block).
//!             Some bytes of this array are going to be modified (CEC PA and Check Sum).
//!
//! @param[in]  blockNumber - zero based EDID block number,
//! @param[in]  pEdidBlock  - pointer to a 128-byte size buffer of EDID block,
//!
//! @return     success flag.
//-------------------------------------------------------------------------------------------------

bool_t EdidTxWriteBlockToRx(int blockNumber, uint8_t *pEdidBlock, uint8_t cecPhysAddrLocation)
{
    bool_t status = SiiTxEdidCbBlockWrite(blockNumber, pEdidBlock, cecPhysAddrLocation);

    return( status );
}

