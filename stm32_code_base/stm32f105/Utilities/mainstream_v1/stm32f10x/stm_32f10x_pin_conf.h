#ifndef __STM_32F10X_PIN_CONF_H__
#define __STM_32F10X_PIN_CONF_H__

#include "stm32f10x_conf.h"

//! ADC @{
#define ADC1_IN0	GPIO_Pin_0	
#define ADC2_IN1	GPIO_Pin_1	
#define ADC_PORT	GPIOA
#define ADC_PORT_CLK	RCC_APB2Periph_GPIOA
//! @}


//! Virtual COM port	@{
#define EVAL_COM1                        USART2
#define EVAL_COM1_CLK                    RCC_APB1Periph_USART2
#define EVAL_COM1_TX_PIN                 GPIO_Pin_5
#define EVAL_COM1_TX_GPIO_PORT           GPIOD
#define EVAL_COM1_TX_GPIO_CLK            RCC_APB2Periph_GPIOD
#define EVAL_COM1_RX_PIN                 GPIO_Pin_6
#define EVAL_COM1_RX_GPIO_PORT           GPIOD
#define EVAL_COM1_RX_GPIO_CLK            RCC_APB2Periph_GPIOD
#define EVAL_COM1_IRQn                   USART2_IRQn
//!	@}

//! IR configuration	@{
#define IR_RCV_RCC			RCC_APB2Periph_TIM1
#define IR_RCV_PORT_RCC		RCC_APB2Periph_GPIOE
#define IR_RCV_NVIC_ISR		TIM1_CC_IRQn
#define IR_RCV_IN_CH 		TIM_Channel_1
#define IR_RCV_TRIG_EDGE	TIM_ICPolarity_Falling
#define IR_RCV				TIM1
#define IR_RCV_IN			GPIO_Pin_9
#define IR_RCV_PORT			GPIOE
//!	@}

//!	SPI Interface pins definition @{
#define _SPI                       SPI1

//!	SPI CLOCK @{
#define _SPI_CLK                   RCC_APB2Periph_SPI1

#define _SPI_SCK_PIN               GPIO_Pin_5                 
#define _SPI_SCK_GPIO_PORT         GPIOA
#define _SPI_SCK_GPIO_CLK          RCC_APB2Periph_GPIOA
//!@}

//! SPI MISO	@{
#define _SPI_MISO_PIN              GPIO_Pin_6                  
#define _SPI_MISO_GPIO_PORT        GPIOA
#define _SPI_MISO_GPIO_CLK         RCC_APB2Periph_GPIOA
//!	@}

//! SPI MOSI	@{
#define _SPI_MOSI_PIN              GPIO_Pin_7                 
#define _SPI_MOSI_GPIO_PORT        GPIOA                     
#define _SPI_MOSI_GPIO_CLK         RCC_APB2Periph_GPIOA
//!	@}

//! SPI NSS	@{
#define _SPI_CS0_PIN               GPIO_Pin_4                 
#define _SPI_CS0_GPIO_PORT         GPIOA                      
#define _SPI_CS0_GPIO_CLK          RCC_APB2Periph_GPIOA
//! @}

//!	@}

//! PWM Definition @{
#define _PWM_0_TIM	TIM2
#define _PMW_0_CLK	RCC_APB1Periph_TIM2
#define _PWM_0_CH1_PA0	GPIO_Pin_0			
#define _PWM_0_CH2_PA1	GPIO_Pin_1			
#define _PWM_0_CH3_PA2	GPIO_Pin_2			
#define _PWM_0_CH4_PA3	GPIO_Pin_3			

#define _PWM_1_TIM	TIM3
#define _PMW_1_CLK	RCC_APB1Periph_TIM3
#define _PWM_1_CH1_PA6	GPIO_Pin_6			
#define _PWM_1_CH2_PA7	GPIO_Pin_7			
#define _PWM_1_CH3_PB0	GPIO_Pin_0			
#define _PWM_1_CH4_PB1	GPIO_Pin_1			

#define _PWM_2_TIM	TIM4
#define _PMW_2_CLK	RCC_APB1Periph_TIM4
#define _PWM_2_CH1_PD12	GPIO_Pin_12			
#define _PWM_2_CH2_PD13	GPIO_Pin_13			
#define _PWM_2_CH3_PD14	GPIO_Pin_14			
#define _PWM_2_CH4_PD15	GPIO_Pin_15			
//!	@}


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

//! I2C2 interface definitaion @{ 
//#define I2C2_DMA

#define sEE_I2C2                          I2C2
#define sEE_I2C2_CLK                      RCC_APB1Periph_I2C2
#define sEE_I2C2_SCL_PIN                 GPIO_Pin_10                  /* PB.10 */
#define sEE_I2C2_SCL_GPIO_PORT            GPIOB                       /* GPIOB */
#define sEE_I2C2_SCL_GPIO_CLK             RCC_APB2Periph_GPIOB
#define sEE_I2C2_SDA_PIN                  GPIO_Pin_11                  /* PB.11 */
#define sEE_I2C2_SDA_GPIO_PORT            GPIOB                       /* GPIOB */
#define sEE_I2C2_SDA_GPIO_CLK             RCC_APB2Periph_GPIOB

#if defined ( I2C2_DMA ) && defined ( I2S_DMA )
	#error "I2C_DMA and I2S DMA conflict "
#elif defined ( I2C2_DMA)
#define sEE_I2C2_DMA                      DMA1   
#define sEE_I2C2_DMA_CHANNEL_TX           DMA1_Channel4
#define sEE_I2C2_DMA_CHANNEL_RX           DMA1_Channel5 
#define sEE_I2C2_DMA_FLAG_TX_TC           DMA1_IT_TC4   
#define sEE_I2C2_DMA_FLAG_TX_GL           DMA1_IT_GL4 
#define sEE_I2C2_DMA_FLAG_RX_TC           DMA1_IT_TC5 
#define sEE_I2C2_DMA_FLAG_RX_GL           DMA1_IT_GL5    
#define sEE_I2C2_DMA_CLK                  RCC_AHBPeriph_DMA1
#define sEE_I2C2_DR_Address               ((uint32_t)0x40005810)
#define sEE_USE_DMA2

#define sEE_I2C2_DMA_TX_IRQn              DMA1_Channel4_IRQn
#define sEE_I2C2_DMA_RX_IRQn              DMA1_Channel5_IRQn
#define sEE_I2C2_DMA_TX_IRQHandler        DMA1_Channel4_IRQHandler
#define sEE_I2C2_DMA_RX_IRQHandler        DMA1_Channel5_IRQHandler   
#define sEE_DIRECTION2_TX                 0
#define sEE_DIRECTION2_RX                 1 
#endif /*defined (I2C2_DMA ) */


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

#endif /*__STM_32F10X_PIN_CONF_H__*/
