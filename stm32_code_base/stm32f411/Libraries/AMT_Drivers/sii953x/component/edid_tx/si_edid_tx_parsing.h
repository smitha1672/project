//************************************************************************************************
//! @file   si_edid_tx_parsing.h
//! @brief  Structure types and constant definitions for the EDID parser
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/


#ifndef SII_EDID_PARSING_H
#define SII_EDID_PARSING_H

#include "si_common.h"
#include "si_edid_tx_defs.h"
#include "si_cbus_config.h"

//#define COLLECT_EXCESSIVE_INFO
//#define COLLECT_EXTENDED_DATA_BLOCKS

#define MAX_NMB_OF_SADS            10 // 10*3=30 max < 2^5, where 5 is number of bits allocated for the length
#define MAX_NMB_OF_SVDS            32 // 2^5=32, where 5 is number of bits allocated for the length

#define SI_HDMI_1_4_EDID_PROCESSING ENABLE  // enables 4K2K and 3D resolutions during EDID processing
#define HDMI_3D_SVD_STRUCTURE_LENGTH 16     // maximum by HDMI 1.4 spec
#define MAX_INDIVIDUAL_3D_MODES_PER_VIC 3   // maximum number of individual 3D modes per VIC


#ifdef COLLECT_EXTENDED_DATA_BLOCKS
#define SIZE_OF_UNKNOWN_AUDIO_EXTENDED_BLOCKS_BUFFER        32
#define SIZE_OF_UNKNOWN_VIDEO_EXTENDED_BLOCKS_BUFFER        32
#endif // COLLECT_EXTENDED_DATA_BLOCKS

#define MAX_NMB_OF_UNKNOWN_DTDS     3
// 4 DTDs could be placed at block 0 and 7 at block 1. Since currently EDIDs are
// 2 block maximum, it is total 11 DTDs, but minimum 2 of them should be used
// for descriptors: Range Limits and Monitor Name. Therefore, no more then
// 9 DTD blocks could be used with 2 block (1 page) DTD.
#define MAX_NMB_OF_KNOWN_DTDS       38 // as much as we have space for SVDs & HDMI VIC formats

#define MAX_NMB_OF_HDMI_VIC_FORMATS 7

//! @defgroup TX_EDID_ERROR_MASKS       EDID Error masks
//! @{
#define EDID_ERROR__OK                      0
#define EDID_ERROR__INCORRECT_VERSION       0x00000001
#define EDID_ERROR__CHECK_SUM_MISMATCH      0x00000002
#define EDID_ERROR__INCORRECT_HEADER        0x00000004
#define EDID_ERROR__DEFAULT_EDID_USED       0x00000008 //!< due to one of the errors, the decision to use a default EDID was made
#define EDID_ERROR__CANNOT_READ_BL0         0x00000010 //!< failed to read block 0
#define EDID_ERROR__CANNOT_READ_EXT         0x00000020 //!< failed to read an extension block
#define EDID_ERROR__INCORRECT_MAP_TAG       0x00000040 //!< map block tag is incorrect
#define EDID_ERROR__TOO_MANY_EXTENSIONS     0x00000080 //!< only 1 map block is supported by current version
#define EDID_ERROR__EXT_BLOCK_VIOLATION     0x00000100 //!< an error in extension block structure
#define EDID_ERROR__BLOCK_0_VIOLATION       0x00000200 //!< an error in block 0 structure (different from any other mentioned)
#define EDID_ERROR__DATA_COLLECT_VIOLATION  0x00000400 //!< data block collection in an extension block has a problem
#define EDID_ERROR__NOT_ENOUGH_ROOM         0x00000800 //!< not all data processed due to of some internal limitation
#define EDID_ERROR__FOUND_UNSUPPORTED_DTD   0x00001000 //!< found an unsupported structure in DTD block
#define EDID_ERROR__DTD_VIOLATION           0x00002000 //!< an error in DTD structure
#define EDID_ERROR__INTERNAL_ERROR          0x00004000 //!< an error in this program
#define EDID_ERROR__CANNOT_WRITE            0x00008000 //!< an error during EDID writing
#define EDID_ERROR__RANGE_LIMITATION        0x00010000 //!< some of DTDs or SDTDs were filtered out due to they are out of the Range Limit limitation
#define EDID_ERROR__NO_SPACE_FOR_DBC        0x00020000 //!< not enough room for Data Block Collection
#define EDID_ERROR__INCORRECT_AR_IN_DTD_FIX 0x00040000 //!< found a DTD complaint with CEA-861D but with wrong picture aspect ratio, which was corrected
#define EDID_ERROR__RANGE_LIMITS_CORRECTED  0x00080000 //!< Range Limits block was incorrect, but now it is corrected
#define EDID_ERROR__INCORRECT_PICTURE_SIZE  0x00100000 //!< The picture sizes in DTDs were not complied with HDMI standard but now it is corrected
//
//! @}

//! @defgroup TX_EDID_AUDIO_CODING_MASKS       Audio encoding masks
//! @{
#define AUDIO_CODING__LPCM      1  //!< Linear PCM (e.g., IEC60958)
#define AUDIO_CODING__AC3       2  //!< AC-3
#define AUDIO_CODING__MPEG1     3  //!< MPEG1 (Layers 1 & 2)
#define AUDIO_CODING__MP3       4  //!< MP3 (MPEG1 Layer 3)
#define AUDIO_CODING__MPEG2     5  //!< MPEG2 (multichannel)
#define AUDIO_CODING__AAC       6  //!< AAC
#define AUDIO_CODING__DTS       7  //!< DTS
#define AUDIO_CODING__ATRAC     8  //!< ATRAC
#define AUDIO_CODING__1BIT      9  //!< One Bit Audio
#define AUDIO_CODING__DDP       10 //!< Dolby Digital +
#define AUDIO_CODING__DTSHD     11 //!< DTS-HD
#define AUDIO_CODING__MLP       12 //!< MLP
//
//! @}

//! Short Audio Descriptor (SAD)
typedef union sad PACKED
{
    //uint32_t            Data;
    struct PACKED
    {
        uint8_t         Byte1;
        uint8_t         Byte2;
        uint8_t         Byte3;
    }        ByteData;
    struct PACKED
    {
        bit_fld_t       MaxChannel  : 3;
        bit_fld_t       Coding      : 4;
        bit_fld_t       Reserved    : 1;
        bit_fld_t       Fs32k       : 1;
        bit_fld_t       Fs44k       : 1;
        bit_fld_t       Fs48k       : 1;
        bit_fld_t       Fs88k       : 1;
        bit_fld_t       Fs96k       : 1;
        bit_fld_t       Fs176k      : 1;
        bit_fld_t       Fs192k      : 1;
    }        Common;
    struct PACKED
    {
        uint8_t         Reserved1;
        uint8_t         Reserved2;
        bit_fld_t       Width16bit  : 1;
        bit_fld_t       Width20bit  : 1;
        bit_fld_t       Width24bit  : 1;
    }        PCM;
    struct PACKED
    {
        uint8_t        Reserved1;
        uint8_t        Reserved2;
        uint8_t        MaxBitRate; // *8kHz
    }        Encoded1;
    struct PACKED
    {
        uint8_t        Reserved1;
        uint8_t        Reserved2;
        uint8_t        VendorSpec; // depend on codec vendor
    }        Encoded2;

} sad_t;


                                //! Short video descriptor (SVD)
typedef union svd PACKED
{
    uint8_t         Data;
    struct PACKED
    {
        bit_fld_t   Vic     : 7;
        bit_fld_t   Native  : 1;
    }        Fields;

} svd_t;


                                //! Speaker configuration
typedef union spkCfg PACKED
{
    uint8_t        Data;
    struct PACKED
    {
        bit_fld_t        FL_FR      : 1;
        bit_fld_t        LFE        : 1;
        bit_fld_t        FC         : 1;
        bit_fld_t        RL_RR      : 1;
        bit_fld_t        RC         : 1;
        bit_fld_t        FLC_FRC    : 1;
        bit_fld_t        RLC_RRC    : 1;
    }        Fields;

} spkCfg_t;


                                //! CEC Physical Address
typedef struct cecPhysAddr
{
    uint8_t    a;
    uint8_t    b;
    uint8_t    c;
    uint8_t    d;

} cecPhysAddr_t;


                                //! 3D video structures
typedef struct svd3dStruct
{
    uint8_t     vic_order; // 2D_VIC_order
    uint8_t     struct_3d; // 3D_Structure
    uint8_t     detail_3d; // 3D_Detail
    bit_fld_t   valid : 1;

} svd3dStruct_t;


//! HDMI Vendor Specific Data Block (VSDB)
typedef struct hdmiVsdb
{
#if ( configSII_DEV_953x_PORTING == 0 )
    cecPhysAddr_t   CEC;
#else
	cecPhysAddr_t   cec;
#endif 
    bit_fld_t       HdmiMode: 1; // true if HDMI Signature is detected

    // HDMI 1.3
    uint8_t         Extras[EDID_VSB_LEN__MAX_EXTRAS];

#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
    // HDMI 1.4's 3D
    uint16_t        hdmi_3d_structure_all;
    bit_fld_t       image_size      : 2;
    bit_fld_t       hdmi_3d_present : 1;
#if (MHL_20 == ENABLE)
    bit_fld_t       mask_3d_support : 1;
#endif
#endif // SI_HDMI_1_4_EDID_PROCESSING

} hdmiVsdb_t;


                                //! Detailed Timing Descriptor (DTD)
typedef struct dtd
{
    uint16_t    PixClk_10kHz;
    uint16_t    HAct_Pix;
    uint16_t    HBlank_Pix;
    uint16_t    VAct_Pix;
    uint16_t    VBlank_Pix;
    uint16_t    HSyncOffset;
    uint16_t    HSyncPulseWith;
    uint8_t     VSyncOffset;
    uint8_t     VSyncPulseWith;
    uint16_t    HImageSize;
    uint16_t    VImageSize;
    uint8_t     HBorder;
    uint8_t     VBorder;
    bit_fld_t   Interlaced      : 1; // Flag's bit 7
    bit_fld_t   DisplayType     : 3; // Flag's bits 0, 5, 6
    bit_fld_t   InterfaceType   : 4; // Flag's bits 1, 2, 3, 4
    bit_fld_t   Valid           : 1; // true if this DTS is filled and to be used, false otherwise
    bit_fld_t   PreferredTiming : 1; // true if this DTD was 1st in EDID and bit "Preferred Timing" in block 0 is set
            // Only one DTD could have this bit set.
} dtd_t;

typedef struct Indiv3dMode
{
    bit_fld_t    struct_3d_present   : 1;
    bit_fld_t    struct_3d           : 4;
    bit_fld_t    detail_3d           : 4;
} Indiv3dMode_t;
                                //! Replacement of dtd_t for known video modes
typedef struct shortDtd PACKED
{
    uint8_t     VideoIdx;
    uint8_t     Vic;
    bit_fld_t   Ar16x9          : 1;
    bit_fld_t   DisplayType     : 3;
    bit_fld_t   Valid           : 1;
    bit_fld_t   PreferredTiming : 1;
    bit_fld_t   Native          : 1; // set if corresponding VIC has "Native" attribute
#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
    // HDMI 1.4's 3D and HDMI VIC formats
    bit_fld_t    masked_3d           : 1;
    bit_fld_t    struct_3d_present   : 1;
    bit_fld_t    struct_3d           : 4;
    bit_fld_t    detail_3d           : 4;
    uint8_t     HdmiVic;

    // BZ#2591
    Indiv3dMode_t indiv3dModes[MAX_INDIVIDUAL_3D_MODES_PER_VIC];
    uint8_t     indiv3dModeLength;
#endif // SI_HDMI_1_4_EDID_PROCESSING

} shortDtd_t;


                                //! Decoded EDID data set
typedef struct decodedEdid
{
    uint32_t    ErrorCode;

#ifdef COLLECT_EXCESSIVE_INFO
    uint8_t     Revision; // Version 1.X, where X is Revision
#endif // COLLECT_EXCESSIVE_INFO
    uint16_t    ManufacturerId;
    uint16_t    ProductId;
#ifdef COLLECT_EXCESSIVE_INFO
    uint32_t    ShortSerialNumber;
#endif // COLLECT_EXCESSIVE_INFO
    uint8_t     ManufactureWeek;
    uint8_t     ManufactureYear;
    uint8_t     BasicDispayParameters[5];
    uint8_t     Chromaticity[10];
    uint8_t     EstablishedTiming1;
    uint8_t     EstablishedTiming2;
    uint8_t     ManufacturerReservedTiming;
    uint8_t     StandardTiming[16];

    // Data from the first extension block ---------------------------
    uint8_t     NativeFormatsNumber;
    bit_fld_t   SupportYCbCr        : 1;
    bit_fld_t   SupportAudio        : 1;
    bit_fld_t   SupportUnderscan    : 1;

    // Data from the data collection ---------------------------
    sad_t       SAD[MAX_NMB_OF_SADS];
    uint8_t     SadCount;

    spkCfg_t    SPK;

    hdmiVsdb_t  HDMI_VSDB;

#ifdef COLLECT_EXTENDED_DATA_BLOCKS
    uint8_t     UnknownAudioExtBlocks[SIZE_OF_UNKNOWN_AUDIO_EXTENDED_BLOCKS_BUFFER];
    uint8_t     UnknownAudioExt_Idx;
    uint8_t     UnknownVideoExtBlocks[SIZE_OF_UNKNOWN_VIDEO_EXTENDED_BLOCKS_BUFFER];
    uint8_t     UnknownVideoExt_Idx;
#endif // COLLECT_EXTENDED_DATA_BLOCKS

    // Data from DTDs and DTD descriptors ---------------------------
    // ASCII strings end with 0x0A if the text is less then DTD_ASCII_LENGTH, i.e. 13
    // and padded with 0x20
#ifdef COLLECT_EXCESSIVE_INFO
    uint8_t     AsciiSerialNumber[DTD_DESCRIPTOR_DATA_LENGTH]; // ASCII text with serial number
    uint8_t     AsciiName[DTD_DESCRIPTOR_DATA_LENGTH]; // ASCII text with monitor/TV name
    uint8_t     AsciiString[DTD_DESCRIPTOR_DATA_LENGTH]; // ASCII text string- suppose no more then 1 string will be used
    uint8_t     ColorPoint[DTD_DESCRIPTOR_DATA_LENGTH]; // the structure is not detailed due to it's hardly be used
    uint8_t     StandardTimingInDtd[DTD_DESCRIPTOR_DATA_LENGTH]; // extension of StandardTiming (hardly be used)- suppose one extension is enough
#endif // COLLECT_EXCESSIVE_INFO
    struct
    {
        uint8_t MinV_Hz; // Min. Vertical rate (for interlace this refers to field rate), in Hz.
        uint8_t MaxV_Hz; // Max. Vertical rate (for interlace this refers to field rate), in Hz.
        uint8_t MinH_kHz; // Min. Horizontal in kHz
        uint8_t MaxH_kHz; // Max. Horizontal in kHz,
        uint8_t MaxPixClk_10MHz; // Max. Supported Pixel Clock, in MHz/10, rounded
        uint8_t SecondaryTimingFormula[8]; // Secondary timing formula support
    } RangeLimits;
    struct
    {
#ifdef COLLECT_EXCESSIVE_INFO
        bit_fld_t   SerialNumber    : 1; // true if AsciiSerialNumber field is valid
        bit_fld_t   Name            : 1; // true if AsciiName field is valid
        bit_fld_t   String          : 1; // true if AsciiString field is valid
        bit_fld_t   ColorPoint      : 1; // true if ColorPoint field is valid
        bit_fld_t   StandardTiming  : 1; // true if StandardTimingInDtd field is valid
#endif // COLLECT_EXCESSIVE_INFO
        bit_fld_t   RangeLimits     : 1; // true if RangeLimits field is valid
    } DescriptorValid;
    dtd_t       DTD[MAX_NMB_OF_UNKNOWN_DTDS]; // data from decoded DTDs
    shortDtd_t  SDTD[MAX_NMB_OF_KNOWN_DTDS]; // data from the DTDs described in 861C

    uint16_t    VSize4x3;   // V size in mm for 3:4 aspect ratio video modes
    uint16_t    VSize16x9;  // V size in mm for 16:9 aspect ratio video modes

    uint16_t    RemedyMask; // a mask which tells on what to fix for a specified DS device

    bit_fld_t   SupportPal          : 1;
    bit_fld_t   SupportNtsc         : 1;
    bit_fld_t   SupportAspect4_3    : 1;
    bit_fld_t   SupportAspect16_9   : 1;
    //bit_fld_t    SupportAspectOther    : 1;
    bit_fld_t   isHdmiMode          : 1; // indicates HDMI (or DVI) mode
    bit_fld_t   ExtraVicAdded       : 1; // some video modes absent in DS EDID were added
    // it usually happens with 1 block EDIDs

    bit_fld_t   xv601               : 1; // xvYCC P0 ITU-BT.601 is supported
    bit_fld_t   xv709               : 1; // xvYCC P0 ITU-BT.709 is supported
#if (MHL_20 == ENABLE)
    uint8_t numOfVic;
#endif

} decodedEdid_t;


                                //! EDID parsing options
typedef struct parsingOptions
{
    bit_fld_t    AllowCheckSumMismatchForBlock0     : 1; // some of TVs have incorrect check sum at block 0
    //bit_fld_t    VgaSupportedByScaler             : 1; // if VGA mode is supported by Scaler
    bit_fld_t    BypassScaler                       : 1; // when Scaler is not used
    bit_fld_t    AllowPcModesOnScalerBypass         : 1; // when BypassScaler is selected and PC video modes are allowed to bypass
    bit_fld_t    AudioBypass                        : 1; // when set, audio is coming from repeater input to output without playing by the repeater, otherwise it is played by the repeater
    bit_fld_t    DviInput                           : 1; // set it to true when DVI (not HDMI) input is assumed
    uint8_t      OutIdx; // user selected output mode when scaler is used (index in the Video Table) (0xFF means "Auto Mode")
    bit_fld_t    enableRemedies                     : 1;

} parsingOptions_t;

#endif // SII_EDID_PARSING_H
