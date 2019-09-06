//************************************************************************************************
//! @file   si_tx_hdcp.h
//! @brief  HDCP related methods
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

//#include <string.h>
#include "si_tx_component_internal.h"
#include "si_tx_hdcp.h"
#include "si_tx_info_frm.h"
#include "si_drv_tpi_system.h"
#include "si_drv_tpi_access.h"
#include "si_drv_tpi_ddc_access.h"
#include "si_hdcp_ddc.h"
#include "si_edid_tx_internal.h"
#include "si_edid_black_list.h"
#include "si_drv_repeater.h"
#include "si_connection_monitor.h"

txHdcpInstanceRecord_t HdcpInstRec[SII_NUM_TX];
txHdcpInstanceRecord_t *pHdcp = &HdcpInstRec[0];

extern txInstanceData_t *pTx; // pointer to TX component record


//-------------------------------------------------------------------------------------------------
//  Local service functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Show HDCP authentication phase and error messages, if any.
//-------------------------------------------------------------------------------------------------

static void PrintHdcpStatus(void)
{
    if (pHdcp->prevAuthState != pHdcp->authState)
    {
#if 1
        DEBUG_PRINT(TX_MSG_DBG, "DS HDCP: ");
        switch (pHdcp->authState)
        {
            case SI_TX_HDCP__OFF:
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "OFF\n");
                break;
            case SI_TX_HDCP__WAIT_FOR_START:
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "WAIT FOR START\n");
                break;
            case SI_TX_HDCP__WAIT_FOR_RETRY:
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "WAIT FOR RETRY\n");
                break;
            case SI_TX_HDCP__WAIT_FOR_DONE:
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "WAIT FOR DONE\n");
                break;
            case SI_TX_HDCP__WAIT_FIFO_READY:
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "WAIT FOR FIFO READY\n");
                break;
            case SI_TX_HDCP__KSV_FIFO_READ:
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "KSV FIFO READ\n");
                break;
            case SI_TX_HDCP__V_CHECK:
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "V=V' CHECK\n");
                break;
            case SI_TX_HDCP__AUTHENTICATED:
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "AUTHENTICATED\n");
                break;
            case SI_TX_HDCP__SECURE:
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "SECURE\n");
                break;
            case SI_TX_HDCP__FAILED:
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "FAILED\n");
                break;
            case SI_TX_HDCP__INITIAL:
                break;

        }
#endif
        pHdcp->prevAuthState = pHdcp->authState;
    }

    if (pHdcp->prevError != pHdcp->error)
    {
#if 1
        if (pHdcp->error)
        {
            DEBUG_PRINT(TX_MSG_DBG, "HDCP Errors:\n");
            if (pHdcp->error & ERR_HDCP__NACK)
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tNo ACK at downstream DDC\n");
            if (pHdcp->error & ERR_HDCP__INTERNAL)
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tInternal\n");
            if (pHdcp->error & ERR_HDCP__DEV_LIMIT)
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tDevice limit exceeded\n");
            if (pHdcp->error & ERR_HDCP__V)
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tDownstream V' != V\n");
            if (pHdcp->error & ERR_HDCP__P2_TO)
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tFIFO READY was not set within 5 s\n");
            if (pHdcp->error & ERR_HDCP__DS_EXCEED)
                DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tDownstream device indicated DEV_EXC or CAS_EXC\n");
        }
        else
        {
            DEBUG_PRINT(TX_MSG_DBG, "No HDCP Errors\n");
        }
#endif
        pHdcp->prevError = pHdcp->error;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Show HDCP query status
//-------------------------------------------------------------------------------------------------

static void PrintHdcpQueryStatus(uint8_t query)
{
#if 1
    DEBUG_PRINT(TX_MSG_DBG, "HDCP Query Status:\n");
    DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tLink Encryption: \t");

    switch (query &  MSK_TPI__HDCP_QUERY__STATUS)
    {
        case VAL_TPI__HDCP_QUERY__STATUS_NORMAL:
            DEBUG_PRINT(TX_MSG_DBG_PLAIN, "Normal\n");
            break;
        case VAL_TPI__HDCP_QUERY__STATUS_LOST:
            DEBUG_PRINT(TX_MSG_DBG_PLAIN, "Lost\n");
            break;
        case VAL_TPI__HDCP_QUERY__STATUS_FAILED:
            DEBUG_PRINT(TX_MSG_DBG_PLAIN, "Failed\n");
            break;
        case VAL_TPI__HDCP_QUERY__STATUS_SUSPENDED:
            DEBUG_PRINT(TX_MSG_DBG_PLAIN, "Suspended\n");
            break;
    }

    DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tConnector Type: \t");
    switch (query & MSK_TPI__HDCP_QUERY__CONNECTR)
    {
        case VAL_TPI__HDCP_QUERY__CONNECTR_DVI:
            DEBUG_PRINT(TX_MSG_DBG_PLAIN, "DVI\n");
            break;
        case VAL_TPI__HDCP_QUERY__CONNECTR_HDMI:
            DEBUG_PRINT(TX_MSG_DBG_PLAIN, "HDMI\n");
            break;
    }

    DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tProtection Type: \t");
    if (query & BIT_TPI__HDCP_QUERY__SINK_CAPABLE)
    {
        DEBUG_PRINT(TX_MSG_DBG_PLAIN, "HDCP\n");
    }
    else
    {
        DEBUG_PRINT(TX_MSG_DBG_PLAIN, "None\n");
    }

    DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tRepeater: \t\t");
    if (query & BIT_TPI__HDCP_QUERY__DS_REPEATER)
    {
        DEBUG_PRINT(TX_MSG_DBG_PLAIN, "Yes\n");
    }
    else
    {
        DEBUG_PRINT(TX_MSG_DBG_PLAIN, "No\n");
    }

    DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tLocal Protection: \t");
    if (query & BIT_TPI__HDCP_QUERY__LOCAL_PROTECT)
    {
        DEBUG_PRINT(TX_MSG_DBG_PLAIN, "Yes\n");
    }
    else
    {
        DEBUG_PRINT(TX_MSG_DBG_PLAIN, "No\n");
    }

    DEBUG_PRINT(TX_MSG_DBG_PLAIN, "\tExtended Protection: \t");
    if (query & BIT_TPI__HDCP_QUERY__EXTENDED_PROTECT)
    {
        DEBUG_PRINT(TX_MSG_DBG_PLAIN, "Yes\n");
    }
    else
    {
        DEBUG_PRINT(TX_MSG_DBG_PLAIN, "No\n");
    }
#endif
}

//-------------------------------------------------------------------------------------------------
//! @brief      Begin calculation of a new KSV hash value.
//-------------------------------------------------------------------------------------------------

static void NewKsvHash(void)
{
    // Keep previous hash value
    pHdcp->dsKsvHashPrev = pHdcp->dsKsvHash;
    // Reset new hash value
    pHdcp->dsKsvHash = 0;
    // Reset number of bytes in tail storage
    pHdcp->hashWordLen = 0;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Mixing hash function.
//!
//-------------------------------------------------------------------------------------------------

static uint32_t Mixer(uint32_t word)
{
    uint8_t i;

    // Mixing hash function. Apply many times
    for (i = 0; i < 32; i++)
    {
        word = 0x9069 * (word & 0xFFFF) + (word >> 16);
    }

    return word;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Add new input to KSV hash algorithm.
//!
//! @param[in]  pData   - pointer to KSV data
//! @param[in]  len     - number of bytes in pData[] buffer
//! @param[in]  isLast  - flag of the last portion of KSV list
//-------------------------------------------------------------------------------------------------

static void AddBytesToKsvHash(uint8_t *pData, uint8_t len, bool_t isLast)
{
    uint8_t i;
    uint32_t word;
    uint8_t bytesLeft = len +  pHdcp->hashWordLen;

    while (bytesLeft >= 4)
    {
        // consume 32 bits
        word = 0;
        for (i = 0; i < 4; i++)
        {
            if (pHdcp->hashWordLen != 0)
            {
                word |= (pHdcp->hashWord & 0xFF) << (i*8);
                pHdcp->hashWord >>= 8;
                pHdcp->hashWordLen--;
            }
            else
            {
                word |= (*pData++) << (i*8);
            }
            bytesLeft--;
        }

        word = Mixer(word);

        if (pHdcp->dsKsvHash == 0)
        { // very first word goes to hash stack
            pHdcp->dsKsvHash = word;
            continue;
        }
        else
        { // secondary words LSB will combine with LSB of the hash stack
            pHdcp->dsKsvHash = Mixer(((pHdcp->dsKsvHash & 0xFFFF) << 16) | (word & 0xFFFF));
        }
    }


    if (bytesLeft > 0)
    {
        // Put unprocessed tail to hashWord storage
        pHdcp->hashWord = 0;
        pHdcp->hashWordLen = bytesLeft;
        for (i = 0; i < bytesLeft; i++)
        {
            pHdcp->hashWord |= (*pData++) << (i*8);
        }

        // In case of last transaction, use zero padded hashWord immediately
        if (isLast)
        {
            pHdcp->dsKsvHash = Mixer(((pHdcp->dsKsvHash & 0xFFFF) << 16) | (pHdcp->hashWord & 0xFFFF));
        }
    }

    // When KSV list Hash calculation is complete,
    // set "Same Downstream Device Connected" status flag
    if (isLast)
    {
        pTx->status.isSameDsConnected = (pHdcp->dsKsvHash == pHdcp->dsKsvHashPrev);
        DEBUG_PRINT(TX_MSG_DBG, "KSV HASH: %s downstream detected.\n", pTx->status.isSameDsConnected ? "Same" : "New");
    }

}


//-------------------------------------------------------------------------------------------------
//! @brief      Unit Test for KsvHash.
//-------------------------------------------------------------------------------------------------

#if (SI_TX_COMP_UNIT_TESTS == ENABLE)
static void TestKsvHash(void)
{
    uint8_t i,j;
    uint8_t data[45] =
        {
                0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x0a,
                0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x1a,
                0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x2a,
                0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x3a,
                0x40,0x41,0x42,0x43,0x44
        };

    for (j = 0; j < 10; j++)
    {
        // little change in one byte should change the hash dramatically
        data[0] = j;

        NewKsvHash();

        for (i = 0; i < 4; i++)
        {
            AddBytesToKsvHash(&data[i * 10], 10, false);
        }

        AddBytesToKsvHash(&data[40], 5, true);

        DEBUG_PRINT(TX_MSG_ALWAYS, "KSV HASH TEST:  val = 0x%04x%04x\n", (int) ((pHdcp->dsKsvHash & 0xffff0000) >> 16),
                (int) (pHdcp->dsKsvHash & 0x0000ffff));
    }
}
#endif


//-------------------------------------------------------------------------------------------------
//! @brief      Mute AV during HDCP transient states.
//!
//!             Muting is achieved either by enabling AV Mute bit in GCP, or by CTRL3 gating
//!             (HDCP encryption disabling).
//!
//! @param[in]  isMuted - mute request flag.
//-------------------------------------------------------------------------------------------------

static void AvMuteEnable(bool_t isMuted)
{
    // Never unmute internally, if in repeater mode
    if (pHdcp->isRepeaterMode)
    {
        SiiDrvTpiHdcpEncriptionEnable(!isMuted);
    }
    else
    {
    	RepeaterConAvMuteSet(0x01, isMuted);
       // TxInfoAvMute(isMuted);
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Clear any pending events or activity in HDCP hardware (Reset HDCP FSM).
//-------------------------------------------------------------------------------------------------

static void CancelHdcpAuthentication(void)
{
    switch (pHdcp->authState)
    {
        case SI_TX_HDCP__WAIT_FOR_DONE:
        case SI_TX_HDCP__WAIT_FIFO_READY:
        case SI_TX_HDCP__KSV_FIFO_READ:
	 	case SI_TX_HDCP__V_CHECK:						
            AvMuteEnable(true);
            SiiDrvTpiHdcpProtectionEnable(false);

            // Reset HDCP engine by temporal switching output TMDS OFF.
#if (SI_TX_TMDS_TOGGLE_FOR_HDCP_RESET == ENABLE)
            // SWWA:18705
            TxTmdsBlankPause();
#endif
            break;
        default:
            break;
    }
}


static void ProcessBstatus(uint8_t dsBstatus[2])
{
    uint8_t dsCount = dsBstatus[0] & MSK_HDCP_DDC__BSTATUS1__DEV_COUNT;
    pHdcp->fifoByteCounter = dsCount * LEN_HDCP_DDC__BKSV; // DS Bksv list length

    if (pHdcp->isRepeaterMode)
    {
        if(SiiTxCbHdcpProcessDsBstatus(dsBstatus))
        {
            pHdcp->error |= ERR_HDCP__DEV_LIMIT;
        }
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Send BKSV to US device FIFO.
//-------------------------------------------------------------------------------------------------

static void SendBksvToUsFifo(void)
{
    if(pHdcp->isRepeaterMode)
    {
        // Read BKSV only for repeater mode
        uint8_t aDsBksv[LEN_HDCP_DDC__BKSV];

        // Copy Bksv to the FIFO
        SiiDrvTpiBksvGet(aDsBksv);

        SiiTxCbHdcpFinishUsPart2(aDsBksv);
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Finalize HDCP Authentication Part 2 by sending BKSV to US.
//!
//!             Note: in authentication failure case,
//!             transition to SI_TX_HDCP__FAILED state is done in TxHdcpTimerHandler()
//-------------------------------------------------------------------------------------------------

static void FinishAuthPart2(void)
{
    if(SiiDrvTpiHdcpIsAuthenticationGood())
    {
        SendBksvToUsFifo();
        pHdcp->authState = SI_TX_HDCP__AUTHENTICATED;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Copy a portion of KSV list from DS to an upper layer.
//!
//! @return     Flag of the KSV list last portion.
//! @retval     true - on the last portion of KSVs.
//-------------------------------------------------------------------------------------------------

static bool_t CopyKsvListPortion(void)
{
    uint8_t aKsvBuffer[VAL_TPI__HDCP_FIFO_BUFFER_LENGTH];
    uint8_t bytesToRead;
    bool_t  isDone;

    isDone = SiiDrvTpiKsvListPortionSizeGet(&bytesToRead);

    if((pHdcp->fifoByteCounter != 0) && (bytesToRead != 0))
    {
        if(bytesToRead > pHdcp->fifoByteCounter) // extra check
        {
            DEBUG_PRINT(TX_MSG_ERR, "HDCP Error: Incorrect number of KSVs!\n");
            pHdcp->error |= ERR_HDCP__INTERNAL;
            bytesToRead = pHdcp->fifoByteCounter;
            isDone = true;
        }

        // get DS BKSV list
        SiiDrvTpiKsvListGet(aKsvBuffer, bytesToRead);

        if (pHdcp->isRepeaterMode)
        {
            // Forward portion of KSV list to upper layer for processing
            SiiTxCbHdcpProcessKsvListPortion(aKsvBuffer, bytesToRead);

            // Add new portion to the hash
            AddBytesToKsvHash(aKsvBuffer, bytesToRead, isDone);
        }

        pHdcp->fifoByteCounter -= bytesToRead;
    }

    return isDone;
}


//-------------------------------------------------------------------------------------------------
//! @brief      HDCP Timer event handler.
//!
//!             This function is to be called periodically. The time past from the last call
//!             should be indicated as a parameter.
//!
//! @param[in]  timeDelta - time in ms elapsed since the last call.
//-------------------------------------------------------------------------------------------------

static void OnTimer(time_ms_t timeDelta)
{
    uint8_t ksvListSize;
    bool_t  isKsvListReadingDone;
    uint8_t txPlugStatus = SiiDrvTpiPlugStatusGet();
#if (FPGA_BUILD == ENABLE)
    // FPGA model doesn't support RSEN
    bool_t rsen = true;
#else
    bool_t rsen = ((txPlugStatus & SI_TX_STATUS_MASK_RX_SENSE) != 0);
#endif


    bool_t hp = ((txPlugStatus & SI_TX_STATUS_MASK_HPD) != 0);

    if (pHdcp->tmdsOnCountDownStart)
    {
        pHdcp->tmdsOnCountDownStart = false;
        if ((SiiEdidTxDsDeviceRemedyMaskGet() & SI_TX_EDID_REMEDY_SP_TEST) == 0)
        {
            if (SiiEdidTxDsDeviceRemedyMaskGet() & SI_TX_EDID_REMEDY_DELAY_DS_AUTH)
            {
                pHdcp->tmdsCountdown = TIM_MS_HDCP__TMDS_ON_DELAY_MAX;
            }
            else
            {
                pHdcp->tmdsCountdown = TIM_MS_HDCP__TMDS_ON_DELAY_MIN;
            }
        }
    }

    switch (pHdcp->authState)
    {

        case SI_TX_HDCP__KSV_FIFO_READ:
            if(!SiiTxCbHdcpPrepareForKsvListProcessing())
            {
               // Keep waiting if repeater isn't ready to continue
               break;
            }
            isKsvListReadingDone = SiiDrvTpiKsvListPortionSizeGet(&ksvListSize);

//           NOTE: the following debug message is commented to avoid adverse delays caused by printing
//           DEBUG_PRINT(TX_MSG_DBG, "fifo level before = 0x%02X, done: %s\n", (int) ksvListSize,
//                                    isKsvListReadingDone ? "Y" : "N");

		if (CopyKsvListPortion())
		{
//          NOTE: the following read and debug message is commented to avoid adverse delays caused by printing
//          isKsvListReadingDone = SiiDrvTpiKsvListPortionSizeGet(&ksvListSize);
//			DEBUG_PRINT(TX_MSG_DBG, "fifo level after = 0x%02X, done: %s\n", (int) ksvListSize,
//					                 isKsvListReadingDone ? "Y" : "N");
			pHdcp->authState = SI_TX_HDCP__V_CHECK;
		}
		// don't break here to expedite switching to the "V check" phase
			
        case SI_TX_HDCP__V_CHECK:
			// Downstream HDCP Part 2 is successfully finished,
			// after downstream V' matches own V.
			if (SiiDrvTpiHdcpIsPart2Done())
			{
				FinishAuthPart2();
			}
            break;

        case SI_TX_HDCP__OFF:
            if (pHdcp->isAuthRequested)
            {
                pHdcp->authState = SI_TX_HDCP__WAIT_FOR_START;

                AvMuteEnable(true);

                SiiDrvTpiHdcpProtectionEnable(false);

                pHdcp->timeoutForPart1AndPart2 = TMO_MS_HDCP__MAX_AUTH_ATTEMPT;
                pHdcp->timeoutForDdcNack = TMO_MS_HDCP__MAX_DDC_NACK;

                if (pHdcp->isRepeaterMode)
                {
                    SiiTxCbHdcpSetUsRepeaterBcaps(false);
                }
            }
            else if (pHdcp->dsDdcAckCheck)
            {
                if (SiiDrvTpiIsDownstreamHdcpAvailable())
                {
                    if (pHdcp->isRepeaterMode)
                    {
                        SiiTxCbHdcpDsDdcAck(true);
                    }
                    pHdcp->dsDdcAckCheck = false;
                }
            }
            break;

        case SI_TX_HDCP__WAIT_FOR_START:
            // control comes here when HDCP authentication was requested from US device

            if (TxIsRunning() && hp && rsen && TxIsTmdsEnabled() && (pHdcp->tmdsCountdown == 0))
            {
                // TX chip input is provided and stable
                // (If scaler or/and OSD are installed in the schematic,
                // their output signals are provided and stable as well as their input ones)
                if (SiiDrvTpiIsDownstreamHdcpAvailable()) // DDC ACK check
                {
                    // DS device must be ready to start HDCP authentication

                    SiiDrvTpiHdcpProtectionEnable(false); // just in case
                    SiiDrvTpiHdcpProtectionEnable(true); // start authentication
                    pHdcp->authState = SI_TX_HDCP__WAIT_FOR_DONE;

                    // DS DDC access was with ACK
                    pHdcp->dsDdcAckCheck = false;
                    pHdcp->error = 0;
                }
                else
                {
                    // DS DDC access was with NACK (i.e. NO ACK)
                    if (pHdcp->timeoutForDdcNack == 0)
                    {
                        // If the time is out and there is still no ACK,
                        // consider DS device as not having HDCP function.
                        pHdcp->error |= ERR_HDCP__NACK;
                        pHdcp->authState = SI_TX_HDCP__FAILED;
                        pHdcp->fbMsg = SI_TX_FB_MSG_ERR_HDCP_AUTH_FAILED;

                        SiiDrvTpiHdcpProtectionEnable(false);

                        if (pHdcp->isRepeaterMode)
                        {
                            //SiiDrvTpiDdcMasterReset();
                            //pHdcp->error |= ERR_HDCP__V;
                        	//not fatal here to avoid repeater toggle HPD on Rx
                        	//repeater will use the DdcAck below to decide HPD toggle
                            SiiTxCbHdcpInformUsDeviceAboutFailure(false);
                        }
                        //report the DDC NACK after Inform the fatal failure
                        if (pHdcp->upstreamDdcCtrl)
                        {
                            // No ACK on DS DDC bus
                            // Probably DS device has No HDCP function.
                            SiiTxCbHdcpDsDdcAck(false);
                        }


                    }
                }

            }
            break;

        case SI_TX_HDCP__WAIT_FOR_RETRY:
            if (IS_UPDATED_TIMER_EXPIRED(pHdcp->timeToRetry, timeDelta))
            {
                pHdcp->authState = SI_TX_HDCP__OFF; //SI_TX_HDCP__WAIT_FOR_START;
                //AvMuteEnable(true); // 100218 for SI_TxHdcpForceDsAuthentication()
                //SiiDrvTpiHdcpProtectionEnable(false);
            }
            break;

        case SI_TX_HDCP__WAIT_FOR_DONE:
            break;

        case SI_TX_HDCP__WAIT_FIFO_READY:
            if (SiiDrvTpiIsZeroKsvListReady())
            {
                uint8_t timeOutMs = TMO_MS_HDCP__ZERO_KSV_READY_MAX;

                if(!SiiTxCbHdcpPrepareFor0KsvListProcessing())
                {
                    // Keep waiting if repeater isn't ready to continue
                	break;
                }
                // Wait until Part 2 is done after the above preparations stabilize
                while (!SiiDrvTpiHdcpIsPart2Done() && timeOutMs)
                {
                    SiiOsTimerWait(1);
                    timeOutMs--;
                }

                if (timeOutMs != 0) // Part 2 is done before the timeout
                {
                    uint8_t dsBstatus[2] = { 0, 0 };
                    ProcessBstatus(dsBstatus);
                    FinishAuthPart2();
                    NewKsvHash(); // reset DS KSV list hash because it doesn't exist
                    pHdcp->authState = SI_TX_HDCP__AUTHENTICATED;
                }
                else
                {
                    // Try to re-authenticate if failed to receive KSV Ready (part 2 done)
                    TxHdcpRequestReauthentication();
                }
            }
            else if(SiiDrvTpiHdcpIsPart2Done())
            {
                uint8_t dsBstatus[2] = { 0, 0 };
                ProcessBstatus(dsBstatus);
                FinishAuthPart2();
                NewKsvHash(); // reset DS KSV list hash because it doesn't exist
                pHdcp->authState = SI_TX_HDCP__AUTHENTICATED;
            }
            break;

        case SI_TX_HDCP__AUTHENTICATED:
            if (pHdcp->minimumUnmuteCountdown == 0)
            {
                AvMuteEnable(false);
                pHdcp->authState = SI_TX_HDCP__SECURE;
                pHdcp->fbMsg = SI_TX_FB_MSG_HDCP_AUTH_DONE;
            }
            break;

        case SI_TX_HDCP__SECURE:
            break;

        case SI_TX_HDCP__FAILED:
            if (pHdcp->isAuthRequested)
            {
                if (SiiDrvTpiIsDownstreamHdcpAvailable())
                {
                    // check for DDC ACK;
                    pHdcp->dsDdcAckCheck = false;

                    SiiTxCbHdcpDsDdcAck(true);

                    if (!SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_HDMI))
                    {
                        // shadow BCAPS is only available when DDC ACKs
                        if ((SiiDrvTpiBcapsGet() & BIT_HDCP_DDC__BCAPS__FAST_RE_AUTH) == 0)
                        {
                            // For devices not capable of fast re-authentication.
                            // (see page 19 (2.4) of HDCP 1.2)
                            // Temporary turn TMDS output OFF for minimum 100ms.
                            TxTmdsBlankPause();
                        }
                    }

                    if (0 != pHdcp->timeoutForPart1AndPart2)
                    {
                        // If time for part one is not out,
                        // try another attempt of DS authentication.
                        pHdcp->authState = SI_TX_HDCP__WAIT_FOR_RETRY;
                        pHdcp->timeToRetry = TIM_MS_HDCP__RP_PART1_RETRY;
                    }
                }
                else
                {
                    pHdcp->dsDdcAckCheck = true;
                }

                if (pHdcp->timeoutForPart1AndPart2 == 0)
                {
                    if (pHdcp->isRepeaterMode)
                    {
                        // Repeater mode
                        SiiTxCbHdcpInformUsDeviceAboutFailure(true);
                        pHdcp->isReauthRequested = false;
                        pHdcp->isAuthRequested = false;
                    }
                    else
                    {
                        // TX Only mode
                        pHdcp->isReauthRequested = true;
                    }
                    pHdcp->authState = SI_TX_HDCP__OFF;
                }
            }
            else
            {
                // Authentication is not required.
                // Do nothing.
            }
            break;

        default:
            break;
    }

    // Update HDCP timers
    UPDATE_TIMER(pHdcp->timeoutForPart1AndPart2, timeDelta);
    UPDATE_TIMER(pHdcp->timeoutForDdcNack, timeDelta);
    UPDATE_TIMER(pHdcp->minimumUnmuteCountdown, timeDelta);
    UPDATE_TIMER(pHdcp->tmdsCountdown, timeDelta);

    PrintHdcpStatus();
}



//-------------------------------------------------------------------------------------------------
//  TX HDCP API functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Select Tx device instance in multi-transmitter system.
//!
//! @param[in]  device_idx - zero based index of an instance.
//-------------------------------------------------------------------------------------------------

#if SII_NUM_TX > 1
void TxHdcpInstanceSet(uint8_t deviceIndex)
{
    if(deviceIndex < SII_NUM_TX)
    {
        pHdcp = &HdcpInstRec[deviceIndex];
    }
}
#endif // SII_NUM_TX > 1


//-------------------------------------------------------------------------------------------------
//! @brief      Initialization of TX HDCP module.
//-------------------------------------------------------------------------------------------------

void TxHdcpInit(void)
{
    memset(pHdcp, 0, sizeof(txHdcpInstanceRecord_t));

    pHdcp->prevAuthState = SI_TX_HDCP__INITIAL; // will be changed in print_status()
    TxHdcpReset();

    // option for unconditional re-authentication
    SiiDrvTpiHdcpDynamicAuthenticationEnable(true);

#if (SI_TX_COMP_UNIT_TESTS == ENABLE)
    TestKsvHash();
#endif
}


//-------------------------------------------------------------------------------------------------
//! @brief      TX HDCP module initialization
//!
//!             The function has to be called once during system initialization.
//!             The function provides TX HDCP module with configuration settings.
//!
//! @param[in]  isRepeaterMode     - true for repeater, false for pure source,
//! @param[in]  tmdsOnToUnmuteTime - Minimum time from turning TMDS ON to AV unmute (in 100ms units),
//! @param[in]  usDdcCtrl
//!             - true: upstream HDCP DDC enabling/disabling depends on downstream DDC
//!             - false: upstream HDCP DDC does not depend on downstream
//-------------------------------------------------------------------------------------------------

void TxHdcpConfig(bool_t isRepeaterMode, time_ms_t tmdsOnToUnmuteTime, bool_t usDdcCtrl)
{
    pHdcp->isRepeaterMode = isRepeaterMode;
    pHdcp->tmdsOnToUnmuteTime = tmdsOnToUnmuteTime;
    pHdcp->upstreamDdcCtrl = usDdcCtrl;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Reset TX HDCP module without scheduling new authentication.
//!
//!             Resets HDCP function to initial state. It is needed on input change,
//!             going into of out of standby, etc.
//-------------------------------------------------------------------------------------------------

void TxHdcpStop(void)
{
	TxHdcpReset();
	pHdcp->isAuthRequested = false;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Request to start downstream authentication.
//!
//!             The function starts downstream authentication if the device was not
//!             authenticated before. If it is already authenticated,
//!             the function does nothing.
//-------------------------------------------------------------------------------------------------

void TxHdcpStart(void)
{
    pHdcp->isAuthRequested = true;

	switch (pHdcp->authState)
	{
		case SI_TX_HDCP__AUTHENTICATED:
		case SI_TX_HDCP__SECURE:
			// The upstream KSV FIFO has already been preloaded,
			// no need doing downstream authentication.
			if (pHdcp->isRepeaterMode)
			{
				SiiTxCbHdcpUsOnlyAuthentication();
			}
			break;
		case SI_TX_HDCP__FAILED:
			pHdcp->authState = SI_TX_HDCP__OFF;
			break;
		case SI_TX_HDCP__WAIT_FIFO_READY:
		case SI_TX_HDCP__WAIT_FOR_RETRY:
			// Terminate previous HDCP work, if in progress
			TxHdcpReset();
			break;
		default:
			break;
	}

    PrintHdcpStatus();
}


//-------------------------------------------------------------------------------------------------
//! @brief      TMDS Output Enable event handler.
//!
//!             This function schedules start of HDCP authentication after a short safety delay.
//!             Should be called whenever TMDS output has been enabled after disable state.
//!             Example: resuming from Standby state.
//-------------------------------------------------------------------------------------------------

void TxHdcpOnOutputEnable(void)
{
    pHdcp->tmdsOnCountDownStart = true;
    if(SiiEdidTxDsDeviceRemedyMaskGet() & SI_TX_EDID_REMEDY_SP_TEST)
    {
        pHdcp->minimumUnmuteCountdown = 0;
    }
    else
    {
        // Wait after turning on before starting authentication
        pHdcp->minimumUnmuteCountdown = pHdcp->tmdsOnToUnmuteTime;

        // Increase the delay for noncompliant devices
        if(SiiEdidTxDsDeviceRemedyMaskGet() & SI_TX_EDID_REMEDY_DELAY_DS_AUTH)
        {
            if(pHdcp->minimumUnmuteCountdown < TIM_MS_HDCP__MIN_MUTE_TIME)
                pHdcp->minimumUnmuteCountdown = TIM_MS_HDCP__MIN_MUTE_TIME;
        }
    }
    DEBUG_PRINT(TX_MSG_DBG, "TX HDCP min mute time %d ms\n", (int) pHdcp->minimumUnmuteCountdown);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Reset the HDCP TX engine.
//!
//-------------------------------------------------------------------------------------------------

void TxHdcpReset(void)
{
	CancelHdcpAuthentication();

	pHdcp->authState = SI_TX_HDCP__OFF;
	pHdcp->error = 0;
	pHdcp->timeoutForPart1AndPart2 = 0;
	pHdcp->timeoutForDdcNack = 0;
	pHdcp->timeToRetry = 0;
	pHdcp->isReauthRequested = false;

	// Disable HDCP on TX as well, otherwise under some circumstances
	// the DS device could get encrypted data before authentication
	// (and the data is broken).
	SiiDrvTpiHdcpProtectionEnable(false);

	pHdcp->dsDdcAckCheck = false;

	if (SiiDrvTpiIsDownstreamHdcpAvailable())
	{
		if (pHdcp->isRepeaterMode)
		{
			SiiTxCbHdcpDsDdcAck(true);
		}
	}
	PrintHdcpStatus();
}



//-------------------------------------------------------------------------------------------------
//! @brief      HDCP Interrupt handler routine.
//!
//!             More detailed description.
//!
//! @param[in]  interrupts - a 16-bit word; every bit represents an interrupt flag.
//-------------------------------------------------------------------------------------------------

void TxHdcpInterruptHandler(uint16_t interrupts)
{

    if (interrupts & SI_TX_IRQ__HPD)
    {
        AvMuteEnable(true);
        SiiDrvTpiHdcpProtectionEnable(false);

        pHdcp->isAuthRequested = false;
        pHdcp->authState = SI_TX_HDCP__OFF;
    }

    if (interrupts & SI_TX_IRQ__RX_SENSE)
    {
        AvMuteEnable(true);
        SiiDrvTpiHdcpProtectionEnable(false);

        pHdcp->authState = SI_TX_HDCP__OFF;
    }

    if (interrupts & (SI_TX_IRQ__HDCP_CHNG))
    {
        uint8_t hdcpStatus = SiiDrvTpiHdcpStatusGet();

        PrintHdcpQueryStatus(hdcpStatus);

        switch (hdcpStatus & MSK_TPI__HDCP_QUERY__STATUS)
        {
            case VAL_TPI__HDCP_QUERY__STATUS_NORMAL:
            case VAL_TPI__HDCP_QUERY__STATUS_SUSPENDED:
                if (pHdcp->authState == SI_TX_HDCP__WAIT_FOR_DONE)
                {
                    // Part 1 is done
                    if (hdcpStatus & BIT_TPI__HDCP_QUERY__DS_REPEATER)
                    {
                        pHdcp->authState = SI_TX_HDCP__WAIT_FIFO_READY;
                    }
                    else
                    {
                        // Part 1 is done and no downstream Part 2 needs to be done
                        uint8_t aDsBStatus[2] = { 0, 0 };
                        ProcessBstatus(aDsBStatus);
                        if (pHdcp->isRepeaterMode)
                        {
                            SiiTxCbHdcpPrepareForKsvListProcessing();
                        }
                        SendBksvToUsFifo();

                        pHdcp->authState = SI_TX_HDCP__AUTHENTICATED;
                    }
                }
                break;

            case VAL_TPI__HDCP_QUERY__STATUS_FAILED:
                // failure during authentication case
                AvMuteEnable(true);
                SiiTxCbHdcpInformUsDeviceAboutFailure(false);
                if (pHdcp->isRepeaterMode && ((hdcpStatus & BIT_TPI__HDCP_QUERY__DS_REPEATER)))
                {
                    uint8_t aDsBStatus[2] = { 0, 0 };
                    SiiDrvTpiBStatusGet(aDsBStatus);
                    // Note, in case of R0 failure (for example) the BSTATUS shadow registers
                    // are not read, but since the registers value is automatically cleared
                    // on clearing 0x2A.0, the EXCEED bits are not set. By checking the EXCEED bits,
                    // it is possible to distinguish the DEV_EXCCED and CASCADE_EXCEED
                    // failures from any other failure.
                    if ((aDsBStatus[0] & BIT_HDCP_DDC__BSTATUS1__DEV_EXC) || (aDsBStatus[1] & BIT_HDCP_DDC__BSTATUS2__CAS_EXC))
                    {
                        ProcessBstatus(aDsBStatus);
                        if (pHdcp->isRepeaterMode)
                        {
                            SiiTxCbHdcpSetUsRepeaterBcaps(true);
                        }
                        pHdcp->error |= ERR_HDCP__DS_EXCEED;
                    }
                }
                SiiDrvTpiHdcpProtectionEnable(false);
                if( (pHdcp->authState == SI_TX_HDCP__KSV_FIFO_READ)
					||(pHdcp->authState == SI_TX_HDCP__V_CHECK))					
                {
                    // error happened during the KSV list reading,
                    // need to restart the DDC access state machine.
                    SiiDrvTpiDdcMasterReset();
                    // Most probably the error is caused by V mismatch.
                    pHdcp->error |= ERR_HDCP__V;
                }
                else if (pHdcp->authState == SI_TX_HDCP__WAIT_FIFO_READY)
                {
                    // error happened while waiting for KSV FIFO READY bit
                    pHdcp->error |= ERR_HDCP__P2_TO;
                }
                pHdcp->authState = SI_TX_HDCP__FAILED;
                break;

            case VAL_TPI__HDCP_QUERY__STATUS_LOST:
                // failure on Part 3 case
                pHdcp->authState = SI_TX_HDCP__FAILED;
                AvMuteEnable(true);
                SiiTxCbHdcpInformUsDeviceAboutFailure(true);
                SiiDrvTpiHdcpProtectionEnable(false);
                break;
        }

    }

    if (interrupts & SI_TX_IRQ__KSV_READY)
    {
        if (pHdcp->authState == SI_TX_HDCP__WAIT_FIFO_READY)
        {
            uint8_t aDsBStatus[2];
            SiiDrvTpiBStatusGet(aDsBStatus);
            ProcessBstatus(aDsBStatus);

            // Start new KSV hash calculation
            NewKsvHash();

            pHdcp->authState = SI_TX_HDCP__KSV_FIFO_READ;

        }
    }

    PrintHdcpStatus();
}


//-------------------------------------------------------------------------------------------------
//! @brief      HDCP Timer event handler.
//!
//!             This function is to be called periodically.
//!
//! @param[in]  timeDelta - time in ms elapsed since the last call.
//!
//! @return     Maximum time interval (in ms) to wait before the next call.
//-------------------------------------------------------------------------------------------------

time_ms_t TxHdcpTimerHandler(time_ms_t timeDelta)
{
    time_ms_t maxTimeBeforeNextCallMs = TIM_MS_HDCP__HANDLER_DEFAULT_INVOCATION;

    if (pHdcp->isReauthRequested)
    {
        // The reason to use dedicated flag (pHdcp->isReauthRequested)
        // is to align pHdcp->timeToRetry counter with HDCP task time.
        pHdcp->isReauthRequested = false;

        CancelHdcpAuthentication();

        pHdcp->authState = SI_TX_HDCP__WAIT_FOR_RETRY;
        pHdcp->timeToRetry = TIM_MS_HDCP__DS_PART1_RETRY;
    }
    else
    {
        OnTimer(timeDelta);

        switch (pHdcp->authState)
        {
            case SI_TX_HDCP__WAIT_FOR_START:
                // Time critical case for repeaters to pass 100ms AKSV propagation test.
            case SI_TX_HDCP__KSV_FIFO_READ:
			case SI_TX_HDCP__V_CHECK:				
                // Time critical case for repeaters to process 127KSVs in 500ms.
                maxTimeBeforeNextCallMs = TIM_MS_HDCP__HANDLER_URGENT_INVOCATION;
                break;

            default:
                if (pHdcp->tmdsCountdown)
                {
                    maxTimeBeforeNextCallMs = pHdcp->tmdsCountdown;
                }
        }
    }

    return maxTimeBeforeNextCallMs;
}
 

//-------------------------------------------------------------------------------------------------
//! @brief      Schedules HDCP re-authentication if HDCP is in operational state.
//-------------------------------------------------------------------------------------------------

void TxHdcpRequestReauthentication(void)
{
    switch (pHdcp->authState)
    {
        case SI_TX_HDCP__WAIT_FOR_DONE:
        case SI_TX_HDCP__WAIT_FIFO_READY:
        case SI_TX_HDCP__KSV_FIFO_READ:
        case SI_TX_HDCP__V_CHECK:
        case SI_TX_HDCP__AUTHENTICATED:
        case SI_TX_HDCP__SECURE:
            pHdcp->isReauthRequested = true;
            AvMuteEnable(true);
            SiiDrvTpiHdcpProtectionEnable(false);
            DEBUG_PRINT(TX_MSG_ALWAYS, "HDCP: Re-authentication request\n");
            break;
        default:
            break;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return HDCP error code. @see HDCP_ERRORS
//-------------------------------------------------------------------------------------------------

uint16_t TxHdcpLastErrorGet(void)
{
    return pHdcp->error;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return HDCP message reflecting HDCP engine current state.
//-------------------------------------------------------------------------------------------------

txFeedbackMsg_t TxHdcpLastMessageGet(void)
{
    txFeedbackMsg_t fbMessage = pHdcp->fbMsg;
    pHdcp->fbMsg = SI_TX_FB_MSG_NONE; // clear after reporting

    return fbMessage;
}


