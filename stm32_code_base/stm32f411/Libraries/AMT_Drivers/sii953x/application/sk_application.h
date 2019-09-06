//***************************************************************************
//!file     sk_application.h
//!brief    SK 9535 Starter Kit firmware main module.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SK9535_APPLICATION_H__
#define __SK9535_APPLICATION_H__

#include "si_common.h"
#include "si_platform.h"
#include "si_device_config.h"
#include "si_osal_timer.h"
#include "si_infrared.h"
#include "sk953x_configure.h"
#include "si_cbus_config.h"
#include "si_drv_switch.h"

//------------------------------------------------------------------------------
//  Include app header files appropriate to this build
//------------------------------------------------------------------------------
//#define POWER_UP_ONLY

#if INC_ARC
#include "sk_app_arc.h"
#endif

#if INC_CDC
#include "sk_app_cdc.h"
#endif

#if INC_CEC
#include "si_cec_component.h"
#include "sk_app_cec.h"
#endif

#if INC_DIAG
#include "sk_app_diag.h"
#endif

#if INC_IPV
#include "sk_app_ipv.h"
#endif

#if INC_MSW
#include "sk_app_msw.h"
#endif

#if INC_OSD
#include "sk_app_osd.h"
#endif

#if INC_TX
#include "si_repeater_component.h"
#include "si_tx_component.h"
#endif

#if INC_IV
#include "si_drv_tpg.h"
#endif
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------


typedef enum _SiiAppMode_t
{
	SK_MODE_TV,
	SK_MODE_DEMO,
	SK_MODE_SETUP,
	SK_MODE_EMU,
	SK_MODE_OSD,
	SK_MODE_BTST,
	
	SK_MODE_COUNT
} SiiAppMode_t;

// The SiiAppPowerStatus_t enums are exactly the same as the CEC
// power states, except for APP_POWERSTATUS_INITIAL, which is
// specific to this application. The other enum values must retain
// the same value as the CEC states with the same names.

typedef enum  _SiiAppPowerStatus_t
{
    APP_POWERSTATUS_ON              = 0x00,
    APP_POWERSTATUS_STANDBY         = 0x01,
    APP_POWERSTATUS_STANDBY_TO_ON   = 0x02,
    APP_POWERSTATUS_ON_TO_STANDBY   = 0x03,
    APP_POWERSTATUS_INITIAL         = 0x04,
} SiiAppPowerStatus_t;

//------------------------------------------------------------------------------
//
//  SK9535 Starter Kit Manifest Constants
//
//------------------------------------------------------------------------------

#define DEM_POLLING_DELAY       300     // Main Loop Polling interval (ms)


typedef enum _SiiErrorLedPatterns_t
{
    ERRLED_NONE             = 0,
    ERRLED_BAD_INIT         = 0x06,
} SiiErrorLedPatterns_t;

typedef enum _SiiUserInterfaceIds_t
{
    UI_AUTO_SWITCH      = SW_1,
    UI_REPEATER_ENABLE  = SW_2,
    UI_RC_ENABLE        = SW_3,
    UI_CEC_ENABLE       = SW_4,
    UI_THX_ENABLE		= SW_5,
    UI_FLASH_UPDATE     = SW_6,
    UI_MHL_ENABLE       = SW_7,
} SiiUserInterfaceIds_t;

//-------------------------------------------------------------------------------
//  Starter Kit EDID Read/write extensions
//-------------------------------------------------------------------------------

typedef enum _SiiEdidType_t
{
    EDID_RAM_0,
    EDID_RAM_1,
    EDID_RAM_2,
    EDID_RAM_3,
    EDID_RAM_4,
    EDID_RAM_5,
//    EDID_RAM_VGA,
    EDID_RAM_BOOT,
    EDID_NVRAM,
    EDID_NVRAM_BOOT,
//    EDID_NVRAM_VGA,
    EDID_TV,
    EDID_VGA,
    EDID_FLASH,
    EDID_EEPROM,
    EDID_EEPROM_BOOT,
//    EDID_FLASH_VGA,
    EDID0_EEPROM,
    EDID1_EEPROM,
    EDID2_EEPROM,
    EDID3_EEPROM,
    EDID4_EEPROM,
//    EDID_EEPROM_VGA,
    EDID_FILE
} SiiEdidType_t;

//------------------------------------------------------------------------------
//  Application Instance Data
//------------------------------------------------------------------------------

typedef struct _AppInstanceData_t
{
    int         structVersion;
    int         lastResultCode;          // Contains the result of the last API function called
    uint16_t    statusFlags;

    SiiAppPowerStatus_t powerState;
    SiiAppPowerStatus_t oldPowerState;
    SiiAppMode_t        modeSelect;
    SiiTimer_t          appTimer;       // Application polling timer

    bool_t      switchInterrupt;        // SWITCH interrupt flag
    bool_t      resChangeIntr[2];       // Video Resolution change interrupt flag [MP, SP]

    // EEPROM settings
    bool_t      irEnable;
	uint8_t	    irUartAddr;
	uint8_t	    irUartKey;
	
    bool_t      cecEnable;              // true == CEC enabled
    bool_t      cecPrint;               // true == CEC //DEBUG_PRINT messages displayed
    bool_t      cecEnum;                // true == CEC enum every 20 seconds
                                        // false == CEC enum on start up only.
    bool_t      dbgMsgColoring;         // true - color DEBUG_PRINT messages according to type
    uint8_t     dbgMsgLevel;
    uint8_t     edidLoad;

    // ARC App-specific
    bool_t              arcDemoEnabled;     // true if selected ARC demo mode is enabled
    uint8_t             arcDemoSelect;      // ARC demo configuration index


    // CEC App-specific
    bool_t              cecInterruptRx;     // RX CEC interrupt flag
    bool_t              cecInterruptTx;     // TX CEC interrupt flag
    bool_t              oldCecEnable;       // Previous state of CEC enable
    clock_time_t        msCecDelay;
    clock_time_t        msCecStart;

    // GPIO specific
    bool_t              gpioInterrupt;      // GPIO interrupt flag

    // TX App-specific

    bool_t              txSendOneTouch;     // true: Start One Touch task in CEC component
    bool_t              isTxRepeaterMode[SII_NUM_TX];// true when Tx is in repeater mode
    bool_t              isTxBypassMode[SII_NUM_TX]; // true when in RX HDMI --> TX HDMI bypass mode

    SiiSwitchSource_t   txSourcePipe[SII_NUM_TX];
    txVideoSource_t     currentVideoSrc[SII_NUM_TX];
    videoFormatId_t     currentVideoFormat[SII_NUM_TX];
    txAudioSrc_t        currentAudioType[SII_NUM_TX];
    bool_t              isTxHdcpRequired[SII_NUM_TX];
    bool_t              isRepeaterResume[SII_NUM_TX]; // true when in RX HDMI --> TX HDMI bypass mode

    int_t               txTmdsFreqRange[SII_NUM_TX];    // 0 - Auto, 1 - < 156 MHz, 2 - < 225 MHz, 3 - < 300 MHz

    video3dFormat_t     v3dMode;
    uint8_t             vSyncMode;
    // IV Specific
#if INC_IV
    bool_t              isExtTpg;
    bool_t              isTPGEnabled;
    tpgVideoPattern_t   tpgVideoPattern;
    tpgVideoFormat_t    tpgVideoFormat;
    tpgClockSource_t    tpgClockSrc;
    bool_t              isOsdBackgroundMode;
#endif

    // IPV specific
#if INC_IPV
    bool_t              ipvAllowed;
    bool_t              ipvHighlight;
    bool_t              ipvEnabled;
    bool_t              ipvPipEnabled;
    clock_time_t        ipvTimeLastMs;
    SiiIpvMode_t        ipvMode;
    SiiIpvTheme_t       ipvTheme;
    int_t               ipvHighlightWindow;
#endif
    SiiSwitchSource_t   pipSource;
#if (SII_NUM_SWITCH > 1)
    SiiSwitchSource_t   pipSaveOrigSource;
#endif
    // EDID TX specific
    bool_t              isEdidMhlLimits;
    uint8_t             numOfEdidTxInst;

    // TV App-specific
    uint8_t                 rotarySwitch;
    bool_t                  rotaryDisabled;
    uint8_t                 lastRotarySwitch;
    bool_t                  zoneEnabled[2];     // Zone (TX) current enable status
    bool_t                  newZoneEnabled[2];  // Zone (TX) new enable status
    SiiSwitchSource_t       newSource[2];       // One for each zone (Video)
    SiiSwitchSource_t       currentSource[2];   // One for each zone (Video)
    SiiSwitchSource_t       audioZoneSource[2]; // Audio Zone (Extraction) source (current)
    SiiSwitchAudioSource_t  newAudioSource[2];      // One for each zone (Audio)
    SiiSwitchAudioSource_t  currentAudioSource[2];  // One for each zone (Audio)
    audioSampleRate_t       audioSampleRate[2]; // audio sample rate for each zone
    bool_t                  isAudioExtractionEnabled[2]; // One for each pipe
    int_t                   currentZone;
    bool_t                  autoSwitch;
    SiiTimer_t              autoSwitchTimer;    // Auto Switch delay timer
    bool_t                  isOsdMenuEnabled;
    bool_t                  isSubPipeAudioExtSpdif; // I2S(false) / SPDIF(true) selection for sub pipe audio extraction (pin shared)

    // Setup App-specific

    uint8_t             setupMenuLevel;
    uint16_t            markerOptionFlagsMp;
    bool_t              marker3dGpioEnableMp;
    uint16_t            markerOptionFlagsRp;
    bool_t              marker3dGpioEnableRp;
    uint16_t            markerDelay720p;
    uint16_t            markerDelay1080p;

    // Demo App-specific

    uint8_t             demoMenuLevel;

    // Repeater App-specific
    bool_t              repeaterInterrupt;      // Repeater interrupt flag
    bool_t              repeaterEnable;         // true == Repeater enabled
    bool_t              oldRepeaterEnable;      // Previous state of Repeater enable
    bool_t              wasRepEnabledBySwitch;  // Previous state of repeater enable by dip switch
    clock_time_t        msRepeaterDelay;
    clock_time_t        msRepeaterStart;

    // Firmware update
    bool_t              fwUpdEnabled;           // set when fw update mode requested

    // THX use
    bool_t				isThxDemo;
    clock_time_t        msThxDelay;
    clock_time_t        msThxStart;

    bool_t              isAudioByPass;
    bool_t              isCecAsSwitch;
	bool_t				isresolutionVaild;
    bool_t              isTxDisconnectedImmed; // true when in RX HDMI --> TX HDMI bypass mode
    uint8_t				currentaudExtSource;
    uint8_t				newaudExtSource;
    uint8_t				muteON;
    bool_t 				initialPowerOn;
    bool_t 				isUserInfoEnabled;
    clock_time_t        cbusTimeLastMs;
    bool_t				mhlcabledetect;
}   AppInstanceData_t;

extern AppInstanceData_t app;
extern AppInstanceData_t *pApp;

typedef struct
{
    uint8_t rcpKeyCode;         // RCP CBUS Key Code
    uint8_t rc5KeyCode;         // RC5 Key Code
    char   rcpName[30];
} SiiCea931Translate_t;

//-------------------------------------------------------------------------------
//  Function Prototypes
//-------------------------------------------------------------------------------

void    SkAppInitInstanceData( void );
bool_t  SkAppInitialize( void );
bool_t  SkAppPreInitialize( void );
void    SkAppSignon( void );
bool_t  SkAppEepromInitialize( bool_t useDefaults, bool_t forceInit );
void    SkAppEepromUpdateControl( uint8_t mask, uint8_t value );
void    SkAppUiUpdate( void );
void    SkAppBlinkTilReset( uint8_t errorLedPattern );
void    SkAppInstaPortLedUpdate ( uint8_t portBits );
void    SkAppDisplayData( uint8_t msgLevel, uint8_t *pData, uint16_t length );
void 	SkAppCmpData ( uint8_t msgLevel, uint8_t *pData, uint8_t type);

bool_t  SkAppDeviceInitialize( void );
void    SkAppDevicePower ( void );
void    SkAppTaskCbusCdsenseMonitoring( void );
void    SkAppTaskCbusStandByMonitoring( void );
void    SkAppLowPowerStandby( void );

//-------------------------------------------------------------------------------------------------
//! @brief      DEBUG_PRINT( MSG_ALWAYS ) helpers
//-------------------------------------------------------------------------------------------------
extern char tenDashes [];
extern char asterisks [];
extern char demoMenuExit [];
extern char setupSelectMenu [];
extern char ccDisplayThisMenuMsg [];
extern char returnToSetupMenuMsg [];
extern char returnToMainDemoMsg [];

void    PrintMenu( char *pMenu[] );
void    PrintStringCheckMark( char *pString, bool_t isChecked );
void    PrintStringOnOff( char *pString, bool_t isOn );
void    PrintStringYesNo( char *pString, bool_t isYes );
void    PrintAlways( char *pStr );

//-------------------------------------------------------------------------------------------------
//! @brief      Monitors SiI9535 device interrupts and notifies components that have received
//!             an interrupt.
//-------------------------------------------------------------------------------------------------
void SkAppTaskSiiDevice( void );

    // sk_app_edid.c

void    SkAppInitEdid ( void );
void    SkAppTaskEdid ( void );
bool_t  SkAppEdidRead ( uint8_t source, uint8_t *pDest );
bool_t  SkAppEdidWrite ( uint8_t target, const uint8_t *pSource );

    // sk_app_switch.c

bool_t  SkAppDeviceInitMsw( void );
void    SkAppTaskSwitch( void );
uint8_t SkAppSwitchGetNextPort( uint8_t currentPort, bool_t isForward);
void    SkAppSwitchPortUpdate( void );
void    SkAppServiceNonHdmiVideo(bool_t forceResChange );

	// sk_app_repeater.c
bool_t  SkAppDeviceInitRepeater( bool_t isPowerUp );
void    SkAppTaskRepeater( void );
bool_t  SkAppRepeaterConfigure( bool_t enable  );
//void    SkAppRepeaterSourceConfig( SiiSwitchSource_t sourceSel[], SiiMswInstanceCommonData_t PipeConf );
void SkAppRepeaterSourceConfig( SiiSwitchSource_t sourceSel[],
                                SiiMswInstanceCommonData_t PipeConf,
                                bool_t txHdcpRestart[]);

void    SkAppRepeaterHdcpUsOnlyAuthentication(void);
bool_t  SkAppRepeaterHdcpProcessDsBstatus(uint8_t dsBstatus[2]);
void    SkAppRepeaterHdcpFinishUsPart2(uint8_t aDsBksv[5]);
void    SkAppRepeaterHdcpSetBcapsFifoRdyBits(bool_t switchOn);
bool_t  SkAppRepeaterHdcpPrepareForKsvListProcessing(void);
bool_t  SkAppRepeaterHdcpPrepareFor0KsvListProcessing(void);
void    SkAppRepeaterHdcpInformUsDeviceAboutFailure(bool_t isFatal);
void    SkAppRepeaterHdcpDsDdcAck(bool_t isAcknowledged);
bool_t SkAppTxRepeaterIsResume( uint8_t inst);

    // sk_app_tx.c

void    SkAppTxInstanceSet(uint8_t instance);
bool_t  SkAppDeviceInitTx( void );
void    SkAppTaskTx( void );
void 	SkAppTxHpdConnection( rptHpdTxEvent_t qOn);
void	SkAppTxHdcpAuthDone( void );
void    SkAppProcessTxEdid(uint8_t txInstance);
void    SkAppDismissTxEdid(uint8_t txInstance);
#if (MHL_20 == ENABLE)
void    SkAppTxConvertMhlVSI(uint8_t *pMhlIf);
#endif
void    SkAppTxCopyAif(uint8_t txInstance, bool_t isMainPipe);
void    SkAppTxCopyVsif(uint8_t txInstance, bool_t isMainPipe);
void    SkAppTxAdjustAnalogSettings(int_t txInstance, int_t pixClock);
void    SkAppTxAdjustVideoSettings(int_t txInstance, uint8_t *pAvi, uint8_t aviLen);
void    SkAppTxInfoFrameUpdate(bool_t isMainPipeSrc, bool_t isAvi, bool_t isAud, bool_t isVsi);
void    SkAppTxAdjustAudioClkDivider(int_t txInstance, int_t tmdsClock);


#if INC_IV
void    SkAppTpgEnable ( bool_t isEnabled );
void    SkAppInternalVideoConfig (uint_t txInstance, bool_t isTxEnabled );
#endif

void    SkAppExtAudioInsertConfig(int_t txInstance, bool_t isEnabled);

bool_t SkAppDeviceInitEdidTx ( bool_t isAudioByPass);

// sk_app_infrared.c
uint8_t SkAppIrConvertToCea931( uint8_t keyData );
void    SkAppInitInfrared( void );
void    SkAppTaskIrRemote( bool_t fullAccess );
uint8_t SkAppIrGetSingleDigit( uint8_t rangeEnd );
void    SkAppIrRemoteReturnToTV( void );

// sk_app_tv.c
void 	SkAppTv( void );
void    SkAppTvIrRemote( RC5Commands_t key, bool_t fullAccess );
void 	SkAppParseKey ( uint16_t irKey, bool_t fullAccess  );
void SkAppTv_AMT_RxPortSelect( uint8_t input_path );


// sk_app_demo.c
void 	SkAppDemo( void );
bool_t  SkAppDemoIrRemote( RC5Commands_t key, bool_t fullAccess );
#if INC_IV
void    DemoTpgModeEnable( bool_t isEnabled );
#endif

    // sk_app_setup.c

void   SkAppSetup( void );
bool_t SkAppSetupIrRemote( RC5Commands_t key, bool_t fullAccess );

// si_app_gpio.c

bool_t SkAppDeviceInitGpio( void );
void SkAppTaskGpio( uint8_t instance );
void SkAppTaskCbusGpio( uint8_t instance );
    // sk_app_emu_main.c
bool_t SkAppEmulationIrRemote(RC5Commands_t key, bool_t fullAccess);
void  SkEmuMatrixSwitch();
void  SkEmuEdid();
void  SkEmuAudio();
void  SkEmuOsd();
void  SkEmuArc();

bool_t  SkAppDeviceInitAudio ( void );
void    SkAppTaskAudio( void );
#if ( configSII_DEV_953x_PORTING == 1 )
bool AmTAudiogetMultiChannel (void);
#endif

#if INC_CEC_SAC
bool_t  SkAppAudioProcessSacCommand( uint8_t cmdData );
#endif

void SkEmuSpi();

    // sk_app_btst.c
#if INC_BENCH_TEST
void SkAppBenchTest ( void );
void SkAppBenchTestIrRemote( RC5Commands_t key, bool_t fullAccess );
#endif

//------------------------------------------------------------------------------
// Function:    SkAppDeviceInitThx
// Description: initialization of THX app, populate pkt headers etc.
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

bool_t SkAppDeviceInitThx ( void );

//------------------------------------------------------------------------------
// Function:    SkAppTaskThx
// Description: Wrapper for the Thx Component at the application level
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTaskThx ( void );


#endif // __SK9535_APPLICATION_H__
