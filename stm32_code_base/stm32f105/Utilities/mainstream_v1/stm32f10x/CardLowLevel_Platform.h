#ifndef __CARD_LOW_LEVEL_PLATFORM_H__
#define __CARD_LOW_LEVEL_PLATFORM_H__
#include "Defs.h"
#include "SOUND_BAR_V1_PinDefinition.h"
#include "device_config.h"
#include "I2C1LowLevel.h"
#include "I2C2LowLevel.h"
#include "GPIOMiddleLevel.h"


//The timeout of QDT only can be use with 1,2,4,8... secs without stblib,johnson
#define WATCH_TIMEOUT  (8)

/*-------------------------------------------------------------------------------------------------------*/
void CardLowLevelPlatform_initializeCard(void);

void CardLowLevelPlatform_resetSystem(void);



#endif /*__CARD_LOW_LEVEL_PLATFORM_H__*/
