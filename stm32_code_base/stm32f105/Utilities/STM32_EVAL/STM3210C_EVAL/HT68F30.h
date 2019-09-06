#ifndef __HT68F30_H__
#define __HT68F30_H__

#include "Defs.h"

void HT68F30_LightBar_Set(bool Enable);

void HT68F30_LightBar_USB_DeviceAttached( bool bValue );

void HT68F30_LightBar_USB_StateError( bool bBlinking );

void HT68F30_LightBar_PowerIndicator( bool bValue );

void HT68F30_LightBar_USB_UpgradingDone( void );

void HT68F30_LightBar_EraseInternalFlash( void );

void HT68F30_LightBar_USB_isUpgrading( void );

void HT68F30_LightBar_EraseExtFlash( void );

void HT68F30_LightBar_ProgramExtFlash( void );

#endif /*__HT68F30_H__*/
