//************************************************************************************************
//! @file   si_edid_black_list.c
//! @brief  Blacklisting related manipulations
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/

#include "si_edid_black_list.h"

extern ROM const remedyListElement_t EdidRemedyList[];


//-------------------------------------------------------------------------------------------------
//! @brief      Generates a remedy mask for a given DS EDID data.
//!
//! @param[in]  pEdidInfo - pointer to decoded EDID structure
//!
//! @return     @see TX_EDID_REMEDY_MASKS.
//-------------------------------------------------------------------------------------------------

uint16_t EdidTxRemedyMaskBuild(decodedEdid_t *pEdidInfo)
{
	uint16_t remedy = 0;
	int i;
	uint16_t m_id = pEdidInfo->ManufacturerId;
	uint16_t p_id = pEdidInfo->ProductId;
	
	// search beside white models first
	for (i=0; tEndOfList != EdidRemedyList[i].Type; i++)
	{
		if (tNoLimitation == EdidRemedyList[i].Type)
			if ((EdidRemedyList[i].ManufacturerId == m_id) &&
				(EdidRemedyList[i].ProductId == p_id))
				goto exit;
	}

	// search for bad manufacturer
	for (i=0; tEndOfList != EdidRemedyList[i].Type; i++)
	{
		if (tManufacturerLimitation == EdidRemedyList[i].Type)
			if (EdidRemedyList[i].ManufacturerId == m_id)
			{
				remedy |= EdidRemedyList[i].RemedyMask;
				break;
			}
	}

	// search for bad products
	for (i=0; tEndOfList != EdidRemedyList[i].Type; i++)
	{
		if (tProductLimitation == EdidRemedyList[i].Type)
			if ((EdidRemedyList[i].ManufacturerId == m_id) &&
				(EdidRemedyList[i].ProductId == p_id))
			{
				remedy |= EdidRemedyList[i].RemedyMask;
				break;
			}
	}

	// search for bad products made by a manufacturer during specified period of time;
	// if manufacturer ID in the list is 0, the remedy applies to any manufacturer
	for (i=0; tEndOfList != EdidRemedyList[i].Type; i++)
	{
		if (tLimitationOnTheseDays == EdidRemedyList[i].Type)
			if ((EdidRemedyList[i].ManufacturerId == 0) || (EdidRemedyList[i].ManufacturerId == m_id))
			{
				uint8_t t_year_begin = (uint8_t) (EdidRemedyList[i].ProductId >> 8);
				uint8_t t_year_end = (uint8_t) EdidRemedyList[i].ProductId;
				uint8_t m_year = pEdidInfo->ManufactureYear;
				if (/*m_year &&*/ (m_year >= t_year_begin) && (m_year <= t_year_end))
				{
					remedy |= EdidRemedyList[i].RemedyMask;
					break;
				}
			}
	}

exit:
	return remedy;
}

