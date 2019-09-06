//***************************************************************************
//!file     si_repeater_component.h
//!brief    Silicon Image Repeater component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_REPEATER_COMPONENT_H__
#define __SI_REPEATER_COMPONENT_H__

#include "si_debug.h"
#include "si_repeater_avr_config.h"
#include "si_tx_config.h"
#include "si_drv_audio_mix.h"


//------------------------------------------------------------------------------
//  Manifest Constants
//------------------------------------------------------------------------------


#define UNPLUG_TIMEOUT_MS         500     //add margin of 300ms for timer started ahead of HPD low
#define UNPLUG_TIMEOUT_MHL_MS     150     //Fix for CBUS CTS failure

//-------------------------------------------------------------------------------------------------
// Repeater Component type definitions
//-------------------------------------------------------------------------------------------------

typedef enum rptHpdTxEvent
{
    SI_RPT_TX_HPD_OFF       = 0x0,		//Downstream has HPD off, RSEN off
    SI_RPT_TX_HPD_ON        = 0x01,     //Downstream has HPD on, RSEN on
    SI_RPT_TX_HPD_TOGGLE	= 0x02		//Downstream has HPD off, RSEN on
} rptHpdTxEvent_t;

typedef struct
{
	bit_fld_t isRptTxOn					:1;	/*if the tx in the pipe*/
	bit_fld_t isRptTxHpdOn				:1; /*if the tx has HPD active*/
	bit_fld_t isHdcpRequested			:1; /*if the HDCP is turned on for the tx*/
	bit_fld_t isBstatRdy				:1; /*if the tx has BSTATUS ready*/
	bit_fld_t isRptTxKsvRdy				:1; /*if the tx has KSV ready*/
	bit_fld_t isRptTxShaDone			:1; /*if SHA is done for the tx */
	bit_fld_t isRptDsNoHdcp 		    :1; /*if the ds supports HDCP */
	bit_fld_t isRptTxHdcpDone			:1; /*if the tx HDCP is done*/
}repeaterTxState_t;

typedef struct
{
    repeaterTxState_t				txStat;  			/*which tx are connected*/
    bool_t							wasAuthened;		/*the DS was authened, */
    uint16_t						ksv_length;			/*ksv length*/
    uint8_t 						txsBksv[5];			/*the BKSVs from tx in the pipe*/
    uint8_t							txBstatus[2]; 		/*Bstatus of  tx*/
}repeaterTxHdcpData_t;
 
typedef struct
{
    uint8_t							inputPort;                  /*the single input for the rpt*/
    repeaterTxHdcpData_t		    txOnThePipe[SII_NUM_TX];    /*info of this tx on the rpt*/
    uint8_t							numberOfTxAuthed;           /*num of authenticated tx*/
    uint8_t							numberOfTxHasBstat;         /*num of bstat ready tx*/
    uint8_t							numberOfTx;				    /*num of tx connected to the rept*/
    uint16_t						total_ksv_length;			/*add up the ksv length*/
    uint8_t 						txsBksv[SII_NUM_TX * 5];	/*all the BKSVs from each tx */
    uint8_t							rxBstatus[2]; 	            /*Bstatus for rx*/
    bool_t							repeaterActive;	            /*if pipe doesn't have Downstream
                                                                associates to it, it becomes sink*/
}RepeaterInstanceData_t;


typedef enum rptErrCode
{
    SI_RPT_ERROR_CODE_NO_ERROR                 = 0x00, //!< Success
    SI_RPT_ERROR_CODE_INSTANCE_NOT_EXIST       = 0x01, //!< Non-existing instance record index provided
    SI_RPT_ERROR_CODE_INVALID_PARAMETER        = 0x02, //!< Parameter of a function has prohibited value
    SI_RPT_TOTAL_NUM_OF_ERROR_CODES,

} rptErrCode_t;

typedef enum RepeaterTopoMode
{
	SI_RPT_MODE_INITIAL,
	SI_RPT_MODE_SINGLE_REPEATER,
	SI_RPT_MODE_MULTIPLE_REPEATER
}RepeaterTopoMode_t;

bool_t SiiRepeaterInitialize(bool_t enable, bool_t isPowerUp);
void SiiRepeaterHandler(clock_time_t  msDiff);
void SiiRepeaterSwitchModeSet(RepeaterTopoMode_t repeaterMode);
void SiiRepeaterInstanceSet(uint8_t instance);

void SiiRepeaterPipeTxConnectionSet(uint8_t txId, bool_t qOn);
void SiiRepeaterPipeConfigureSet(RepeaterInstanceData_t *conf);
void SiiRepeaterSetActiveSource(uint8_t portIndex);
bool_t SiiRepeaterInterruptPending( void );
void SiiRepeaterCbTxSelect(uint8_t instance);
void SiiRepeaterCbTxResume(void);
void SiiRepeaterCbTxStandby(void);

void SiiRepeaterOnTxHpdChange(rptHpdTxEvent_t newHpd, uint8_t txId);
void SiiRepeaterOnTxHdcpDone(uint8_t txId);
void SiiRepeaterHdcpUsOnlyAuthentication(uint8_t txId);
bool_t SiiRepeaterHdcpProcessDsBstatus(uint8_t dsBstatus[2], uint8_t txId);
void SiiRepeaterHdcpFinishUsPart2(uint8_t aDsBksv[5], uint8_t txId);
void SiiRepeaterHdcpSetBcapsFifoRdyBits(bool_t switchOn);
bool_t SiiRepeaterHdcpPrepareForKsvListProcessing(uint8_t txId);
bool_t SiiRepeaterHdcpPrepareFor0KsvListProcessing(uint8_t txId);
void SiiRepeaterHdcpInformUsDeviceAboutFailure(bool_t isFatal, uint8_t txId);
void SiiRepeaterHdcpDsDdcAck(bool_t isAcknowledged, uint8_t txId);
void SiiRepeaterForceShaSet(uint8_t txNum, bool_t isStart  );
void SiiRepeaterHdcpDsOnlyAuthentication(uint8_t portIndex, bool_t qOn);




#endif // __SI_HDCP_REPEATER_MAIN_H__

