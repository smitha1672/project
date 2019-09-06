//***************************************************************************
//!file     si_rtpi_component.h
//!brief    Silicon Image RTPI Component header.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_RTPI_COMPONENT_H__
#define __SI_RTPI_COMPONENT_H__

#include "si_common.h"
#include "si_cfg_eeprom.h"

//------------------------------------------------------------------------------
//  Manifest Constants
//------------------------------------------------------------------------------
#define SII_NUM_RTPI	        1

#define RTPI_MAX_REGISTERS      256
#define RTPI_CMD_BUFFER_SIZE    32
#define RTPI_CMD_ADDR           0
#define RTPI_CMD_DATA           1


#define RP_L__TOTAL_REGS		0xB0

// fixed registers
#define RP_A__DEV_ID_L              0x00
#define RP_DEV_ID_L                         0x35
#define RP_A__DEV_ID_H              0x01
#define RP_DEV_ID_H                         0x95
#define RP_A__VERSION_L             0x02
#define RP_VERSION_L                        0x02
#define RP_A__VERSION_H             0x03
#define RP_VERSION_H                        0x03
#define RP_A__RTPI_REVISION         0x04
#define RP_RTPI_REVISION                    0x06
#define RP_A__MANUFACTURE_ID_1      0x05
#define RP_A__MANUFACTURE_ID_2      0x06
#define RP_A__PRODUCT_ID_1          0x07
#define RP_A__PRODUCT_ID_2          0x08
#define RP_A__SERIAL_NMB_1          0x09
#define RP_A__SERIAL_NMB_2          0x0A
#define RP_A__SERIAL_NMB_3          0x0B
#define RP_A__SERIAL_NMB_4          0x0C
#define RP_A__MANUFACTURE_WEEK      0x0D
#define RP_A__MANUFACTURE_YEAR      0x0E


#define EDID__MANUFACTURER_ID		0x05
#define EDID__MANUFACTURE_ID_2		0x06
#define EDID__PRODUCT_ID			0x07
#define EDID__PRODUCT_ID_2			0x08
#define EDID__SERIAL_NUMBER			0x09
#define EDID__SERIAL_NUMBER_2		0x0A
#define EDID__SERIAL_NUMBER_3		0x0B
#define EDID__SERIAL_NUMBER_4		0x0C
#define EDID__MANUFACTURE_WEEK		0x0D
#define EDID__MANUFACTURE_YEAR		0x0E

#define RP_A__RUNTIME_REGS              0x18        // First normal R/W register

#define RP_A__RX_CFG					0x18
#define RP_M__RX_CFG__AUD_MCLK								0x03
#define RP_M__RX_CFG__128FS									0x00
#define RP_M__RX_CFG__256FS									0x01
#define RP_M__RX_CFG__384FS									0x02
#define RP_M__RX_CFG__512FS									0x03
#define RP_M__RX_CFG__AUDIO_SOFT_MUTE						0x10
#define RP_M__RX_CFG__DSD_USES_I2S_SPDIF_BUSES				0x20
#define RP_M__RX_CFG__INVERT_OUT_VIDEO_CLOCK				0x40

#define RP_A__TX_CFG					0x1C
#define RP_M__TX_CFG__AUD_MCLK								0x03
#define RP_M__TX_CFG__128FS									0x00
#define RP_M__TX_CFG__256FS									0x01
#define RP_M__TX_CFG__384FS									0x02
#define RP_M__TX_CFG__512FS									0x03
#define RP_M__TX_CFG__INVERT_INP_VIDEO_CLOCK				0x40
#define RP_M__TX_CFG__FOLLOW_RX_EFFECTIVE_OUT_COLOR_DEPTH	0x80

#define RP_A__BOARD_OPTIONS1            0x20
#define RP_M__BOARD_OPTIONS1__ENABLE_CEC_POWER_CTRL         0x20
#define RP_M__BOARD_OPTIONS1__ENABLE_CEC_SWITCHING          0x40

#define RP_A__BOARD_OPTIONS2			0x21
#define RP_M__BOARD_OPTIONS2__USE_ONBOARD_INPUT_SELECTOR	0x01
#define RP_M__BOARD_OPTIONS2__COMM_ONLY_MODE				0x40
#define RP_M__BOARD_OPTIONS2__ENABLE_LOG_PRINT				0x80

#define RP_A__AUDIO_OPTIONS				0x23
#define AUDIO_MODE__DISABLE									0x00
#define RP_M__AUDIO_OPTIONS__BYPASS							0x01

#define RP_A__INPUT_NMB					0x24
#define RP_C__VM_NOT_FOUND									0xFF

#define RP_A__REQUEST					0x28
#define RP_M__REQUEST__TX_AV_MUTE							0x01
#define RP_M__REQUEST__RESTART								0x04
#define RP_M__REQUEST__POWER_DOWN							0x80
#define RP_ON_TO_STANDBY									0x00
#define RP_STANDBY_TO_ON									0x01
#define RP_RESTART											0x02

#define RP_A__PIPE_STATUS				0x2A // RO
#define RP_M__PIPE_STATUS__PIPE_PORT_A						0x70
#define RP_M__PIPE_STATUS__PIPE_A_DISCONECCTED				0x80 // set if pipe A is OFF

#define RP_A__CEC_STAT					0x2B
#define RP_M__CEC_STAT__STANDBY								0x01

#define RP_A__INTERRUPT					0x2C
#define RP_M__INTERRUPT__RESOLUTION_DETECTED				0x01
#define RP_M__INTERRUPT__AVI_RECEIVED						0x02
#define RP_M__INTERRUPT__AUDIO_FORMAT						0x04
#define RP_M__INTERRUPT__CEC								0x08
#define RP_M__INTERRUPT__AUDIO_STATUS_CHANGED				0x10
#define RP_M__INTERRUPT__HDMI_DVI_CHANGED					0x20
#define RP_M__INTERRUPT__BUS_CHANGED						0x40
#define RP_M__INTERRUPT__RX_READY_CHANGED					0x80

#define RP_A__INTERRUPT2					0x2D
#define RP_M__INTERRUPT2__CBUS_MSG_RECEIVED					0x04


#define RP_A__INTERRUPT_MASK			0x2E
#define RP_A__INTERRUPT2_MASK			0x2F

#define RP_A__RX_OUTPUT					0x30 // RO
#define RP_M__RX_OUTPUT_PWR				                    0x01

#define RP_A__RX_AVI1					0x33
#define RP_A__RX_AVI2					0x34
#define RP_A__RX_AVI3					0x35
#define RP_A__RX_AVI4					0x36 // VIC
#define RP_A__RX_AVI5					0x37
#define RP_A__RX_AVI_VALID				0x38
#define RP_M__RX_AVI_VALID									0x01

#define RP_A__RX_AUDIO_FORMAT			0x3A
#define RP_M__RX_AUDIO_FORMAT__LAYOUT1			0x01
#define RP_M__RX_AUDIO_FORMAT__DSD				0x02
#define RP_M__RX_AUDIO_FORMAT__DST				0x04 // reserved
#define RP_M__RX_AUDIO_FORMAT__HBR				0x08
#define RP_M__RX_AUDIO_FORMAT__CHST_RDY			0x40 // set if CHST data is ready and valid, cleared otherwice
#define RP_M__RX_AUDIO_FORMAT__AUD_IF_RDY		0x80 // set if Audio Infoframe data is ready and valid, cleared otherwice


#define RP_A__RX_I2S_CHST1				0x3B //
#define RP_M__I2S_CHST1__APLICATION				0x01 // 1-professional, 0-consumer
#define RP_M__I2S_CHST1__ENCODED				0x02 // 0-PCM, 1- for other purposes
#define RP_M__I2S_CHST1__COPYRIGHT				0x04 // 1-copyright asserted, 0- not asserted

#define RP_M__VBI_CTRL__PRINT_CC								0x0F
#define RP_M__AUDIO_OPTIONS__BYPASS							0x01

#define RP_A__RX_I2S_CHST2              0x3C //
#define RP_A__RX_I2S_CHST3              0x3D //
#define RP_A__RX_I2S_CHST4              0x3E //

#define RP_A__RX_I2S_CHST5				0x3F // width & original Fs
#define RP_M__I2S_CHST5__MAX_WIDTH				0x01 //
#define RP_M__I2S_CHST5__WIDTH					0x0E //
#define RP_M__I2S_CHST5__WIDTH_NOT_IND_A		0x00 // not indicated
#define RP_M__I2S_CHST5__WIDTH_16A				0x02 // 16 bit
#define RP_M__I2S_CHST5__WIDTH_18A				0x04 // 18 bit
#define RP_M__I2S_CHST5__WIDTH_19A				0x08 // 19 bit
#define RP_M__I2S_CHST5__WIDTH_20A				0x0A // 20 bit
#define RP_M__I2S_CHST5__WIDTH_17A				0x0C // 17 bit
#define RP_M__I2S_CHST5__WIDTH_NOT_IND_B		0x01 // not indicated
#define RP_M__I2S_CHST5__WIDTH_20B				0x03 // 20 bit
#define RP_M__I2S_CHST5__WIDTH_22B				0x05 // 22 bit
#define RP_M__I2S_CHST5__WIDTH_23B				0x09 // 23 bit
#define RP_M__I2S_CHST5__WIDTH_24B				0x0B // 24 bit
#define RP_M__I2S_CHST5__WIDTH_21B				0x0D // 21 bit
#define RP_M__I2S_CHST5__ORIG_FS				0xF0 //
#define RP_C__MUX__OFF										15

#define RP_A__RX_AUDIO_INFO_FRAME1		0x40 // number of channels
#define RP_A__RX_AUDIO_INFO_FRAME4		0x41 // CA
#define RP_A__RX_AUDIO_INFO_FRAME5		0x42 // downmixing


// local devices access window
#define RP_A__CMD_STATUS				0x68 // RO
#define RP_M__CMD_STATUS__ERROR						0x0F
#define RP_STS__NO_ERROR							0x00
#define RP_STS__ERROR_TIME_OUT						0x01
#define RP_STS__ERROR_NO_RESPONSE					0x02
#define RP_STS__ERROR_FAILED                        0x03
#define RP_STS__ERROR_INVALID_OPERAND               0x04
#define RP_STS__NOT_SUPPORTED						0x0E
#define RP_STS__ANY_ERROR							0x0F
#define RP_M__CMD_STATUS__RESULT					0xF0
#define RP_STS__NOT_READY							0x00
#define RP_STS__READY								0x10

#define RP_A__CMD_OPTIONS				0x69 // RW
#define RP_M__CMD_OPTIONS__NO_AUTOINCREMENT			0x01 // 0-normal mode, 1-access 1 byte at a time (works only for I2C reading)

#define RP_A__CMD_ACCESS_TYPE			0x6A // RW

// Access type definitions for RP_CMD__READ / RP_CMD__WRITE
#define RP_C__CAT_LOCAL_I2C							0
#define RP_C__CAT_DS_DDC							1
#define RP_C__CAT_US_EDID							2
#define RP_C__CAT_CFG_EEPROM						3
#define RP_C__CAT_RX_CHIP_REG						4
#define RP_C__CAT_TX_CHIP_REG						5

// Access type definitions for RP_CMD__EXECUTE
#define RP_C__CAT_CMD__SAC_CONFIG                       0x40
#define RP_C__CAT_CMD__SAC_INITIATE                     0x41
#define RP_C__CAT_CMD__SAC_TERMINATE                    0x42
#define RP_C__CAT_CMD__SAC_STATUS_GET                   0x43
#define RP_C__CAT_CMD__SAC_SHORT_AUDIO_DESCRIPTOR_GET   0x44
#define RP_C__CAT_CMD__SAC_STATUS_SET                   0x45
#define RP_C__CAT_CMD__SAC_SHORT_AUDIO_DESCRIPTOR_SET   0x46

#define RP_C__CAT_CMD__CEC_SET_OSD_NAME                 0x50

#define RP_C__CAT_CMD__MHL_WRITE_MSG                    0x60
#define RP_C__CAT_CMD__MHL_READ_MSG                     0x61
#define RP_C__CAT_CMD__MHL_WRITE_BURST                  0x62

#define RP_A__CMD_DEVICE_ID				0x6B // RW I2C Address
#define RP_A__CMD_ADDR_L				0x6C // RW Register Address (low byte)
#define RP_A__CMD_ADDR_H				0x6D // RW Register Address (high byte) or segment address dor DDC
#define RP_A__CMD_LENGTH				0x6E // RW number of bytes to transfer (no more then the buffer size)
#define RP_A__CMD_CMD					0x6F // RW the command: 0-idle, 1-read I2C, 2-write I2C
#define RP_CMD__IDLE								0x00
#define RP_CMD__READ								0x01
#define RP_CMD__WRITE								0x02
#define RP_CMD__EXECUTE								0x03

#define RP_A__CMD_BUF					0x70 // RW start of the data buffer
#define RP_L__CMD_BUF								0x10 // length of the data buffer

#define RP_A__CMD_BUF1					0x71 // data buffer byte 1
#define RP_A__CMD_BUF2                  0x72 // data buffer byte 2
#define RP_A__CMD_BUF3                  0x73 // data buffer byte 3

#define VAL_Input_NoRep 				0x00
#define VAL_Input_Rep2 					0x01
#define VAL_Input_Rep4 					0x03
#define VAL_Input_Rising 				1
#define VAL_Input_Falling  				0
#define VAL_Input_DualEdge 				0
#define VAL_Input_SingleEdge  			1

//------------------------------------------------------------------------------
//  Standard component functions
//------------------------------------------------------------------------------

bool_t  SiiRtpiInstanceSet ( uint_t instanceIndex );
bool_t  SiiRtpiInitialize ( uint8_t instance );
void    SiiRtpiProcessRtpi( void);

//------------------------------------------------------------------------------
//  RTPI-specific component functions
//------------------------------------------------------------------------------

void SiiRtpiSetInt( uint16_t val );
void SiiRtpiClearInt( uint16_t val );
void SiiRtpiOnAudioInfoFrameChange( void );
void SiiRtpiOnAviInfoFrameChange( void );
void SiiRtpiOnAudioStatusChange(uint8_t formatVal);
void SiiRtpiOnPwrStateChange( bool_t qOn);
void SiiRtpiPort(uint8_t port, uint8_t flag);
void SiiRtpiPortConnect(uint8_t pwr5vStatus);

//------------------------------------------------------------------------------
//  Component call back functions
//  Implementation of the callback functions is on the application level
//------------------------------------------------------------------------------
void    SiiRtpiCbAudioByPassSet( bool_t isByPass );
bool_t  SiiRtpiCbPowerControl( int newState );
void    SiiRtpiCbDeviceInitTx( bool_t isMatrixMode, bool_t isAudioByPass );
bool_t  SiiRtpiCbUpdateControl ( uint8_t mask, uint8_t value );
uint8_t SiiRtpiCbUpdateSource( uint8_t functionSelect, uint8_t val );
uint8_t SiiRtpiCbPowerState( void );

#endif // __SI_RTPI_COMPONENT_H__
