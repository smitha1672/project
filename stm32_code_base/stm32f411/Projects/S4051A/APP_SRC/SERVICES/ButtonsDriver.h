#ifndef __APP_IF_BUTTON_DRIVER_H__
#define __APP_IF_BUTTON_DRIVER_H__

#include "Defs.h"

/**
 * @defgroup ButtonLowLevel Buttons
 * @ingroup LowLevel
 *
 * This component is in charge of notifying when one of the onboard buttons
 * is pressed.
 */
/*@{*/

/** */
#define BUTTON_NONE_VALID_TIME     0xFFFFFFFE






/**
 * Initializes the Button Low Level.
 */
void ButtonsDriver_initialize(void);

/**
 * Returns whether or not the a given button had been pressed since the last
 * time this function was called.
 *
 * @param button        The button to check if it has been pressed [0 based].
 *
 * @return  The time in msec that the button was last pressed. In case that the
 *          button is currently pressed, return BUTTON_STATE_IS_PRESSED
 */
uint32 ButtonsDriver_hasButtonBeenPressed(uint8 button);

/**
 * Returns whether or not the a given button is currently pressed.
 *
 * @param button        The button to check if it has been pressed [0 based].
 *
 * @return Whether or not the given button is currently pressed.
 */
bool ButtonsDriver_isButtonPressed(uint8 button);

uint32 ButtonLowLevel_hasButtonBeenPressedContinue(uint8 button) ;

/*@}*/


#endif /* __APP_IF_BUTTON_DRIVER_H__ */

