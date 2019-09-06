#ifndef __FREE_RTOS_TYPEDEF_H__
#define __FREE_RTOS_TYPEDEF_H__

#if defined ( FREE_RTOS )
/* Structure used to pass parameters to the blocking queue tasks. */
typedef struct QUEUE_PARAMETERS
{
	xQueueHandle xQueue;					/*< The queue to be used by the task. */
	portTickType xBlockTime;				/*< The block time to use on queue reads/writes. */
	volatile short *psCheckVariable;	/*< Incremented on each successful cycle to check the task is still running. */
} xQueueParameters;

/* Structure used to pass parameters to each task. */
typedef struct SEMAPHORE_PARAMETERS
{
	xSemaphoreHandle xSemaphore;
	volatile unsigned long *pulSharedVariable;
	portTickType xBlockTime;
} xSemaphoreParameters;

#define BLOCK_TIME(X) (( portTickType )( (X*portTICK_RATE_MS)/portTICK_RATE_MS ))
#define FRTOS_SIZE(X) ((unsigned portBASE_TYPE )(sizeof(X)/sizeof(uint8)))

#else

#if !defined ( STM32_IAP )
#error "FREE_RTOS no definition"
#endif

#endif 



#endif
