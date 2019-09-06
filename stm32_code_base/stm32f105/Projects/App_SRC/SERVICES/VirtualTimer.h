#ifndef __APP_UTILS_VIRTUAL_TIMER_H__
#define __APP_UTILS_VIRTUAL_TIMER_H__

#include "Defs.h"

/**
 * @defgroup VirtualTimer Virtual Timer API
 * @ingroup Managers
 *
 * This component implements a mechanism to let applications schedule delayed
 * execution of specific functions or post delayed events so that applications
 * don't need busy loops and system performance can be maximized.
 * The requirements for the virtual timer to work is that an hardware timer is
 * reserved for this purpose, acting as a fixed time base and calling the
 * Virtual Event core function each time the timer expires (each tick). A
 * manager in the main loop will periodically check the event list for events
 * scheduled for execution and transfer control to the event's callback.
 * This approach allows removing the bottleneck of busy loops without the need
 * for implementing a real multitasking environment still preserving code
 * execution linearity and ease of debugging.
 *
 * @section VTBackground Background Assumptions
 *
 * The event list has a fixed number of entries that needs to be defined at
 * build time. Any event registration that exceeds the list capacity will be
 * dropped and the event registration function will return an error,
 * Since this approach implements cooperative scheduling every callback function
 * shall return in the shortest possible time. This means that blocking calls
 * should be avoided as much as possible as these could interfere with the
 * execution other callbacks. <br>
 * @note Please note that this implementation is not reentrant so methods should
 * not be called by an ISP or in a multi tasking environment
 *
 * @section VTImplementation_Details Implementation Details
 *
 * The chosen approach is to use a hardware timer to periodically call an ISP
 * that increment a static member variable (m_now) which represents a software
 * timer which is checked against a list of registered events at each interrupt.
 * Whenever the timer is equal to the time of a registered event the event is
 * scheduled for execution and, depending on the nature of the event (single
 * shot or periodic) the event is deleted.<br>
 * To allow maximum flexibility the event structure contains also a pointer to a
 * data structure so that a single function could implement different tasks
 * based on different contexts.<br>
 * In addition to the above, each event will have a flag specifying if the event
 * is a real time one or not. Real time events are called directly from the ISP
 * while non real time events are executed from the main loop that checks off
 * line the event list and calls all the handlers that are flagged for
 * execution.<br>
 * Once registered timers can be canceled through VirtualTimer_cancel() which
 * means that events are not generated anymore without deleting the entry in the
 * timer list so that they can subsequently be rescheduled through
 * VirtualTimer_reschedule(). <br>
 * This approach can be useful for managers that require timers frequently but
 * do not implement a periodic task.<br>
 * For functions that simply require time consciousness VirtualTimer_now() is
 * provided to read back the current time so that eventual delays can be based
 * on this.<br>
 * Time base for events is defined in ticks. The ::VIRTUAL_TIMER_MSEC2TICKS
 * macro to convert from milliseconds to ticks is provided in the header file.
 *
 * @section VTRecommended_Operation Recommended Operation
 *
 * The recommended operation is to register functions that act as state machines
 * and pass them at least a variable (or even better a data structure) that
 * contain the function's state. This will allow for example the function to be
 * executed in chunks and restart from where it left since last call by
 * advancing the state at each step and implementing function's code in
 * individual cases of a switch statement.<br>
 *
 * @author Fabrizio Guglielmo
 */

/*@{*/

/**
 * macro to convert from milliseconds to ticks
 */
/**
* Virtual Timer timebase in microseconds
*/
#define VIRTUAL_TIMER_TIME_BASE_USEC 50

/**
* maximum number of virtual timer events
*/
#define VIRTUAL_TIMER_MAX_EVENTS 15



#define VIRTUAL_TIMER_MSEC2TICKS(msec) ((msec)*1000/VIRTUAL_TIMER_TIME_BASE_USEC)

#define VIRTUAL_TIMER_50USEC2TICKS(us_50) (us_50)

/**
 * macro to convert from CPU ticks to milliseconds
 */
#define VIRTUAL_TIMER_TICKS2MSEC(ticks) ((ticks)*VIRTUAL_TIMER_TIME_BASE_USEC)/1000

#define _MSEC_500 VIRTUAL_TIMER_MSEC2TICKS(500)
#define _SEC_1 VIRTUAL_TIMER_MSEC2TICKS(1000)



/**
 * Virtual Timer callback function pointer type
 */
typedef union
{
    struct
    {
#ifdef __ARM_CORTEX_MX__
        uint8 realtime : 1; /**< if set callback is called from ISP            */
        uint8 periodic : 1; /**< if set event is periodic                      */
        uint8 reserved : 3;
        uint8 valid    : 1; /**< item data is valid                            */
        uint8 execute  : 1; /**< flags item for immediate execution            */
        uint8 active   : 1; /**< if true item is active                        */
#endif

#ifdef __AVR32__
        uint8 active   : 1; /**< if true item is active                        */
        uint8 execute  : 1; /**< flags item for immediate execution            */
        uint8 valid    : 1; /**< item data is valid                            */
        uint8 reserved : 3;
        uint8 periodic : 1; /**< if set event is periodic                      */
        uint8 realtime : 1; /**< if set callback is called from ISP            */
#endif
    } bits;
    uint8 data;
} VirtualTimer_flags;

#define VIRTUALTIMER_REALTIME  0x1
#define VIRTUALTIMER_PERIODIC  0x2
#define VIRTUALTIMER_NO_ACTIVE 0x0

#define VIRTUALTIMER_ACTIVE   0x80
/**
 * Virtual Timer callback function pointer type
 */
typedef void (* VirtualTimer_callback)(void *data);

/**
 * Initializes Virtual timer manager. must be called before any API in this
 * component
 */
void VirtualTimer_init(void);

/**
 * Virtual Timer main loop handler. must be called regularly in the main loop
 * to allow non real time timers to be serviced
 */
void VirtualTimer_poll(void);

/**
 * registers a virtual timer event. note that to make sure the event gets
 * scheduled the active flag must be set in the parameters
 *
 * @param time     time in which the event will occur
 * @param callback function to be called at event
 * @param data     data to be passed to event callback
 * @param flags    flags for event
 *
 * @return event id. if -1 timer registration failed (event list full)
 */
int VirtualTimer_register(
    uint32                time,
    VirtualTimer_callback callback,
    void                 *data,
    uint8                 flags
);

/**
 * cancels a virtual timer event. this doesn't remove timer entry so that it
 * can be reused by calling VirtualTimer_reschedule without the need to
 * register it again
 *
 * @param event event id to be stopped from timer event list
 *
 * @return TRUE if successfully removed, FALSE if event was empty
 */
bool VirtualTimer_cancel(uint8 event);

/**
 * removes a virtual timer event
 *
 * @param event event id to be removed from timer event list
 *
 * @return TRUE if successfully removed, FALSE if event was empty
 */
bool VirtualTimer_remove(uint8 event);

/**
 * reschedules a virtual timer event
 *
 * @param event event id to be rescheduled
 * @param time  new time at which the event should occur
 *
 * @return TRUE if successfully rescheduled, FALSE if event was empty
 */
bool VirtualTimer_reschedule(uint8 event, uint32 time);

/**
 * returns the current system time
 *
 * @return system time in CPU ticks
 */
uint32 VirtualTimer_now(void);

/**
 * returns the current system time in milliseconds
 *
 * @return system time in milliseconds
 */
uint32 VirtualTimer_nowMs(void);


/**
 * busy loop for a give number of milliseconds. during wait the poll function
 * is called to allow scheduled events to be serviced
 *
 * @param timeMsec number of milliseconds to wait
 */
void VirtualTimer_sleep(uint32 timeMsec);

void VirtualTimer_minisleep(uint32 time50usec);

/**
 * checks if a given timer is active
 *
 * @param event event number to check
 *
 * @param TRUE if event is active and valid.
 */
bool VirtualTimer_isActive(uint8 event);

/**
 * Dump virtual timers status
 */
void VirtualTimer_dump();

uint32 VirtualTimer_getCurrentTime( );

bool VirtualTimer_isTimeout(uint32 *pTimeStamp, uint32 time);


/*@}*/

#endif /*__APP_UTILS_VIRTUAL_TIMER_H__ */

