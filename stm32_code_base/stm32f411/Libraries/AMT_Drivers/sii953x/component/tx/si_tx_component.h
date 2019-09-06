//************************************************************************************************
//! @file   si_tx_component.h
//! @brief  Transmitter software component
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#ifndef SI_TX_COMPONENT_H
#define SI_TX_COMPONENT_H

#include "si_common.h"
#include "si_tx_config.h"

#include "si_info_frames.h"         // Info Frame related definitions
#include "si_video_tables.h"        // Video format related definitions


//-------------------------------------------------------------------------------------------------
// TX Component type definitions
//-------------------------------------------------------------------------------------------------

//! TX error codes for lastErrorCode in txError_t structure
typedef enum txErrCode
{
    SI_TX_ERROR_CODE_NO_ERROR                 = 0x00, //!< Success
    SI_TX_ERROR_CODE_INSTANCE_NOT_EXIST       = 0x01, //!< Non-existing instance record index provided
    SI_TX_ERROR_CODE_INVALID_PARAMETER        = 0x02, //!< Parameter of a function has prohibited value
    SI_TX_ERROR_CODE_INCOMPATIBLE_POWER_MODE  = 0x03, //!< Current power mode prevented operation
    SI_TX_ERROR_CODE_HDMI_NOT_SUPPORTED_BY_DS = 0x04, //!< Downstream Device doesn't support HDMI mode
    SI_TX_ERROR_CODE_INVALID_VIDEO_SOURCE     = 0x05, //!< Video source of given type is not supported
    SI_TX_WARN_CODE_NO_WARNING                = 0x06, //!< Empty warning (border marker)
    SI_TX_WARN_CODE_POWER_MODE_ALREADY_SET    = 0x07, //!< Attempt to reset the same power mode
    SI_TX_TOTAL_NUM_OF_ERROR_CODES,

} txErrCode_t;

                                //! TX output modes
typedef enum txHdmiMode
{
    SI_TX_OUTPUT_MODE_DVI       = 0x0,
    SI_TX_OUTPUT_MODE_HDMI      = 0x1

} txHdmiMode_t;

                                //! Feedback Messages
typedef enum txFeedbackMsg
{
    SI_TX_FB_MSG_NONE                   = 0x00, //!< Default (void) message
    SI_TX_FB_MSG_HDCP_AUTH_DONE         = 0x01, //!< HDCP Authentication successfully done
    SI_TX_FB_MSG_HOT_PLUG_CONNECTED     = 0x02, //!< Hot Plug is detected (device may still be in standby)
    SI_TX_FB_MSG_HOT_PLUG_DISCONNECTED  = 0x03, //!< Cable disconnected
    SI_TX_FB_MSG_HOT_PLUG_RECONNECT     = 0x04, //!< high->low->high pulse on HPD line (fast reconnect)
    SI_TX_FB_MSG_SINK_READY             = 0x05, //!< Downstream device is connected and ready to accept A/V
    SI_TX_FB_MSG_ERR_HDCP_AUTH_FAILED   = 0xE1, //!< HDCP Authentication failure
    SI_TX_FB_MSG_ERR_HDCP_OTHER         = 0xE2  //!< Other HDCP related problem. @see HDCP_ERRORS

} txFeedbackMsg_t;

                                //! Input Video Source options
typedef enum txVideoSource
{
    SI_TX_VIDEO_SOURCE_VBUS             = 0x00, //!< Video bus
    SI_TX_VIDEO_SOURCE_HDMI             = 0x01, //!< HDMI->[internal bus, no conversion]->HDMI
    SI_TX_VIDEO_SOURCE_PGEN             = 0x02  //!< Internal Pattern Generator

} txVideoSource_t;


                                //! Input bus color space modes
typedef enum txVbusColSpace
{
    SI_TX_VBUS_COLOR_SPACE_RGB          = 0x0,
    SI_TX_VBUS_COLOR_SPACE_YCBCR444     = 0x1,
    SI_TX_VBUS_COLOR_SPACE_YCBCR422     = 0x2

} txVbusColSpace_t;

                                //! Color depth
typedef enum txVbusColDepth
{
    SI_TX_VBUS_BITS_PER_COLOR_8         = 0x0,
    SI_TX_VBUS_BITS_PER_COLOR_10        = 0x1,
    SI_TX_VBUS_BITS_PER_COLOR_12        = 0x2,
    SI_TX_VBUS_BITS_PER_COLOR_16        = 0x3,
    SI_TX_VBUS_BITS_PER_COLOR_LENGTH            //!< Number of different values in the type

} txVbusColDepth_t;

                                //! Input Video Clock usage fashion
typedef enum txVbusClockMode
{
    SI_TX_VBUS_CLOCK_NORMAL             = 0x0,  //!< Latch 1 pixel on rising OR falling edges
    SI_TX_VBUS_CLOCK_DOUBLE_RATE        = 0x1,  //!< Clock frequency is doubled as compared to Normal
    SI_TX_VBUS_CLOCK_LATCH_BOTH_EDGES   = 0x2   //!< Latch 1/2 pixel on rising AND falling edges

} txVbusClockMode_t;


                                //! Pixel repetition
typedef enum txVbusPixRep
{
    SI_TX_VIDEO_SRC_PIXEL_REP_1         = 0x0,  //!< No repetition
    SI_TX_VIDEO_SRC_PIXEL_REP_2         = 0x1,  //!< send 1 pixel 2 times
    SI_TX_VIDEO_SRC_PIXEL_REP_3         = 0x2,
    SI_TX_VIDEO_SRC_PIXEL_REP_4         = 0x3,
    SI_TX_VIDEO_SRC_PIXEL_REP_5         = 0x4,
    SI_TX_VIDEO_SRC_PIXEL_REP_6         = 0x5,
    SI_TX_VIDEO_SRC_PIXEL_REP_7         = 0x6,
    SI_TX_VIDEO_SRC_PIXEL_REP_8         = 0x7,
    SI_TX_VIDEO_SRC_PIXEL_REP_9         = 0x8,
    SI_TX_VIDEO_SRC_PIXEL_REP_10        = 0x9

} txVbusPixRep_t;


                                //! Colorimetry standard
typedef enum txVbusColorimetry
{
    SI_TX_VIDEO_COLORIMETRY_NO_INFO     = 0x0,
    SI_TX_VIDEO_COLORIMETRY_ITU601      = 0x1,
    SI_TX_VIDEO_COLORIMETRY_ITU709      = 0x2,
    SI_TX_VIDEO_COLORIMETRY_EXTENDED    = 0x3, // if extended, but unknown
    SI_TX_VIDEO_COLORIMETRY_XV601       = 0xA,
    SI_TX_VIDEO_COLORIMETRY_XV709       = 0xB

} txVbusColorimetry_t;

                                //! RGB quantization range
typedef enum txVbusQuantRange
{
    SI_TX_VIDEO_QRANGE_DEFAULT  = 0x0,
    SI_TX_VIDEO_QRANGE_LIMITED  = 0x1,
    SI_TX_VIDEO_QRANGE_FULL     = 0x2

} txVbusQuantRange_t;


                                //! Wired video synchronization signals (not embedded into data)
typedef enum syncSignals
{
    SI_TX_VIDEO_HVSYNC_DE   = 0x0, //!< HSYNC, VSYNC and DE are present
    SI_TX_VIDEO_HVSYNC      = 0x1, //!< HSYNC and VSYNC are present
    SI_TX_VIDEO_DE          = 0x2, //!< Only DE is present
    SI_TX_VIDEO_NONE        = 0x3  //!< All synchronization signals are embedded

} syncSignals_t;

                                //! 3D structure variants
typedef enum video3dFormat
{
    SI_TX_VIDEO_3D_FRAME_PACKING        = 0x0, //!< Vertical ribbon: L_odd, R_odd, L_even, R_even (one frame)
    SI_TX_VIDEO_3D_FIELD_ALTERNATIVE    = 0x1, //!< Vertical ribbon: L_odd, R_odd, L_even, R_even (two frames)
    SI_TX_VIDEO_3D_LINE_ALTERNATIVE     = 0x2, //!< Line intermittent: 1L, 1R, 2L, 2R, ... (one frame)
    SI_TX_VIDEO_3D_SIDE_BY_SIDE_FULL    = 0x3, //!< Horizontal ribbon: L, R (one frame)
    SI_TX_VIDEO_3D_L_PLUS_DEPTH         = 0x4, //!< Vertical ribbon: L, depth (one frame)
    SI_TX_VIDEO_3D_L_PLUS_DEPTH_GFX     = 0x5, //!< Vertical ribbon: L, depth, GFX, GFX_depth (one frame)

    SI_TX_VIDEO_3D_SIDE_BY_SIDE_HALF_HORIZ_OL_OR    = 0x08,//!< Horizontal sub-sampling: Odd L, Odd R
    SI_TX_VIDEO_3D_SIDE_BY_SIDE_HALF_HORIZ_OL_ER    = 0x18,//!< Horizontal sub-sampling: Odd L, Even R
    SI_TX_VIDEO_3D_SIDE_BY_SIDE_HALF_HORIZ_EL_OR    = 0x28,//!< Horizontal sub-sampling: Even L, Odd R
    SI_TX_VIDEO_3D_SIDE_BY_SIDE_HALF_HORIZ_EL_ER    = 0x38,//!< Horizontal sub-sampling: Even L, Even R
    SI_TX_VIDEO_3D_SIDE_BY_SIDE_HALF_QUINX_OL_OR    = 0x08,//!< Quincunx matrix: Odd L, Odd R
    SI_TX_VIDEO_3D_SIDE_BY_SIDE_HALF_QUINX_OL_ER    = 0x18,//!< Quincunx matrix: Odd L, Even R
    SI_TX_VIDEO_3D_SIDE_BY_SIDE_HALF_QUINX_EL_OR    = 0x28,//!< Quincunx matrix: Even L, Odd R
    SI_TX_VIDEO_3D_SIDE_BY_SIDE_HALF_QUINX_EL_ER    = 0x38,//!< Quincunx matrix: Even L, Even R

    SI_TX_VIDEO_3D_NONE                 = 0xFF //!< Non 3D mode

} video3dFormat_t;


                                //! Audio Source type
typedef enum txAudioSrc
{
    SI_TX_AUDIO_SRC_NONE    = 0x0,  //!< Input audio is disabled
    SI_TX_AUDIO_SRC_I2S_L0  = 0x1,  //!< One I2S line, layout 0, PCM or encoded
    SI_TX_AUDIO_SRC_I2S_L1  = 0x2,  //!< Up to 4 I2S lines, layout 1, PCM
    SI_TX_AUDIO_SRC_HBRA    = 0x3,  //!< 4 I2S lines, High Bit Rate Audio
    SI_TX_AUDIO_SRC_DSD_L0  = 0x4,  //!< 2 channels DSD, layout 0
    SI_TX_AUDIO_SRC_DSD_L1  = 0x5,  //!< Up to 8 DSD channels, layout 1
    SI_TX_AUDIO_SRC_SPDIF   = 0x6,  //!< SPDIF input

} txAudioSrc_t;


                                //! I2S Master Clock multiplier factors
typedef enum i2sMstClk
{
    SI_TX_AUDIO_I2S_MCLK_MULT_128   = 0x0,
    SI_TX_AUDIO_I2S_MCLK_MULT_256   = 0x1,
    SI_TX_AUDIO_I2S_MCLK_MULT_384   = 0x2,
    SI_TX_AUDIO_I2S_MCLK_MULT_512   = 0x3,
    SI_TX_AUDIO_I2S_MCLK_MULT_768   = 0x4,
    SI_TX_AUDIO_I2S_MCLK_MULT_1024  = 0x5,
    SI_TX_AUDIO_I2S_MCLK_MULT_1152  = 0x6,
    SI_TX_AUDIO_I2S_MCLK_MULT_192   = 0x7

} i2sMstClk_t;

                                //! Edge of the audio sampling clock
typedef enum i2sSckEdge
{
    SI_TX_AUDIO_I2S_SCK_EDGE_FALLING = 0x0,
    SI_TX_AUDIO_I2S_SCK_EDGE_RISING  = 0x1

} i2sSckEdge_t;

                                //! Polarity of WS signal
typedef enum i2sWsPolar
{
    SI_TX_AUDIO_I2S_WS_LOW  = 0x0,
    SI_TX_AUDIO_I2S_WS_HIGH = 0x1

} i2sWsPolar_t;

                                //! Justification of the audio data
typedef enum i2sSdJust
{
    SI_TX_AUDIO_I2S_SD_JUSTIF_LEFT  = 0x0,
    SI_TX_AUDIO_I2S_SD_JUSTIF_RIGHT = 0x1

} i2sSdJust_t;

                                //! Direction of SD
typedef enum i2sSdDir
{
    SI_TX_AUDIO_I2S_SD_DIR_MSB = 0x0,
    SI_TX_AUDIO_I2S_SD_DIR_LSB = 0x1

} i2sSdDir_t;

                                //! WS to SD first bit shift
typedef enum i2sSdBitShft
{
    SI_TX_AUDIO_I2S_SD_BIT0_SHIFTED     = 0x0,
    SI_TX_AUDIO_I2S_SD_BIT0_NOT_SHIFTED = 0x1

} i2sSdBitShft_t;

                                //! Audio encoding type
typedef enum audioEncoding
{
    SI_TX_AUDIO_ENC_UNKNOWN = 0x0,
    SI_TX_AUDIO_ENC_PCM     = 0x1,
    SI_TX_AUDIO_ENC_AC3     = 0x2,
    SI_TX_AUDIO_ENC_MPEG1   = 0x3,
    SI_TX_AUDIO_ENC_MPEG2   = 0x4,
    SI_TX_AUDIO_ENC_MP3     = 0x5,
    SI_TX_AUDIO_ENC_AAC     = 0x6,
    SI_TX_AUDIO_ENC_DTS     = 0x7,
    SI_TX_AUDIO_ENC_ATRAC   = 0x8,
    SI_TX_AUDIO_ENC_OBA     = 0x9,
    SI_TX_AUDIO_ENC_DDPLUS  = 0xa,
    SI_TX_AUDIO_ENC_DTSHD   = 0xb,
    SI_TX_AUDIO_ENC_MAT     = 0xc,
    SI_TX_AUDIO_ENC_DST     = 0xd,
    SI_TX_AUDIO_ENC_WMA     = 0xe

} audioEncoding_t;

//! Audio sample rate
typedef enum audioSampleRate
{
    SI_TX_AUDIO_SMP_RATE_UNKNOWN    = 0x0,
    SI_TX_AUDIO_SMP_RATE_32_KHZ     = 0x1,
    SI_TX_AUDIO_SMP_RATE_44_KHZ     = 0x2,
    SI_TX_AUDIO_SMP_RATE_48_KHZ     = 0x3,
    SI_TX_AUDIO_SMP_RATE_88_KHZ     = 0x4,
    SI_TX_AUDIO_SMP_RATE_96_KHZ     = 0x5,
    SI_TX_AUDIO_SMP_RATE_176_KHZ    = 0x6,
    SI_TX_AUDIO_SMP_RATE_192_KHZ    = 0x7

} audioSampleRate_t;

                                //! Audio sample size
typedef enum audioSampleSize
{
    SI_TX_AUDIO_SMP_SIZE_UNKNOWN    = 0x0,
    SI_TX_AUDIO_SMP_SIZE_16_BIT     = 0x1,
    SI_TX_AUDIO_SMP_SIZE_20_BIT     = 0x2,
    SI_TX_AUDIO_SMP_SIZE_24_BIT     = 0x3

} audioSampleSize_t;

                                //! Speaker allocation
                                //! Acronyms:
                                //! FL   Front Left
                                //! FC   Front Center
                                //! FR   Front Right
                                //! FLC  Front Left Center
                                //! FRC  Front Right Center
                                //! RL   Rear Left
                                //! RC   Rear Center
                                //! RR   Rear Right
                                //! RLC  Rear Left Center
                                //! RRC  Rear Right Center
                                //! LFE  Low Frequency Effect
typedef enum audioSpkAlloc
{
    SI_TX_AUDIO_SPK_FL_FR                       = 0x00,
    SI_TX_AUDIO_SPK_FL_FR_LFE                   = 0x01,
    SI_TX_AUDIO_SPK_FL_FR_FC                    = 0x02,
    SI_TX_AUDIO_SPK_FL_FR_LFE_FC                = 0x03,
    SI_TX_AUDIO_SPK_FL_FR_RC                    = 0x04,
    SI_TX_AUDIO_SPK_FL_FR_LFE_RC                = 0x05,
    SI_TX_AUDIO_SPK_FL_FR_FC_RC                 = 0x06,
    SI_TX_AUDIO_SPK_FL_FR_LFE_FC_RC             = 0x07,
    SI_TX_AUDIO_SPK_FL_FR_RL_RR                 = 0x08,
    SI_TX_AUDIO_SPK_FL_FR_LFE_RL_RR             = 0x09,
    SI_TX_AUDIO_SPK_FL_FR_FC_RL_RR              = 0x0A,
    SI_TX_AUDIO_SPK_FL_FR_LFE_FC_RL_RR          = 0x0B,
    SI_TX_AUDIO_SPK_FL_FR_RL_RR_RC              = 0x0C,
    SI_TX_AUDIO_SPK_FL_FR_LFE_RL_RR_RC          = 0x0D,
    SI_TX_AUDIO_SPK_FL_FR_FC_RL_RR_RC           = 0x0E,
    SI_TX_AUDIO_SPK_FL_FR_LFE_FC_RL_RR_RC       = 0x0F,
    SI_TX_AUDIO_SPK_FL_FR_RL_RR_RLC_RRC         = 0x10,
    SI_TX_AUDIO_SPK_FL_FR_LFE_RL_RR_RLC_RRC     = 0x11,
    SI_TX_AUDIO_SPK_FL_FR_FC_RL_RR_RLC_RRC      = 0x12,
    SI_TX_AUDIO_SPK_FL_FR_LFE_FC_RL_RR_RLC_RRC  = 0x13,
    SI_TX_AUDIO_SPK_FL_FR_FLC_FRC               = 0x14,
    SI_TX_AUDIO_SPK_FL_FR_LFE_FLC_FRC           = 0x15,
    SI_TX_AUDIO_SPK_FL_FR_FC_FLC_FRC            = 0x16,
    SI_TX_AUDIO_SPK_FL_FR_LFE_FC_FLC_FRC        = 0x17,
    SI_TX_AUDIO_SPK_FL_FR_RC_FLC_FRC            = 0x18,
    SI_TX_AUDIO_SPK_FL_FR_LFE_RC_FLC_FRC        = 0x19,
    SI_TX_AUDIO_SPK_FL_FR_FC_RC_FLC_FRC         = 0x1A,
    SI_TX_AUDIO_SPK_FL_FR_LFE_FC_RC_FLC_FRC     = 0x1B,
    SI_TX_AUDIO_SPK_FL_FR_RL_RR_FLC_FRC         = 0x1C,
    SI_TX_AUDIO_SPK_FL_FR_LFE_RL_RR_FLC_FRC     = 0x1D,
    SI_TX_AUDIO_SPK_FL_FR_FC_RL_RR_FLC_FRC      = 0x1E,
    SI_TX_AUDIO_SPK_FL_FR_LFE_FC_RL_RR_FLC_FRC  = 0x1F


} audioSpkAlloc_t;


                                //! TX Power modes
typedef enum txPowerMode
{
    SI_TX_POWER_MODE_UNKNOWN = 0x00,
    SI_TX_POWER_MODE_OFF     = 0x01,
    SI_TX_POWER_MODE_STANDBY = 0x02,
    SI_TX_POWER_MODE_FULL    = 0x03

} txPowerMode_t;


                                //! Status
typedef struct txStatus
{
    bit_fld_t isHotPlugDetected : 1; //!< true if HPD line current state is High
    bit_fld_t isRxSensed        : 1; //!< true if powered Rx is attached
    bit_fld_t isDsConnected     : 1; //!< debounced status of downstream connection
    bit_fld_t isNewConnection   : 1; //!< true if new Hot Plug event is detected
    bit_fld_t isSinkReady       : 1; //!< true if DS is considered ready to receive A/V
    bit_fld_t isSameDsConnected : 1; //!< true if newly connected downstream is the same as previous one
    bit_fld_t isIrqPending      : 1; //!< true if there is a pending interrupt request

} txStatus_t;

                                //! Last result (error state)
typedef struct txError
{
    uint_t      errorCount;     //!< Indicates how many errors have occurred
    txErrCode_t lastErrorCode;  //!< code of the last error

} txError_t;

                                //! TX/TPI Revision
typedef struct txRevision
{
    uint16_t deviceId;          //!< Transmitter chip ID code
    uint8_t  component;         //!< Tx component revision
    uint8_t  tpi;               //!< TPI revision

} txRevision_t;

                                //! Input Video bus Mode
typedef struct txVideoBusMode
{
    txVbusColSpace_t    colorSpace;         //!< Input color space
    txVbusColDepth_t    bitsPerColor;       //!< Bit depth
    txVbusPixRep_t      pixelRepetition;    //!< Pixel repetition factor of incoming video
    txVbusColorimetry_t colorimetry;        //!< Colorimetry standard
    txVbusQuantRange_t  quantRange;         //!< RGB/YCbCr quantization range

    //! Video data latching on raising/falling edge of the video clock signal.
    //! For SI_TX_VBUS_CLOCK_LATCH_BOTH_EDGES clock mode this flag indicates
    //! which edge is latched first.
    bool_t              isLatchOnRisingEdge;
    txVbusClockMode_t   clockMode;          //!< Video data clocking mode

} txVideoBusMode_t;

                                //! Video format descriptor
typedef struct videoFormat
{
    uint16_t pixelFrequency;    //!< Pixel frequency in tens of kHz
    uint8_t  verticalFrequency; //!< Vertical frame rate in Hz
    uint16_t pixelsPerLine;     //!< Total number of pixels per line
    uint16_t linesPerFrame;     //!< Total number of lines in a frame

} videoFormat_t;


                                //! Defining the way how synchronization signals (VSYNC, HSYNC, DE)
                                //! are extracted or regenerated from video data stream
typedef struct syncRegen
{
    //! VBIT to VSYNC adjustment. Valid values: -1,0,1
    int8_t   adjustVbitToVsync;

    //! Setting this to true (recommended) enables detection circuits to locate
    //! the position of VSYNC relative to HSYNC and only include HSYNC edges
    //! that are greater than 3/4 lines from VSYNC in the line count for DE_TOP.
    bool_t   isVsyncAdjusted;

    //! Set to true when the 656 Flag bit is opposite the standard polarity
    //! for Field1 and Field2. Inverting the field polarity causes the sync
    //! extraction to format HSYNC and VSYNC properly based on the F bit.
    //! In embedded sync mode, the HDMI Transmitter does not detect even
    //! from odd field, except based on the setting of the F bit.
    bool_t   isFieldPolarityInverted;

    //! Set the delay from the detection of an EAV sequence (H bit change from 1 to 0)
    //! to the active edge of HSYNC. The unit of measure is pixels. Valid range: 1-1023
    uint16_t hbitToHsyncDelayPix;

    //! Set VSYNC pixel offset for the odd field of an interlaced source.
    //! It is recommended to set this to half the number of pixels/line.
    uint16_t oddFieldVsyncOffsetPix;

    //! Width of the HSYNC pulses. Set this register to the desired HSYNC pulse width.
    //! The unit of measure is pixels. Valid range: 1-1023.
    uint16_t hsyncWidthPix;

    //! Delay between the time the V bit changes from 1 to 0 in an EAV sequence,
    //! to the asserting edge of VSYNC. The unit of measure is lines. Valid range: 1-63.
    uint8_t  vbitToVsyncDelayLn;

    //! Width of VSYNC pulse. The unit of measure is lines. Valid range: 1-63.
    uint8_t  vsyncWidthLn;

    //! Set to true for negative and false for positive polarity of HSYNC.
    //! For embedded syncs, set this bit to the desired HSYNC polarity that is generated
    //! from the embedded sync codes.
    bool_t   isHsyncPolarityNeg;

    //! Set to true for negative and false for positive polarity of VSYNC.
    //! For embedded syncs, set this bit to the desired VSYNC polarity that is generated
    //! from the embedded sync codes.
    bool_t   isVsyncPolarityNeg;

    //! Width of the area to the left of the active display. The unit of measure is pixels.
    //! This register should be set to the sum of
    //! (HSYNC width) + (horizontal back porch) + (horizontal left border),
    //! and is used only for DE generation. Valid range: 1-1023.
    uint16_t deDelayPix;

    //! Height of the area above the active display. The unit of measure is lines (HSYNC pulses).
    //! This register should be set to the sum of
    //! (VSYNC width) + (vertical back porch) + (vertical top border). Valid range: 1-128.
    uint8_t  deTopLn;

    //! Width of the active display. The unit of measure is pixels. Set this register to the desired
    //! horizontal resolution. Valid range: 1-2047.
    uint16_t deWidthPix;

    //! Height of the active display. The unit of measure is lines (HSYNC pulses).
    //! Set this register to the desired vertical resolution. For interlaced modes,
    //! set this register to the number of lines per field, which is half the overall
    //! vertical resolution. Valid range: 1-2047.
    uint16_t deHeightLn;

} syncRegen_t;

                                //! Audio Source configuration descriptor
typedef struct txAudioSrcConfig
{
    i2sMstClk_t     mclkMult;           //!< Master clock frequency multiplier (Fmclk = M * Fs)
    i2sSckEdge_t    sampleEdge;         //!< Edge of the audio sampling clock
    i2sWsPolar_t    wsPolarity;         //!< Polarity of WS signal
    i2sSdJust_t     sdJustify;          //!< Justification of the audio data
    i2sSdDir_t      sdDirection;        //!< Direction of SD
    i2sSdBitShft_t  sdFirstBitShifted;  //!< WS to SD first bit shift

} txAudioSrcConfig_t;

                                //! Audio stream format descriptor
typedef struct audioFormat
{
    audioEncoding_t   encoding;       //!< Encoding of the audio
    uint8_t           channelCount;   //!< Number of channels. Valid Range: 2-8
    audioSampleRate_t smpRate;        //!< Sample rate
    audioSampleSize_t smpSize;        //!< Sample size
    audioSpkAlloc_t   spkMap;         //!< Speaker allocation code
    bit_fld_t         lShift     : 4; //!< Level shift value in dB. Valid range: 0-15 dB
    bit_fld_t         lfePbLevel : 4; //!< LFE Playback Level. Valid values: 0 - none, 1 - 0dB, 2 - +10dB
    bool_t            isDwnMxInhibit; //!< Downmix Inhibit flag (for DVD-Audio only)

    uint8_t           catCode;        //!< IEC-60958-3 Category Code. Indicates generating audio equipment.
    bit_fld_t         srcNumber  : 4; //!< IEC-60958-3 Source number. Valid range: 0-15
    bit_fld_t         clkAccuracy: 2; //!< IEC-60958-3 Sampling frequency clock accuracy. Valid Range: 0-3
    bit_fld_t         origSmpRate: 4; //!< IEC-60958-3 Original sampling frequency

} audioFormat_t;

                                //! Output Video configuration descriptor
typedef struct txVideoOutMode
{
    bool_t            isForceColorSpace; //!< Enable explicit setting of output color space

    //! Output color space. It has effect if (isForceColorSpace == true),
    //! otherwise the output color space is defined equal to input video color space setting
    txVbusColSpace_t  colorSpace;

    //! Enables or disables video dithering for video content
    //! in YCbCr 4:2:2 color space mode
    bool_t            isDitheringEnabled;

} txVideoOutMode_t;


//-------------------------------------------------------------------------------------------------
//  TX API functions
//-------------------------------------------------------------------------------------------------

// TX Configuration functions
void         SiiTxInitialize(void);
uint8_t      SiiTxInstanceGet(void);
void         SiiTxInstanceSet(uint8_t instance);
txStatus_t   SiiTxStatusGet(void);
txError_t    SiiTxGetLastResult(void);
txRevision_t SiiTxRevisionGet (void);
void         SiiTxOutputInterfaceSet(txHdmiMode_t mode);

// Power Control
void         SiiTxStandby(void);
void         SiiTxResume(void);
void         SiiTxShutDown(void);

// Task Dispatching
txFeedbackMsg_t SiiTxTaskProcess(time_ms_t elapsedTimeMs);
time_ms_t    SiiTxTaskRecallTimeMsGet(void);

// Input Audio and Video configuration
void        SiiTxVideoSourceConfig(txVideoSource_t source, txVideoBusMode_t *pBusMode);
void        SiiTxInputVideoModeSet(uint8_t stdFormatId, videoFormat_t *pFmt, video3dFormat_t v3dFormatId,
                            syncSignals_t syncSrc, syncRegen_t *pSynRg);

void        SiiTxAudioSourceConfig(txAudioSrc_t source, txAudioSrcConfig_t *pInterfaceSettings);
void        SiiTxInputAudioModeSet(txAudioSrc_t source,  audioFormat_t *pFmt);

// Audio controls
void        SiiTxInputAudioMute(void);
void        SiiTxInputAudioUnmute(void);

// Video interruption handling
void        SiiTxRecoverAfterSourceClockInterruption(void);

// Output Audio and Video configuration
void        SiiTxOutputVideoConfig(txVideoOutMode_t  *pMode);

// Output A/V control
void        SiiTxAvMute(void);
void        SiiTxAvUnmute(void);

// Info Frames and Auxiliary HDMI Packets
void        SiiTxInfoFrameSet(infoFrameType_t ifType, void *pFrame);
void *      SiiTxInfoFrameGet(infoFrameType_t ifType);
void        SiiTxInfoFrameEnable(infoFrameType_t ifType);
void        SiiTxInfoFrameDisable(infoFrameType_t ifType);


// Information from EDID (optional)
uint16_t    SiiTxPhysicalAddressGet(void);
//bool_t      SiiTxIsVideoModeSupported(uint8_t stdFormatId, videoFormat_t *pVideoMode);
//bool_t      SiiTxIsAudioModeSupported(audioFormat_t *pAudioMode);

// Reading Downstream EDID over DDC (interface for EDID component)
//void        SiiTxEdidGet(uint8_t *pEdidBuffer, uint8_t blockIndex);


// HDCP configuration and control
void        SiiTxHdcpConfigure(bool_t isRepeaterMode, uint_t umuteDelayMs, bool_t isNotifyRxOnDdcFailure);
void        SiiTxHdcpEnable(void);
void        SiiTxHdcpDisable(void);


// Callback function prototypes
// Definition of the callback functions shall be done on the application level
void        SiiTxCbHdcpUsOnlyAuthentication(void);
void        SiiTxCbHdcpDsDdcAck(bool_t isAcknowledged);
bool_t      SiiTxCbHdcpProcessDsBstatus(uint8_t dsBstatus[2]);
void        SiiTxCbHdcpFinishUsPart2(uint8_t aDsBksv[5]);
void        SiiTxCbHdcpProcessKsvListPortion(uint8_t *pKsvBuffer, uint8_t len);
void        SiiTxCbHdcpSetUsRepeaterBcaps(bool_t isFifoReady);
bool_t      SiiTxCbHdcpPrepareForKsvListProcessing(void);
void        SiiTxCbHdcpInformUsDeviceAboutFailure(bool_t isFatal);
bool_t      SiiTxCbHdcpPrepareFor0KsvListProcessing(void);


#endif // SI_TX_COMPONENT_H
