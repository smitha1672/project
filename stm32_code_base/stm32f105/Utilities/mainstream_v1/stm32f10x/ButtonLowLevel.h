#ifndef __APP_IF_LOWLEVEL_BUTTON_LOWLEVEL_H__
#define __APP_IF_LOWLEVEL_BUTTON_LOWLEVEL_H__

#include "Defs.h"


/**
 * @defgroup ButtonLowLevel Buttons
 * @ingroup LowLevel
 *
 * This component is in charge of notifying when one of the onboard buttons
 * is pressed.
 *
 */
/*@{*/

#define BTN_POWER        0   ///< Driver internal power button id
#define BTN_INPUT_SRC    1
#define BTN_BT           2
#define BTN_VOL_UP       3
#define BTN_VOL_DOWN     4
#define BTN_MULTI_RET    5
#define BTN_MULTI_DEMO1  6 //Mike, add new combine key
#define BTN_MULTI_DEMO2  7 //Mike, add new combine key
#define BTN_MULTI_DEMO3  8 //Mike, add new combine key
#define BTN_MULTI_VER    9 // Brendan - sys version key

#define BUTTON_PRESSED_HIGH 1
#define BUTTON_PRESSED_LOW 0

typedef struct
{
	unsigned char 	btn_num;
	unsigned long	adc_valueMin;
	unsigned long	adc_valueMax;
} buttonsType;
/**
 * Return TRUE if button is pressed
 */

void ButtonLowLevel_initialIze( void );

bool ButtonLowLevel_isPressed(uint8 buttonID);

uint8 ButtonLowLevel_numberOfButtons();


#endif /* __APP_IF_LOWLEVEL_BUTTON_LOWLEVEL_H__ */
