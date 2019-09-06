//***************************************************************************
//!file     si_rtpi_register_functions.c
//!brief    Silicon Image RTPI Component register processing.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************

#include "string.h"
#include "si_rtpi_internal.h"
#include "si_drv_switch.h"
#include "si_drv_audio.h"
#include "si_eeprom.h"
#include "si_tx_component.h"
#include "si_drv_tpi_ddc_access.h"
#include "si_drv_nvram_sram.h"
#include "si_i2c.h"
#if INC_CBUS
#include "si_cbus_internal.h"
#include "si_cbus_component.h"
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Register 0x18 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x18( uint8_t val )
{

	if ((val & RP_M__RX_CFG__AUDIO_SOFT_MUTE) == RP_M__RX_CFG__AUDIO_SOFT_MUTE)
	{
		SiiDrvRxAudioSoftMute(true);
	}
	else
	{
		SiiDrvRxAudioSoftMute(false);
	}
	val &= ~RP_M__RX_CFG__AUD_MCLK;
	val |= SiiDrvRxAudioGetMclk();

	return val;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Register 0x1C processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x1C( uint8_t val )
{
    uint8_t org;

    org = SiiPlatformI2cCbSlaveReadByte(1, RP_A__TX_CFG);
    val = val & RP_M__TX_CFG__INVERT_INP_VIDEO_CLOCK;
    if( val )
    {
        siiTpiInputBusSetupVal.risingEdge = VAL_Input_Rising;
    }
    else
    {
        siiTpiInputBusSetupVal.risingEdge = VAL_Input_Falling;
    }
    val |= SiiPlatformI2cCbSlaveReadByte(1, RP_A__RX_CFG) & RP_M__RX_CFG__AUD_MCLK; // just copy Rx config.
    return val;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Register 0x20 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x20( uint8_t val )
{

    return val;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Register 0x21 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x21( uint8_t val )
{

    if (val & RP_M__BOARD_OPTIONS2__USE_ONBOARD_INPUT_SELECTOR)
    {
        val |=  RP_M__BOARD_OPTIONS2__USE_ONBOARD_INPUT_SELECTOR;
        SiiRtpiCbUpdateControl( EEFLAG_ROTARY_ENABLE, EEFLAG_ROTARY_ENABLE );
    }
    else
    {
        val &= ~RP_M__BOARD_OPTIONS2__USE_ONBOARD_INPUT_SELECTOR;
        SiiRtpiCbUpdateControl( EEFLAG_ROTARY_ENABLE, 0 );
    }

    if( val & RP_M__BOARD_OPTIONS2__COMM_ONLY_MODE )
    {

        SiiPlatformInterruptDisable();
    }
    else    // Normal Mode
    {
        SiiPlatformInterruptEnable();

    }

//#if DEBUG_PRINT == 1
    if( val & RP_M__BOARD_OPTIONS2__ENABLE_LOG_PRINT )
    {
        pRtpi->debugPrintMode = 0;
    }
    else
    {
        pRtpi->debugPrintMode = 3;
    }
//#endif
    return val;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Register 0x23 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x23 ( uint8_t val )
{
	bool_t matrixMode = SiiDrvSwitchMatrixModeEnableGet();
	if ( val & RP_M__AUDIO_OPTIONS__BYPASS )
	{
		SiiRtpiCbAudioByPassSet( true );
		SiiRtpiCbDeviceInitTx( matrixMode, true );
	}
	else
	{
		SiiRtpiCbAudioByPassSet( false );
		SiiRtpiCbDeviceInitTx( matrixMode, false );
	}

	return val;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Register 0x24 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x24( uint8_t val )
{
	return( SiiRtpiCbUpdateSource( 1, val ));
}

//-------------------------------------------------------------------------------------------------
//! @brief      Register 0x28 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x28( uint8_t val )
{
    #define TXMUTE 0x01
    #define RSVD1  0x02
    #define RST    0x04
    #define SAVE   0x08
    #define RSVD2  0x70
    #define PD     0x80

    if( (val & TXMUTE) != siiTpiSystemControlVal.avMuteEn )
    {
        siiTpiSystemControlVal.avMuteEn = (val & TXMUTE);
        if( siiTpiSystemControlVal.avMuteEn )
        {
            SiiTxAvMute();
        }
        else
        {
            SiiTxAvUnmute();
        }
    }

    if( (val & PD) != siiTpiSystemControlVal.powerDown )
    {
        siiTpiSystemControlVal.powerDown = (val & PD);
        if( (val & PD) )
        {
             //SiI_TPI_SetState_Unstable();
             //SiI_Rx_SetState_PowerOff();
        	SiiRtpiCbPowerControl(RP_ON_TO_STANDBY);
         }
        else
        {
            //SiI_TPI_SetState_Unknown();
            //SiI_Rx_SetState_CheckPCLK();
        	SiiRtpiCbPowerControl(RP_STANDBY_TO_ON);
        }
    }
    if( (val & SAVE) )
    {

        val = (val & ~SAVE);
    }
    if( (val & RST) )
    {
        val = val & ~RST;
        pRtpi->needReset = 1;
        // Toggle power.
        SiiRtpiCbPowerControl(RP_RESTART);

    }
    return val;
}

//-------------------------------------------------------------------------------------------------
//! @brief      RP_A__CEC_STAT Register 0x2B processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x2B( uint8_t val )
{
    // Just clear the interrupt bit
    return( SiiPlatformI2cCbSlaveReadByte( 1, RP_A__CEC_STAT ) & ~val );
}

//-------------------------------------------------------------------------------------------------
//! @brief      RP_A__INTERRUPT Register 0x2C processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x2C( uint8_t val )
{
    // Just clear the interrupt bit
    return( SiiPlatformI2cCbSlaveReadByte( 1, RP_A__INTERRUPT ) & ~val );
}

//-------------------------------------------------------------------------------------------------
//! @brief      RP_A__INTERRUPT2 Register 0x2D processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x2D( uint8_t val )
{
    // Just clear the interrupt bit
    return( SiiPlatformI2cCbSlaveReadByte( 1, RP_A__INTERRUPT2 ) & ~val );
}

//-------------------------------------------------------------------------------------------------
//! @brief      RP_A__RX_OUTPUT Register 0x30 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x30( uint8_t val )
{
	return(SiiRtpiCbPowerState());
}

//-------------------------------------------------------------------------------------------------
//! @brief      RP_A__RX_AVI1 Register 0x33 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x33( uint8_t val )
{
	uint8_t pInfoFrame[20];
	uint8_t pLen = 0;

    SiiDrvSwitchInfoFrameGet( true, IF_AVI, &pInfoFrame[0], &pLen);
    return( pInfoFrame[4] );
}

//-------------------------------------------------------------------------------------------------
//! @brief      RP_A__RX_AVI2 Register 0x34 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x34( uint8_t val )
{
	uint8_t pInfoFrame[20];
	uint8_t pLen = 0;

	SiiDrvSwitchInfoFrameGet( true, IF_AVI, &pInfoFrame[0], &pLen);
	return( pInfoFrame[5] );
}

//-------------------------------------------------------------------------------------------------
//! @brief      RP_A__RX_AVI3 Register 0x35 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x35( uint8_t val )
{
	uint8_t pInfoFrame[20];
	uint8_t pLen = 0;

    SiiDrvSwitchInfoFrameGet( true, IF_AVI, &pInfoFrame[0], &pLen);
    return( pInfoFrame[6] );
}

//-------------------------------------------------------------------------------------------------
//! @brief      RP_A__RX_AVI4 Register 0x36 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x36( uint8_t val )
{
    uint8_t pInfoFrame[20];
    uint8_t pLen = 0;

    SiiDrvSwitchInfoFrameGet( true, IF_AVI, &pInfoFrame[0], &pLen);
    return( pInfoFrame[7] );
}

//-------------------------------------------------------------------------------------------------
//! @brief      RP_A__RX_AVI5 Register 0x37 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x37( uint8_t val )
{
    uint8_t pInfoFrame[20];
    uint8_t pLen = 0;

    SiiDrvSwitchInfoFrameGet( true, IF_AVI, &pInfoFrame[0], &pLen);
    return( pInfoFrame[8] );
}

//-------------------------------------------------------------------------------------------------
//! @brief      RP_A__RX_AVI_VALID Register 0x38 processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x38( uint8_t val )
{
    return( SiiDrvSwitchInfoFrameValidGet(true) );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Unused register processor - returns what was written
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegsDummy( uint8_t val )
{
    return val;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Register 0x6A processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x6A( uint8_t val ) { return val; }
uint8_t RtpiRegs0x6B( uint8_t val ) { return val; }
uint8_t RtpiRegs0x6C( uint8_t val ) { return val; }
uint8_t RtpiRegs0x6D( uint8_t val ) { return val; }
uint8_t RtpiRegs0x6E( uint8_t val ) { return val; }

//-------------------------------------------------------------------------------------------------
//! @brief      Register 0x6F processor
//! @param[in]  val     - new register value from external device
//! @return     uint8_t - updated register value to be written into register file.
//-------------------------------------------------------------------------------------------------
uint8_t RtpiRegs0x6F( uint8_t val )
{
    uint8_t buf[16];
    uint8_t slave;
    uint8_t cmdType = 0;
    unsigned short offset;
    int length;
    int i, err = 0;

    if ( val == 0 )
    {
        return( val );
    }

    SiiRpForceWriteByte(RP_A__CMD_STATUS, 0);  // Clear ready bit.

    SiiRpForceWriteByte( RP_A__CMD_CMD, val);
    SiiRpReadBlock(RP_A__CMD_BUF, buf, 16); // prepare for the write operation
    slave = SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_DEVICE_ID);
    offset = SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_ADDR_L );
    length = SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_LENGTH );
    cmdType = SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_ACCESS_TYPE);

    if ( length > 16 )
    {
        length = 16;
    }

//#if DEBUG_PRINT == 1
        if (pRtpi->debugPrintMode == 0)
        {
        	switch (val)
        	{
        	case RP_CMD__READ:
        	    DEBUG_PRINT( MSG_DBG, "RtpiRegs0x6f: RD: cmd type %x address %x Length %d \n", cmdType, offset, length);
        	    break;
        	case RP_CMD__WRITE:
        		DEBUG_PRINT( MSG_DBG, "RtpiRegs0x6f: WR: cmd type %x address %x Length %d \n", cmdType, offset, length);
        		break;
        	case RP_CMD__EXECUTE:
        		DEBUG_PRINT( MSG_DBG, "RtpiRegs0x6f: EX: cmd type %x address %x Length %d \n", cmdType, offset, length);
        		break;
        	default:
        		DEBUG_PRINT( MSG_DBG, "RtpiRegs0x6f: IDLE\n");
        		break;
        	}
        }
//#endif // DEBUG_PRINT

    if (val == RP_CMD__EXECUTE)
    {
        do {
#if INC_CBUS
            if ( RtpiExecuteCbusCmd( cmdType ))  break;
#endif
#if INC_CEC
            if ( RtpiExecuteCecCmd( cmdType ))   break;
#endif
        } while (0);
    }
    else switch( (val << 8) | cmdType )
    {
        case (RP_CMD__READ<<8)|RP_C__CAT_LOCAL_I2C:
        case (RP_CMD__READ<<8)|RP_C__CAT_RX_CHIP_REG:
        case (RP_CMD__READ<<8)|RP_C__CAT_TX_CHIP_REG:
            if( SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_OPTIONS) == RP_M__CMD_OPTIONS__NO_AUTOINCREMENT )
    	    {
                for( i=0; i<length; i++ )
                {
                    err = SiiPlatformI2cReadBlock( 0, slave, offset, buf+i, 1);
                }
            }
            else
            {
                err = SiiPlatformI2cReadBlock( 0, slave, offset, buf, length);
            }
            for (i = 0; i < 16; i++)
            {
                SiiRpForceWriteByte(RP_A__CMD_BUF+i, buf[i]);
            }
            break;

        case (RP_CMD__WRITE<<8)|RP_C__CAT_LOCAL_I2C:
        case (RP_CMD__WRITE<<8)|RP_C__CAT_RX_CHIP_REG:
        case (RP_CMD__WRITE<<8)|RP_C__CAT_TX_CHIP_REG:
            if( SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_OPTIONS) == RP_M__CMD_OPTIONS__NO_AUTOINCREMENT )
    	    {
                for( i=0; i<length; i++ )
                {
                    err = SiiPlatformI2cWriteBlock( 0, slave, offset, buf+i, 1);
                }
            }
            else
            {
                err = SiiPlatformI2cWriteBlock( 0, slave, offset, buf, length);
            }
            break;

        case (RP_CMD__READ<<8)|RP_C__CAT_DS_DDC:
            if( SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_OPTIONS) == RP_M__CMD_OPTIONS__NO_AUTOINCREMENT )
    	    {
                for( i=0; i<length; i++ )
                {
                    err = SiiDrvTpiDdcReadBlock( offset, SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_ADDR_H ), buf+i, 1);
                }
            }
            else
            {
                err = SiiDrvTpiDdcReadBlock( offset, SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_ADDR_H ), buf, length);
            }
            for (i = 0; i < 16; i++)
            {
                SiiRpForceWriteByte(RP_A__CMD_BUF+i, buf[i]);
            }

            break;

        case (RP_CMD__WRITE<<8)|RP_C__CAT_DS_DDC:
            if( SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_OPTIONS) == RP_M__CMD_OPTIONS__NO_AUTOINCREMENT )
    	    {
                for( i=0; i<length; i++ )
                {
                    err = SiiDrvTpiDdcWriteBlock( offset, buf+i, 1);
                }
            }
            else
            {
                err = SiiDrvTpiDdcWriteBlock( offset, buf, length);
            }
            break;

        case (RP_CMD__READ<<8)|RP_C__CAT_US_EDID:
            SiiDrvNvramSramRead( SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_ADDR_H ), buf, offset, length );

            for (i = 0; i < 16; i++)
            {
                SiiRpForceWriteByte(RP_A__CMD_BUF+i, buf[i]);
            }

            break;

        case (RP_CMD__WRITE<<8)|RP_C__CAT_US_EDID:
        	for (i = 0; i < 16; i++)
            {
                  buf[i] = SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_BUF+i);
            }
            SiiDrvNvramSramWrite( SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_ADDR_H ), buf, offset, length );
            break;

        case (RP_CMD__READ<<8)|RP_C__CAT_CFG_EEPROM:
            offset = offset | (SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_ADDR_H ) << 8);
            SiiPlatformEepromReadBlock (offset, buf, length);

            for (i = 0; i < 16; i++)
            {
                SiiRpForceWriteByte(RP_A__CMD_BUF+i, buf[i]);
            }
            break;

        case (RP_CMD__WRITE<<8)|RP_C__CAT_CFG_EEPROM:
        	for (i = 0; i < 16; i++)
            {
                 buf[i] = SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_BUF+i);
            }
            offset = offset | (SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_ADDR_H ) << 8);
            SiiPlatformEepromWriteBlock(offset, buf, length);
            break;

        default: break;
    }

    SiiRpForceWriteByte( RP_A__CMD_CMD, RP_CMD__IDLE);
    SiiRpForceWriteByte( RP_A__CMD_STATUS, SiiPlatformI2cCbSlaveReadByte( 1, RP_A__CMD_STATUS ) | RP_STS__READY );  // Set ready bit.

    return 0; // set to idle status
}
