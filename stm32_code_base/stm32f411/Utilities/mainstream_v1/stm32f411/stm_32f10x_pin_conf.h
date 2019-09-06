#ifndef __STM_32F10X_PIN_CONF_H__
#define __STM_32F10X_PIN_CONF_H__

#include "stm32f4xx_conf.h"

//! ADC @{
#define ADC1_IN1	GPIO_Pin_1	
#define ADC1_IN2	GPIO_Pin_2	
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
#endif /*__STM_32F10X_PIN_CONF_H__*/
