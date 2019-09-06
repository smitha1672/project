#include "BSP.h"

#if defined ( FREE_RTOS )
#include "freertos_conf.h"
#include "freertos_task.h"
#endif

#include "AudioSystemHandler.h"
//_____________________________________________________________________________
#define config_SPI_RELEASE 0
//_____________________________________________________________________________
extern BSP_BOARD_OBJECT *pBSP_BOARD_ObjCtrl;
extern BSP_GPIO_OBJECT *pBSP_GPIO_OBJECT;
extern BSP_SPI_OBJECT *pBSP_SPI_OBJECT;
extern AUDIO_SYSTEM_HANDLER_OBJECT *pASH_ObjCtrl;

void main( void )
{    
    pBSP_BOARD_ObjCtrl->board_config( );

#if ( config_SPI_RELEASE == 1 )
    pBSP_SPI_OBJECT->dsp_spi_deconfig( );
    while( 1 );
#else
    pASH_ObjCtrl->CreateTask( );
    vTaskStartScheduler(); /* Start the scheduler. */
#endif    
}


