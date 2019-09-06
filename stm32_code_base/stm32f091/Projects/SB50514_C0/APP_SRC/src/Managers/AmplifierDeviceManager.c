#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "api_typedef.h"
#include "GPIOMiddleLevel.h"
#include "device_config.h"

#if ( configDAP_TAS5711 == 1 )    
#include "TAS5711.h"
#endif 

#if ( configDAP_TAS5727 == 1 ) 
#include "TAS5727.h"
#endif 

#if ( configDAP_TAS5707 == 1 ) 
#include "TAS5707.h"
#endif 

#include "AmplifierDeviceManager.h"

//________________________________________________________________________________________________
static void AmpDeviceManager_RearMuteControl( bool value );

static void AmpDeviceManager_FrontMuteCtrl( bool val );

static void AmpDeviceManager_Initialize( void );
//________________________________________________________________________________________________

extern DAP_C_CTRL_OBJECT *pDAP_C_ObjCtrl;
extern DAP_LR_CTRL_OBJECT *pDAP_LR_ObjCtrl;

const AMP_DEV_MANAGER_OBJECT AMP_DEV_ObjCtrl = 
{
    AmpDeviceManager_Initialize,
    AmpDeviceManager_FrontMuteCtrl,
    AmpDeviceManager_RearMuteControl
};

const AMP_DEV_MANAGER_OBJECT *pAMP_DEV_ObjCtrl = &AMP_DEV_ObjCtrl;

static void AmpDeviceManager_Initialize( void )
{
    pDAP_C_ObjCtrl->initialize();
    pDAP_LR_ObjCtrl->initialize();
}

static void AmpDeviceManager_FrontMuteCtrl( bool val )
{
    pDAP_LR_ObjCtrl->mute_ctrl( val );
    pDAP_C_ObjCtrl->mute_ctrl( val );
}

static void AmpDeviceManager_RearMuteControl( bool value )
{
    if ( value == SOUND_MUTE )
    {
         GPIOMiddleLevel_Clr( __O_MUTE_REAR );   
    }
    else if ( value == SOUND_DEMUTE )
    {
         GPIOMiddleLevel_Set( __O_MUTE_REAR );   
    }
}

