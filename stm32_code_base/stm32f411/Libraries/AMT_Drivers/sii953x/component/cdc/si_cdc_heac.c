//***************************************************************************
//! @file     si_cdc_heac.c
//! @brief    Low level CDC API
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2009, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_drv_cpi.h"
#include "si_drv_arc.h"
#include "si_cec_component.h"
#include "si_cdc_component.h"
#include "si_cdc_internal.h"

#ifndef CDC_PC_SIMULATION
#include <string.h>    
#else
#include "stdio.h"
#include "memory.h"
#include "debug_print.h"
#endif

//------------------------------------------------------------------------------
// Service Functions
//------------------------------------------------------------------------------

// Local function prototypes
static bool_t CdcPhysAddrToAliveListInsert(const uint16_t physAddr);
static bool_t CdcPhysAddrFromAliveListDelete(const uint16_t physAddrToDel, const uint8_t hecIdx);
static bool_t CdcIsLinkedDirectlyInPortGet(const uint16_t sinkPhysAddr, const uint16_t sourcePhysAddr, uint8_t *pinputPortIndex);



//-------------------------------------------------------------------------------------------------
//! @brief      Pseudo-random numbers generator with uniform distribution
//!
//!             This implementation is based on George Marsaglia's MWC algorithm
//!
//! @param[in]  seed  - non-zero value is initial value that is expected to be
//!                     pure random or at least time dependent,
//!                     if seed == 0, Mrand generates next random output
//!
//! @return     pseudo-random, quasi-uniform number in the range [0;(2^32)-1]
//-------------------------------------------------------------------------------------------------

uint32_t Mrand(uint32_t seed)
{
    static uint32_t mz = 0x3F218942; // default initial values
    static uint32_t mw = 0x19A46E81; 

    if (seed != 0)
    {
        mz = seed >> 16 ;
        mw = seed & 0xFFFF;    
    }
    
    mz = 0x9069 * (mz & 0xFFFF) + (mz >> 16);
    mw = 0x4650 * (mw & 0xFFFF) + (mw >> 16);
    
    return (mz << 16) + mw;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Initializes current Instance Record
//-------------------------------------------------------------------------------------------------

void CdcHecInstanceRecordReset(void)
{    
    memset(pCdc, 0, sizeof(CdcInstanceData_t)); // erase all data   

    // mark entire array as vacant
    memset(&pCdc->aAliveDevice[0], 0xFF, sizeof(pCdc->aAliveDevice));
}

//-------------------------------------------------------------------------------------------------
//! @brief      Creates new HEC record in the Instance Record
//!
//! @param[in]  isActivator - indicates if the device is going to be an activator for the HEC
//!
//! @return     index of newly created HEC
//! @retval     0xFF - error
//-------------------------------------------------------------------------------------------------

uint8_t CdcHecDescriptorAdd(const bool_t isActivator)
{
    uint8_t i = 0xFF;
    bool_t isFound;
    CdcHecDescriptor_t *pNewHec = NULL;
    
    if (pCdc->numberOfHecs < CDC_NUMBER_OF_HECS_MAX)
    {
        // Find first vacant HEC record
        isFound = false;
        for (i = 0; i < CDC_NUMBER_OF_HECS_MAX; i++)
        {
            if (pCdc->aHec[i].status.isInUse == false)
            {
                pNewHec = &pCdc->aHec[i];
                pNewHec->status.isInUse = true;
                pNewHec->status.isActivator = isActivator;
                pCdc->numberOfHecs++;
                isFound = true;
                break;
            }
        }

        if (isFound == false)
        {
            DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHecDescriptorAdd(): All HEC records are in use\n" );
            return 0xFF;            
        }        
    }
    else
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHecDescriptorAdd(): Maximum number of HECs is exceeded\n" );
        return 0xFF;
    }

    return i;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Deletes HEC record in the Instance Record and
//!             frees memory for activator's HEC path
//!
//! @param[in]  hecIdx - index of a HEC to dismiss
//!
//! @retval     false - success,
//! @retval     true - error
//-------------------------------------------------------------------------------------------------

bool_t CdcHecDescriptorRemove(const uint8_t hecIdx)
{
    if ((pCdc->numberOfHecs > 0) && (pCdc->aHec[hecIdx].status.isInUse == true))
    {
        // Release the HEC descriptor
        pCdc->aHec[hecIdx].status.isInUse = false;
        pCdc->numberOfHecs--;

        // Cleanup other fields
        pCdc->aHec[hecIdx].status.isActivator  = false;
        pCdc->aHec[hecIdx].status.isAdjacent   = false;
        pCdc->aHec[hecIdx].status.channelState = CDC_PHEC;
        pCdc->aHec[hecIdx].activatorPhysAddr   = 0;
        pCdc->aHec[hecIdx].firstDevPhysAddr    = 0;
        pCdc->aHec[hecIdx].lastDevPhysAddr     = 0;
    }
    else
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHecDescriptorRemove(): The HEC descriptor is not in use\n" );
        return true;
    }
    
    return false;    
}


//-------------------------------------------------------------------------------------------------
//! @brief      Searches HEC records in order to find an index of a HEC
//!             having given first and last physical addresses
//!
//! @param[in]  actPhysAddr   - physical address of HEC's activator
//! @param[in]  firstPhysAddr - first physical address of a HEC
//! @param[in]  lastPhysAddr  - last physical address of a HEC
//!
//! @return     index of found HEC,
//! @retval     0xFF - if search was unsuccessful
//-------------------------------------------------------------------------------------------------

uint8_t CdcHecIndexGet(const uint16_t actPhysAddr, const uint16_t firstPhysAddr, const uint16_t lastPhysAddr)
{
    uint8_t i;
    uint8_t hecIdx = 0xFF; // error code by default

    for (i = 0; i < CDC_NUMBER_OF_HECS_MAX; i++)
    {
        if (pCdc->aHec[i].status.isInUse == true)
        {
            // First and Last physical address order makes no difference
            if ((pCdc->aHec[i].activatorPhysAddr == actPhysAddr) &&
                (((pCdc->aHec[i].firstDevPhysAddr == firstPhysAddr) && 
                (pCdc->aHec[i].lastDevPhysAddr == lastPhysAddr)) ||
                ((pCdc->aHec[i].firstDevPhysAddr == lastPhysAddr) && 
                (pCdc->aHec[i].lastDevPhysAddr == firstPhysAddr))))
            {
                hecIdx = i;
                break; // quit here because the HEC entry must be unique
            }
            
        }
    }

  if (hecIdx == 0xFF) 
  {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "CdcHecIndexGet(): HEC not found in existing records\n" );
  }

  return hecIdx;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Adds new devices from HEC to aAliveDevice device list
//!
//!             This function shall be called after CdcHecDescriptorAdd() and
//!             initialization of a HEC
//!
//! @param[in]  hecIdx - index of a new HEC
//!
//! @retval     false - success,
//! @retval     true - error
//-------------------------------------------------------------------------------------------------

bool_t CdcAliveDeviceListAdd(const uint8_t hecIdx)
{
    uint16_t    physAddr;
    uint16_t    firstPhysAddr;
    uint16_t    lastPhysAddr;
    bool_t      restart = true;
    bool_t      errStatus = false;

    
    if (pCdc->aHec[hecIdx].status.isInUse == true)
    {
        if (pCdc->aHec[hecIdx].status.isActivator == true)
        {
            firstPhysAddr = pCdc->aHec[hecIdx].firstDevPhysAddr;
            lastPhysAddr  = pCdc->aHec[hecIdx].lastDevPhysAddr;
            // add all members of a HEC except the Activator (this device)
            do
            {
                physAddr = CdcHecNextDeviceGet(firstPhysAddr, lastPhysAddr, restart); 
                restart = false; // restart == true for the first cycle only
                errStatus = CdcPhysAddrToAliveListInsert(physAddr);
            }
            while ((physAddr != lastPhysAddr) && (errStatus == false));
        }
        else // passive devices
        {
            // add only activator to the list
            physAddr = pCdc->aHec[hecIdx].activatorPhysAddr;
            errStatus = CdcPhysAddrToAliveListInsert(physAddr);
        }
    }
        
    return errStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Auxiliary function for CdcAliveDeviceListAdd()
//!
//!             This function is to be called from within CdcAliveDeviceListAdd()
//!
//! @param      physAddr - physical address of the device to insert
//!
//! @retval     false - success,
//! @retval     true - error
//-------------------------------------------------------------------------------------------------

static bool_t CdcPhysAddrToAliveListInsert(const uint16_t physAddr)
{
    uint16_t    listIdx;
    uint16_t    vacantCellIdx = 0;
    bool_t      isMatchFound;
    bool_t      isVacantCellFound;
    bool_t      errStatus = false;
    
    isMatchFound = false;
    isVacantCellFound = false;
    for (listIdx = 0; listIdx < CDC_NUMBER_OF_DEVICES_MAX; listIdx++)
    {
        if (pCdc->aAliveDevice[listIdx].physAddr == physAddr)
        {
            isMatchFound = true;
            break;  // all aAliveDevice[] members are unique, so there is no reason to continue
        }
        // simultaneous search for a first empty cell
        else if (!isVacantCellFound && (pCdc->aAliveDevice[listIdx].physAddr == 0xFFFF)) 
        {
            vacantCellIdx = listIdx;
            isVacantCellFound = true;
        }
    }
    // add every device of a HEC except this device
    if ((isMatchFound == false) && (physAddr != pCdc->physAddr))
    {
        if (isVacantCellFound == true) 
        {
            pCdc->aAliveDevice[vacantCellIdx].physAddr = physAddr; // add new device for alive monitoring
            CdcTimeCounterSet(CDC_TIME_SEC2TCK(CDC_MAX_NTF_ALIVE_TIME_SEC), &pCdc->aAliveDevice[vacantCellIdx].TimeCount);
            pCdc->numberOfAliveDev++;
        }
        else
        {
            DEBUG_PRINT( CDC_MSG_DBG, "ERROR in CdcPhysAddrToAliveListInsert(): aAliveDevice[] list is full\n" );
            errStatus = true;
        }
    }

    return errStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Removes devices that should not be monitored anymore
//!             from aAliveDevice device list
//!
//!             This function shall be called before CdcHecDescriptorRemove()
//!
//! @param[in]  hecIdx - index of the dismissed HEC
//!
//! @retval     false - success,
//! @retval     true - error
//-------------------------------------------------------------------------------------------------

bool_t CdcAliveDeviceListRemove(const uint8_t hecIdx)
{
    uint16_t    physAddr;
    uint16_t    firstPhysAddr;
    uint16_t    lastPhysAddr;
    bool_t      restart = true;
    bool_t      errStatus = false;

    
    if (pCdc->aHec[hecIdx].status.isInUse == true)
    {
        if (pCdc->aHec[hecIdx].status.isActivator == true)
        {
            firstPhysAddr = pCdc->aHec[hecIdx].firstDevPhysAddr;
            lastPhysAddr  = pCdc->aHec[hecIdx].lastDevPhysAddr;
            // remove all members of a HEC except the Activator (this device)
            do
            {
                physAddr = CdcHecNextDeviceGet(firstPhysAddr, lastPhysAddr, restart); 
                restart = false; // restart == true for the first cycle only
                // only delete a physical address from the list
                // if it isn't a subject of alive monitoring in the other active HECs
                errStatus = CdcPhysAddrFromAliveListDelete(physAddr, hecIdx);
            }
            while ((physAddr != lastPhysAddr) && (errStatus == false));
        }
        else // passive devices
        {
            // remove only activator from the list
            // if it isn't monitored as a member of other AHECs
            physAddr = pCdc->aHec[hecIdx].activatorPhysAddr;
            errStatus = CdcPhysAddrFromAliveListDelete(physAddr, hecIdx);
        }
    }
        
    return errStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Auxiliary function for CdcAliveDeviceListRemove()
//!
//!             This function is to be called from within CdcAliveDeviceListRemove()
//!
//! @param[in]  physAddrToDel - physical address of the device to delete
//! @param[in]  hecIdx        - index of a HEC that is to be closed
//!
//! @retval     false - success,
//! @retval     true - error
//-------------------------------------------------------------------------------------------------

static bool_t CdcPhysAddrFromAliveListDelete(const uint16_t physAddrToDel, const uint8_t hecIdx)
{
    uint16_t    listIdx;
    uint8_t     i;
    bool_t      isGoodToDelete = true;
    bool_t      errStatus = false;

    if (physAddrToDel != pCdc->physAddr) // this device should never be in the self AliveDevice List
    {
        // Check if the device doesn't belong to other AHECs that need to be monitored hereafter
        for (i = 0; i < CDC_NUMBER_OF_HECS_MAX; i++)
        {
            if ((i != hecIdx) && (pCdc->aHec[i].status.isInUse == true) && (pCdc->aHec[i].status.channelState == CDC_AHEC))
            {
                if (pCdc->aHec[i].status.isActivator)
                {
                    if (CdcIsDeviceBelongsToHec(pCdc->aHec[i].firstDevPhysAddr, pCdc->aHec[i].lastDevPhysAddr, physAddrToDel))
                    {
                        isGoodToDelete = false;
                        break; // stop immediately, if the device shall stay in the Alive list
                    }
                }
                else // passive device
                {
                    if (pCdc->aHec[i].activatorPhysAddr == physAddrToDel)
                    {
                        isGoodToDelete = false;
                        break; // stop immediately, if the device shall stay in the Alive list                        
                    }
                }
            }
        }

        if (isGoodToDelete)
        {
            // Delete a record in AliveDevice List
            for (listIdx = 0; listIdx < CDC_NUMBER_OF_DEVICES_MAX; listIdx++)
            {
                if (pCdc->aAliveDevice[listIdx].physAddr == physAddrToDel)
                {
                    pCdc->aAliveDevice[listIdx].physAddr = 0xFFFF; // mark as vacant
                    
                    // reset time counter
                    CdcTimeCounterSet(CDC_TIME_SEC2TCK(CDC_MAX_NTF_ALIVE_TIME_SEC), &pCdc->aAliveDevice[listIdx].TimeCount);
                    
                    pCdc->numberOfAliveDev--;
                    break;  // all aAliveDevice[] members are unique, so there is no reason to continue
                }
            }
        }
    }

    return errStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sets the time counter to given time in the future relatively
//!             to provided system timer reading
//!
//!             CAUTION: this function shall never be followed by the
//!             CdcTimeCounterUpdate() for the same timer in one
//!             TaskProcess invocation
//!
//! @param[in]  delayTimeTck   - the delay time to set (in ticks)
//! @param[in]  pTimeCounter   - pointer to the time counter to set
//-------------------------------------------------------------------------------------------------

void CdcTimeCounterSet(const uint16_t delayTimeTck, CdcTimeCounter_t *pTimeCounter)
{
    uint32_t totalTimeMs;

    totalTimeMs = (uint32_t) pCdc->sysTimerCountCurMs + CDC_TIME_TCK_MS * ((uint32_t) delayTimeTck);
    pTimeCounter->tRov = (uint16_t)((totalTimeMs / 0x10000) & 0x3);  // 0x10000 = 2^16
    pTimeCounter->tRes = (uint16_t)((totalTimeMs / CDC_TIME_TCK_MS) & 0x3FFF); // mod (2^16) 
}


//-------------------------------------------------------------------------------------------------
//! @brief      Checks if the time counter has expired and updates the timer counter
//!
//!             Resets the time counter if it is expired
//!             CAUTION: this function shall never be called more than once per
//!             TaskProcess invocation for the same timer
//!
//! @param[in]  pTimeCounter   - pointer to the time counter to check
//!
//! @retval     true - if timer is expired,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CdcTimeCounterUpdate(CdcTimeCounter_t *pTimeCounter)
{
    bool_t isExpired = false;

    if ((pTimeCounter->tRov != 0) || (pTimeCounter->tRes != 0)) // if not expired already
    {    
        if (pCdc->sysTimerCountPrvMs > pCdc->sysTimerCountCurMs) // if timer overflow detected
        {
            if (pTimeCounter->tRov > 0)
            {
                pTimeCounter->tRov--;
            }
        }

        if ((pTimeCounter->tRov == 0) && (pCdc->sysTimerCountCurMs > (CDC_TIME_TCK_MS * pTimeCounter->tRes)))
        {
            isExpired = true;

            // reset the counter
            pTimeCounter->tRov = 0;
            pTimeCounter->tRes = 0;            
        }      
    }
    else
    {
        isExpired = true;
    }

    // Update minimum time counter
    if (CdcTimeCounterCompare(pTimeCounter, &pCdc->cdcRecallTimeCount) == 0)
    {   // if (*pTimeCounter < cdcRecallTimeCount)
        pCdc->cdcRecallTimeCount = *pTimeCounter;
    }

    return isExpired;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Checks if the time counter has expired
//!
//! @param[in]  pTimeCounter - pointer to the time counter to check
//!
//! @retval     true - if timer is expired,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CdcIsTimeCounterExpired(CdcTimeCounter_t *pTimeCounter)
{
    return ((pTimeCounter->tRov == 0) && (pTimeCounter->tRes == 0));   
}


//-------------------------------------------------------------------------------------------------
//! @brief      Resets the time counter to zero
//!
//! @param[in]  pTimeCounter - pointer to the time counter to check
//-------------------------------------------------------------------------------------------------

void CdcTimeCounterReset(CdcTimeCounter_t *pTimeCounter)
{
    pTimeCounter->tRov = 0;
    pTimeCounter->tRes = 0;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Compares two time counters
//!
//!             CAUTION: time counters shall be updated before comparison
//!
//! @param[in]  pTimeCounterA - pointer to the time counter A to compare
//! @param[in]  pTimeCounterB - pointer to the time counter B to compare
//!
//! @retval     0 - if time A < B,
//! @retval     1 - if time A == B
//! @retval     2 - if time A > B
//-------------------------------------------------------------------------------------------------

uint8_t CdcTimeCounterCompare(CdcTimeCounter_t *pTimeCounterA, CdcTimeCounter_t *pTimeCounterB)
{
    uint8_t cmpRet;

    if (pTimeCounterA->tRov > pTimeCounterB->tRov)
    {
        cmpRet = 2;
    }
    else if (pTimeCounterA->tRov < pTimeCounterB->tRov)
    {
        cmpRet = 0;
    }
    else if (pTimeCounterA->tRes > pTimeCounterB->tRes)
    {
        cmpRet = 2;
    }
    else if (pTimeCounterA->tRes < pTimeCounterB->tRes)
    {
        cmpRet = 0;
    }
    else
    {
        cmpRet = 1;
    }

    return cmpRet;

}


//-------------------------------------------------------------------------------------------------
//! @brief      Returns time left until counter expires in seconds
//!
//! @param[in]  pTimeCounter   - pointer to the time counter
//!
//! @return     time in seconds
//-------------------------------------------------------------------------------------------------

uint16_t CdcTimeCounterLeftSecGet(CdcTimeCounter_t *pTimeCounter)
{
    return ((pTimeCounter->tRov * (0x10000 / CDC_TIME_TCK_MS) + pTimeCounter->tRes - 
            (pCdc->sysTimerCountCurMs / CDC_TIME_TCK_MS)) / (1000 / CDC_TIME_TCK_MS));
}


//-------------------------------------------------------------------------------------------------
//! @brief      Resets the watchdog's timer for the device that responded
//!             with NotifyAlive message
//!
//! @param[in]  physAddr - physical address of alive device
//!
//! @retval     false    - success,
//! @retval     true - error (device not found)
//-------------------------------------------------------------------------------------------------

bool_t CdcAliveDeviceListTimeReset(const uint16_t physAddr)
{  
    uint16_t    listIdx;
    bool_t      isFound = false;

    for (listIdx = 0; listIdx < CDC_NUMBER_OF_DEVICES_MAX; listIdx++)
    {
         if (pCdc->aAliveDevice[listIdx].physAddr == physAddr)
         {
            isFound = true;
            CdcTimeCounterSet(CDC_TIME_SEC2TCK(CDC_MAX_NTF_ALIVE_TIME_SEC), &pCdc->aAliveDevice[listIdx].TimeCount);
            break; // because physAddr must be present uniquely
         }
    }
    
    return isFound;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set AliveDevice List times up-to-date
//!
//! @retval     0xFFFF            - no timeouts have been detected,
//! @retval     other numbers     - index of a device failed to notify it's
//!                                 alive status. If more than one failed device
//!                                 is detected, the fuction reports the last
//!                                 one in the list. Other timeouts will be
//!                                 pending and reported in subsequent calls of
//!                                 the function
//-------------------------------------------------------------------------------------------------

uint16_t CdcAliveDeviceListTimeUpdate(void)
{    
    uint16_t    listIdx;    
    uint16_t    timeoutedDeviceDetected = 0xFFFF;

    
    for (listIdx = 0; listIdx < CDC_NUMBER_OF_DEVICES_MAX; listIdx++)
    {
        if (pCdc->aAliveDevice[listIdx].physAddr != 0xFFFF) // for all nonempty records
        {
            if (CdcTimeCounterUpdate(&pCdc->aAliveDevice[listIdx].TimeCount) == true)            
            {   // if counter has expired
                timeoutedDeviceDetected = listIdx;
                DEBUG_PRINT( CDC_MSG_DBG, "CdcAliveDeviceListTimeUpdate(): Timeout detected for PA = %04x\n", pCdc->aAliveDevice[listIdx].physAddr );
            }
            
        }
    }

    return timeoutedDeviceDetected;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Figures out whether a device is in the path of the eHDMI channel
//!
//! @param[in]  firstPhysAddr - PA of the HEC's beginning device,
//! @param[in]  lastPhysAddr  - PA of the HEC's end device,
//! @param[in]  testPhysAddr  - PA of the device to examine
//!
//! @retval     true, if the device with testPhysAddr belongs to the HEC,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CdcIsDeviceBelongsToHec(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr, const uint16_t testPhysAddr)
{
    uint16_t mask = 0xF000;

    if (firstPhysAddr == lastPhysAddr) // prohibited combination that should never happen. This "if" prevents deadlock
    {
        return (firstPhysAddr == testPhysAddr);
    }
    
    while ((firstPhysAddr & mask) == (lastPhysAddr & mask)) // Find root level of the HEC tree
    {
        mask = (mask >> 4) | 0xF000;
    }
    
    mask <<= 4;
    if (testPhysAddr == (firstPhysAddr & mask)) // check if there is a match with the HEC tree root
    {
        return true;
    }

    // step down the tree (left and right from root) and see if there is a match
    while(mask != 0xFFFF)
    {
        mask = (mask >> 4) | 0xF000;

        if ((testPhysAddr == (firstPhysAddr & mask)) || (testPhysAddr == (lastPhysAddr & mask)))
        {
            return true;
        }
    }

    return false;
}



//-------------------------------------------------------------------------------------------------
//! @brief      Consecutively calculates physical addresses of HEC devices
//!             starting from firstPhysAddr and ending with lastPhysAddr
//!
//!             Calling the function after reaching the lastPhysAddr without
//!             the parameter "restart" set to true will result in starting over
//!             again from the firstPhysAddr.
//!
//! @param[in]  firstPhysAddr - PA of the HEC's beginning device,
//! @param[in]  lastPhysAddr  - PA of the HEC's end device,
//! @param[in]  restart       - forces the function to start over from the beginning
//!
//! @return     physical address of the next device in the HEC's path
//-------------------------------------------------------------------------------------------------

uint16_t CdcHecNextDeviceGet(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr, const bool_t restart)
{
    static uint16_t mask = 0xFFF0;
    static bool_t   reverse = false;
    uint16_t physAddr;
    uint16_t pa;

    if (restart)
    {
        mask = 0x0000; 
        reverse = false;
        // skip absent levels (right zeros)
        pa = firstPhysAddr;
        while (pa != 0)
        {
            pa <<= 4;
            mask = (mask >> 4) | 0xF000;
        }
    }

    if (reverse == false)  // go up from first PA to the top of HEC's tree
    {
        physAddr = firstPhysAddr & mask;
        if (physAddr == (lastPhysAddr & mask)) 
        {
            reverse = true;
            mask = (mask >> 4) | 0xF000;
        }
        else
        {
            mask <<= 4;
        }
    }
    else    // go down from the HEC's tree top to the last PA
    { 
        physAddr = lastPhysAddr & mask;
        if ((lastPhysAddr & mask) == lastPhysAddr) // start over when the end of HEC is reached
        {
            reverse = false;
            mask = 0x0000;
            // skip absent levels (right zeros)
            pa = firstPhysAddr;
            while (pa != 0)
            {
                pa <<= 4;
                mask = (mask >> 4) | 0xF000;
            }
        }
        else
        {
            mask = (mask >> 4) | 0xF000;
        }
    }
    
    return physAddr;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Calculates length (number of devices) of a HEC
//!
//! @param[in]  firstPhysAddr - PA of the HEC's beginning device,
//! @param[in]  lastPhysAddr  - PA of the HEC's end device,
//!
//! @return     number of devices in the HEC
//-------------------------------------------------------------------------------------------------

uint8_t CdcHecLengthGet(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr)
{
    uint8_t     deviceCount = 1; // minimal length is 2 (fist & last devices only)
    uint16_t    physAddr;

    physAddr = CdcHecNextDeviceGet(firstPhysAddr, lastPhysAddr, true);
    while (physAddr != lastPhysAddr)
    {
        physAddr = CdcHecNextDeviceGet(firstPhysAddr, lastPhysAddr, false);
        deviceCount++;
    }
    return deviceCount;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Figures out whether two devices have relationship
//!             of a Sink and a Source.
//!
//!             This function is used in HPD signaling protocol implementation
//!
//! @param[in]  sinkPhysAddr    - PA of the possible Sink device,
//! @param[in]  sourcePhysAddr  - PA of the possible Source device,
//! @param[in]  inputPortIndex  - index of an input port of the Sink
//!
//! @retval     true, if devices are linked over the given input port
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CdcIsLinkedDirectly(const uint16_t sinkPhysAddr, const uint16_t sourcePhysAddr, const uint8_t inputPortIndex)
{
    uint8_t     actualInputPortIndex;
    bool_t      isLinked;

    isLinked = CdcIsLinkedDirectlyInPortGet(sinkPhysAddr, sourcePhysAddr, &actualInputPortIndex);

    if (actualInputPortIndex != inputPortIndex)
    {
        isLinked = false;
    }

    return isLinked;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Calculates physical address of a device, potential or real, that
//!             is adjacent to the reference device.
//!
//! @param[in]  basePhysAddr   - PA of the reference device,
//! @param[in]  isAtOutput     - if true, returns a device's PA adjacent to output port,
//!                              if false, returns a device's PA adjacent PA to
//!                              specified input port
//! @param[in]  inputPortIndex - zero-based index of the input port of the adjacent device
//!                             (ignored, if isAtOutput == true)
//!
//! @return     Physical Address of the Adjacent device,
//! @retval     0xFFFF - if the Adjacent device PA doesn't exist
//!                       (sink of a root or source of 5th level)
//-------------------------------------------------------------------------------------------------

uint16_t CdcAdjacentPhysAddrGet(const uint16_t basePhysAddr, const bool_t isAtOutput, const uint8_t inputPortIndex)
{
    uint16_t mask = 0xFFF0; 
    uint16_t adjPhysAddr = 0xFFFF;
    uint8_t  n = 0;

    if (inputPortIndex >= 0xF) // illegal port index
    {
        return 0xFFFF;
    }

    // find the last non-zero digit in basePhysAddr
    while ((n < 4) && ((basePhysAddr & mask) == basePhysAddr))
    {
        n++;
        mask <<= 4;
    }
 
    if (isAtOutput)
    {
        if (basePhysAddr != 0x0000)
        {
            adjPhysAddr = basePhysAddr & mask;
        }
    }
    else // adjacent to input
    {
        if (n != 0) // if last digit is 0
        {
           adjPhysAddr = basePhysAddr | ((inputPortIndex + 1) << (4 * (n - 1)));
        }
    }

    return adjPhysAddr;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Figures out whether two devices have relationship
//!             of Sink and Source (i.e. they are adjacent) and the adjacent
//!             device has HEC capable input or output port that connects
//!             it to the initiator
//!
//!             This function is used in control for Adjacent devices.
//!
//! @param[in]  initiatorPhysAddr - PA of the initiating device,
//! @param[in]  adjacentPhysAddr  - PA of the adjacent device,
//! @param[in]  isAdjacent        - (return value) pointer to adjacent flag
//!                                 that is true, if 2 devices are adjacent
//!
//! @retval     1, if devices are linked over HEC capable port
//! @retval     0 - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CdcIsCapableToAdjacent(const uint16_t initiatorPhysAddr, const uint16_t adjacentPhysAddr, bool_t *isAdjacent)
{
    uint8_t inputPortIndex;
    bool_t  isLinkedToSink = false;
    bool_t  isLinkedToSource = false;

    isLinkedToSink = CdcIsLinkedDirectlyInPortGet(initiatorPhysAddr, adjacentPhysAddr, &inputPortIndex);
    isLinkedToSource = CdcIsLinkedDirectlyInPortGet(adjacentPhysAddr, initiatorPhysAddr, &inputPortIndex);
    *isAdjacent = isLinkedToSink || isLinkedToSource;

    if (/* if initiator is a Sink, need capable output to establish a connection */
        ( isLinkedToSink && 
         (pCdc->ports.numberOfOutputs > 0) && (pCdc->ports.output.hecSupport == CDC_HEC_SUPPORTED)) || 
        /* if initiator is a Source, need capable input to establish a connection */
        ( isLinkedToSource && 
         (pCdc->ports.numberOfInputs > inputPortIndex) && (pCdc->ports.input[inputPortIndex].hecSupport == CDC_HEC_SUPPORTED)))
    {
        return true;
    }
    else
    {
        return false;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Figures out whether two devices have relationship
//!             of a Sink and a Source. If so, returns index of the Sink's
//!             input port that is connected to the Source output port
//!
//! @param[in]  sinkPhysAddr      - PA of the possible Sink device,
//! @param[in]  sourcePhysAddr    - PA of the possible Source device,
//! @param[out] pInputPortIndex   - (return value) pointer to an index of an input
//!                                 port of the Sink that must be used for the HEC
//!                                 connection.
//!                                 0xFF - in the case when Sink-Source relationship
//!                                 hasn't been verified
//!
//! @retval     true, if devices are linked directly over some of the Sink's inputs
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

static bool_t CdcIsLinkedDirectlyInPortGet(const uint16_t sinkPhysAddr, const uint16_t sourcePhysAddr, uint8_t *pInputPortIndex)
{
    // criteria: 1. Sink must be one level above the Source
    //           2. The input port number (= inputPortIdx+1) must be equal to 
    //              the Source's physical address last nonzero digit (p. 8.7.2. of HDMI 1.4 spec)  
    uint8_t     i;
    uint8_t     n = 0;    
    uint16_t    mask = 0x000F;
    uint16_t    sourceMasked;
    uint16_t    sinkMasked;
    bool_t      isLinked = false;

    // crit.1 meets if and only if sink and source have one of the following patterns: 
    // {0000, n000}, {x000, xn00}, {xy00, xyn0}, {xyz0, xyzn}
    for (i = 0; i < 4; i++)
    {
        sourceMasked = sourcePhysAddr & mask;
        sinkMasked   = sinkPhysAddr & mask;

        if (n == 0) // n point not found yet
        {
             // skip zero-zero pairs
             if ((sinkMasked == 0) && (sourceMasked !=0)) // first zero-nonzero pair found 
             {
                 n = sourceMasked >> (4 * i); // input port number (nonzero)
                 *pInputPortIndex = n - 1;      // input port index to return
                 isLinked = true;             // potential detection
             }
             else if ((sinkMasked != 0) && (sourceMasked !=0))
             {
                break;
             }
             else if ((sinkMasked != 0) && (sourceMasked ==0))
             {
                break;
             }      
        }
        else // all other digit pairs must be non-zero & equal
        {
            if ((sinkMasked == 0) || (sinkMasked != sourceMasked))
            {
                isLinked = false; // reset linked flag if wrong MSB pattern found 
                *pInputPortIndex = 0xFF;
                break;
            }
        }

        mask <<= 4; // scan all 4 digits        
    }


    return isLinked;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Finds out what input and/or output ports are utilized
//!             for a HEC that includes this device
//!
//!             Verifies if those ports are capable to maintain the HEC. This function shall be called
//!             after verifying that the HEC includes this device
//!
//! @param[in]  firstPhysAddr   - first device PA of the HEC,
//! @param[in]  lastPhysAddr    - last device PA of the HEC,
//! @param[out] aInPortIdx      - pointer to input port index array (return value),
//!                               containing 2 elements: ports of the HEC
//!                               coming-in and coming-out.
//!                               0xFF value indicates that the HEC route
//!                               doesn't come through the input
//! @param[out] pOutPortFlg     - pointer to output port flag (return value)
//!                               0 - indicates that the HEC route
//!                               doesn't come through the output
//!                               1 - indicates that the HEC route
//!                               come through the output
//!
//! @retval     true, if routing is verified for the HEC
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t  CdcCapablePortGet(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr, 
                           uint8_t aInPortIdx[2], uint8_t *pOutPortFlg)
{
    uint8_t  i;
    uint8_t  inpCnt = 0;
    bool_t   restart = true;
    uint16_t physAddr;
    uint8_t  isVerified = true;
    

    *pOutPortFlg    = false;    // output port isn't in use by default
    aInPortIdx[0]   = 0xFF;     // no input port is in use by default
    aInPortIdx[1]   = 0xFF;      

    do // for all devices in the HEC
    {
        physAddr = CdcHecNextDeviceGet(firstPhysAddr, lastPhysAddr, restart);        
        restart = false; // restart == true for the first cycle only

        // check if a device from the HEC is a Sink for this device, so the output shall be in use
        if (*pOutPortFlg == false) // do it only if output usage has not been determined yet
        {
            *pOutPortFlg = CdcIsLinkedDirectlyInPortGet(physAddr, pCdc->physAddr, &i); 
        }

        // check if a device from the HEC is a Source for this device 
        // and find out the input ports that are connected to the Source (maximum 2)
        if ((inpCnt == 0) || ((inpCnt == 1) && (!*pOutPortFlg))) // do it only if input usage has not been determined yet
        {
            if (CdcIsLinkedDirectlyInPortGet(pCdc->physAddr, physAddr, &i))
            {
                aInPortIdx[inpCnt++] = i;
            }
            
        }
    } 
    while ((physAddr != lastPhysAddr) && ((*pOutPortFlg == false) || (inpCnt == 0)));
    
    // verify HEC capability of the affected ports
    if (*pOutPortFlg == true) // output 
    {
        if ((pCdc->ports.numberOfOutputs == 0) || (!pCdc->ports.output.hecSupport))
        {
            isVerified = false;
        }
    }

    for (i = 0; i < inpCnt; i++) // inputs (up to 2)
    {

        if ((pCdc->ports.numberOfInputs <= aInPortIdx[i]) || (!pCdc->ports.input[aInPortIdx[i]].hecSupport))
        {
            isVerified = false;
            break;
        }
        
    }

    if ((inpCnt == 0) && (*pOutPortFlg == false)) // not verified, if no ports participate in the HEC
    {
        isVerified = false;
    }
    
    return isVerified;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Figures out if this device has all capabilities to become a part of a HEC
//!
//!             I.e. the device is HEC capable and all ports
//!             that must be activated to create the HEC are HEC capable too.
//!
//! @param[in]  firstPhysAddr   - first device PA of the HEC,
//! @param[in]  lastPhysAddr    - last device PA of the HEC,
//! @param[in]  isStrict        - false will allow to verify this device
//!                               positively, if it's outside of the HEC
//!
//!
//! @retval     true, if routing is verified for the HEC
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t  CdcIsCapableForHec(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr, const bool_t isStrict)
{
    uint8_t aInPortIdx[2];
    uint8_t outPortFlg;
    bool_t  isVerified;

    if (firstPhysAddr != lastPhysAddr)
    {
        if (CdcIsDeviceBelongsToHec(firstPhysAddr, lastPhysAddr, pCdc->physAddr))
        { 
            isVerified = CdcCapablePortGet(firstPhysAddr, lastPhysAddr, aInPortIdx, &outPortFlg);
        }
        else
        {
            isVerified = !isStrict;
        }
    }
    else
    {
        isVerified = false; // Situation when (firstPhysAddr == lastPhysAddr) is prohibited 
    }

    return isVerified;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Activates I/O ports of this device when new AHEC is created
//!
//!             This service function shall be called after calling of
//!             CdcIsDeviceBelongsToHec() and CdcCapablePortGet(), both
//!             has to return true, as it lacks a few condition checks
//!             in order to improve performance
//!
//! @param[in]  aInPortIdx   - pointer to input port index array,
//!                            containing 2 elements: ports of the HEC
//!                            coming-in and coming-out.
//!                            0xFF value indicates that the HEC route
//!                            doesn't come through the input
//! @param[in]  outPortFlg  - pointer to output port flag
//!                            0 - indicates that the HEC route
//!                            doesn't come through the output
//!                            1 - indicates thar the HEC route
//!                            come through the output
//-------------------------------------------------------------------------------------------------

void CdcIoPortsActivate(const uint8_t aInPortIdx[2], const bool_t outPortFlg)
{
    uint8_t i;
    
    // update output port status
    if (outPortFlg && (pCdc->ports.output.hecActive == CDC_HEC_INACTIVE))
    {
        // Activate output's hardware
        if (CdcEhdmiIoPortHwEnable(true, 0, true))
        {
            pCdc->ports.output.hecActive = CDC_HEC_ACTIVE;
        }
    }
    // update input ports status
    for (i = 0; i < 2; i++)
    {
        if ((aInPortIdx[i] != 0xFF) && (pCdc->ports.input[aInPortIdx[i]].hecActive == CDC_HEC_INACTIVE))
        {
            // Activate input's hardware
            if (CdcEhdmiIoPortHwEnable(false, aInPortIdx[i], true))
            {            
                pCdc->ports.input[aInPortIdx[i]].hecActive = CDC_HEC_ACTIVE;
            }
        }
    }

}


//-------------------------------------------------------------------------------------------------
//! @brief      Updates I/O ports status of this device to reflect current state
//!             and participation in active HECs
//!
//!             This function shall be called
//!             whenever HECs are activated or deactivated
//-------------------------------------------------------------------------------------------------

void CdcIoPortsStatusUpdate(void)
{
    uint8_t isInputInUse[CDC_EHDMI_INPUTS_MAX];
    bool_t  isOutputInUse = false;
    uint8_t aInPortIdx[2];
    uint8_t outPortFlg;
    uint8_t i;

    // initialize isInputInUse[] flags
    for (i = 0; i < pCdc->ports.numberOfInputs; i++)
    {
        isInputInUse[i] = false;
    }

    // set flags for ports that support at least one AHEC
    for (i = 0; i < CDC_NUMBER_OF_HECS_MAX; i++)
    {
        if ((pCdc->aHec[i].status.isInUse == true) && (pCdc->aHec[i].status.channelState == CDC_AHEC))
        {
            if (CdcCapablePortGet(pCdc->aHec[i].firstDevPhysAddr, 
                                  pCdc->aHec[i].lastDevPhysAddr, aInPortIdx, &outPortFlg))
            {
                if (aInPortIdx[0] != 0xFF)
                {
                    isInputInUse[aInPortIdx[0]] = true;
                }
                if (aInPortIdx[1] != 0xFF)
                {
                    isInputInUse[aInPortIdx[1]] = true;
                }
                if (outPortFlg)
                {
                    isOutputInUse = true;
                }
                
            }
        }
    }

    // set input ports status
    for (i = 0; i < pCdc->ports.numberOfInputs; i++)
    {
        if (pCdc->ports.input[i].hecSupport == CDC_HEC_SUPPORTED)
        {
            if (isInputInUse[i])
            {
                // Activate input's hardware
                if (CdcEhdmiIoPortHwEnable(false, i, true))
                {
                    pCdc->ports.input[i].hecActive = CDC_HEC_ACTIVE;
                }
            }
            else
            {
                // Deactivate input's hardware
                if (CdcEhdmiIoPortHwEnable(false, i, false))
                {            
                    pCdc->ports.input[i].hecActive = CDC_HEC_INACTIVE;
                }
                
            }
        }
    }

    // set output port status
    if (pCdc->ports.numberOfOutputs > 0)
    {
        if (pCdc->ports.output.hecSupport == CDC_HEC_SUPPORTED)
        {
            if (isOutputInUse)
            {
                // Activate output's hardware
                if (CdcEhdmiIoPortHwEnable(true, 0, true))
                {            
                    pCdc->ports.output.hecActive = CDC_HEC_ACTIVE;
                }
            }
            else
            {
                // deactivate output's hardware
                if (CdcEhdmiIoPortHwEnable(true, 0, false))
                {                        
                    pCdc->ports.output.hecActive = CDC_HEC_INACTIVE;
                }
                
            }
        }
    }
    
}


//-------------------------------------------------------------------------------------------------
//! @brief      Finds out if a HEC is enabled or possible in the given Device
//!
//!             This function should be used when analyzing the Device ReportState
//!             extended message, including the bit field stating the ports status
//!
//! @param[in]  hecIdx          - Index of existing HEC to check,
//! @param[in]  devPhysAddr     - the Device (an initiator of ReportState msg) physical address,
//! @param[in]  portBitField    - bit field indicating the I/O ports status (see CdcHecReportStateSend())
//!
//! @retval     true, if support/activation is verified for the HEC
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t  CdcIsHecEnabledInDev(const uint8_t hecIdx, const uint16_t devPhysAddr, const uint16_t portBitField) 
{
    uint8_t  i;
    uint8_t  inpCnt = 0;
    bool_t   outPortFlg;
    uint8_t  aInPortIdx[2];
    bool_t   restart = true;
    uint16_t physAddr;
    bool_t   isVerified = false;
    

    outPortFlg      = false;    // output port isn't in use by default
    aInPortIdx[0]   = 0xFF;     // no input port is in use by default
    aInPortIdx[1]   = 0xFF;      

    do // for all devices in the HEC
    {
        physAddr = CdcHecNextDeviceGet(pCdc->aHec[hecIdx].firstDevPhysAddr, 
                                       pCdc->aHec[hecIdx].lastDevPhysAddr, restart);        
        restart = false; // restart == true for the first cycle only

        // check if a device from the HEC is a Sink for this device, so the output shall be in use
        if (outPortFlg == false) // do it only if output usage has not been determined yet
        {
            outPortFlg = CdcIsLinkedDirectlyInPortGet(physAddr, devPhysAddr, &i); 
        }

        // check if a device from the HEC is a Source for this device 
        // and find out the input ports that are connected to the Source (maximum 2)
        if ((inpCnt == 0) || ((inpCnt == 1) && (!outPortFlg))) // do it only if input usage has not been determined yet
        {
            if (CdcIsLinkedDirectlyInPortGet(devPhysAddr, physAddr, &i))
            {
                aInPortIdx[inpCnt++] = i;
            }
            
        }
    } 
    while ((physAddr != pCdc->aHec[hecIdx].lastDevPhysAddr) && ((outPortFlg == false) || (inpCnt == 0)));

    // Verify that required ports for this HEC are HEC-capable or active 
    // (unused connections can have any state)
    if ((!outPortFlg || (((portBitField >> 14) & 0x01) == 1)) &&
        ((aInPortIdx[0] == 0xFF) || (((portBitField >> (13 - aInPortIdx[0])) & 0x01) == 1)) &&
        ((aInPortIdx[1] == 0xFF) || (((portBitField >> (13 - aInPortIdx[1])) & 0x01) == 1)))
    {
        isVerified = true;
    }
        
    return isVerified;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Identifies all found HOST or ENC capable devices that aren't
//!             reachable from this device, and then cleans up and rearranges
//!             the cdcFoundHostsList to leave only capable & reachable devices
//!
//!             This function shall be used after collection of all responses
//!             in the Discovery process
//-------------------------------------------------------------------------------------------------

void CdcDiscoveryDataArrange(void)
{
    uint8_t  i;
    uint8_t  j;
    uint8_t  inpIdx;
    bool_t   isVerified;
    uint16_t prevPhysAddr;
    uint16_t curPhysAddr;
    uint8_t  prevIdx = 0; // index of the prevPhysAddr in cdcFoundHostsList[]
    uint8_t  curIdx = 0;  // index of the curPhysAddr in cdcFoundHostsList[]
    bool_t   isPrevPortCapable;
    bool_t   outPortFlg;    
    CdcFoundListEntry_t tempListEntry;

    // remove unreachable devices 
    for (i = 0; i < pCdc->numberOfAllHosts; i++)
    {
        if ( ((pCdc->cdcFoundHostsList[i].devStat.hecFuncState != CDC_HOST_NOT_SUPPORTED) ||
              (pCdc->cdcFoundHostsList[i].devStat.encFuncState != CDC_ENC_NOT_SUPPORTED)) &&
             (pCdc->cdcFoundHostsList[i].devStat.cdcErrorState == CDC_ERRCODE_NO_ERROR) )
        {
             pCdc->cdcFoundHostsList[i].hostInfo.isSelected = true; // select all potential connections 
            // check that all HEC@port capable devices in the PHEC are present
            // in the cdcFoundHostsList
            prevPhysAddr = CdcHecNextDeviceGet(pCdc->physAddr, pCdc->cdcFoundHostsList[i].physAddr, 1); // skip this device       
            
            do // for the rest of the device chain in the tested HEC
            {
                curPhysAddr = CdcHecNextDeviceGet(pCdc->physAddr, pCdc->cdcFoundHostsList[i].physAddr, 0);   

                // find curPhysAddr in the cdcFoundHostsList[], it must be present to allow the HEC possibility
                isVerified = false; // initial value; it will be true if HEC is verified to be possible                 
                for (j = 0; j < pCdc->numberOfAllHosts; j++)
                {
                    if (pCdc->cdcFoundHostsList[j].physAddr == curPhysAddr)
                    {
                        isVerified = true;
                        curIdx = j;
                        break; // quit this search cycle, if the match has been found
                    }
                }

                if (isVerified)
                { // Verify that connecting ports of the prevPhysAddr & curPhysAddr devices are HEC capable 
	
                    // check if the curPhysAddr is a Sink for prevPhysAddr, so the output of prevPhysAddr shall be in use
                    outPortFlg = CdcIsLinkedDirectlyInPortGet(curPhysAddr, prevPhysAddr, &inpIdx);

                    if (outPortFlg) // curPhysAddr is a Sink for prevPhysAddr
                    {
                        // verify that connected output and input are HEC capable
                        if (prevPhysAddr != pCdc->physAddr) // for devices from list
                        {
                            isPrevPortCapable = (pCdc->cdcFoundHostsList[prevIdx].hostInfo.portHecCapBits & 0x4000) != 0;
                        }
                        else if (pCdc->ports.numberOfOutputs != 0) // for this device, that isn't in list (by definition)
                        {
                            isPrevPortCapable = pCdc->ports.output.hecSupport;                            
                        }
                        else // this device is incapable
                        {
                            isVerified = false;
                            isPrevPortCapable = false;
                        }
                        
                        if (!((pCdc->cdcFoundHostsList[curIdx].hostInfo.portHecCapBits & (0x2000 >> inpIdx)) && isPrevPortCapable))
                        {
                            isVerified = false;
                        }

                    }
                    else // If not, the opposite has to be true: curPhysAddr is a Source for prevPhysAddr
                    {
                        outPortFlg = CdcIsLinkedDirectlyInPortGet(prevPhysAddr, curPhysAddr, &inpIdx);

                        if (outPortFlg) //curPhysAddr is a Source for prevPhysAddr
                        {
                            // verify that connected output and input are HEC capable
                            if (prevPhysAddr != pCdc->physAddr) // for devices from list
                            {
                                isPrevPortCapable = (pCdc->cdcFoundHostsList[prevIdx].hostInfo.portHecCapBits & (0x2000 >> inpIdx)) != 0;
                            }
                            else if (pCdc->ports.numberOfInputs > inpIdx) // for this device, that isn't in list (by definition)
                            {
                                isPrevPortCapable = pCdc->ports.input[inpIdx].hecSupport;                            
                            }
                            else // this device is incapable
                            {
                                isVerified = false;
                                isPrevPortCapable = false;
                            }                            
                            
                            if (!(((pCdc->cdcFoundHostsList[curIdx].hostInfo.portHecCapBits & 0x4000) != 0) && isPrevPortCapable))
                            {
                                isVerified = false;
                            }
                            
                        }
                        else // curPhysAddr and prevPhysAddr aren't connected directly (should be logically impossible)
                        {
                            DEBUG_PRINT( CDC_MSG_DBG, "(IMPROBABLE) ERROR in CdcDiscoveryDataArrange(): devices must be connected directly\n" );
                        }
                    }
    	
                    prevPhysAddr = curPhysAddr;
                    prevIdx = curIdx;
                }
            }
            while (isVerified && (curPhysAddr != pCdc->cdcFoundHostsList[i].physAddr)); 
	
	    
            // clear "Host or Enc" flag, if the device is unreachable
            if (!isVerified)
            {
                pCdc->cdcFoundHostsList[i].hostInfo.isSelected = 0; 
                DEBUG_PRINT( CDC_MSG_DBG, "CdcDiscoveryDataArrange(): unreachable device (PA = %04x) found. Omitted from list.\n",
                        pCdc->cdcFoundHostsList[i].physAddr );
            }
            
        }
    }

    // rearrange the verified list of found Host/ENC capable devices: 
    // put all "intermediate" devices not labeled as Host or EN capable after all labeled ones
    j = 0; // counts a number of "intermediate" devices
    for (i = 0; i < pCdc->numberOfAllHosts; i++)
    {
        if (pCdc->cdcFoundHostsList[i].hostInfo.isSelected)
        {   
            // swap entries
            tempListEntry = pCdc->cdcFoundHostsList[i - j];
            pCdc->cdcFoundHostsList[i - j] = pCdc->cdcFoundHostsList[i];
            pCdc->cdcFoundHostsList[i] = tempListEntry;
        }
        else
        {
            j++;
        }
    }

    pCdc->numberOfFoundHosts = pCdc->numberOfAllHosts - j; // shorten the list length to the number of Host/ENC capable devices
    
}


//-------------------------------------------------------------------------------------------------
//! @brief      Turns on or off the eHDMI hardware function when it is required by CDC protocol
//!
//! @note       This function must return success if HW already was in the requested state
//!
//! @param[in]  isEnabled  - true,  if eHDMI should be turned on,
//!                          false, if eHDMI should be turned off
//!
//! @retval     true, if eHDMI hardware enabled or disabled successfully
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CdcEhdmiHwEnable(const bool_t isEnabled)
{
    bool_t isSuccess = isEnabled || true; // Suppress compiler warning

    /* For the SiI9489, eHDMI hardware is enabled on a port-by-port */
    /* basis, so we just return success.                            */

    if (!isSuccess)
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcEhdmiHwEnable(): Could not access HEC hardware.\n" );
        pCdc->deviceStatus.cdcErrorState = CDC_ERRCODE_OTHER_ERROR;
    }        

    return isSuccess;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Turns on or off the eHDMI I/O ports hardware, if it has to be
//!             done independently from enabling the eHDMI function as a whole
//!
//! @note       This function must return success if HW already was in the requested state
//!
//! @param[in]  isOutput   - true, if output state has to be changed
//!                          false,if input state has to be changed
//! @param[in]  portIndex  - 0-based index of input port, if (isOutput == false)
//! @param[in]  isEnabled  - true,  if eHDMI should be turned on,
//!                          false, if eHDMI should be turned off
//! @retval     true, if eHDMI I/O hardware enabled or disabled successfully
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CdcEhdmiIoPortHwEnable( const bool_t isOutput, const uint8_t portIndex, const bool_t isEnabled)
{
    bool_t isSuccess = isOutput || portIndex || true;
        
    /* The SiI9489 is input only, so we can ignore the isOutput flag.   */
    /* This should be changed if the CDC code is used in a system that  */
    /* incorporates both input and out HEC ports.                       */

    if (isOutput) 
    {
        isSuccess = false;  // HEC function at output isn't available
    }
    else
    {
#if ( errSII_DEV_953x_PORTING_BUILD_ERROR  == 1 )
        isSuccess = SiiDrvHeacHecEnable( portIndex, isEnabled );
#else
		isSuccess = false;
#endif
    }

    if (!isSuccess)
    {
       // DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcEhdmiHwEnable(): couldn't %s HEC function of %s port [%d].\n",
         //       isEnabled ? "enable" : "disable", isOutput ? "output" : "input", (int) portIndex ); RX: HPD
        pCdc->deviceStatus.cdcErrorState = CDC_ERRCODE_NOT_SUPPORTED_CAPABILITY;
    }        
    
    return( isSuccess );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Sends a CDC message over the CEC bus using CPI
//!
//! @param[in]  opCode      - CDC operation code
//! @param[in]  argCount    - number of byte-size arguments in the CDC message
//! @param[in]  pCdcMsg     - pointer to the CDC message frame (structure)
//-------------------------------------------------------------------------------------------------

static void CdcCpiWrite (uint8_t opCode, uint8_t argCount, SiiCpiData_t *cdcMsg)
{
    uint16_t physAddr = pCdc->physAddr;

    /* Add standard CDC stuff to CEC message */

#if (IS_CEC == ENABLE)
    cdcMsg->srcDestAddr =  MAKE_SRCDEST( SiiCecGetDeviceLA(), 0x0F); // if CEC & CDC coexist
#else
    cdcMsg->srcDestAddr     = CDC_CEC_SRCDEST_ADDR_DEFAULT;          // CDC only implementation
#endif    
    cdcMsg->opcode          = CDC_CEC_OPCODE;
    cdcMsg->args[0]         = (physAddr >> 8) & 0xFF;   // Initiator Physical Address (MSB)
    cdcMsg->args[1]         = physAddr & 0xFF;          // Initiator Physical Address (LSB)
    cdcMsg->args[2]         = opCode;
    cdcMsg->argCount        = argCount;
    SiiDrvCpiWrite(cdcMsg);                            // Send CDC message over CPI
}

//------------------------------------------------------------------------------
// Low level API Functions
//------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CDC HEC Discover" in order to collect HEC capability of
//!             all devices in the network
//!
//!             This function is to be used by Activator.
//-------------------------------------------------------------------------------------------------

void CdcHecDiscoverSend(void)
{
    SiiCpiData_t cdcDiscoveryMsg;
    
    //Send CDC message over CPI
    CdcCpiWrite(CDCOP_HEC_DISCOVER, 3, &cdcDiscoveryMsg);
    
}



//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CDC HEC InquireState" in order to inquire HEC capability of
//!             all devices in the potential channel
//!
//!             This function is to be used by Activator.
//! @param[in]  hecIdx - index of a HEC in the Instance Record
//-------------------------------------------------------------------------------------------------

void CdcHecInquireStateSend(const uint8_t hecIdx)
{
    SiiCpiData_t cdcInquireStateMsg;
    uint16_t firstPhysAddr, lastPhysAddr;

    // Check for necessary conditions required to carry out this operation
    if (pCdc->aHec[hecIdx].status.isActivator == false)
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHecInquireStateSend(): device must be an Activator\n" );
        return;
    }

    firstPhysAddr = pCdc->aHec[hecIdx].firstDevPhysAddr;
    lastPhysAddr  = pCdc->aHec[hecIdx].lastDevPhysAddr;
    
    cdcInquireStateMsg.args[3]      = (firstPhysAddr >> 8) & 0xFF;
    cdcInquireStateMsg.args[4]      = firstPhysAddr & 0xFF;
    cdcInquireStateMsg.args[5]      = (lastPhysAddr >> 8) & 0xFF;
    cdcInquireStateMsg.args[6]      = lastPhysAddr & 0xFF;

    //Send CDC message over CPI
    CdcCpiWrite(CDCOP_HEC_INQUIRE_STATE, 7, &cdcInquireStateMsg);     
    
}


//-------------------------------------------------------------------------------------------------
//! @brief      Description: Sends "CDC HEC InquireState" in order to inquire HEC capability of
//!             an adjacent device
//!
//!             This function is useful for HPD capability verification
//!
//! @param[in]  isAtOutput   - if true, the message is sent to a device
//!                            adjacent to output port;
//!                            if false, the message is sent to a device adjacent
//!                            to the specified input port
//!
//! @param[in]  inputPortIndex - zero-based index of the input port of the adjacent device
//!                             (ignored, if isAtOutput == true)
//!
//! @retval     true  - if the adjacent device doesn't exist,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CdcHecInquireStateOfAdjacentSend(const bool_t isAtOutput, const uint8_t inputPortIndex)
{
    SiiCpiData_t cdcInquireStateMsg;
    uint16_t firstPhysAddr, lastPhysAddr;

    firstPhysAddr = pCdc->physAddr;
    lastPhysAddr  = CdcAdjacentPhysAddrGet(pCdc->physAddr, isAtOutput, inputPortIndex);

    if (lastPhysAddr == 0xFFFF)
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHecInquireStateOfAdjacentSend(): adjacent device doesn't exist\n" );
        return true; // error
    }
    
    cdcInquireStateMsg.args[3]      = (firstPhysAddr >> 8) & 0xFF;
    cdcInquireStateMsg.args[4]      = firstPhysAddr & 0xFF;
    cdcInquireStateMsg.args[5]      = (lastPhysAddr >> 8) & 0xFF;
    cdcInquireStateMsg.args[6]      = lastPhysAddr & 0xFF;

    //Send CDC message over CPI
    CdcCpiWrite(CDCOP_HEC_INQUIRE_STATE, 7, &cdcInquireStateMsg);     
    
    return false; // no error

}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CDC HEC Report State" in response for Discover,
//!             IquireState or SetState message
//!
//!             This function is to be used by all devices
//!
//! @param[in]  targetPhysAddr  - physical address of the device to report.
//!                               If the physical address is 0xFFFF, the message
//!                               will be send out as broadcast notification
//!                               with "HEC State" and "HEC Support Field included"
//! @param[in]  reportStateMode - selection of short, discover or multiVHEC report
//-------------------------------------------------------------------------------------------------

void CdcHecReportStateSend(const uint16_t targetPhysAddr, const CdcReportStateMode_t reportStateMode)
{
    uint8_t  i;
    SiiCpiData_t cdcReportStateMsg;
    uint8_t  hecState;
    uint8_t  argCount;
    uint16_t hecExtraField;

    if (pCdc->physAddr != 0xFFFF) // don't report if own physical address is invalid
    {
        cdcReportStateMsg.args[3]      = (targetPhysAddr >> 8) & 0xFF;
        cdcReportStateMsg.args[4]      = targetPhysAddr & 0xFF;
    
        if (reportStateMode == CDC_REPORT_STATE_DISCOVER_MODE || targetPhysAddr == 0xFFFF) // HEC support field
        {
            hecExtraField = 0;
            if (pCdc->ports.numberOfOutputs > 0)
            {
                hecExtraField = pCdc->ports.output.hecSupport << 14; // HEC (eHDMI) support for the output
            }

            for (i = 0; i < pCdc->ports.numberOfInputs; i++)
            {
                hecExtraField |= pCdc->ports.input[i].hecSupport << (13 - i); // HEC support bits for inputs
            }

            cdcReportStateMsg.args[6] = (hecExtraField >> 8) & 0xFF;
            cdcReportStateMsg.args[7] = hecExtraField & 0xFF;
            argCount = 8;

        }
        else if (reportStateMode == CDC_REPORT_STATE_MULTIVHEC_MODE) // HEC activation field
        {
            pCdc->deviceStatus.hecFuncState = CDC_HEC_ACTIVATION_FLD;

            hecExtraField = 0;
            if (pCdc->ports.numberOfOutputs > 0)
            {
                hecExtraField = pCdc->ports.output.hecActive << 14; // HEC activation bit for the output
            }

            for (i = 0; i < pCdc->ports.numberOfInputs; i++)
            {
                hecExtraField |= pCdc->ports.input[i].hecActive << (13 - i); // HEC activation bit for inputs
            }

            cdcReportStateMsg.args[6] = (hecExtraField >> 8) & 0xFF;
            cdcReportStateMsg.args[7] = hecExtraField & 0xFF;
            argCount = 8;
        }
        else // short mode
        {
            argCount = 6;
        }
    
        hecState = (pCdc->deviceStatus.hecFuncState  << 6) |
                   (pCdc->deviceStatus.hostFuncState << 4) |
                   (pCdc->deviceStatus.encFuncState  << 2) |
                    pCdc->deviceStatus.cdcErrorState;

        cdcReportStateMsg.args[5]      = hecState;

        pCdc->deviceStatus.cdcErrorState = CDC_ERRCODE_NO_ERROR; // clear error status after report

        //Send CDC message over CPI
        CdcCpiWrite(CDCOP_HEC_REPORT_STATE, argCount, &cdcReportStateMsg);
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CDC HEC SetState" in order to activate from 1 up to 4 VHECs
//!             or deactivate 1 AHEC
//!
//!             This function is to be used by Activator
//!
//! @param[in]  hecActivState - type of a state to set (Activate or Deactivate),
//! @param[in]  numberOfHecs  - minimum 1 and maximum 4 HECs to activate,
//!                             it must be 1 for deactivation,
//! @param[in]  aHecIdx       - array of indexes of HECs in the Instance Record
//-------------------------------------------------------------------------------------------------

void CdcHecSetStateSend(const CdcHecActivation_t hecActivState, 
                        const uint8_t numberOfHecs,
                        const uint8_t aHecIdx[])
{
    uint8_t i,j;
    SiiCpiData_t cdcSetStateMsg;
    uint16_t firstPhysAddr, lastPhysAddr;
    bool_t errFlag = false;

    // Check for necessary conditions required to carry out this operation
    
    if (numberOfHecs < 1 || numberOfHecs > 4 || (numberOfHecs > 1 && hecActivState != CDC_HEC_ACTIVE))
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHecSetStateSend(): invalid number of HECs\n" );
        return;
    }

    // Check appropriateness of HEC channel state for the operation
    if (hecActivState == CDC_HEC_ACTIVE) 
    {
        for (i = 0; i < numberOfHecs; i++) 
        {
            if (pCdc->aHec[aHecIdx[i]].status.channelState != CDC_VHEC) // activation is only allowed for VHEC
            {
                errFlag = true;
                break;
            }
        }
    }
    else
    {
        for (i = 0; i < numberOfHecs; i++) 
        {
            if (pCdc->aHec[aHecIdx[i]].status.channelState == CDC_PHEC) // deactivation is allowed for AHEC & VHEC
            {
                errFlag = true;
                break;
            }
        }        
    }
    
    if (errFlag)
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHecSetStateSend(): invalid HEC channel state\n" );
        return;
    }

    
    firstPhysAddr = pCdc->aHec[aHecIdx[0]].firstDevPhysAddr;
    lastPhysAddr  = pCdc->aHec[aHecIdx[0]].lastDevPhysAddr;

    cdcSetStateMsg.args[3] = (firstPhysAddr >> 8) & 0xFF;
    cdcSetStateMsg.args[4] = firstPhysAddr & 0xFF;
    cdcSetStateMsg.args[5] = (lastPhysAddr >> 8) & 0xFF;
    cdcSetStateMsg.args[6] = lastPhysAddr & 0xFF;    

    cdcSetStateMsg.args[7] = hecActivState; // HEC set state

    j = 7;
    for (i = 1; i < numberOfHecs; i++)  // Physical Addresses of additional terminators of the HECs
    {
        lastPhysAddr  = pCdc->aHec[aHecIdx[i]].lastDevPhysAddr;

        cdcSetStateMsg.args[++j] = (lastPhysAddr >> 8) & 0xFF;
        cdcSetStateMsg.args[++j] = lastPhysAddr & 0xFF;        
    }

    //Send CDC message over CPI
    CdcCpiWrite(CDCOP_HEC_SET_STATE, j + 1, &cdcSetStateMsg);     
    
}



//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CDC HEC SetState Adjacent" in order to activate HEC to adjacent device
//!
//!             This function is to be used by Activator
//!
//! @param[in]  hecActivState - type of a state to set (Activate or Deactivate),
//! @param[in]  adjHecIdx     - index of a HEC to be activated or deactivated
//-------------------------------------------------------------------------------------------------

void CdcHecSetStateAdjacentSend(const CdcHecActivation_t hecActivState, const uint8_t adjHecIdx)
{
    SiiCpiData_t cdcSetStateAdjacentMsg;
    uint16_t lastPhysAddr;

    lastPhysAddr  = pCdc->aHec[adjHecIdx].lastDevPhysAddr;

    cdcSetStateAdjacentMsg.args[3] = (lastPhysAddr >> 8) & 0xFF;
    cdcSetStateAdjacentMsg.args[4] = lastPhysAddr & 0xFF;    
    cdcSetStateAdjacentMsg.args[5] = hecActivState; // HEC set state

    //Send CDC message over CPI
    CdcCpiWrite(CDCOP_HEC_SET_STATE_ADJACENT, 6, &cdcSetStateAdjacentMsg);     
    
}



//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CDC HEC RequestDeactivation" to ask Activator of an AHEC
//!             to deactivate it
//!
//!             This function is to be used by an AHEC member addressing to
//!             the HEC's Activator
//!
//! @param[in]  hecIdx - index of an AHEC to be deactivated
//-------------------------------------------------------------------------------------------------

void CdcHecDeactivationRequestSend(const uint8_t hecIdx)
{
    SiiCpiData_t cdcRequestDeactivationMsg;
    uint16_t actPhysAddr;
    uint16_t firstPhysAddr;
    uint16_t lastPhysAddr;

    // Check for necessary conditions required to carry out this operation
    
    if (pCdc->aHec[hecIdx].status.channelState != CDC_AHEC) // Channel must be in active state 
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHecDeactivationRequestSend(): state of HEC must be active\n" );
        return;
    }

    if (pCdc->aHec[hecIdx].status.isActivator == true)
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHecDeactivationRequestSend(): device must not be an Activator\n" );
        return;
    }

    actPhysAddr    = pCdc->aHec[hecIdx].activatorPhysAddr;
    firstPhysAddr  = pCdc->aHec[hecIdx].firstDevPhysAddr;
    lastPhysAddr   = pCdc->aHec[hecIdx].lastDevPhysAddr;

    cdcRequestDeactivationMsg.args[3] = (actPhysAddr >> 8) & 0xFF;   // PA of Activator
    cdcRequestDeactivationMsg.args[4] =  actPhysAddr & 0xFF;    
    cdcRequestDeactivationMsg.args[5] = (firstPhysAddr >> 8) & 0xFF; // PA of first terminator
    cdcRequestDeactivationMsg.args[6] =  firstPhysAddr & 0xFF;    
    cdcRequestDeactivationMsg.args[7] = (lastPhysAddr >> 8) & 0xFF;  // PA of last terminator in the HEC
    cdcRequestDeactivationMsg.args[8] =  lastPhysAddr & 0xFF;    

    //Send CDC message over CPI
    CdcCpiWrite(CDCOP_HEC_REQUEST_DEACTIVATION, 9, &cdcRequestDeactivationMsg);     
    
}



//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CDC HEC NotifyAlive" to keep AHECs active
//!
//!             This function is to be used by an AHEC members
//-------------------------------------------------------------------------------------------------

void CdcHecNotifyAliveSend(void)
{
    SiiCpiData_t cdcNotifyAliveMsg;        
    
    //Send CDC message over CPI
    CdcCpiWrite(CDCOP_HEC_NOTIFY_ALIVE, 3, &cdcNotifyAliveMsg);     
    
}



//-------------------------------------------------------------------------------------------------
//! @brief:     Sends "CDC HPD SetState" over input of a sink device
//!             This function substitutes physical HPD pin signaling
//!
//!             To be used by a Sink device
//! @param[in]  inputPortIndex  - zero based index of an input port that state will be sent
//-------------------------------------------------------------------------------------------------

void CdcHpdSetStateSend(const uint8_t inputPortIndex)
{
    SiiCpiData_t cdcHpdSetStateMsg;
    CdcHpdState_t hpdState;

    // Check for necessary conditions required to carry out this operation
    
    if (inputPortIndex >= pCdc->ports.numberOfInputs)
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHpdSetStateSend(): invalid input port index\n" );
        return;
    }

    hpdState = pCdc->ports.input[inputPortIndex].hpdState;
    cdcHpdSetStateMsg.args[3] = ((inputPortIndex << 4) & 0xF0) | (hpdState & 0x0F);

    //Send CDC message over CPI
    CdcCpiWrite(CDCOP_HPD_SET_STATE, 4, &cdcHpdSetStateMsg);     
    
}



//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CDC HPD ReportState" over input of a sink device
//!
//!             This function substitutes physical HPD pin signaling.
//!             To be used by a Source device.
//-------------------------------------------------------------------------------------------------

void CdcHpdReportStateSend(void)
{
    SiiCpiData_t cdcHpdReportStateMsg;
    CdcHpdState_t hpdState;
    CdcHpdErrCode_t hpdErrCode;

    // Check for necessary conditions required to carry out this operation

    if (pCdc->ports.numberOfOutputs == 0)
    {
        DEBUG_PRINT( CDC_MSG_ALWAYS, "ERROR in CdcHpdReportStateSend(): root device can't report HPD state\n" );
        return;
    }

    hpdState = pCdc->ports.output.hpdState;
    hpdErrCode = pCdc->ports.output.hpdErrorState;

    pCdc->ports.output.hpdErrorState = CDC_HPD_ERRCODE_NO_ERROR; // clear error status after report

    cdcHpdReportStateMsg.args[3] = ((hpdState << 4) & 0xF0) | (hpdErrCode & 0x0F);

    //Send CDC message over CPI
    CdcCpiWrite(CDCOP_HPD_REPORT_STATE, 4, &cdcHpdReportStateMsg);

}

