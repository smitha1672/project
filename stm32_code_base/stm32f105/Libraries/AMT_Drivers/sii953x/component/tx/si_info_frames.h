//************************************************************************************************
//! @file   si_info_frames.h
//! @brief  Generic definitions related to Info Frames
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#ifndef SI_INFO_FRAMES_H
#define SI_INFO_FRAMES_H


#define	IF_HEADER_LENGTH		4
#define	IF_TITLE_INDEX			0
#define	IF_VERSION_INDEX		1
#define	IF_LENGTH_INDEX		    2
#define	IF_CHECKSUM_INDEX		3
#define	IF_DATA_INDEX			IF_HEADER_LENGTH

#define IF_MAX_LENGTH           27  // maximum size of InfoFrame allowed by TPI

#define	IF_MIN_AVI_LENGTH       13  // the 861C standard defines the length as 13
#define	IF_MAX_AVI_LENGTH       15  // maximum allowed by the chip
#define	IF_MIN_AUDIO_LENGTH     10  // the 861C standard defines the length as 10
#define	IF_MAX_AUDIO_LENGTH     10  // maximum allowed by the chip
#define	IF_MIN_MPEG_LENGTH      10  // the 861C standard defines the length as 10
#define	IF_MAX_MPEG_LENGTH      IF_MAX_LENGTH
#define	IF_MIN_SPD_LENGTH       25  // the 861C standard defines the length as 25
#define	IF_MAX_SPD_LENGTH       IF_MAX_LENGTH
#define	IF_MIN_VSIF_LENGTH      4   // minimum length by HDMI 1.4
#define	IF_MAX_VSIF_LENGTH      IF_MAX_LENGTH



#define AUDIO_INFO_FRAME_LENGTH (IF_HEADER_LENGTH + IF_MAX_AUDIO_LENGTH)


                        //! Info Frame type.
                        //! Numbering comes from HDMI Specification.
                        //! IDs starting from 8 are standard info frames
                        //! defined in the CEA861D.
typedef enum
{
    SI_INFO_FRAME_AVI     = 0x82,        //!< Auxiliary Video IF
    SI_INFO_FRAME_AUDIO   = 0x84,        //!< Audio IF
    SI_INFO_FRAME_SPD     = 0x83,        //!< Source Product Descriptor IF
    SI_INFO_FRAME_MPEG    = 0x85,        //!< MPEG IF is not recommended for use
    SI_INFO_FRAME_ISRC1   = 0x05,        //!< International Standard Recording Codes 1 IF
    SI_INFO_FRAME_ISRC2   = 0x06,        //!< International Standard Recording Codes 2 IF
    SI_INFO_FRAME_ACP     = 0x04,        //!< Audio Content Protection IF
    SI_INFO_FRAME_GBD     = 0x0a,        //!< Gamut Boundary Descriptor IF
    SI_INFO_FRAME_VSIF    = 0x81         //!< Vendor Specific 3D IF

} infoFrameType_t;


                        //! ACP Info Frame types.
typedef enum
{
    SI_ACP_GENERIC_AUDIO    = 0,
    SI_ACP_IEC60958_AUDIO   = 1,
    SI_ACP_DVD_AUDIO        = 2,
    SI_ACP_SUPER_AUDIO_CD   = 3

} acpInfoFrameType_t;

#endif // SI_INFO_FRAMES_H

