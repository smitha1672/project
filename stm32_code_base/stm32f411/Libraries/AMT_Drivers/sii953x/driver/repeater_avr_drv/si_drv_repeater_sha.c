//***************************************************************************
//!file     si_drv_repeater_sha.c
//!brief    Silicon Image Repeater component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include <stdio.h>
#include <string.h>
#include "si_common.h"
#include "si_drv_repeater_sha.h"
#include "si_regs_repeater953x.h"
#include "si_hdcp_ddc.h"
#include "si_drv_internal.h"
#include "si_tx_component.h"
#include "si_drv_tpi_system.h"

#define KSV_FIFO_SEL_CLEAR	0xFF
#define TX0_KSV_LENGTH(x)	(x)
#define TX1_KSV_LENGTH(x)	(x + 0x140)

RepeaterTopoMode_t currentMode;
repeaterShaRequest_t currentSrq;
bool_t firstTime;

void SiiDrvShaInit(void)
{
	currentMode = SI_RPT_MODE_INITIAL;
	currentSrq.isTx = false;
	currentSrq.manualTxStart = false;
	currentSrq.portNum = 0;
	currentSrq.isValid = false;
	firstTime = true;
}


repeaterShaRequest_t SiiDrvGetCurrentShaReq()
{
	return currentSrq;
}
//-------------------------------------------------------------------------------------------------
//! @brief      Set the current switch mode
//!				single repeater - InstraPort mode
//!				multiple repeater - Matrix mode
//-------------------------------------------------------------------------------------------------

void SiiDrvShaSetSwitchMode(RepeaterTopoMode_t mode)
{
	currentMode = mode;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Monitors if the SHA is busy
//!
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvShaInUse(void)
{
    bool_t result = true;
    uint8_t regVal = 0;

    if (!currentSrq.isValid)
    {
    	result = false; /*if not a valid req, not busy*/
    }
    else if (!(SiiRegRead( REG_HDCP_STAT ) & BIT_SHA_BUSY))
	{
		/*since KSV FIFO cannot be read/write until HDCP done for the previous requester*/
		if (currentSrq.isTx)
		{
		    regVal =  SiiTxInstanceGet();
			/*wait till HDCP done*/
#if (SII_NUM_TX > 1)
			SiiDrvTpiInstanceSet(currentSrq.portNum);
#endif
			if (!SiiDrvTpiHdcpIsPart2Done())
			{
				result = true;
			}
			else
			{
				result = false;
			}
#if (SII_NUM_TX > 1)
			SiiDrvTpiInstanceSet(regVal);
#endif
		}
		else
		{
			result = false; /*For rx, SHA busy bit is enough to indicate the SHA occupancy*/
		}
	}
	else
	{
		result = true;
	}
	return result;
}

//-------------------------------------------------------------------------------------------------
//! @brief      service the SHA request, point SHA to the right position
//! @param[in]  srq: the SHA requester 
//-------------------------------------------------------------------------------------------------

void SiiDrvShaRequestStart(repeaterShaRequest_t srq)
{
	srq.isValid = true;
	currentSrq = srq;
	if (srq.manualTxStart)
	{
		if (srq.isTx)
		{
			SiiRegWrite(REG_SHA_CTRL, VAL_SHA_DS_SWMODE); //manual start
			if (srq.portNum == 0)
			{
				SiiRegModify(REG_SHA_CTRL3, BIT_SHA_START_TX0,SET_BITS); //write enable
			}
#if (SII_NUM_TX > 1)
			else if (srq.portNum == 1)
			{
				SiiRegModify(REG_SHA_CTRL3, BIT_SHA_START_TX1,SET_BITS);
			}
#endif
		}
	}
	else /*SHA HW start*/
	{
		if (srq.isTx)
		{
			SiiDrvRepeaterKsvPointTo(srq.portNum);
			SiiRegWrite(REG_SHA_CTRL, BIT_SHA_EN); //no DS enable, HW does it itself
			DEBUG_PRINT( RPT_MSG_DBG, "Tx %d SHA req\n ", srq.portNum);
		}
		else  /*rx ports*/
		{
			SiiDrvRepeaterKsvPointTo(KSV_FIFO_SEL_CLEAR);
			SiiDrvShaAdjustKsvStartAddr( 0 );
			DEBUG_PRINT( RPT_MSG_DBG, "Rx %d SHA req\n ", srq.portNum);
			SiiRegWrite(REG_SHA_CTRL, BIT_SHA_EN|BIT_SHA_CTRL_MANUAL_US);
			if (currentMode == SI_RPT_MODE_SINGLE_REPEATER)
			{
				SiiRegWrite(REG_SHA_CTRL, VAL_SHA_US_HWMODE);
			}
			else //matrix mode
			{
				SiiRegWrite(REG_SHA_CTRL, VAL_SHA_US_SWMODE);
				//SiiRegWrite(REG_MS_US_PORT_REQ, 0x01<<srq.portNum);
			}
		}/*end of rx ports*/
	}
}

void SiiDrvShaRequestStop(bool_t isTx, uint8_t portNumber)
{
	if (isTx)
	{
		if ((currentSrq.isTx) && (currentSrq.portNum == portNumber))
		{
			currentSrq.isValid = false;
		}
	}
	else
	{
		if ((!currentSrq.isTx) && (currentSrq.portNum == portNumber))
		{
			currentSrq.isValid = false;
		}
	}
}
//-------------------------------------------------------------------------------------------------
//! @brief      set the SHA length register
//! @param[in]  slength - SHA length of this SHA calculation
//-------------------------------------------------------------------------------------------------

void SiiDrvSetShaLength(uint16_t slength)
{
	SiiRegWrite(REG_SHA_LEN1, slength & 0xFF);
	SiiRegWrite(REG_SHA_LEN2, (slength>>8) & 0xFF);
}

//-------------------------------------------------------------------------------------------------
//! @brief      set the Ksv start address
//! @param[in]  startPtr: Ksv start address
//-------------------------------------------------------------------------------------------------

void SiiDrvShaAdjustKsvStartAddr(uint16_t startPtr )
{
	uint8_t temp;/*save the original FIFO select*/
	temp = SiiRegRead(REG_EDID_FIFO_SEL);

	/*needs to clear the VAL_KSV_SELECT_MASK before modify the SHA start address*/
	SiiRegModify(REG_EDID_FIFO_SEL, VAL_KSV_SELECT_MASK, CLEAR_BITS);
	SiiRegWrite(REG_HDCP_KSV_SHA_START1, startPtr & 0xFF);
	SiiRegWrite(REG_HDCP_KSV_SHA_START2, (startPtr>>8) & 0xFF);

	DEBUG_PRINT( RPT_MSG_DBG, "adjust start adr to %d \n ", startPtr);
	SiiRegWrite(REG_EDID_FIFO_SEL, temp); /*write back the original setting.*/

}
//-------------------------------------------------------------------------------------------------
//! @brief      fill the ksv fifo with the BKSV(s) from DS(s), and set the ksv length registers
//!
//! @param[in]  selectedInstance - current instance
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterFillUsKsvInfo(uint16_t ksv_length, uint8_t txsBksv[], uint8_t num_of_tx, uint8_t txIndx)
{

	if ( currentMode == SI_RPT_MODE_SINGLE_REPEATER )
	{
		SiiDrvShaAdjustKsvStartAddr( ksv_length );
		SiiDrvRepeaterKsvPointTo(KSV_FIFO_SEL_CLEAR);
		SiiRegWriteBlock(REG_KSV_FIFO, txsBksv, num_of_tx * LEN_HDCP_DDC__BKSV);

	}
	else if ( currentMode == SI_RPT_MODE_MULTIPLE_REPEATER )
	{
		uint8_t temp;
		temp = SiiRegRead(REG_EDID_FIFO_SEL);

		SiiRegModify(REG_EDID_FIFO_SEL, VAL_KSV_SELECT_MASK, CLEAR_BITS);
        //SHA is pointing to the current Tx.
		if (txIndx == 1)
		{
			SiiDrvShaAdjustKsvStartAddr( TX1_KSV_LENGTH(ksv_length));
		}
		else
		{
			SiiDrvShaAdjustKsvStartAddr( TX0_KSV_LENGTH(ksv_length));
		}

		SiiRegWriteBlock(REG_KSV_FIFO, txsBksv, num_of_tx * LEN_HDCP_DDC__BKSV);
		SiiRegWrite(REG_EDID_FIFO_SEL, temp); /*write back the original setting*/
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief      define which tx will forward the KSV first, for 1to2 case
//!
//! @param[in]  tx_number - current tx first
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterKsvPointTo(uint8_t tx_number)
{
	SiiRegModify(REG_EDID_FIFO_SEL, VAL_KSV_SELECT_MASK, CLEAR_BITS);
	if ((tx_number == 0))
	{
		SiiRegModify(REG_EDID_FIFO_SEL, VAL_KSV_SELECT_TX0, SET_BITS);
		DEBUG_PRINT( RPT_MSG_DBG, "KSV FIFO to Tx 0\n ");

	}
	else if (tx_number == 1)
	{
		SiiRegModify(REG_EDID_FIFO_SEL, VAL_KSV_SELECT_TX1, SET_BITS);
		DEBUG_PRINT( RPT_MSG_DBG, "KSV FIFO to Tx 1\n ");

	}


}

//-------------------------------------------------------------------------------------------------
//! @brief      disable the auto SHA calculation started by HW
//!
//! @param[in]  none
//-------------------------------------------------------------------------------------------------

void SiiDrvHwAutoShaDisable(void)
{
	SiiRegModify(REG_SHA_CTRL, BIT_SHA_CTRL_MANUAL_US, CLEAR_BITS);
}




