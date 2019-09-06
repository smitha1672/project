#ifndef __FREERTOS_CONF_H__
#define __FREERTOS_CONF_H__

#if defined ( FREE_RTOS ) 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

/*configure NVIC interrupt priorty */
#define configLIB_VCP_INTERRUPT_PRIORITY ( configLIBRARY_KERNEL_INTERRUPT_PRIORITY )
#define configLIB_VIRUTAL_TIMER_INTERRUPT_PRIORITY (configLIBRARY_KERNEL_INTERRUPT_PRIORITY - 1 )
#define configLIB_IR_DECODE_INTERRUPT_PRIORITY (configLIBRARY_KERNEL_INTERRUPT_PRIORITY - 2 )
#define configLIB_MSC_INTERRUPT_PRIORITY (configLIBRARY_KERNEL_INTERRUPT_PRIORITY - 3 )
#define configLIB_I2C_DMA_INTERRUPT_PRIORITY (configLIBRARY_KERNEL_INTERRUPT_PRIORITY - 4 )
#define configLIB_AUDIO_INTERRUPT_PRIORITY (configLIBRARY_KERNEL_INTERRUPT_PRIORITY - 5 )
#define configLIB_SIL9535_INTERRUPT_PRIORITY (configLIBRARY_KERNEL_INTERRUPT_PRIORITY - 6 )
#define configLIB_CS8422_INTERRUPT_PRIORITY (configLIBRARY_KERNEL_INTERRUPT_PRIORITY - 7 )

#else

#if !defined ( STM32_IAP )
#error "FREE_RTOS no definition"
#endif

#endif 

#endif 
