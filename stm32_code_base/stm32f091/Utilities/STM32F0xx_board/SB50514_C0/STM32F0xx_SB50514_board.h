#ifndef __STM32F0XX_BOARD_H__
#define __STM32F0XX_BOARD_H__

#include "stm32f0xx_conf.h"
#include "stdio.h"
#include "Debug.h"
#include "STM32f0xx_it.h"
/*Board chips*/

#include "Command.h"
#ifndef BOOTLOADER

#endif

#define EEPROM_FUNCTION_ON

#define AUDIO_COMMON_POLL_STOP
#define BT_AMP_MUTE_SOLUTION 

/*******************************************************************************
 * GPIO define
 ******************************************************************************/
typedef enum { 
    __I_DSP_RES =0,
    __I_SPI1_NSS,    
    __I_SPI_SCK, 
    __I_SPI1_MISO,
    __I_SPI1_MOSI,
    __SPI_IO_MAX = 5    
} SPI1Pin;

typedef enum { 
    __I_SPI2_CS =0,   
    __I_SPI2_SCK, 
    __I_SPI2_MISO,
    __I_SPI2_MOSI,
    __SPI2_IO_MAX = 4    
} SPI2Pin;

/* */

/*******************************************************************************
 * Interrupt define
 ******************************************************************************/
#define USER_INTERFACE_PREEMPTION              1
#define DEVICE_CONTROL_INTREFACE_PREEMPTION    0

#define TIMERS_PREPRIO                  DEVICE_CONTROL_INTREFACE_PREEMPTION
#define TIMERS_SUBPRIO                  0

#define EVAL_AUDIO_IRQ_PREPRIO          DEVICE_CONTROL_INTREFACE_PREEMPTION
#define EVAL_AUDIO_IRQ_SUBRIO           1

#define MSC_PREPRIO                     USER_INTERFACE_PREEMPTION
#define MSC_SUBPRIO                     1
    
#define VCP_PREPRIO                     USER_INTERFACE_PREEMPTION
#define VCP_SUBPRIO                     2

#define RTC_PREPRIO                     USER_INTERFACE_PREEMPTION
#define RTC_SUBPRIO                     3

#define EXTI_PREPRIO                    USER_INTERFACE_PREEMPTION
#define EXTI_SUBPRIO                    4

#define UART3_PREPRIO                   USER_INTERFACE_PREEMPTION
#define UART3_SUBPRIO                   5

/*******************************************************************************
 * Timmer define
 ******************************************************************************/

/*******************************************************************************
 * Uart 1
 ******************************************************************************/
#define USART1_EN                       USART1
#define USART1_CLK                      RCC_APB2Periph_USART1
#define USART1_APBPERIPHCLOCK           RCC_APB2PeriphClockCmd

#define USART1_TX_PIN                   GPIO_Pin_9               
#define USART1_TX_GPIO_PORT             GPIOA                       
#define USART1_TX_GPIO_CLK              RCC_AHBPeriph_GPIOA
#define USART1_TX_SOURCE                GPIO_PinSource9
#define USART1_TX_AF                    GPIO_AF_1

#define USART1_RX_PIN                   GPIO_Pin_10                
#define USART1_RX_GPIO_PORT             GPIOA                    
#define USART1_RX_GPIO_CLK              RCC_AHBPeriph_GPIOA
#define USART1_RX_SOURCE                GPIO_PinSource10
#define USART1_RX_AF                    GPIO_AF_1

#define TXBUFFERSIZE                    (countof(TxBuffer) - 1)
#define RXBUFFERSIZE                    TXBUFFERSIZE

/*******************************************************************************
 * Uart 2
 ******************************************************************************/
#define USART2_EN                       USART2
#define USART2_CLK                      RCC_APB1Periph_USART2
#define USART2_APBPERIPHCLOCK           RCC_APB1PeriphClockCmd

#define USART2_TX_PIN                   GPIO_Pin_2               
#define USART2_TX_GPIO_PORT             GPIOA                       
#define USART2_TX_GPIO_CLK              RCC_AHBPeriph_GPIOA
#define USART2_TX_SOURCE                GPIO_PinSource2
#define USART2_TX_AF                    GPIO_AF_1

#define USART2_RX_PIN                   GPIO_Pin_3                
#define USART2_RX_GPIO_PORT             GPIOA                    
#define USART2_RX_GPIO_CLK              RCC_AHBPeriph_GPIOA
#define USART2_RX_SOURCE                GPIO_PinSource3
#define USART2_RX_AF                    GPIO_AF_1

/*******************************************************************************
 * I2C1
 ******************************************************************************/
#define I2C1_SPEED                      0x10805E89     
#define I2C1_CLK                        RCC_APB1Periph_I2C1
#define I2C1_SCL_PIN                    GPIO_Pin_6                 /* PB.6 */
#define I2C1_SCL_GPIO_PORT              GPIOB                      /* GPIOB */
#define I2C1_SCL_GPIO_CLK               RCC_AHBPeriph_GPIOB
#define I2C1_SCL_SOURCE                 GPIO_PinSource6
#define I2C1_SDA_PIN                    GPIO_Pin_7                 /* PB.7 */
#define I2C1_SDA_GPIO_PORT              GPIOB                      /* GPIOB */
#define I2C1_SDA_GPIO_CLK               RCC_AHBPeriph_GPIOB
#define I2C1_SDA_SOURCE                 GPIO_PinSource7

#define I2C1_FLAG_TIMEOUT               ((uint32_t)0x1000)
#define I2C1_LONG_TIMEOUT               ((uint32_t)(300 * I2C1_FLAG_TIMEOUT))

#define I2C1_OK                         1
#define I2C1_FAIL                       0

#define I2C1_SLAVE_ADDRESS7             0

#define I2C_INTERVAL_TIME               5


/**
 * Wait until a device a ready for I2C operations. 
 *
 * @param control   The slave control ID (I2C Address).   
 *
 * @return  TRUE in case the devie is ready. FALSE in case after a TIMEOUT, the
 *          device is still not ready.
 */
bool I2C_wait4DeviceReady(byte control);

static __IO uint32_t     I2C1_Timeout = I2C1_LONG_TIMEOUT;   
static __IO uint16_t*    I2C1DataReadPointer;   
static __IO uint8_t*     I2C1DataWritePointer;  

/*******************************************************************************
 * I2C2
 ******************************************************************************/
#define I2C2_SPEED                      0x10805E89     
#define I2C2_CLK                        RCC_APB1Periph_I2C2
#define I2C2_SCL_PIN                    GPIO_Pin_10                  /* PB.10 */
#define I2C2_SCL_GPIO_PORT              GPIOB                        /* GPIOB */
#define I2C2_SCL_GPIO_CLK               RCC_AHBPeriph_GPIOB
#define I2C2_SCL_SOURCE                 GPIO_PinSource10
#define I2C2_SDA_PIN                    GPIO_Pin_11                  /* PB.11 */
#define I2C2_SDA_GPIO_PORT              GPIOB                        /* GPIOB */
#define I2C2_SDA_GPIO_CLK               RCC_AHBPeriph_GPIOB
#define I2C2_SDA_SOURCE                 GPIO_PinSource11

#define I2C2_FLAG_TIMEOUT               ((uint32_t)0x1000)
#define I2C2_LONG_TIMEOUT               ((uint32_t)(300 * I2C2_FLAG_TIMEOUT))

#define I2C2_OK                         1
#define I2C2_FAIL                       0

#define I2C2_SLAVE_ADDRESS7             0

#define I2C_INTERVAL_TIME               5


/**
 * Wait until a device a ready for I2C operations. 
 *
 * @param control   The slave control ID (I2C Address).   
 *
 * @return  TRUE in case the devie is ready. FALSE in case after a TIMEOUT, the
 *          device is still not ready.
 */
//bool I2C_wait4DeviceReady(byte control);
static __IO uint32_t     I2C2_Timeout = I2C2_LONG_TIMEOUT;   
static __IO uint16_t*    I2C2DataReadPointer;   
static __IO uint8_t*     I2C2DataWritePointer;  


/*******************************************************************************
 * SPI
 ******************************************************************************/
//!    SPI Interface pins definition
#define _SPI                            SPI1

#define _SPI_CLK                        RCC_APB2Periph_SPI1
#define _SPI_SCK_PIN                    GPIO_Pin_5                 
#define _SPI_SCK_GPIO_PORT              GPIOA
#define _SPI_SCK_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define _SPI_SCK_SOURCE                 GPIO_PinSource5

#define _SPI_MISO_PIN                   GPIO_Pin_6                  
#define _SPI_MISO_GPIO_PORT             GPIOA
#define _SPI_MISO_GPIO_CLK              RCC_AHBPeriph_GPIOA
#define _SPI_MISO_SOURCE                GPIO_PinSource6

#define _SPI_MOSI_PIN                   GPIO_Pin_7                 
#define _SPI_MOSI_GPIO_PORT             GPIOA                     
#define _SPI_MOSI_GPIO_CLK              RCC_AHBPeriph_GPIOA
#define _SPI_MOSI_SOURCE                GPIO_PinSource7

#define _SPI_CS0_PIN                    GPIO_Pin_4                 
#define _SPI_CS0_GPIO_PORT              GPIOA                      
#define _SPI_CS0_GPIO_CLK               RCC_AHBPeriph_GPIOA

/** Time out to wait for an SPI write cmd ack */
#define SPI_TIME_OUT_WRITE_CMD          VIRTUAL_TIMER_MSEC2TICKS(10)

/** Time out to wait for an SPI write cmd ack */
#define SPI_TIME_OUT_READ_CMD           VIRTUAL_TIMER_MSEC2TICKS(10)

#define CS498xx_SPI_NPCS                0

#define SPI_LITTEL_ENDIAN               TRUE
#define SPI_BIG_ENDIAN                  FALSE
#define SPI_RET_ERROR_LEN               -1

/*******************************************************************************
 * SPI2
 ******************************************************************************/
//!    SPI2 Interface pins definition
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

#define _SPI2_CS0_LOW()                 GPIO_Clr( __O_SCP2_FLASH_CS )
#define _SPI2_CS0_HIGH()                GPIO_Set( __O_SCP2_FLASH_CS )

/*******************************************************************************
 * Function defines
 ******************************************************************************/


#endif
