//************************************************************************************************
//! @file   si_edid_tx_internal.h
//! @brief  Silicon Image EDID Tx Component internal definitions
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/

#ifndef SI_EDIDTX_INTERNAL_H
#define SI_EDIDTX_INTERNAL_H


#include "string.h"
#include "si_common.h"
#include "si_edid_tx_component.h"
#include "si_edid_tx_config.h"
//#include "si_device_config.h"
//#include "si_debug.h"
#include "si_edid_tx_parsing.h"
#include "si_edid_tx_analyzer.h"
#include "si_edid_tx_fmf.h"
#include "si_edid_tx_tables.h"
#include "si_video_tables.h"
#include "si_scaler_tables.h"
#include "si_edid_black_list.h"
#include "si_drv_tpi_ddc_access.h"

//-------------------------------------------------------------------------------------------------
// TX manifest constants
//-------------------------------------------------------------------------------------------------

#define EDID_TX_MSG_DBG                   (MSG_DBG | DBGF_TS | DBGF_CN), DBG_EDID_TX
#define EDID_TX_MSG_DBG_PLAIN             (MSG_DBG)
#define EDID_TX_MSG_ALWAYS                (MSG_ALWAYS | DBGF_CN), DBG_EDID_TX
#define EDID_TX_MSG_ALWAYS_PLAIN          (MSG_ALWAYS)

//-------------------------------------------------------------------------------------------------
//  The EDID TX component is able to process EDID data from multiple TXs. However, resulting
//  upstream EDID is always one. Therefore the component data is split into two categories:
//  common data (general configuration, upstream related data) and instance data sets
//  (copies of TX specific data)
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  EdidTx type definitions
//------------------------------------------------------------------------------

typedef struct SiiEdidConfig
{
    uint16_t    mfgId;          // Two byte manufacturer ID
    uint16_t    productId;      // Two byte product ID
    uint32_t    serialNumber;   // Four byte serial number (may be last 4 digits)
    uint8_t     mfgWeek;        // week manufactured
    uint8_t     mfgYear;        // year manufactured

                                // Limits of this device capabilities
    uint8_t     minVertHz;
    uint8_t     maxVertHz;
    uint8_t     minHorizKhz;
    uint8_t     maxHorizKhz;
    uint8_t     maxPixClock10Mhz;
    uint8_t     maxTmdsClock5Mhz;

} SiiEdidTxConfig_t;


//-------------------------------------------------------------------------------------------------
//  EdidTx Component Instance Data
//-------------------------------------------------------------------------------------------------

typedef struct EdidTxDecodData
{
    bool_t          isActive;          // true, if TX is connected to downstream and EDID can be read

    uint8_t         dtdOpt;            // DTD_OPT byte: must be the same for all extension blocks
    int_t           nmbOfDecodedExtBlocks;
    bool_t          isNotFirstDtd;     // set after processing DTD at the first position

#ifndef COLLECT_EXCESSIVE_INFO
    uint8_t         Revision;          // Version 1.X, where X is Revision
#endif // NOT COLLECT_EXCESSIVE_INFO

    svd_t           SVD[MAX_NMB_OF_SVDS];
    uint8_t         SVD_idx;

#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
    // HDMI 1.4's 3D
    uint16_t        hdmi3dMask;
    svd3dStruct_t   hdmi3dSvdStructure[HDMI_3D_SVD_STRUCTURE_LENGTH];
    bit_fld_t       isAll3dFormats : 1; // true: hdmi_3d_structure_all is for all formats
                                        // false: hdmi_3d_structure_all is only for hdmi3dMask VICs
    uint8_t         hdmi_vic[MAX_NMB_OF_HDMI_VIC_FORMATS];
#endif // SI_HDMI_1_4_EDID_PROCESSING

    decodedEdid_t   edidInfo;           // Contains decoded EDID information
    edidAnalysis_t  edidAnalysis;       // EDID Analyzer database

}   EdidTxDecodData_t;



typedef struct EdidTxInstanceData
{
    txEdidErrCode_t     lastResultCode;       // Contains the result of the last API function called
    uint16_t            statusFlags;

    uint8_t             numOfTx;              // Total number of serviceable TX channels
    uint8_t             numOfActiveTx;        // Current number of connected TXs

    // Pointer to currently selected TX EDID decoding data. There are numOfTx instances of EdidTxDecodData
    // structures within one instance of EdidTxInstanceData structure. Memory allocation for this
    // is done during EDID TX component configuration.
    EdidTxDecodData_t  *pEdidDecodData;

    parsingOptions_t    parseOptions;
    SiiEdidTxConfig_t   edidConfig;           // Contains configuration info from SiiEdidTxConfigure()

    uint8_t             usEdidBlockCount;     // Number of blocks in US EDID (maximum 128 in current version)

    bit_fld_t           isSoundBarMode  : 1;  // true if EDID processed for sound bar mode
    bit_fld_t           isDecodeAudio   : 1;
    bit_fld_t           isDecodeNonAudio: 1;

    bit_fld_t           isDeepColor3036 : 1;  // Set if repeater is able to support Deep Color 30 and 36 bit
    bit_fld_t           isDeepColorOvrd : 1;  // Set if DS DC settings to be overridden by the repeater
    bit_fld_t           isXvYccProfile0 : 1;  // xvYCC profile P0
    bit_fld_t           isSpdifOnly     : 1;  // Leave only audio modes that can pass through SPDIF
    bit_fld_t           isHdmi3DEnabled : 1;  // Allow 3D formats

    decodedEdid_t       edidInfo;             // Contains common decoded EDID information

}  EdidTxInstanceData_t;

extern EdidTxInstanceData_t *pEdidTx;


//-------------------------------------------------------------------------------------------------
//  Internal component EDID Analyzer functions
//-------------------------------------------------------------------------------------------------

void        EdidTxAnalyze( void );
void        EdidTxImageSizeGet(decodedEdid_t *pEdidInfo, bool_t isAspectRatio16x9, uint16_t *pVert, uint16_t *pHoriz);
void        EdidTxDsSdtdsGet(shortDtd_t p_sdtds[]);

//-------------------------------------------------------------------------------------------------
//  Internal component EDID Composer functions
//-------------------------------------------------------------------------------------------------

void        EdidTxCompose ( void );
bool_t      EdidTxWriteBlockToRx(int block_nmb, uint8_t *p_edid, uint8_t cec_position);

//-------------------------------------------------------------------------------------------------
//  Internal component EDID Converter functions
//-------------------------------------------------------------------------------------------------

void        EdidTxConvert( void );
void        EdidTxUsCecPhysAddrDiscovery(cecPhysAddr_t *p_cec, uint8_t input_nmb);
void        EdidTxOwnSadAndSpkSet( void );

//-------------------------------------------------------------------------------------------------
//  Internal component EDID Decoder functions
//-------------------------------------------------------------------------------------------------

bool_t        EdidTxDecodeEdid( void );
void        EdidTxParseBlock0( uint8_t p_block[EDID_BLOCK_SIZE] );
void        EdidTxParseBlock( uint8_t p_block[EDID_BLOCK_SIZE], int blockIndex );
void        EdidTxAddDviVideoModes(void);
void        EdidTxPostDecode(void);
bool_t      EdidTxCheckBlockCheckSum(uint8_t pBlock[EDID_BLOCK_SIZE]);
void        EdidTxDecodeDtd (uint8_t *p_dtd);
bool_t      EdidTxReadBlock( int blockIndex, uint8_t *pEdidBlock );

//-------------------------------------------------------------------------------------------------
//  Internal component EDID Scaler functions
//-------------------------------------------------------------------------------------------------

void        EdidTxScalerConvert( void );

#endif // SI_EDIDTX_INTERNAL_H
