//************************************************************************************************
//! @file   si_edid_black_list.h
//! @brief  Blacklisting related manipulations
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/

#ifndef SII_EDID_BLACK_LIST_H
#define SII_EDID_BLACK_LIST_H

#include "si_common.h"
#include "si_edid_tx_component.h"
#include "si_edid_tx_parsing.h"


typedef enum blackLimits PACKED
{
	tEndOfList = 0, // a marker of the end of the list
	tNoLimitation, // useful for good products made by a manufacturer marked as tManufacturerLimitation
	tManufacturerLimitation, // applied for any products by given manufacturer
	tProductLimitation, // applied for a given product only
	//tLimitationBeforeThisDate, // applied for any product of given manufacturer
	//							//made before given date (the date is in ProductId field)
	//tLimitationAfterThisDate, // applied for any product of given manufacturer
	//							//made after given date (the date is in ProductId field)
	tLimitationOnTheseDays, // a manufacturer produced a product with limitations during given period of time

} blackLimits_t;


typedef struct blackListElement PACKED
{
	uint16_t		ManufacturerId;
	uint16_t		ProductId; // or a date
	uint16_t		RemedyMask;
	blackLimits_t	Type;

} remedyListElement_t;


//-------------------------------------------------------------------------------------------------
//  Internal API functions
//-------------------------------------------------------------------------------------------------

uint16_t EdidTxRemedyMaskBuild( decodedEdid_t *pEdidInfo);


#endif // SII_EDID_BLACK_LIST_H
