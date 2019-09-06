//***************************************************************************
//!file     si_cbus_internal.h
//!brief    Silicon Image CBUS Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CBUS_INTERNAL_H__
#define __SI_CBUS_INTERNAL_H__
#include "si_device_config.h"
#include "si_cbus_enums.h"
#include "si_cbus_component.h"

//------------------------------------------------------------------------------
//  Manifest Constants
//------------------------------------------------------------------------------

#define 	CBUS_HPD_WAIT_TIMER         	120
#define 	CBUS_ABORT_TIMER				2100
#define 	CBUS_MSG_RSP_TIMER				3200
#define 	CBUS_WB_REQ_TIMER				1100
#define 	CBUS_INIT_TIMER					500
#define 	CBUS_RCPE_RCPK_GAP_TIMER		300
#define		CBUS_RCP_RCP_ABORT_TIMER		1050

//------------------------------------------------------------------------------
//  CBUS Component Instance Data
//------------------------------------------------------------------------------
typedef struct
{
	int         		instanceIndex;			// cbus instance index
    bool_t  			connected;      		// True if a connected MHL port
    uint8_t             port;           		// Port assigned to this CBUS channel
    uint8_t             supportMask;            // Logical Device support bitmap for this CBUS channel
	uint8_t 			state;          		// State of command execution for this channel
    uint8_t 			activeIndex;    		// Active queue entry.
    uint8_t				queueDepth;				// cbus queue depth
	uint8_t				lastErrorCode;			// last error code of the called API
    uint8_t				lastCbusSentCmd;		// last cmd sent on cbus
    uint8_t				lastCbusRcvdCmd;		// last cmd received on cbus
    uint8_t				lastCbusRcvdData;		// last data received on cbus
    uint8_t				lastReadDevCapReg;		// last read device capability re
    uint8_t				wbStartOffset;			// write burst start offset
	uint8_t 			wbLength;				// write burst length
	bool_t				pathEnableSent;			// PATH_EN sent or not

	uint16_t    		miscFlags;				// misc flags on cbus showing the state it is in

	clock_time_t		abortTimer;				// timer to stop sending cmds after an abort is received
	clock_time_t		hpdWaitTimer;			// Timer for hpd_toggle
	clock_time_t		cbusMsgRspWaitTimer;	// Timer for hpd_toggle
	clock_time_t		initTimer;				// The initial timer after which we can send dev_cap_rdy
	clock_time_t		reqWrtTimer;			// Timer for request write during a WRITE_BURST command
	clock_time_t		rcpRapAbortTimer;		// if we don't receive RCPE/RCPK by this time, we should abort RCP/RAP cmd
	clock_time_t		rcpeRcpkGapTimer;		// Time gap between two RCPE and RCPK commands
	clock_time_t		waitTimer;				// wait until the cbus cmd is completed before sending a new one..

	uint8_t				mscXfrAbortReason;		// MSC cmd abort reason, cmd initiated from this device
	uint8_t				mscFwrAbortReason;		// MSC cmd abort reason, cmd initiated from peer device
												// 9587/9589 is the follower this case
	uint8_t				ddcAbortReason;			// DDC abort reason

	cbus_req_t  		request[ CBUS_MAX_COMMAND_QUEUE ];	// cbus queue

	// 3D related
	uint8_t			sending_3D_info;		// sending 3D info
	uint8_t			cbus3Dstate;			// state of sending 3D info

	uint8_t*			ptr3dVic;				// ptr to 2D SVDs supported in 3D
	uint8_t*			ptr3dDtd;				// ptr to 2D DTDs that are supported in 3D
	uint8_t				cur3dVicSeq;			// current Write Burst sequence for SVDs' transfer
	uint8_t				cur3dDtdSeq;			// current Write Burst sequence for DTDs' transfer
	bool_t				serving3dVicReq;		// if we are sending SVD data
	bool_t				serving3dDtdReq;		// if we are sending DTD data
	//  Bug 31921 - Melbourne on power up, Unconnected port to MHL connected port switch not working 
	bool_t				sendMHLCommands;

}	CbusInstanceData_t;

#if (MHL_20 == ENABLE)
typedef enum  // 3D state
{
    WB_3D_IDLE = 0,
    WB_3D_SENDING_VIC,
    WB_3D_SENDING_DTD
}Cbus3DState_t;

#define BURST_3D_DTD (0x0011)   //Indicates Video Index Codes for 3D
#define BURST_3D_DTD_H (0x00)
#define BURST_3D_DTD_L (0x11)
#define BURST_3D_VIC (0x0010)   //Indicates Detailed Timing Descriptors for 3D
#define BURST_3D_VIC_H (0x00)
#define BURST_3D_VIC_L (0x10)

#define BIT_LR_SUPP (BIT2)
#define BIT_TB_SUPP (BIT1)
#define BIT_FS_SUPP (BIT0)

typedef struct MHL_3D_VDI
{
    uint8_t vdi_l;
    uint8_t vdi_h;    
}  MHL_3D_VDI_t;

typedef struct MHL3DSupportData
{
    uint8_t        tot;            // Total Entries
    MHL_3D_VDI_t        vdi[32];     // VDI
}   MHL3DSupportData_t;
#endif

extern CbusInstanceData_t cbusInstance[MHL_MAX_CHANNELS];
extern CbusInstanceData_t *pCbus;

bool_t  SiiMhlRxSendRcpk( uint8_t keyCode);
bool_t  SiiMhlRxSendRcpe( uint8_t cmdStatus );
#if ((MHL_20 == ENABLE) || (MHL_EXTRA == ENABLE))
bool_t SiiMhlRxSendUcpk ( uint8_t utf8Code);
bool_t SiiMhlRxSendUcpe ( uint8_t cmdStatus );
#endif
void    CbusProcessRcpRap( uint8_t cmd, uint8_t msgData );
bool_t  CBusSendMscMsgCmd( uint8_t subCmd, uint8_t data );

#endif // __SI_CBUS_INTERNAL_H__
