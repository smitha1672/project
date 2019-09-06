#ifndef __STM32F0XX_BOOT_BOARD_H__
#define __STM32F0XX_BOOT_BOARD_H__

#include "Defs.h"
#include "ext_flash_driver.h"
typedef enum { 
    eOUT_PIN = 0,
    eIN_PIN  = 1,
    eOD_PIN = 2,
} IO_Pin;

typedef struct{
    GPIO_TypeDef* PORTx; 
    uint16_t PINx;
    IO_Pin IO;
} GPIO_PhysicalPin; 

typedef enum {     
    __O_RST_DSP,
    __O_SCP2_FLASH_CS,    
    __IO_MAX,
} VirtualPin;

void GPIO_Initialization( void );
void GPIO_Set( VirtualPin pin );
void GPIO_Clr( VirtualPin pin );
void ConfigBoot_USART(void);
void SPI2_LowLevel_initialize( void );
bool FlashDriver_test(void);
/*******************************************************************************
 * SPI2
 ******************************************************************************/
//!    SPI2 Interface pins definition
#define CS498xx_SPI_NPCS                0

#define SPI_LITTEL_ENDIAN               TRUE
#define SPI_BIG_ENDIAN                  FALSE
#define SPI_RET_ERROR_LEN               -1

#define _SPI2                           SPI2

#define _SPI2_CLK                       RCC_APB1Periph_SPI2
#define _SPI2_SCK_PIN                   GPIO_Pin_13                 
#define _SPI2_SCK_GPIO_PORT             GPIOB
#define _SPI2_SCK_GPIO_CLK              RCC_AHBPeriph_GPIOB
#define _SPI2_SCK_SOURCE                GPIO_PinSource13

#define _SPI2_MISO_PIN                  GPIO_Pin_14                  
#define _SPI2_MISO_GPIO_PORT            GPIOB
#define _SPI2_MISO_GPIO_CLK             RCC_AHBPeriph_GPIOB
#define _SPI2_MISO_SOURCE               GPIO_PinSource14

#define _SPI2_MOSI_PIN                  GPIO_Pin_15                 
#define _SPI2_MOSI_GPIO_PORT            GPIOB                    
#define _SPI2_MOSI_GPIO_CLK             RCC_AHBPeriph_GPIOB
#define _SPI2_MOSI_SOURCE               GPIO_PinSource15

#define _SPI2_CS0_PIN                   GPIO_Pin_12                 
#define _SPI2_CS0_GPIO_PORT             GPIOB                  
#define _SPI2_CS0_GPIO_CLK              RCC_AHBPeriph_GPIOB

#define _SPI2_CS0_LOW()                 GPIO_Clr(__O_SCP2_FLASH_CS);
#define _SPI2_CS0_HIGH()                GPIO_Set(__O_SCP2_FLASH_CS);

#endif
