#ifndef __FREERTOS_TASK_H__
#define __FREERTOS_TASK_H__

#if defined ( FREE_RTOS ) 
#include "task.h"

#define BTASK_SUSPENDED  TRUE
#define BTASK_RESUME   FALSE

typedef enum{
    xOS_TASK_NULL = 0,
    xOS_setTASK_SUSPENDED,
    xOS_setTASK_RESUME
}xOS_TaskCtrl;

typedef enum{
    xOS_TASK_ERR_NULL = 0,
    xOS_TASK_CREATE_FAIL = -1,
    xOS_TASK_QUEUE_CREATE_FAIL = -2,
    xOS_TASK_QUEUE_IS_NULL = -3,
    xOS_TASK_QUEUE_SET_FAIL = -4,
    xOS_TASK_QUEUE_GET_FAIL = -5,
}xOS_TaskErrIndicator;


typedef enum{
	TASK_SUSPENDED = 0,
	TASK_READY,
	TASK_RUNING, 
	TASK_BLOCKED
}TaskHandleState;



#define STACK_SIZE configMINIMAL_STACK_SIZE

#define TASK_MSEC2TICKS(msec) (( portTickType )( (msec*portTICK_RATE_MS)/portTICK_RATE_MS ))

/*Human Machine Interface priority, includes IR, button, uart...etc*/
#define tskUSB_DEVICE_PRIORITY ( tskIDLE_PRIORITY+1 )

#define tskHMI_APC_PRIORITY (tskIDLE_PRIORITY+1)
//#define tskUI_PRIORITY (tskIDLE_PRIORITY) /*unused - Smith*/
#define tskH68F30_PRIORITY (tskIDLE_PRIORITY+1)
#define tskDemo1_3_TIMEOUT_DETECT_PRIORITY (tskIDLE_PRIORITY+1)

#define tskHMI_STANDBY_CTRL_PRIORITY (tskIDLE_PRIORITY+1)

#define tskHMI_SERVICE_PRIORITY (tskIDLE_PRIORITY+7)
#define tskFCI_SERVICE_PRIORITY (tskIDLE_PRIORITY+6)
#define tskCCK_MANAGER_PRIORITY ( tskIDLE_PRIORITY+1)
#define tskHDMI_SERVICE_PRIORITY (tskIDLE_PRIORITY+11)

#define tskHMI_EVENT_PRIORITY (tskIDLE_PRIORITY+2)
#define tskUSB_MANAGER_PRIORITY ( tskIDLE_PRIORITY+11)
#define tskUSB_MEDIA_PRIORITY ( tskIDLE_PRIORITY+2)
//#define tskBT_MANAGER_PRIORITY ( tskIDLE_PRIORITY+2) /*unused - Smith*/
#define tskAID_MANAGER_PRIORITY (tskIDLE_PRIORITY+1)
#define tskAUD_HANDLER_PRIORITY (tskIDLE_PRIORITY+8)

#define tskRF_HANDLER_PRIORITY  (tskIDLE_PRIORITY+3)	
#define tskBT_HANDLER_PRIORITY  (tskIDLE_PRIORITY+3)
//#define tskSII9535_PRIORITY ( tskIDLE_PRIORITY+3 )
#define tskSII9535_PRIORITY ( tskIDLE_PRIORITY+10 )

#define tskAUD_BACKCTRL_PRIORITY (tskIDLE_PRIORITY+9)
#define tskAUD_EXCEPTION_PRIORITY (tskIDLE_PRIORITY+9)
#define tskAUD_VOLUMERAMP_PRIORITY (tskIDLE_PRIORITY+9)
#define tskADM_SIGNAL_DETECTOR_PRIORITY ( tskIDLE_PRIORITY +9 )
#define tskCS4953x_LowLevelDetection_PRIORITY ( tskIDLE_PRIORITY +4 )

#define tskADM_DIGITAL_DETECTOR_PRIORITY ( tskIDLE_PRIORITY+9 )

#define tskCS4953x_Loader_PRIORITY ( tskIDLE_PRIORITY+9 )
#define tskCS4953x_FCH_HANDLER_PRIORITY ( tskIDLE_PRIORITY+5 )
#define tskCS8422_ISR_HANDLER_PRIORITY ( tskIDLE_PRIORITY+8 )

#define tskIR_LEARN_MANAGER_PRIORITY ( tskIDLE_PRIORITY+8)
#define tskPOWER_HANDLER_PRIORITY (tskIDLE_PRIORITY+12)

#define tskUSB_HOST_PRIORITY ( tskIDLE_PRIORITY+10)
#define tskUSB_COMMAND_SERVICE_PRIORITY ( tskIDLE_PRIORITY+10)

#define tskWatchdog_PRIORITY ( tskIDLE_PRIORITY+1 )






#else

#if !defined ( STM32_IAP )
#error "FREE_RTOS no definition"
#endif

#endif

#endif /*__FREERTOS_TASK_H__*/
