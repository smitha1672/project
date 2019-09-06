/*---------------------------------------------------------------------------*/
/* Copyright (c) 2004-2007 by Amimon LTD.                                    */
/*                                                                           */
/*   This software is the property SOLELY of AMIMON LTD.                     */
/*   All rights, title, ownership AND other interests in the software remain */
/*   the property of Amimon LTD.  This software may only be used in          */
/*   accordance with A WRITTEN license agreement BETWEEN THE USER AND        */
/*   AMIMON LTD.                                                             */
/*   Any duplication, transmission, distribution or disclosure of this       */
/*   software is expressly forbidden UNLESS IN STRICT ACCORDANCE WITH SUCH   */
/*   LICENSE AGREEMENT. THIS SOFTWARE IS PROTECTED BY US AND INTERNATIONAL   */
/*   COPYRIGHTS LAWS.                                                        */
/*---------------------------------------------------------------------------*/

#include "ButtonsDriver.h"
#include "ButtonLowLevel.h"
#include "VirtualTimer.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "Debug.h"

/*----------------------- Private Member Definitions ------------------------ */

/**
 * @defgroup ButtonDriver_Private Buttons - Private
 * @ingroup ButtonDriver
 *
 * These are the private members of @ref ButtonDriver.
 */
/*@{*/

/** */
#define BUTTON_INFINITE_TIME       0xFFFFFFFF 

/** */
#define MAX_NUM_BUTTONS            0x10


/** A button's container (class) */
typedef struct
{
    /** The last state of the button (pressed/unpressed) */
    bool   lastState;

    /** The time the button was pressed or unpressed. */
    uint32 timeDelta;
    
    /* The last time any action was taken on the button - press or release. */
    uint32 lastTime;

} button;

/**
 * A button state array holding whether or not a certain button had been pressed 
 */
static button m_buttons[MAX_NUM_BUTTONS];



//__________________________________________________________________________

//__________________________________________________________________________
/**
 * Interrupt handler called whenever a button is pressed.
 */
static void ButtonsDriver_poll(void *data);

/*@}*/

/*----------------------------- Implementations ----------------------------- */

static void ButtonsDriver_handleState(uint8 button)
{
    uint32 time;
    bool buttonState;

    /* Keep the current button state */
    buttonState = ButtonsDriver_isButtonPressed(button);

    /*
        * Checks whether there is a change from last time. If so, check the time
        * delta from last action to NOW.
        */
    if ( buttonState != m_buttons[button].lastState )
    {
        time = VirtualTimer_now();

        /* In case that last time is not updated it is not possible to calc  the delta */
        if (m_buttons[button].lastTime != BUTTON_NONE_VALID_TIME)
        {
            /* Calc the time that the button was in last state - pressed or released */
            m_buttons[button].timeDelta = time - m_buttons[button].lastTime;
        }
        /* Update the time */
        m_buttons[button].lastTime = time;

        /* Update button's state */
        m_buttons[button].lastState = buttonState;
    }
}

void ButtonsDriver_poll(void * data) 
{
 	for(;;)
	{
		uint8 i;
		uint8 size;

	    size = ButtonLowLevel_numberOfButtons();
	    
	    for ( i = 0 ; i < size; i++ ) 
	    {
	        ButtonsDriver_handleState(i);
	    }
		vTaskDelay( TASK_MSEC2TICKS(1));
	}
}

void ButtonsDriver_initialize(void) 
{
    uint8 i;
    uint8 size;

	ButtonLowLevel_initialIze();

    size = ButtonLowLevel_numberOfButtons();
    
    for (i = 0 ; i < size ; i++) 
    {
        /* The last button state is the real current button state */
        m_buttons[i].lastState = ButtonsDriver_isButtonPressed(i);
        m_buttons[i].lastTime = BUTTON_NONE_VALID_TIME;
        m_buttons[i].timeDelta = BUTTON_NONE_VALID_TIME;
    }


#if defined ( VIRTUAL_TIMER )
    /* Check the button state every 50msec */
    VirtualTimer_register(
        VIRTUAL_TIMER_MSEC2TICKS(50),
        ButtonsDriver_poll,
        NULL,
        VIRTUALTIMER_PERIODIC|VIRTUALTIMER_ACTIVE
        );
#endif 

#if defined ( FREE_RTOS )
	if (xTaskCreate( ButtonsDriver_poll, ( portCHAR * ) "ButtonsDriver_poll", configMINIMAL_STACK_SIZE, NULL, tskHMI_EVENT_PRIORITY, NULL ) != pdPASS )
	{
		TRACE_ERROR((0, " ButtonsDriver_poll task create error !! "));
	}
#endif 

}

/*--------------------------------------------------------------------------- */

bool ButtonsDriver_isButtonPressed(uint8 button) 
{
    bool retVal = FALSE;

    retVal = ButtonLowLevel_isPressed(button);

    return retVal;
}

/*--------------------------------------------------------------------------- */

uint32 ButtonsDriver_hasButtonBeenPressed(uint8 button) 
{
    uint32 res = BUTTON_NONE_VALID_TIME;

    if ( ButtonsDriver_isButtonPressed(button) == FALSE ) /*buttons has been released !!*/
    {
        /* Get the status of the button */
        res = m_buttons[button].timeDelta;

        /* Reset the delta time */
        m_buttons[button].timeDelta = BUTTON_NONE_VALID_TIME;
    }
   
    /* Return the time the button was pressed */
    return res;
}

uint32 ButtonLowLevel_hasButtonBeenPressedContinue(uint8 button) 
{
    uint32 res = BUTTON_NONE_VALID_TIME;

    if (ButtonsDriver_isButtonPressed(button) == TRUE)
    {
        if (m_buttons[button].lastTime != BUTTON_NONE_VALID_TIME)
	        res = VirtualTimer_now()- m_buttons[button].lastTime;
    }
    /* Return the time the button was pressed */
    return res;
}

