#include "ButtonCmdDispatcher.h"
#include "ButtonsDriver.h"
#include "VirtualTimer.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "device_config.h"

#include "command.h"
#include "Debug.h"

#include "HMI_Service.h" 

/** Number of buttons on the board */
#define NUMBER_OF_BUTTONS       12

/** Up to 3 assignments per button for continue,short and long press */
#define NUMBER_OF_ASSIGNMENTS   5

/** Defines the table index for short press assignments */
#define SHORT_INDEX    0

/** Defines the table index for long press assignments */
#define LONG_INDEX     1

#define LONG_INDEX_4s     2

#define LONG_INDEX_3s     3

/** Defines the table index for continue press assignments */
#define CONTINUE_INDEX     4

/* Any button press shorter than 10 sec will be taken */
#define BUTTON_PRESS_LONG       VIRTUAL_TIMER_MSEC2TICKS(10000)

#define BUTTON_PRESS_LONG_4s       VIRTUAL_TIMER_MSEC2TICKS(4000)

#define BUTTON_PRESS_LONG_3s       VIRTUAL_TIMER_MSEC2TICKS(2500)

/* Any button press shorter than 500 msec will be taken */
#define BUTTON_PRESS_SHORT      VIRTUAL_TIMER_MSEC2TICKS(600)

/* Any button press more than 1 sec will be taken */
#define BUTTON_PRESS_CONTINUE        VIRTUAL_TIMER_MSEC2TICKS(1000)

/* Any button press more than 100 sec will be taken */
#define BUTTON_PRESS_REPEAT      VIRTUAL_TIMER_MSEC2TICKS(100)

/** Minimal interval time to wait between button presses */
#define MIN_TIME_BETWEEN_PRESSES VIRTUAL_TIMER_MSEC2TICKS(100)


#define BTN_DURATION_TOLERANCE(X) ((X/100)*5)
#define BTN_DURATION_MIN(X) (X - BTN_DURATION_TOLERANCE(X))
#define BTN_DURATION_MAX(X) (X + BTN_DURATION_TOLERANCE(X))


/**
 * List of commands per button. The dimensions of this container are the
 * # of buttons X # of assigment per button.
 * For example, we have 3 buttons and each button can have to assignments, one
 * for SHORT press and one for LONG press.
 */
static struct
{
    /* The command to execute */
    xHMIUserEvents event;

} m_commandAssignment[NUMBER_OF_BUTTONS][NUMBER_OF_ASSIGNMENTS];

static bool m_continue_set[NUMBER_OF_BUTTONS];

static xHMISrvEventParams srv_parms = { xHMI_EVENT_BUTTON, USER_EVENT_UNDEFINED };

//_____________________________________________________________________________
extern HMI_SERVICE_OBJECT *pHS_ObjCtrl;

//_____________________________________________________________________________

static void ButtonCmdDispatcher_registerCommand(
    byte button,
    uint32 time,
    xHMIUserEvents event
);

static void ButtonCmdDispatcher_registerUserCommand( void );

static void ButtonCmdDispatcher_registerDemo_1n3_Command( void );
static void ButtonCmdDispatcher_registerDemo_2_Command( void );

static bool isPowerAndVolButtonPressed( byte PressedKey, xHMISrvEventParams *psrv_parms);

const BTN_CONF_OBJECT BTN_ConfObj =
{
    ButtonCmdDispatcher_registerUserCommand,
    ButtonCmdDispatcher_registerDemo_1n3_Command,
    ButtonCmdDispatcher_registerDemo_2_Command
};
const BTN_CONF_OBJECT *pBTN_ObjCtrl = &BTN_ConfObj;


void ButtonCmdDispatcher_switchControl( bool val )
{
	if ( val == TRUE ) /*enable*/
	{
		srv_parms.event_id = xHMI_EVENT_BUTTON;
	}
	else
	{
		srv_parms.event_id = xHMI_EVENT_NULL;
	}
}

uint8 ButtonCmdDispatcher_getSwitchStatus( void )
{
	if ( srv_parms.event_id == xHMI_EVENT_BUTTON )
		return 1;
	
	return 0;
}

/*----------------------------- Implementations ----------------------------- */
void ButtonCmdDispatcher_poll( void *data )
{
    byte i;
    uint32 time;

	for( ;; )
	{
	    /* Iterate on all buttons */
	    for ( i = 0 ; i < NUMBER_OF_BUTTONS; i++ ) /*keypad scan*/
	    {
			time = ButtonLowLevel_hasButtonBeenPressedContinue(i);/*Get btn is pressed delta time*/
			if ( time != BUTTON_NONE_VALID_TIME )
			{			
				if ( ( time > BUTTON_PRESS_LONG ) && ( m_continue_set[i] == FALSE ) )
				{
					if (m_commandAssignment[i][LONG_INDEX].event != USER_EVENT_UNDEFINED)
					{
						srv_parms.event = m_commandAssignment[i][LONG_INDEX].event;
						pHS_ObjCtrl->SendEvent( &srv_parms );
						m_continue_set[i] = TRUE;
					}
				}
				else if ( ( time > BUTTON_PRESS_LONG_4s ) && ( m_continue_set[i] == FALSE ) )
				{
					if ( isPowerAndVolButtonPressed(i, &srv_parms) )
					{
						pHS_ObjCtrl->SendEvent( &srv_parms );
						m_continue_set[i] = TRUE;
					}
					else if ( (m_commandAssignment[i][LONG_INDEX].event == USER_EVENT_UNDEFINED) &&
						(m_commandAssignment[i][LONG_INDEX_4s].event != USER_EVENT_UNDEFINED) )
					{
						srv_parms.event = m_commandAssignment[i][LONG_INDEX_4s].event;
						pHS_ObjCtrl->SendEvent( &srv_parms );
						m_continue_set[i] = TRUE;
					}
				}
				else if ( ( time > BUTTON_PRESS_LONG_3s ) && ( m_continue_set[i] == FALSE ) )
				{
					if ( (m_commandAssignment[i][LONG_INDEX].event == USER_EVENT_UNDEFINED) &&
						(m_commandAssignment[i][LONG_INDEX_4s].event == USER_EVENT_UNDEFINED) &&
						   (m_commandAssignment[i][LONG_INDEX_3s].event != USER_EVENT_UNDEFINED) )
					{
						srv_parms.event = m_commandAssignment[i][LONG_INDEX_3s].event;
						pHS_ObjCtrl->SendEvent( &srv_parms );
						m_continue_set[i] = TRUE;
					}
				}
				
				if ( time > BUTTON_PRESS_CONTINUE )
				{
					if (m_commandAssignment[i][CONTINUE_INDEX].event != USER_EVENT_UNDEFINED)
					{
						if ( ButtonLowLevel_hasButtonBeenPressedContinue(0) == BUTTON_NONE_VALID_TIME )
						{
						    srv_parms.event = m_commandAssignment[i][CONTINUE_INDEX].event;
						    pHS_ObjCtrl->SendEvent( &srv_parms );
						    m_continue_set[i] = TRUE;
						}
					}
				}
				
			}
			else
			{
				m_continue_set[i] = FALSE;			
			}

	        time = ButtonsDriver_hasButtonBeenPressed(i); /*Get btn is released delta time*/
	        
	        if ( time != BUTTON_NONE_VALID_TIME )
	        {
	            if ( time <= BUTTON_PRESS_SHORT )
	            {
	                /* In case of SHORT button press */
	                /* Check whether there is assigned command for this case */
            
                        srv_parms.event = m_commandAssignment[i][SHORT_INDEX].event;;
                        pHS_ObjCtrl->SendEvent( &srv_parms );
	            }
		    /*else if ( ( time >= BUTTON_PRESS_LONG ) && ( m_continue_set[i] == FALSE ) )
		    {
			if (m_commandAssignment[i][LONG_INDEX].event != USER_EVENT_UNDEFINED)
			{
				srv_parms.event = m_commandAssignment[i][LONG_INDEX].event;
				pHS_ObjCtrl->SendEvent( &srv_parms );
				m_continue_set[i] = TRUE;
			}
		    }
		    else if ( ( time >= BUTTON_PRESS_LONG_4s ) && ( m_continue_set[i] == FALSE ) )
		    {
			if ( isPowerAndVolButtonPressed(i, &srv_parms) )
			{
				pHS_ObjCtrl->SendEvent( &srv_parms );
				m_continue_set[i] = TRUE;
			}
			else if ( (m_commandAssignment[i][LONG_INDEX].event == USER_EVENT_UNDEFINED) &&
				(m_commandAssignment[i][LONG_INDEX_4s].event != USER_EVENT_UNDEFINED) )
			{
				srv_parms.event = m_commandAssignment[i][LONG_INDEX_4s].event;
				pHS_ObjCtrl->SendEvent( &srv_parms );
				m_continue_set[i] = TRUE;
			}
		    }
		    else if ( ( time >= BUTTON_PRESS_LONG_3s ) && ( m_continue_set[i] == FALSE ) )
		    {
			if ( (m_commandAssignment[i][LONG_INDEX].event == USER_EVENT_UNDEFINED) &&
				(m_commandAssignment[i][LONG_INDEX_4s].event == USER_EVENT_UNDEFINED) &&
				   (m_commandAssignment[i][LONG_INDEX_3s].event != USER_EVENT_UNDEFINED) )
			{
				srv_parms.event = m_commandAssignment[i][LONG_INDEX_3s].event;
				pHS_ObjCtrl->SendEvent( &srv_parms );
				m_continue_set[i] = TRUE;
			}
		    }*/
	        }

	    }
		vTaskDelay( TASK_MSEC2TICKS(100) );
	}
}

void ButtonCmdDispatcher_initialize( void )
{
#if ( configAPP_BTN_CTRL == 1 )

    uint8 i;
    uint8 j;

    /* Reset all command assignment */
    for (j = 0 ; j < NUMBER_OF_BUTTONS ; j++)
    {
        for (i = 0 ; i < NUMBER_OF_ASSIGNMENTS ; i++)
        {
            m_commandAssignment[j][i].event = USER_EVENT_UNDEFINED;
        }

        m_continue_set[j] = FALSE;
    }

    ButtonsDriver_initialize( );
    ButtonCmdDispatcher_registerUserCommand( );
    
	if ( xTaskCreate( ButtonCmdDispatcher_poll, ( portCHAR * ) "ButtonCmdDispatcher_poll", configMINIMAL_STACK_SIZE, NULL, tskHMI_EVENT_PRIORITY, NULL ) != pdPASS)
	{
		TRACE_ERROR((0, " ButtonCmdDispatcher_poll task create error !! "));
	}
#endif 

}

static void ButtonCmdDispatcher_registerCommand(
    byte button,
    uint32 time,
    xHMIUserEvents event
)
{
    /* Assign button command */
    if (time == BUTTON_PRESS_LONG)
    {
        m_commandAssignment[button][LONG_INDEX].event = event;
    }
    else if (time == BUTTON_PRESS_LONG_4s)
    {
        m_commandAssignment[button][LONG_INDEX_4s].event = event;
    }
    else if (time == BUTTON_PRESS_LONG_3s)
    {
        m_commandAssignment[button][LONG_INDEX_3s].event = event;
    }
    else if (time == BUTTON_PRESS_SHORT)
    {
        m_commandAssignment[button][SHORT_INDEX].event = event;
    }
    else if (time == BUTTON_PRESS_CONTINUE)
    {
        m_commandAssignment[button][CONTINUE_INDEX].event = event;
    }
}

static void ButtonCmdDispatcher_registerUserCommand( void )
{
	ButtonCmdDispatcher_registerCommand(
    0,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_POWER
    );

    ButtonCmdDispatcher_registerCommand(
    0,
    BUTTON_PRESS_LONG_4s, 
    USER_EVENT_RF_PAIR
    );

    ButtonCmdDispatcher_registerCommand(
    1,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_SEL_SRC
    );

    ButtonCmdDispatcher_registerCommand(
    1,
    BUTTON_PRESS_LONG_4s, 
    USER_EVENT_START_AUTO_SEL_SRC
    );

    ButtonCmdDispatcher_registerCommand(
    2,
    BUTTON_PRESS_LONG_3s, 
    USER_EVENT_BT_PAIR
    );
	
    ButtonCmdDispatcher_registerCommand(
    2,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_SEL_BT
    );
	
    ButtonCmdDispatcher_registerCommand(
    3,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_VOL_UP
    );

    ButtonCmdDispatcher_registerCommand(
    4,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_VOL_DN
    );

    ButtonCmdDispatcher_registerCommand(
    3,
    BUTTON_PRESS_CONTINUE, 
    USER_EVENT_VOL_UP
    );

    ButtonCmdDispatcher_registerCommand(
    4,
    BUTTON_PRESS_CONTINUE, 
    USER_EVENT_VOL_DN
    );

    ButtonCmdDispatcher_registerCommand(
    5,
    BUTTON_PRESS_LONG_4s, 
    USER_EVENT_DEVICE_RESET
    );

    ButtonCmdDispatcher_registerCommand(
    6,
    BUTTON_PRESS_LONG, 
    USER_EVENT_MODE_DEMO1
    );

    ButtonCmdDispatcher_registerCommand(
    7,
    BUTTON_PRESS_LONG, 
    USER_EVENT_MODE_DEMO2
    );

    ButtonCmdDispatcher_registerCommand(
    8,
    BUTTON_PRESS_LONG, 
    USER_EVENT_MODE_DEMO3
    );

    ButtonCmdDispatcher_registerCommand( 
    9,
    BUTTON_PRESS_LONG, 
    USER_EVENT_VERSION	
    );
}

static void ButtonCmdDispatcher_registerDemo_1n3_Command( void )
{
	ButtonCmdDispatcher_registerCommand(
    0,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_NULL
    );

    ButtonCmdDispatcher_registerCommand(
    0,
    BUTTON_PRESS_LONG_4s, 
    USER_EVENT_NULL
    );

    ButtonCmdDispatcher_registerCommand(
    1,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_NULL
    );

    ButtonCmdDispatcher_registerCommand(   //For debug BT
    2,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_NULL
    );

    ButtonCmdDispatcher_registerCommand(
    2,
    BUTTON_PRESS_LONG_3s, 
    USER_EVENT_NULL
    );
	
	ButtonCmdDispatcher_registerCommand(
	3,
	BUTTON_PRESS_SHORT, 
	USER_EVENT_VOL_UP
	);

	ButtonCmdDispatcher_registerCommand(
	4,
	BUTTON_PRESS_SHORT, 
	USER_EVENT_VOL_DN
	);

    ButtonCmdDispatcher_registerCommand(
    3,
    BUTTON_PRESS_CONTINUE, 
    USER_EVENT_VOL_UP
    );

    ButtonCmdDispatcher_registerCommand(
    4,
    BUTTON_PRESS_CONTINUE, 
    USER_EVENT_VOL_DN
    );

    ButtonCmdDispatcher_registerCommand(
    5,
    BUTTON_PRESS_LONG_4s, 
    USER_EVENT_DEVICE_RESET
    );

    ButtonCmdDispatcher_registerCommand(
    6,
    BUTTON_PRESS_LONG, 
    USER_EVENT_MODE_DEMO1
    );

    ButtonCmdDispatcher_registerCommand(
    7,
    BUTTON_PRESS_LONG, 
    USER_EVENT_MODE_DEMO2
    );

    ButtonCmdDispatcher_registerCommand(
    8,
    BUTTON_PRESS_LONG, 
    USER_EVENT_MODE_DEMO3
    );

    ButtonCmdDispatcher_registerCommand( 
    9,
    BUTTON_PRESS_LONG, 
    USER_EVENT_VERSION	// Brendan - sys version key registation
    );

}



static void ButtonCmdDispatcher_registerDemo_2_Command( void )
{
    ButtonCmdDispatcher_registerCommand(
    0,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_START_MODE
    );

    ButtonCmdDispatcher_registerCommand(
    0,
    BUTTON_PRESS_LONG_4s, 
    USER_EVENT_NULL
    );

    ButtonCmdDispatcher_registerCommand(
    1,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_STOP_MODE
    );

    ButtonCmdDispatcher_registerCommand(
    2,
    BUTTON_PRESS_LONG_3s, 
    USER_EVENT_NULL
    );
	
    ButtonCmdDispatcher_registerCommand(
    2,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_NEXT
    );
	
    ButtonCmdDispatcher_registerCommand(
    3,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_VOL_UP
    );

    ButtonCmdDispatcher_registerCommand(
    4,
    BUTTON_PRESS_SHORT, 
    USER_EVENT_VOL_DN
    );

    ButtonCmdDispatcher_registerCommand(
    3,
    BUTTON_PRESS_CONTINUE, 
    USER_EVENT_VOL_UP
    );

    ButtonCmdDispatcher_registerCommand(
    4,
    BUTTON_PRESS_CONTINUE, 
    USER_EVENT_VOL_DN
    );

    ButtonCmdDispatcher_registerCommand(
    5,
    BUTTON_PRESS_LONG_4s, 
    USER_EVENT_DEVICE_RESET
    );

    ButtonCmdDispatcher_registerCommand(
    6,
    BUTTON_PRESS_LONG, 
    USER_EVENT_MODE_DEMO1
    );

    ButtonCmdDispatcher_registerCommand(
    7,
    BUTTON_PRESS_LONG, 
    USER_EVENT_MODE_DEMO2
    );

    ButtonCmdDispatcher_registerCommand(
    8,
    BUTTON_PRESS_LONG, 
    USER_EVENT_MODE_DEMO3
    );

    ButtonCmdDispatcher_registerCommand( 
    9,
    BUTTON_PRESS_LONG, 
    USER_EVENT_VERSION	// Brendan - sys version key registation
    );
	
}

static bool isPowerAndVolButtonPressed( byte PressedKey, xHMISrvEventParams *psrv_parms)
{
	if ( PressedKey == 0 )	// Power pressed
	{
		if ( ButtonLowLevel_hasButtonBeenPressedContinue(3) != BUTTON_NONE_VALID_TIME ) //if vol+ pressed too
		{
		    psrv_parms->event = USER_EVENT_VIZ_RMT_TOOGLE;
		    m_continue_set[3] = TRUE;
	            return TRUE;
		}
		else if ( ButtonLowLevel_hasButtonBeenPressedContinue(4) != BUTTON_NONE_VALID_TIME ) //if vol- pressed too
		{
		    psrv_parms->event = USER_EVENT_APD_TOOGLE;
		    m_continue_set[4] = TRUE;
	            return TRUE;
		}
	}
	else if ( PressedKey == 3 )	// Vol+ pressed
	{
		if ( ButtonLowLevel_hasButtonBeenPressedContinue(0) != BUTTON_NONE_VALID_TIME ) //if power pressed too
		{
		    psrv_parms->event = USER_EVENT_VIZ_RMT_TOOGLE;
		    m_continue_set[0] = TRUE;
	            return TRUE;
		}
	}
	else if ( PressedKey == 4 )	// Vol- pressed
	{
		if ( ButtonLowLevel_hasButtonBeenPressedContinue(0) != BUTTON_NONE_VALID_TIME ) //if power pressed too
		{
		    psrv_parms->event = USER_EVENT_APD_TOOGLE;
		    m_continue_set[0] = TRUE;
	            return TRUE;
		}
	}
	
	return FALSE;
}
