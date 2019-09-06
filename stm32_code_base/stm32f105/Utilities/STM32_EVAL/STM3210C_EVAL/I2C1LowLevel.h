#ifndef __I2C1_LOW_LEVEL_H__
#define __I2C1_LOW_LEVEL_H__

#include "Defs.h"

//! I2C interface definitaion @{ 
#define sEE_I2C                          I2C1
#define sEE_I2C_CLK                      RCC_APB1Periph_I2C1
#define sEE_I2C_SCL_PIN                  GPIO_Pin_6                  /* PB.06 */
#define sEE_I2C_SCL_GPIO_PORT            GPIOB                       /* GPIOB */
#define sEE_I2C_SCL_GPIO_CLK             RCC_APB2Periph_GPIOB
#define sEE_I2C_SDA_PIN                  GPIO_Pin_7                  /* PB.07 */
#define sEE_I2C_SDA_GPIO_PORT            GPIOB                       /* GPIOB */
#define sEE_I2C_SDA_GPIO_CLK             RCC_APB2Periph_GPIOB

#define sEE_I2C_DMA                      DMA1   
#define sEE_I2C_DMA_CHANNEL_TX           DMA1_Channel6
#define sEE_I2C_DMA_CHANNEL_RX           DMA1_Channel7 
#define sEE_I2C_DMA_FLAG_TX_TC           DMA1_IT_TC6   
#define sEE_I2C_DMA_FLAG_TX_GL           DMA1_IT_GL6 
#define sEE_I2C_DMA_FLAG_RX_TC           DMA1_IT_TC7 
#define sEE_I2C_DMA_FLAG_RX_GL           DMA1_IT_GL7    
#define sEE_I2C_DMA_CLK                  RCC_AHBPeriph_DMA1
#define sEE_I2C_DR_Address               ((uint32_t)0x40005410)
#define sEE_USE_DMA
   
#define sEE_I2C_DMA_TX_IRQn              DMA1_Channel6_IRQn
#define sEE_I2C_DMA_RX_IRQn              DMA1_Channel7_IRQn
#define sEE_I2C_DMA_TX_IRQHandler        DMA1_Channel6_IRQHandler
#define sEE_I2C_DMA_RX_IRQHandler        DMA1_Channel7_IRQHandler   
#define sEE_DIRECTION_TX                 0
#define sEE_DIRECTION_RX                 1 

/* Time constant for the delay caclulation allowing to have a millisecond 
   incrementing counter. This value should be equal to (System Clock / 1000).
   ie. if system clock = 72MHz then sEE_TIME_CONST should be 72. */
#define sEE_TIME_CONST                   72 

#define _I2C_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define _I2C_LONG_TIMEOUT         ((uint32_t)(10 * _I2C_FLAG_TIMEOUT))
#define _I2C_SPEED      			100000 /*!< I2C Speed */
#define _I2C_OK                    0
#define _I2C_FAIL                  1   
//! I2C interface definitaion @}



/*-------------------------------------------------------------------------------------------------------*/
bool I2C_writeSlave(byte control, uint16 address, byte *data, uint16 length, bool longAddr);

bool I2C_readSlave(byte control, uint16 address, byte *data, uint16 length, bool longAddr); 

bool i2c_write_block(byte dev_addr_in, uint16 reg_addr_in, byte *data_in, int count_in);

bool i2c_read_block(byte dev_addr_in, uint16 reg_addr_in, byte *data_out, int count_in);

bool i2c_write_byte(byte dev_addr_in, uint16 reg_addr_in, byte value);

int i2c_read_byte(byte dev_addr_in, uint16 reg_addr_in);


/**
 * Wait until a device a ready for I2C operations. 
 *
 * @param control   The slave control ID (I2C Address).   
 *
 * @return  TRUE in case the devie is ready. FALSE in case after a TIMEOUT, the
 *          device is still not ready.
 */
bool I2C_wait4DeviceReady(byte control);


#endif 
