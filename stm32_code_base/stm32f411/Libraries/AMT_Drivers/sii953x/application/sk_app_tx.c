//***************************************************************************
//! @file     sk_app_tx.c
//! @brief    Wraps board and device functions for the TX component
//            and the application
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"
#include "si_tx_component.h"
#include "si_drv_tpi_system.h"
#include "si_edid_tx_component.h"
#include "si_drv_nvram_sram.h"
#include "si_cbus_config.h"
#if INC_CEC
#include "si_cec_component.h"
#include "sk_app_cec.h"
#endif
#if INC_AUDIO_RX
#include "si_drv_audio.h"
#endif

#include "si_drv_audio_mix.h"
#include "si_drv_switch.h"
#include "si_osal_timer.h"
#if INC_OSD
#include "si_osd_component.h"
#endif
#include "si_drv_repeater.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "Debug.h"
#endif 

//-------------------------------------------------------------------------------------------------
// Manifest constants
//-------------------------------------------------------------------------------------------------

// Minimal time delay (in ms) that has to have elapsed before repeated call of
// the TX Task Process function
#define SI_TX_TIME_STEP_MIN_MS         20

extern int_t SiiDrvRxPipePortGet ( int_t pipe );

#if (MHL_20 == ENABLE) /*Smith modifies*/
extern void CreateMHL3DData(bool_t isDismissEDID);
#endif 

static bool_t CheckMhl3DVsi(uint8_t *pVsi);
//-------------------------------------------------------------------------------------------------
//! @brief      Configure current TX audio and video mode
//!
//! @param[in]  videoSrc    - source of video for TX
//! @param[in]  videoFormat - format of the video from source
//! @param[in]  audioSrc    - source of audio for TX
//-------------------------------------------------------------------------------------------------

static void InputAudVidConfig(txVideoSource_t videoSrc, videoFormatId_t videoFormat,
                              txAudioSrc_t audioSrc, audioSampleRate_t audioSampRate,
                              bool_t isNonHdmiAudio)
{
    txVideoBusMode_t    busMode;
    audioFormat_t       audioFormat;
    txAudioSrcConfig_t  audioI2sConfig;
    uint8_t txInstance;
    uint8_t Ri128CompValue[2][3]={{0x0, 0x0, 0x7f},{0x0, 0x7f, 0x7f}};

    // Reset audio format descriptor
    memset(&audioFormat, 0, sizeof(audioFormat_t));

    // Get instance index
    txInstance = SiiTxInstanceGet();

    // VIDEO --------------------------------------------------------------
    switch (videoSrc)
    {
        case SI_TX_VIDEO_SOURCE_PGEN:
            // Set input Video for TPG Source
             busMode.clockMode = SI_TX_VBUS_CLOCK_NORMAL;
             busMode.colorSpace = SI_TX_VBUS_COLOR_SPACE_RGB;
             busMode.bitsPerColor = SI_TX_VBUS_BITS_PER_COLOR_8;
             busMode.colorimetry = SI_TX_VIDEO_COLORIMETRY_ITU601;
             busMode.pixelRepetition = SI_TX_VIDEO_SRC_PIXEL_REP_1;
             busMode.isLatchOnRisingEdge = false;

             SiiTxVideoSourceConfig(videoSrc, &busMode);
             SiiTxInputVideoModeSet(videoFormat, NULL, SI_TX_VIDEO_3D_NONE, SI_TX_VIDEO_HVSYNC_DE, NULL);

             SiiDrvTpiHdcpRi128Comp(Ri128CompValue[txInstance][SI_TX_VIDEO_SOURCE_PGEN]);

             break;
        case SI_TX_VIDEO_SOURCE_HDMI:
        default:
            SiiTxVideoSourceConfig(SI_TX_VIDEO_SOURCE_HDMI, NULL);
            if(!isNonHdmiAudio)
            {
				SiiTxInputAudioModeSet(SI_TX_AUDIO_SRC_NONE, &audioFormat);
            }

            SiiDrvTpiHdcpRi128Comp(Ri128CompValue[txInstance][SI_TX_VIDEO_SOURCE_HDMI]);

            break;
    }

    // AUDIO --------------------------------------------------------------
    if (videoSrc != SI_TX_VIDEO_SOURCE_HDMI || isNonHdmiAudio)
    {
        if(audioSrc != SI_TX_AUDIO_SRC_NONE)
        {
            switch (audioSrc)
            {
                default:
                case SI_TX_AUDIO_SRC_SPDIF:
                case SI_TX_AUDIO_SRC_I2S_L0:
                    // Set Audio parameters
                    audioFormat.channelCount = 2;
                    audioFormat.encoding = SI_TX_AUDIO_ENC_PCM;
                    audioFormat.smpRate = audioSampRate;
                    audioFormat.smpSize = SI_TX_AUDIO_SMP_SIZE_24_BIT;
                    audioFormat.isDwnMxInhibit = false;
                    audioFormat.lShift = 0;
                    audioFormat.lfePbLevel = 0;
                    audioFormat.spkMap = SI_TX_AUDIO_SPK_FL_FR;

                    // I2S only parameters
                    audioI2sConfig.mclkMult = SI_TX_AUDIO_I2S_MCLK_MULT_256;
                    audioI2sConfig.sampleEdge = SI_TX_AUDIO_I2S_SCK_EDGE_FALLING;
                    audioI2sConfig.wsPolarity = SI_TX_AUDIO_I2S_WS_LOW;
                    audioI2sConfig.sdJustify = SI_TX_AUDIO_I2S_SD_JUSTIF_LEFT;
                    audioI2sConfig.sdDirection = SI_TX_AUDIO_I2S_SD_DIR_MSB;
                    audioI2sConfig.sdFirstBitShifted = SI_TX_AUDIO_I2S_SD_BIT0_SHIFTED;
                    break;

                case SI_TX_AUDIO_SRC_I2S_L1:
                    // Set Audio parameters
                    audioFormat.channelCount = 8;
                    audioFormat.encoding = SI_TX_AUDIO_ENC_PCM;
                    audioFormat.smpRate = audioSampRate;
                    audioFormat.smpSize = SI_TX_AUDIO_SMP_SIZE_24_BIT;
                    audioFormat.isDwnMxInhibit = false;
                    audioFormat.lShift = 0;
                    audioFormat.lfePbLevel = 0;
                    audioFormat.spkMap = SI_TX_AUDIO_SPK_FL_FR_LFE_FC_RL_RR_RLC_RRC;

                    // I2S only parameters
                    audioI2sConfig.mclkMult = SI_TX_AUDIO_I2S_MCLK_MULT_256;
                    audioI2sConfig.sampleEdge = SI_TX_AUDIO_I2S_SCK_EDGE_FALLING;
                    audioI2sConfig.wsPolarity = SI_TX_AUDIO_I2S_WS_LOW;
                    audioI2sConfig.sdJustify = SI_TX_AUDIO_I2S_SD_JUSTIF_LEFT;
                    audioI2sConfig.sdDirection = SI_TX_AUDIO_I2S_SD_DIR_MSB;
                    audioI2sConfig.sdFirstBitShifted = SI_TX_AUDIO_I2S_SD_BIT0_SHIFTED;

                    break;
                case SI_TX_AUDIO_SRC_HBRA:
                    // Set Audio parameters
                    audioFormat.channelCount = 8;
                    audioFormat.encoding = SI_TX_AUDIO_ENC_DTSHD;
                    audioFormat.smpRate = SI_TX_AUDIO_SMP_RATE_192_KHZ;
                    audioFormat.smpSize = SI_TX_AUDIO_SMP_SIZE_24_BIT;
                    audioFormat.isDwnMxInhibit = false;
                    audioFormat.lShift = 0;
                    audioFormat.lfePbLevel = 0;
                    audioFormat.spkMap = SI_TX_AUDIO_SPK_FL_FR_LFE_FC_RL_RR_RLC_RRC;

                    // I2S only parameters
                    audioI2sConfig.mclkMult = SI_TX_AUDIO_I2S_MCLK_MULT_128;
                    audioI2sConfig.sampleEdge = SI_TX_AUDIO_I2S_SCK_EDGE_FALLING;
                    audioI2sConfig.wsPolarity = SI_TX_AUDIO_I2S_WS_LOW;
                    audioI2sConfig.sdJustify = SI_TX_AUDIO_I2S_SD_JUSTIF_LEFT;
                    audioI2sConfig.sdDirection = SI_TX_AUDIO_I2S_SD_DIR_MSB;
                    audioI2sConfig.sdFirstBitShifted = SI_TX_AUDIO_I2S_SD_BIT0_SHIFTED;
                    break;
            }

            //SiiDrvTpiAudioClkDividerSet(0, (audioSrc == SI_TX_AUDIO_SRC_SPDIF));
            SiiTxAudioSourceConfig(audioSrc, &audioI2sConfig);
            SiiTxInputAudioModeSet(audioSrc, &audioFormat);
            SiiTxInputAudioUnmute();
        }
    }
    else
    {
        SiiTxInputAudioModeSet(SI_TX_AUDIO_SRC_NONE, &audioFormat);
    }

}

//-------------------------------------------------------------------------------------------------
//! @brief      Configure TX instance to start sending HDCP protected
//!             (if required) A/V content.
//!
//! @param[in]  instance - TX instance
//!
//-------------------------------------------------------------------------------------------------

static void StartTx(uint8_t instance)
{
    SiiTxOutputInterfaceSet(SI_TX_OUTPUT_MODE_HDMI);
    //  Bug 33155 - Compatibility: Melbourne vs. LGTV 42LM5700-CE: Hot-unplug/plug HDMI cable causes NO video on TV
    if (SiiTxGetLastResult().lastErrorCode == SI_TX_ERROR_CODE_HDMI_NOT_SUPPORTED_BY_DS)
    {
        SiiTxOutputInterfaceSet(SI_TX_OUTPUT_MODE_DVI);
        SiiDrvRxAudioMixPassThroughConfig(false, false);
    }

    InputAudVidConfig(app.currentVideoSrc[instance],
                      app.currentVideoFormat[instance],
                      app.currentAudioType[instance],
                      app.audioSampleRate[instance], false);


    SiiTxResume();          // enable TMDS
#if INC_OSD
//    SiiDrvOsdStateUpdate();
#endif

    // Copy AIF from selected Rx source if in audio overlay mode
    if ((app.currentAudioSource[instance] == SiiSwitchAudioSourceMainPipe) ||
        (app.currentAudioSource[instance] == SiiSwitchAudioSourceSubPipe))
    {
        bool_t isMainPipeAudio =  (app.currentAudioSource[instance] == SiiSwitchAudioSourceMainPipe);

        SiiDrvTpiAudioRxSource(true, isMainPipeAudio);
        // Copy AIF to TX
        SkAppTxCopyAif(instance, isMainPipeAudio);
        SiiDrvRxAudioInstanceSet(isMainPipeAudio);
        SiiDrvRxAudioFifoRedirSet(true);
        SiiTxInputAudioUnmute();
    }
    else
    {
        SiiDrvTpiAudioRxSource(false, true);
        SiiDrvRxAudioInstanceSet(0);
        SiiDrvRxAudioFifoRedirSet(false);
        SiiDrvRxAudioInstanceSet(1);
        SiiDrvRxAudioFifoRedirSet(false);
    }

    SiiTxRecoverAfterSourceClockInterruption();

    if (!app.isTxRepeaterMode[instance])
    {
        if (app.isTxHdcpRequired[instance])
        {
            SiiRepeaterForceShaSet(instance, true); // Tie SHA engine to resuming TX
            SiiTxAvMute();
            SiiTxHdcpEnable();
        }
        else
        {
            SiiTxHdcpDisable();
            SiiTxAvUnmute();
        }
    }

}

//-------------------------------------------------------------------------------------------------
//! @brief      Configures TX instance to stop sending A/V content.
//!
//! @param[in]  instance - TX instance
//!
//-------------------------------------------------------------------------------------------------

static void StopTx(uint8_t instance)
{
    app.currentVideoSrc[instance] = SI_TX_VIDEO_SOURCE_HDMI;
    app.currentVideoFormat[instance] = vm1_640x480p; // default setting, has no effect
    //app.currentAudioType[instance] = SI_TX_AUDIO_SRC_NONE;

    InputAudVidConfig(app.currentVideoSrc[instance],
                      app.currentVideoFormat[instance],
                      app.currentAudioType[instance],
                      app.audioSampleRate[instance], false);

    if (!app.isTxRepeaterMode[instance])
    {
        if (app.isTxHdcpRequired[instance])
        {
            SiiTxHdcpDisable();
            SiiRepeaterForceShaSet(instance, false); // release SHA
        }
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Configures TX to start or stop sending A/V content.
//!
//! @param[in]  instance  - TX instance (0 or 1)
//! @param[in]  isEnabled - determines if to start (true) or stop (false) transmitters
//!
//-------------------------------------------------------------------------------------------------

static void EnableTransmitter(uint8_t instance, bool_t isEnabled)
{
        SiiDrvRxAudioMixInstanceSet(instance);
        SkAppTxInstanceSet(instance);

        if (isEnabled)
        {
            // Disable HDCP in case previous mode left it enabled
            SiiTxHdcpDisable();

            // Disable bypassing audio packet by audio mixer
            SiiDrvRxAudioMixPassThroughConfig(false, false);

            if (SiiTxStatusGet().isSinkReady)
            {
                StartTx(instance);
            }
        }
        else
        {
            StopTx(instance);
            // Enable bypassing audio packet by audio mixer
            SiiDrvRxAudioMixPassThroughConfig(true, true);
        }
}


//-------------------------------------------------------------------------------------------------
//  TX application level API functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Set instance of Tx and dependent components (EdidTx).
//!
//!             This function must be used instead of direct call to the component Tx API
//!             instance setting function.
//!
//! @param[in]  instance - instance of the current TX
//-------------------------------------------------------------------------------------------------

void SkAppTxInstanceSet(uint8_t instance)
{
    SiiTxInstanceSet(instance);

    if (app.numOfEdidTxInst == 2)
    {
        // "I" topology case.
        // In 2 instance mode of EdidTx, only single Tx per EdidTx instance is serviced
        // So that there is 1:1 relationship between Tx and EdidTx component instances
        SiiEdidTxInstanceSet(instance);
    }
    else
    {
        // Select EDID TX input for the case of "Y" topology
        // when one EdidTx component service both transmitters
        // Limitation: TX branches in "Y" topology shall have instance indexes [0,1,...]
        SiiEdidTxInputSet(instance);
    }
}


#if INC_IV
//-------------------------------------------------------------------------------------------------
//! @brief      Enable or Disable internal video pattern generator.
//-------------------------------------------------------------------------------------------------

void SkAppTpgEnable ( bool_t isEnabled )
{
    if (isEnabled)
    {
        // Configure Audio PLLs if TPG will uses their output as clock source
        SiiDrvRxAudioInstanceSet(0);
        SiiDrvRxAudioPllConfig(app.tpgClockSrc == SI_TPG_CLK_P0);
        SiiDrvRxAudioInstanceSet(1);
        SiiDrvRxAudioPllConfig(app.tpgClockSrc == SI_TPG_CLK_P1);

        // Enable TPG (or ETPG)
        SiiDrvTpgVideoFormatSet(app.tpgVideoFormat, app.tpgClockSrc);
        if (app.isExtTpg)
        {
        	SiiDrvExtTpgPatternSet(app.tpgVideoPattern);
            SiiDrvExtTpgEnable(true);
        }
        else
        {
            SiiDrvTpgEnable(true);
        }
        SiiOsTimerWait(10);// wait for input video clock to settle
    }
    else
    {
        // Release audio PLLs if used
        SiiDrvRxAudioInstanceSet(0);
        SiiDrvRxAudioPllConfig(false);
        SiiDrvRxAudioInstanceSet(1);
        SiiDrvRxAudioPllConfig(false);
       // SiiDrvTpgVideoFormatSet(app.tpgVideoFormat, app.tpgClockSrc);
        // Disable TPG (or ETPG)
        if (app.isExtTpg)
        {
            SiiDrvExtTpgEnable(false);
        }
        else
        {
            SiiDrvTpgEnable(false);
        }
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Configure TX to show test pattern and transmit external audio.
//-------------------------------------------------------------------------------------------------

void SkAppInternalVideoConfig ( uint_t txInstance, bool_t isTxEnabled)
{
    videoFormatId_t videoFormat;

    // Convert to proper video format for TPG video
    switch (app.tpgVideoFormat)
    {
        default:
        case SI_TPG_FMT_VID_720_60:
            videoFormat = vm4_1280x720p;
            break;
        case SI_TPG_FMT_VID_576_50:
            videoFormat = vm17_18_720x576p;
            break;
        case SI_TPG_FMT_VID_480_60:
            videoFormat = vm2_3_720x480p;
            break;
    }

    app.currentVideoSrc[txInstance] = SI_TX_VIDEO_SOURCE_PGEN;
    app.currentVideoFormat[txInstance] = videoFormat;
    EnableTransmitter(txInstance, isTxEnabled);
#if INC_OSD
    // Adjust OSD font for the resolution, if TX0 switches in TPG mode
    if (txInstance == 0)
    {
        if (isTxEnabled)
        {
            SiiDrvOsdResolutionChange( videoFormat, 0, 0 );
            if ( app.isOsdMenuEnabled )
            {
                SiiOsdEnableOsdDisplay( true );
            }
        }
    }
#endif //#if INC_OSD
}
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Configure TX to transmit external audio (video is bypassed).
//-------------------------------------------------------------------------------------------------

void SkAppExtAudioInsertConfig(int_t txInstance, bool_t isEnabled)
{
	uint8_t audioSampleRate;

	audioSampleRate = SiiDrvRxAudioSampleRateGet();

    //DEBUG_PRINT(MSG_DBG, "\n\n audioSampleRate =%x\n\n",audioSampleRate);
	app.audioSampleRate[txInstance] = audioSampleRate + 1;


    app.currentVideoSrc[txInstance]  = SI_TX_VIDEO_SOURCE_HDMI;




    //DEBUG_PRINT(MSG_DBG, "\n\n SkAppExtAudioInsertConfig =%x %x %x %x %x\n\n", app.currentVideoSrc[txInstance],app.currentVideoFormat[txInstance],app.currentAudioType[txInstance], app.audioSampleRate[txInstance],txInstance);

    InputAudVidConfig(app.currentVideoSrc[txInstance],
                      app.currentVideoFormat[txInstance],
                      app.currentAudioType[txInstance],
                      app.audioSampleRate[txInstance], isEnabled);

}



//-------------------------------------------------------------------------------------------------
//! @brief      TX Edid component initialization.
//!
//! @param[in]  isMatrixMode - indicate if device is in matrix mode (two independent pipes)
//! @retval     true - if initialization was successful.
//-------------------------------------------------------------------------------------------------

bool_t SkAppDeviceInitEdidTx (bool_t isAudioByPass)
{
    bool_t   success = false;
    uint16_t videoFeatureMask, audioFeatureMask, miscOptionsMask;
    uint8_t  numOfTxPerEdidTxInst;
    uint8_t  instance;

    // In non-matrix mode there is only one Rx source providing content for 1 or 2 Tx.
    // Therefore, only one instance of EDID Tx component servicing up to 2 Tx is required.
    // If 2 transmitters are connected to one Rx, a merged EDID structure is created.
    // The merged EDID includes the capability declarations common for both TXs
    app.numOfEdidTxInst = 1;
    numOfTxPerEdidTxInst = 1;
    // In case of the joint TX EDID, it's more convenient to use legacy Rx EDID mode
    // (One common EDID is shared across all ports)
    SiiDrvNvramEdidModeSet(SII_NVRAM_LEGACY_EDID_MODE);

    DEBUG_PRINT(MSG_DBG, "EDID Tx Component has %d instances.\n", app.numOfEdidTxInst);
	TRACE_DEBUG((0, "[ HDMI MSG ] SkAppDeviceInitEdidTx"));

    for (instance = 0; instance < app.numOfEdidTxInst; ++instance)
    {
        SiiEdidTxInstanceSet(instance);

        if ( SiiEdidTxInitialize())
        {
            videoFeatureMask =
                SI_TX_EDID_VIDEO_CFG_ALLOW_BAD_CSUM_FOR_BLOCK0          |
                SI_TX_EDID_VIDEO_CFG_ALLOW_PC_MODES_NO_SCALER           |
                SI_TX_EDID_VIDEO_CFG_BYPASS_SCALER                      |
                SI_TX_EDID_VIDEO_CFG_ENABLE_REMEDIES                    |
                SI_TX_EDID_VIDEO_CFG_ENABLE_3D                          |
                SI_TX_EDID_VIDEO_CFG_ENABLE_XVYCC                       |
                SI_TX_EDID_VIDEO_CFG_DC30_36                            |
                (app.isEdidMhlLimits ? SI_TX_EDID_VIDEO_CFG_ENABLE_MHL_LIMITS : 0);

            audioFeatureMask = 0;

            if ( isAudioByPass )
            {
                audioFeatureMask |= SI_TX_EDID_AUDIO_CFG_BYPASS;// | SI_TX_EDID_AUDIO_CFG_SPDIF_INPUT;
            }
            else
            {
            	audioFeatureMask &= ~SI_TX_EDID_AUDIO_CFG_BYPASS;
            }

            miscOptionsMask = 0;

            if ( SiiEdidTxConfigure( numOfTxPerEdidTxInst, videoFeatureMask, audioFeatureMask, miscOptionsMask ))
            {
                success = true;
            }

#if (MHL_20 == ENABLE) /*Smith modifies*/
            CreateMHL3DData(true);
#endif    
        }
        
    }

    DEBUG_PRINT(MSG_ALWAYS, "MHL limits to TX EDID: %s\n", app.isEdidMhlLimits ? "Y" : "N");
    TRACE_DEBUG((0, "MHL limits to TX EDID: %s ", app.isEdidMhlLimits ? "Y" : "N" ));
    
    return( success );
}

#if INC_RTPI
//-------------------------------------------------------------------------------------------------
//! @brief      Call back RTPI function to initialize EDID Tx.

//-------------------------------------------------------------------------------------------------

void SiiRtpiCbDeviceInitTx(bool_t isMatrixmode, bool_t isAudioByPass)
{
	SkAppDeviceInitEdidTx(isAudioByPass);
}
#endif // INC_RTPI

//-------------------------------------------------------------------------------------------------
//! @brief      TX component initialization.
//!
//! @retval     true - if initialization was successful.
//-------------------------------------------------------------------------------------------------

bool_t SkAppDeviceInitTx(void)
{
    uint8_t instance;
    txRevision_t txRevision;

    //PrintAlways( "Transmitter Revision:\n");
    for (instance = 0; instance < SII_NUM_TX; instance++)
    {
        SkAppTxInstanceSet(instance);
        SiiTxInitialize();
        SiiTxHdcpConfigure(false, 10, false);
    
        txRevision = SiiTxRevisionGet();
        DEBUG_PRINT(MSG_ALWAYS, "\tTX%d [COMP: %X, DEV: %X, TPI: %X]\n", instance,
                                txRevision.component, txRevision.deviceId, txRevision.tpi);

		TRACE_DEBUG((0, "TX%d [COMP: %X, DEV: %X, TPI: %X]", instance,
                                txRevision.component, txRevision.deviceId, txRevision.tpi ));

        // Configure Audio Mixer to bypass audio and other packets
        SiiDrvRxAudioMixInstanceSet(instance);
        SiiDrvRxAudioMixPassThroughConfig(true, true);
        // GCP packets shall never be passed through
        // as AVMUTE status must be controlled by repeater (not the upstream source only)
        SiiDrvRxAudioMixGcpPassThroughConfig(false);
    }

    return (true);
}

SiiTimer_t testTimer;       	// used in wake-up detection by switching between instances

//-------------------------------------------------------------------------------------------------
//! @brief      TX main task routine.
//!
//!             TX related background tasks run here.
//-------------------------------------------------------------------------------------------------

void SkAppTaskTx(void)
{
    uint8_t instance;
    txFeedbackMsg_t     fbMsg = SI_TX_FB_MSG_NONE;
    static clock_time_t lastTimeMs[SII_NUM_TX];
    clock_time_t        currentTimeMs = SiiOsTimerTotalElapsed();
    clock_time_t        timeStep;

    for (instance = 0; instance < SII_NUM_TX; instance++)
    {
        timeStep = SkTimeDiffMs(lastTimeMs[instance], currentTimeMs);

        SkAppTxInstanceSet(instance);

        // Call Task Processor periodically, but not too frequently
        // However, call it right away if TX interrupt is pending
        if (SiiTxStatusGet().isIrqPending || (timeStep > SI_TX_TIME_STEP_MIN_MS))
        {
            fbMsg = SiiTxTaskProcess(timeStep);
    
            // React on feedback messages
            switch (fbMsg)
            {
                default:
                case SI_TX_FB_MSG_NONE:
#if INC_CEC
                    if ( app.cecEnable && app.txSendOneTouch )
                    {
                        // Wake up TV and tell it we're the one!
                        // Keep trying until task queue is free
    
                        SkAppCecInstanceSet( CEC_INSTANCE_AVR );
                        if ( SiiCecOneTouchPlay())
                        {
                            app.txSendOneTouch = false;
                        }
                    }
#endif
                    break;
    
                case SI_TX_FB_MSG_HDCP_AUTH_DONE:
                    SkAppTxHdcpAuthDone();
                    SiiRepeaterForceShaSet(instance, false); // release SHA
                    DEBUG_PRINT(MSG_ALWAYS, "APP TX%d: HDCP Auth SUCCESSFUL\n", instance);
					TRACE_DEBUG((0, "APP TX%d: HDCP Auth SUCCESSFUL", instance ));
                    {
                    	unsigned long timeElapsed = SkTimeDiffMs( testTimer.msStart, SiiOsTimerTotalElapsed());
                    	DEBUG_PRINT(MSG_ALWAYS, "\n Port Switching Time: %d millisec\n", timeElapsed);
						TRACE_DEBUG((0, "Port Switching Time: %d millisec", timeElapsed ));
                    }

                    break;
    
                case SI_TX_FB_MSG_ERR_HDCP_AUTH_FAILED:
                    DEBUG_PRINT(MSG_ALWAYS, "APP TX%d: HDCP Auth FAILED!\n", instance);
					TRACE_DEBUG((0, "APP TX%d: HDCP Auth FAILED!", instance ));
                    //if (!app.isTxBypassMode[instance])
                    {
                        // Mute AV if HDCP failed
                        // (unmuting will be done after successful HDCP authentication)
                        SiiTxAvMute();
                    }
                    break;
    
                case SI_TX_FB_MSG_HOT_PLUG_RECONNECT:
                    // Put special handling for short HPD disconnection here, if required.
                    DEBUG_PRINT(MSG_ALWAYS, "APP TX%d: HPD Reconnecting\n", instance);
					TRACE_DEBUG((0, " APP TX%d: HPD Reconnecting ", instance ));
                	app.isTxDisconnectedImmed=false;
                    if (app.isTxBypassMode[instance])
                    {
                        // Propagate HPD Toggle event to the upstream
                        SkAppTxHpdConnection(SI_RPT_TX_HPD_TOGGLE);
                    }
                    break;

    
                case SI_TX_FB_MSG_HOT_PLUG_CONNECTED:

                    DEBUG_PRINT(MSG_ALWAYS, "APP TX%d: HPD Connected\n", instance);

#if 0					
					TRACE_DEBUG((0, " APP TX%d: HPD Connected ", instance ));
					TRACE_DEBUG((0, " APP TX: app.isTxRepeaterMode = %d", app.isTxRepeaterMode[instance] ));
					TRACE_DEBUG((0, " APP TX: app.repeaterEnable = %d", app.repeaterEnable ));
					TRACE_DEBUG((0, " APP TX: app.cecEnable = %d", app.cecEnable ));
#endif 					
					
                	app.isTxDisconnectedImmed=true;
                    if (app.isTxRepeaterMode[instance])
                    {
                        SkAppTxHpdConnection(OFF);
                    }
                    else   // Bug 33051 - [CP9533_FW] MHL1/2:Video color is incorrect after hotplug DS HDMI cable of 9533
                    {
                        SiiDrvRepeaterSwitchRxHotPlug(OFF);
                    }

                    // Read & process DS EDID, if a new DS device has been connected
                    if (SiiTxStatusGet().isHotPlugDetected)
                    {
                        SkAppProcessTxEdid(instance);
    
#if INC_CEC
                        if ( app.cecEnable )
                        {						
							// Get our CEC physical address (possible new DS EDID)
							// and allocate a (possibly) new logical address.
							SkAppCecInstanceSet( CEC_INSTANCE_AVR );
							SiiCecEnumerateDevices( appCecSourceLaList );
							SiiCecEnumerateDeviceLa( appCecSourceLaList );
							//                                app.txSendOneTouch = true;

                            SiiCecSetDevicePA( SiiTxPhysicalAddressGet() );
#if ( configSII_DEV_953x_PORTING == 1 )
                            AmTCecTaskAssign(Cec_TASK_CHANGE_PA);
#endif
                        }
#endif
                    }
                    break;
    
                case SI_TX_FB_MSG_HOT_PLUG_DISCONNECTED:
                    DEBUG_PRINT(MSG_ALWAYS, "APP TX%d: HPD Disconnected\n", instance);
					TRACE_DEBUG((0, "APP TX%d: HPD Disconnected ", instance ));
                    SiiTxStandby();
    
                    SkAppDismissTxEdid(instance);

                    if (app.isTxRepeaterMode[instance])
                    {
                        SkAppTxHpdConnection(OFF);
                    }
                    else    // Bug 33051 - [CP9533_FW] MHL1/2:Video color is incorrect after hotplug DS HDMI cable of 9533
                    {
                        SiiDrvRepeaterSwitchRxHotPlug(OFF);
                    }

#if INC_CEC
                    if ( app.cecEnable && (instance == 0))
                    {
                        // Set sink mode CEC
                        SkAppCecInstanceSet( CEC_INSTANCE_AVR );
                        SiiCecSetDevicePA( 0x0000 );
                    }
#endif  //INC_CEC
                    break;
    
                case SI_TX_FB_MSG_SINK_READY:
                	app.isTxDisconnectedImmed=false;
                    DEBUG_PRINT(MSG_ALWAYS, "APP TX%d: Downstream sink is ready and Tx Bypasmode is %d\n", instance, (int)app.isTxBypassMode[instance]);
					TRACE_DEBUG((0, " APP TX%d: Downstream sink is ready and Tx Bypasmode is %d ", instance, (int)app.isTxBypassMode[instance] ));
					/*if (app.isTxBypassMode[instance])
					{
						StopTx(instance);
					}
					else*/
                    {
                        StartTx(instance);
                    }

                    if (app.isTxRepeaterMode[instance])
                    {
                        SkAppTxHpdConnection(ON);
                    }
                    else    // Bug 33051 - [CP9533_FW] MHL1/2:Video color is incorrect after hotplug DS HDMI cable of 9533
                    {
                        SiiDrvRepeaterSwitchRxHotPlug(ON);
                    }
                    break;
            }
    
            lastTimeMs[instance] = currentTimeMs;
        }
    }

}


//-------------------------------------------------------------------------------------------------
//! @brief      Process EDID from newly connected transmitter.
//!
//! @param[in]  txInstance - instance of the current TX
//-------------------------------------------------------------------------------------------------

void SkAppProcessTxEdid(uint8_t txInstance)
{
    if (app.numOfEdidTxInst == 2)
    {
        // In 2 instance mode of EdidTx, only single Tx is serviced
        SiiEdidTxInstanceSet(txInstance);
        SiiEdidTxProcessEdid(0);
    }
    else
    {
        // In 1 instance mode of EdidTx, both Tx instances are serviced
        SiiEdidTxProcessEdid(txInstance);
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Get rid of EDID from disconnected transmitter.
//!
//! @param[in]  txInstance - instance of the current TX
//-------------------------------------------------------------------------------------------------

void SkAppDismissTxEdid(uint8_t txInstance)
{
    uint8_t rxPort;

    if (app.numOfEdidTxInst == 2)
    {
        // Only one EDID exist in 2 pipe mode
        SiiEdidTxInstanceSet(txInstance);
        SiiEdidTxDismissEdid(0);
        // The Rx EDID RAM associated with the disconnected Tx
        // must be replaced with NVRAM EDID
        SiiDrvNvramCopyToSram( NVRAM_HDMI_EDID, txInstance, true );
    }
    else
    {
        // Try to re-make US EDID from other TX EDID.
        // If all TXs are disconnected, this function won't be able to create a new EDID
        SiiEdidTxDismissEdid(txInstance);
        if (SiiEdidTxGetLastResult() == SI_TX_EDID_ERROR_CODE_NO_US_EDID_CREATED)
        {
            // Replace DS EDID in all RX ports by the default EDID,
            // if both TXs are disconnected
            for (rxPort = 0; rxPort < SII_INPUT_PORT_COUNT; ++rxPort)
            {
                SiiDrvNvramCopyToSram( NVRAM_HDMI_EDID, rxPort, true );
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Copy AIF from Rx pipe to Tx.
//!
//!             This function is used in audio overlay mode, when audio stream gets extracted
//!             from main or sub pipe and inserted into Tx.
//!
//! @param[in]  txInstance - instance of the current TX
//! @param[in]  isMainPipe - source pipe
//-------------------------------------------------------------------------------------------------

void SkAppTxCopyAif(uint8_t txInstance, bool_t isMainPipe)
{
    uint8_t pIfData[32];
    uint8_t len;
    uint8_t audioMode;
    uint8_t audioLayout;
    uint8_t audioSampleRate;

    SkAppTxInstanceSet(txInstance);

    // Get source audio format
    SiiDrvRxAudioInstanceSet(isMainPipe ? 0 : 1);
    audioMode = SiiDrvRxAudioModeGet();
    audioLayout = SiiDrvRxAudioLayoutGet();
    audioSampleRate = SiiDrvRxAudioSampleRateGet();

    switch (audioMode)
    {
        case SII_AUD_HBRA_MODE:
            app.currentAudioType[txInstance] = SI_TX_AUDIO_SRC_HBRA;
            break;
        case SII_AUD_DSD_MODE:
            app.currentAudioType[txInstance] = (audioLayout == SII_AUD_MULTI_CHANNEL) ?  SI_TX_AUDIO_SRC_DSD_L1 :
                                                                                      SI_TX_AUDIO_SRC_DSD_L0 ;
            break;
        default:
        case SII_AUD_PCM_MODE:
            app.currentAudioType[txInstance] = (audioLayout == SII_AUD_MULTI_CHANNEL) ?  SI_TX_AUDIO_SRC_I2S_L1 :
                                                                                      SI_TX_AUDIO_SRC_I2S_L0 ;
            break;
    }

    app.audioSampleRate[txInstance] = audioSampleRate + 1;

    // Set up Tx to transmit audio of given format
    InputAudVidConfig(app.currentVideoSrc[txInstance],
                      app.currentVideoFormat[txInstance],
                      app.currentAudioType[txInstance],
                      app.audioSampleRate[txInstance], true);

    // Copy AIF to TX
    SiiDrvSwitchInfoFrameGet(isMainPipe, IF_AIF, pIfData, &len);
    SiiTxInfoFrameSet(SI_INFO_FRAME_AUDIO, pIfData);
    SiiTxInfoFrameEnable(SI_INFO_FRAME_AUDIO);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Copy Vsif from Rx pipe to Tx.
//!
//!             This function is used in audio overlay mode, when audio stream gets extracted
//!             from main or sub pipe and inserted into Tx.
//!
//! @param[in]  txInstance - instance of the current TX
//! @param[in]  isMainPipe - source pipe
//-------------------------------------------------------------------------------------------------

void SkAppTxCopyVsif(uint8_t txInstance, bool_t isMainPipe)
{
    uint8_t pIfData[32];
    uint8_t len;

    SkAppTxInstanceSet(txInstance);

    // Copy Vsi to TX
    SiiDrvSwitchInfoFrameGet(isMainPipe, IF_VSI, pIfData, &len);
    SiiTxInfoFrameSet(SI_INFO_FRAME_VSIF, pIfData);
    SiiTxInfoFrameEnable(SI_INFO_FRAME_VSIF);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Adjust TMDS analog settings (termination impedance, etc.) based on pixel clock
//!             of video coming from Rx in bypass mode.
//!
//! @param[in]  txInstance - transmitter instance (zone)
//! @param[in]  pixClock   - video TMDS clock frequency in 10kHz units
//-------------------------------------------------------------------------------------------------

void SkAppTxAdjustAnalogSettings(int_t txInstance, int_t pixClock)
{
    SiiTxInstanceSet(txInstance);

    switch (app.txTmdsFreqRange[txInstance])
    {
        default:
        case 0: // AUTO (default)
            // use provided pixClock value
            break;
        case 1: // 30 MHz < 165 MHz
            pixClock = 7500;
            break;
        case 2: // < 225 MHz
            pixClock = 17000;
            break;
        case 3: // < 300 MHz
            pixClock = 23000;
            break;
    }

    SiiDrvTpiVideoModeDataSet(pixClock, 0,0,0);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Adjust audio clock divider settings based on tmds clock
//!             of video coming from Rx in bypass mode.
//!
//! @param[in]  txInstance - transmitter instance (zone)
//! @param[in]  tmdsClock   - video TMDS clock frequency in 10kHz units
//-------------------------------------------------------------------------------------------------

void SkAppTxAdjustAudioClkDivider(int_t txInstance, int_t tmdsClock)
{
    bool_t toSet;

    toSet = (app.currentAudioType[txInstance] == SI_TX_AUDIO_SRC_SPDIF);

    SiiTxInstanceSet(txInstance);
    //SiiDrvTpiAudioClkDividerSet(tmdsClock, toSet);

}
//-------------------------------------------------------------------------------------------------
//! @brief      Adjust video settings in HDMI bypass mode based on Rx AVI InfoFrame
//!
//! @param[in]  txInstance - transmitter instance (zone)
//! @param[in]  pAvi       - pointer to Rx AVI InfoFrame
//! @param[in]  aviLen     - length of Rx AVI InfoFrame in bytes
//-------------------------------------------------------------------------------------------------

void SkAppTxAdjustVideoSettings(int_t txInstance, uint8_t *pAvi, uint8_t aviLen)
{
    txVideoBusMode_t    busMode;

    if (app.isTxBypassMode[txInstance])
    {
        if (aviLen < 7)
        {
            // Default to RGB
            busMode.colorSpace = SI_TX_VBUS_COLOR_SPACE_RGB;
            busMode.quantRange = SI_TX_VIDEO_QRANGE_DEFAULT;
        }
        else
        {
            // Take from AVI
            busMode.colorSpace = (pAvi[4] & 0x60) >> 5;
            busMode.quantRange = (pAvi[6] & 0x0C) >> 2;
        }

        SiiTxInstanceSet(txInstance);
        SiiTxVideoSourceConfig(SI_TX_VIDEO_SOURCE_HDMI, &busMode);
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Adjust video settings in HDMI bypass mode based on Rx AVI InfoFrame
//!
//! @param[in]  txInstance - transmitter instance (zone)
//! @param[in]  pAvi       - pointer to Rx AVI InfoFrame
//! @param[in]  aviLen     - length of Rx AVI InfoFrame in bytes
//-------------------------------------------------------------------------------------------------

void SkAppTxInfoFrameUpdate(bool_t isMainPipeSrc, bool_t isAvi, bool_t isAud, bool_t isVsi)
{
    int_t instance = 0;
    bool_t isForwardVsif = false;

    if (isAud)
    {
        for (instance = 0 ; instance < SII_NUM_TX; instance++)

        {
            if ((!app.isTxBypassMode[instance]) && (app.currentAudioSource[instance] == (isMainPipeSrc ? SiiSwitchAudioSourceMainPipe : SiiSwitchAudioSourceSubPipe)))
            // Copy AIF from selected Rx source if in audio overlay mode
            {
                SkAppTxInstanceSet(instance);
                // Copy AIF to TX
                SkAppTxCopyAif(instance, isMainPipeSrc);
                SiiTxRecoverAfterSourceClockInterruption();
            }
        }
    }
#if (MHL_20 == DISABLE)
    if(isVsi)
    {
        DEBUG_PRINT( MSG_ALWAYS, "VSIF new Frames ready to pass to TX\n");

    	switch(SiiDrvSwitchSelectedPorttype())
    	{
    	case SiiPortType_HDMI:
    		if (!app.isTxBypassMode[0])
    		{
    			isForwardVsif = true;
    		}
    		break;
    	case SiiPortType_MHL:
            DEBUG_PRINT( MSG_ALWAYS, "VSIF new Frames are from MHL\n");
    		isForwardVsif = true;
    		break;
    	default:
    		break;
    	}
        if ( isForwardVsif )
        {
       	SkAppTxInstanceSet(instance);
        	//copy VSIF to TX
        	SkAppTxCopyVsif(instance, isMainPipeSrc);
           DEBUG_PRINT( MSG_ALWAYS, "VSIF new Frames Copied to Tx\n");
        }
    }
#else
    if(isVsi)
    {
        DEBUG_PRINT( MSG_ALWAYS, "VSIF new Frames ready to pass to TX\n");

    	switch(SiiDrvSwitchSelectedPorttype())
    	{
    	case SiiPortType_HDMI:
    		if (!app.isTxBypassMode[0])
    		{
    			DEBUG_PRINT(MSG_DBG, "Bypass HDMI VSIF!\n");
    			// Disable sending VSIF at Tx
    			if (SiiDrvSwitchMatrixModeEnableGet()){  // Matrix mode, setup for each pipe
    				SkAppTxInstanceSet(isMainPipeSrc? 0 : 1);
    				SiiDrvTpiInfoFrameEnable(false, false, SI_INFO_FRAME_VSIF);
    			}else{  // InstaPrevue mode, setup for both pipe
    				for (instance = 0; instance < SII_NUM_TX; instance++){
    					SkAppTxInstanceSet(instance);
    					SiiDrvTpiInfoFrameEnable(false, false, SI_INFO_FRAME_VSIF);
    				}
    			}
    			// pass through VSIF
    			SiiDrvRxAudioMixInstanceSet(isMainPipeSrc? 0 : 1);
    			SiiDrvRxAudioMixVsifPassThroughConfig(true); // pass through
    			isForwardVsif = true;
    		}
    		break;
    	case SiiPortType_MHL:
            DEBUG_PRINT( MSG_ALWAYS, "VSIF new Frames are from MHL\n");
			uint8_t pIfData[32];
			uint8_t len;

			SiiDrvSwitchInfoFrameGet(isMainPipeSrc, IF_VSI, pIfData, &len);
			if (!CheckMhl3DVsi(pIfData)) return;//if MHL VSIF detects (without converting into HDMI then come down and process to hdmi otherwise return)

			DEBUG_PRINT(MSG_DBG, "MHL 3D VSIF is detected!\n");

			// Convert MHL 3D VSI to HDMI 3D VSI and set InfoFrame to Tx
			if (SiiDrvSwitchMatrixModeEnableGet())
			{  // Matrix mode, setup for each pipe
				SkAppTxInstanceSet(isMainPipeSrc? 0 : 1);
				SkAppTxConvertMhlVSI(pIfData);//in case MHL VSIF detect then we have to convert it into HDMI
				SiiDrvTpiInfoFrameEnable(true, false, SI_INFO_FRAME_VSIF);
			}
			else
			{  // InstaPrevue mode, setup for both pipe
				for (instance = 0; instance < SII_NUM_TX; instance++){
					SkAppTxInstanceSet(instance);
					SkAppTxConvertMhlVSI(pIfData);
					SiiDrvTpiInfoFrameEnable(true, false, SI_INFO_FRAME_VSIF);
				}
			}
			// Block VSIF
			SiiDrvRxAudioMixInstanceSet(isMainPipeSrc? 0 : 1);
			SiiDrvRxAudioMixVsifPassThroughConfig(false); // block
    		isForwardVsif = true;
    		break;
    	default:
    		break;
    	}
//        if ( isForwardVsif )
//        {
//        	SkAppTxInstanceSet(instance);
//        	//copy VSIF to TX
//        	SkAppTxCopyVsif(instance, isMainPipeSrc);
//            DEBUG_PRINT( MSG_ALWAYS, "VSIF new Frames Copied to Tx\n");
//        }
    }
#endif
}

//-------------------------------------------------------------------------------------------------
// Callbacks from TX module (Source: HDCP)
//
// TX Component has repeater links that must be satisfied even if no repeater exists
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater that downstream HDCP authentication
//!             is already done. Only upstream has to be authenticated.
//-------------------------------------------------------------------------------------------------

void SiiTxCbHdcpUsOnlyAuthentication(void)
{
    SkAppRepeaterHdcpUsOnlyAuthentication();
}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater if downstream HDCP is available.
//!
//! @param[in]  isAcknowledged - true:  DS HDCP responded, false: no DDC acknowledge at DS
//-------------------------------------------------------------------------------------------------

void SiiTxCbHdcpDsDdcAck(bool_t isAcknowledged)
{
	SkAppRepeaterHdcpDsDdcAck(isAcknowledged);
}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Send BSTATUS from TX to App/Repeater for processing.
//!
//! @param[in]  dsBstatus - 2-byte array carrying BSTATUS data
//-------------------------------------------------------------------------------------------------

bool_t SiiTxCbHdcpProcessDsBstatus(uint8_t dsBstatus[2])
{
    return SkAppRepeaterHdcpProcessDsBstatus(dsBstatus);
}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Send 20-bit BKSV from TX to App/Repeater in order to
//!             finalize 2nd part of repeater HDCP authentication.
//!
//! @param[in]  aDsBksv - 5-byte array carrying downstream BKSV data
//-------------------------------------------------------------------------------------------------

void SiiTxCbHdcpFinishUsPart2(uint8_t aDsBksv[5])
{
    SkAppRepeaterHdcpFinishUsPart2(aDsBksv);
}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Forward portion of KSV list to App/Repeater in order to
//!             do black listing.
//!
//! @param[in]  pKsvBuffer - pointer to a buffer containing a portion of KSV list
//! @param[in]  len - length of the buffer (number of bytes)
//-------------------------------------------------------------------------------------------------

void SiiTxCbHdcpProcessKsvListPortion(uint8_t *pKsvBuffer, uint8_t len)
{

}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater if BCAPS FIFO is ready.
//!
//! @param[in]  isFifoReady - BCAPS FIFO ready flag.
//-------------------------------------------------------------------------------------------------

void SiiTxCbHdcpSetUsRepeaterBcaps(bool_t isFifoReady)
{
    SkAppRepeaterHdcpSetBcapsFifoRdyBits(isFifoReady);
}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater that TX is about to receive KSV List.
//-------------------------------------------------------------------------------------------------

bool_t SiiTxCbHdcpPrepareForKsvListProcessing(void)
{
    return (SkAppRepeaterHdcpPrepareForKsvListProcessing());
}

//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater that TX is about to receive KSV List.
//-------------------------------------------------------------------------------------------------

bool_t SiiTxCbHdcpPrepareFor0KsvListProcessing(void)
{
    return(SkAppRepeaterHdcpPrepareFor0KsvListProcessing());
}

//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater about HDCP failure.
//!
//! @param[in]  isFatal - true:  fatal HDCP issue, TX won't retry,
//!                       false: minor issue, TX will retry to authenticate
//-------------------------------------------------------------------------------------------------

void SiiTxCbHdcpInformUsDeviceAboutFailure(bool_t isFatal)
{
    SkAppRepeaterHdcpInformUsDeviceAboutFailure(isFatal);
}

//-------------------------------------------------------------------------------------------------
// Callback functions from Repeater module
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//! @brief      Repeater callback function. Enables Tx in repeater mode.
//!             Repeater is expected to set instance for Tx App before calling this function.
//-------------------------------------------------------------------------------------------------

void SiiRepeaterCbTxResume(void)
{
    int_t txInstance = SiiTxInstanceGet();

    if(app.isTxRepeaterMode[txInstance])
    {
        if (app.isTxBypassMode[txInstance])
        {
            SiiTxResume();

            SiiTxOutputInterfaceSet(SI_TX_OUTPUT_MODE_HDMI);
            // Bug 33044 - Melbourne SiI9535/9533 MHL QD882 HDCP CTS 3AB fails at test item 3B-02
            // Bug 33042 - Melbourne SiI9535/9533 HDMI QD882 HDCP CTS 3AB fails at test item 3B-2
            // Fix for HDCP1.4 CTS Test case 3A-05 failue
            if (SiiTxGetLastResult().lastErrorCode == SI_TX_ERROR_CODE_HDMI_NOT_SUPPORTED_BY_DS)
            {
                SiiTxOutputInterfaceSet(SI_TX_OUTPUT_MODE_DVI);
                SiiDrvRxAudioMixPassThroughConfig(false, false);
            }
            else
            {
                SiiDrvRxAudioMixPassThroughConfig(true, true);

				if(app.newAudioSource[txInstance] == SiiSwitchAudioSourceExternal)
				{
					SiiDrvRxAudioMixPassThroughConfig(false, true);
				}
				else
				{
					SiiDrvRxAudioMixPassThroughConfig(true, true);
				}
            }
        }
        else
        {
            EnableTransmitter(txInstance, true);
        }
        DEBUG_PRINT(MSG_DBG, "Tx App%d: Resume from RPTR. Bypass: %d\n", txInstance, app.isTxBypassMode[txInstance]);
        app.isRepeaterResume[txInstance] = true;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Repeater callback function. Disables Tx in repeater mode
//!             Repeater is expected to set instance for Tx App before calling this function.
//-------------------------------------------------------------------------------------------------

void SiiRepeaterCbTxStandby(void)
{
    int_t txInstance = SiiTxInstanceGet();

    if(app.isTxRepeaterMode[txInstance])
    {
        SkAppTxInstanceSet(txInstance);

        if (app.isTxBypassMode[txInstance])
        {
            SiiTxStandby();
        }
        else
        {
            SiiTxStandby();
            //EnableTransmitter(txInstance, false);
        }
        DEBUG_PRINT(MSG_DBG, "Tx App%d: Standby from RPTR. Bypass: %d\n", txInstance, app.isTxBypassMode[txInstance]);
        app.isRepeaterResume[txInstance] = false;
    }
}



//-------------------------------------------------------------------------------------------------
// Callback from EDID TX module
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Distribute generated EDID from TX to all input port EDID SRAMs.
//!
//!             Works in Passthrough mode only. CEC Physical Address isn't inherited from DS
//!             because DS CEC network is separated from the US CEC. The upstream will consider
//!             SiI953x as a CEC root.
//!
//! @param[in]  blockIndex          - zero based EDID block number,
//! @param[in]  pEdidBlock          - pointer to an EDID block,
//! @param[in]  cecPhysAddrLocation - shift (in bytes) of the CEC physical address first byte
//!                                   relative to the EDID Block 1 beginning.
//!
//! @return     success flag.
//-------------------------------------------------------------------------------------------------
bool_t SiiTxEdidCbBlockWrite(uint_t blockIndex, uint8_t* const pEdidBlock, uint8_t cecPhysAddrLocation)
{
    bool_t  isSuccess = true;
    uint8_t rxTxChannel = 0;  // Index of Rx-Tx channel serviced by dedicated instance of EDID Tx component


    if (app.isTxBypassMode[SiiTxInstanceGet()])
    {
        rxTxChannel = SiiEdidTxInstanceGet();
        // This function finds Physical Address location by internal parsing of CEA extension block
        isSuccess = SiiDrvNvramEdidBlockWrite (rxTxChannel, pEdidBlock, blockIndex);

        if (!isSuccess)
        {
            DEBUG_PRINT( MSG_ERR, "EDID Write Error!\n");
        }
		else
		{
			TRACE_DEBUG((0, "[ HDMI MSG ]: EDID Write RX is success "));
		}
    }

    return isSuccess;
}
#if (MHL_20 == ENABLE)
static bool_t CheckMhl3DVsi(uint8_t *pVsi)
{
	if (pVsi[4] != 0x1D || pVsi[5] != 0xA6 || pVsi[6] != 0x7C) return false;  // check OUI
	if ((pVsi[7] & 0x03) != 0x01) return false;  // Check MHL_VID_FMT (3D present)
	return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Convert VSI from MHL to HDMI.
//!
//! @param[in]  txInstance - instance of the current TX
//! @param[in]  isMainPipe - source pipe

//-------------------------------------------------------------------------------------------------

void SkAppTxConvertMhlVSI(uint8_t *pMhlIf)
{
	uint8_t pHdmiIf[32];

	memset (pHdmiIf, 0, 32);

	// Header
	pHdmiIf[0] = 0x81;
	pHdmiIf[1] = 0x01;
	// Content
	pHdmiIf[3] = 0; // checksum will be calculate later
	pHdmiIf[4] = 0x03;
	pHdmiIf[5] = 0x0C;
	pHdmiIf[6] = 0x00;
	pHdmiIf[7] = 0x40;  // 3D format present
	switch (pMhlIf[7]&0x3C){  // MHL_3D_FMT_TYPE
		case (0<<2): // Frame sequential
			pHdmiIf[8] = 0 << 4;  // 3D_structure
			pHdmiIf[2] = 5; // Length
			break;
		case (1<<2):  // Top-Bottom
			pHdmiIf[8] = 6 << 4;  // 3D_structure
			pHdmiIf[2] = 5; // Length
			break;
		case (2<<2): // Left-Right
			pHdmiIf[8] = 8 << 4;  // 3D_structure
			pHdmiIf[9] = 0;  // 3D_Ext_Data
			pHdmiIf[2] = 6; // Length
			break;
	}

	SiiTxInfoFrameSet(SI_INFO_FRAME_VSIF, pHdmiIf);
	SiiTxInfoFrameEnable(SI_INFO_FRAME_VSIF);
}
#endif

//------------------------------------------------------------------------------
// Function:    SkAppTxRepeaterIsResume
// Description: return the tx resume  status
// Parameters:  - inst
// Returns:		- true if connected; false if it is not connected
//------------------------------------------------------------------------------

bool_t SkAppTxRepeaterIsResume( uint8_t inst)
{
   	return app.isRepeaterResume[inst];
}
