//************************************************************************************************
//! @file   si_drv_tpi_system.h
//! @brief  Low level TPI operations. This module directly talks to TPI registers.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#ifndef SI_TX_TPI_SYSTEM_H
#define SI_TX_TPI_SYSTEM_H

//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------

                                            //! TX TPI instance record
typedef struct txTpiInstanceRecord
{
    uint16_t txInterruptWord;       //!< Pending TX interrupt flags and
    uint8_t  plugStatus;            //!< Current status of HPD & RSEN
    bool_t   isWakeUpRequired;      //!< Indicates a request to switch into full power mode

} txTpiInstanceRecord_t;

//-------------------------------------------------------------------------------------------------
//  TPI System API functions
//-------------------------------------------------------------------------------------------------

// General control group
#if (SII_NUM_TX > 1)
void     SiiDrvTpiInstanceSet(uint8_t deviceIndex);
#endif
int      SiiDrvTpiInstanceGet( void );
void     SiiDrvTpiInit(void);
void     SiiDrvTpiSwReset(void);
void     SiiDrvTpiInterruptsEnable(bool_t isInterruptsEnabled);

void     SiiDrvTpiPowerUp(void);
void     SiiDrvTpiStandby(void);
void     SiiDrvTpiPowerDown(void);
bool_t   SiiDrvTpiIsPoweredUp(void);
bool_t	 SiiDrvTmdsIsPoweredUp(void);

void     SiiDrvTpiTmdsOutputEnable(bool_t isEnabled);
void     SiiDrvTpiHdmiOutputModeSet(bool_t isHdmi);
uint8_t  SiiDrvTpiPlugStatusGet(void);
uint16_t SiiDrvTpiDeviceIdGet(void);
uint8_t  SiiDrvTpiRevisionGet(void);

void     SiiDrvTpiProcessInterrupts(void);
bool_t   SiiDrvTpiGetHpdState(void);
uint16_t SiiDrvTpiPendingInterruptsGet(void);
void     SiiDrvTpiPendingInterruptsClear(void);
bool_t   SiiDrvTpiIsWakeUpRequired(void);
void     SiiDrvTpiWakeUpRequestClear(void);

bool_t   SiiDrvTpiIsHdmiLinkStable(void);
void 	 SiiDrvTpiAudioInEnable(void);

void     SiiDrvTpiAudioSamplePacketSourceSet(bool_t isTpi);
void     SiiDrvTpiCtsPacketSourceSet(bool_t isTpi);
void     SiiDrvTpiAvMuteSourceSet(bool_t isTpi);

// Video group
bool_t   SiiDrvTpiVideoSourceSelect(txVideoSource_t source);
void     SiiDrvTpiBlankingLevelsSet(uint8_t levels[3]);
void     SiiDrvTpiVideoModeDataSet(uint16_t pixelClk, uint16_t vertFreqHz, uint16_t totalPixelsPerLine, uint16_t totalLines);
void     SiiDrvTpiPixelCapturingConfig(txVbusClockMode_t clockMode, bool_t isLatchOnRisingEdge, uint8_t pixelRepetitionFactor, bool_t isInputReplicated);
void     SiiDrvTpiInputVideoColorSet(txVbusColSpace_t colorSpace, txVbusColDepth_t bitsPerColor, bool_t isDataRangeExpansionRequired);
void     SiiDrvTpiOutputVideoColorSet(txVbusColSpace_t colorSpace,  txVbusColDepth_t colorDepth, txVbusColorimetry_t colorimetry, bool_t isDataRangeCompressionRequired, bool_t isDitheringEnabled);
void     SiiDrvTpiDeepColorGcpEnable(bool_t isEnabled);
void     SiiDrvTpiAudioClkDividerSet(uint16_t tmdsClock, bool_t isSrcSpdif);
void     SiiDrvTpiAudioClkToggle(void);

// DE and HV Sync regeneration group
void     SiiDrvTpiSyncRegenConfigurationSet(syncRegen_t *pSynRg);
void     SiiDrvTpiDeGenerationEnable(bool_t isEnabled, syncRegen_t *pSynRg);
void     SiiDrvTpiEmbSyncExtractionEnable(bool_t isEnabled, syncRegen_t *pSynRg);
void     SiiDrvTpiYcDemuxEnable(bool_t isEnabled);

// Audio group
void     SiiDrvTpiAudioI2sConfigSet(txAudioSrcConfig_t *pI2sSettings, bool_t isHbr, bool_t isAudioEncoded);
void     SiiDrvTpiAudioSampleConfigSet(audioSampleSize_t smpSize, audioSampleRate_t smpRate, bool_t isHbr);
void     SiiDrvTpiAudioInterfaceSet(txAudioSrc_t source);
void     SiiDrvTpiAudioDisable(void);
void     SiiDrvTpiAudioMuteSet(bool_t isMuted);
void     SiiDrvTpiAudioStatusChannelSet(uint8_t aStatusChannel[5]);
void     SiiDrvTpiAudioI2sPinsToFifoAssign(audioSpkAlloc_t chanAlloc, bool_t isAutoDownsmp);
void     SiiDrvTpiAudioRxSource(bool_t isEnabled, bool_t isMainPipe);

// Info Frames group
void     SiiDrvTpiAviFrameStart(uint8_t *pAviFrame, uint8_t length, uint8_t checksum);
void     SiiDrvTpiAviFrameUpdate(void);
void     SiiDrvTpiAvMuteSendingEnable(bool_t isMuted);
void     SiiDrvTpiInfoFrameEnable(bool_t isEnabled, bool_t isAll, infoFrameType_t ifType);
bool_t   SiiDrvTpiInfoFrameIsEnabled(infoFrameType_t ifType);
void     SiiDrvTpiInfoFrameLoad(infoFrameType_t ifType, uint8_t *pFrame, uint8_t length, bool_t isChecksumIncluded);

// HDCP group
void     SiiDrvTpiHdcpProtectionEnable(bool_t isEnabled);
uint8_t  SiiDrvTpiHdcpStatusGet(void);
bool_t   SiiDrvTpiHdcpIsPart2Done(void);
bool_t   SiiDrvTpiHdcpIsAuthenticationGood(void);
void     SiiDrvTpiHdcpDynamicAuthenticationEnable(bool_t isEnabled);
void     SiiDrvTpiHdcpEncriptionEnable(bool_t isEnable);
bool_t   SiiDrvTpiIsDownstreamHdcpAvailable(void);
void     SiiDrvTpiBksvGet(uint8_t aBksv[5]);
bool_t   SiiDrvTpiKsvListPortionSizeGet(uint8_t *pBytesToRead);
bool_t   SiiDrvTpiIsZeroKsvListReady(void);
void     SiiDrvTpiKsvListGet(uint8_t *pBuffer, uint8_t length);
void     SiiDrvTpiBStatusGet(uint8_t *pDsBStatus);
uint8_t  SiiDrvTpiBcapsGet(void);
void     SiiDrvTpiHdcpRi128Comp(uint8_t value);
void	 SiiDrvTpiHdcpR0CalcConfigSet( int isVsyncBase );
void SiiDrvTpiSourceTerminationEnable(bool_t isEnabled);

#if ( configSII_DEV_953x_PORTING == 1 )
void SiiDrcTpiDisableInterruptStandby( void );
#endif

#endif //  SI_TX_TPI_SYSTEM_H
