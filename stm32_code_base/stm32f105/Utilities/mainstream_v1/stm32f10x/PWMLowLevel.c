#if !defined (__PWM_OUT_ENABLE__)	
#error	The feature has been to enable!!
#else
#include "PWMLowLevel.h"
#include "SOUND_BAR_V1_PinDefinition.h"

//! PWM Definition ______________________________________________________________________________________@{
#define _PWM_CH_MAX	12

//!	@}


//! PWM Private Variables ________________________________________________________________________________@{
static uint16_t mTimerPeriod = 0;
static TIM_OCInitTypeDef  mTIM_OCInitStructure;
//!	@}

	

/*_____________________________________________________________________________________________________*/
int PWMLowLevel_DutySetting( VirtualChannel ch, DutySetup duty )
{
	int ret = 0;
	uint16_t ChannelPulse = 0; 


	if ( ch > _PWM_ch11 )
	{
		return -1;
	}

	ChannelPulse = (uint16_t) (((uint32_t) (25 * duty) * (mTimerPeriod - 1)) / 100);
	mTIM_OCInitStructure.TIM_Pulse = ChannelPulse;

	switch( ch )
	{
		case _PWM_ch0:
		{
			TIM_OC1Init(_PWM_0_TIM, &mTIM_OCInitStructure);
		}
			break;

		case _PWM_ch1:
		{
			TIM_OC2Init(_PWM_0_TIM, &mTIM_OCInitStructure);
		}
			break;

		case _PWM_ch2:
		{
			TIM_OC3Init(_PWM_0_TIM, &mTIM_OCInitStructure);
		}
			break;

        case _PWM_ch3:
		{
			TIM_OC4Init(_PWM_0_TIM, &mTIM_OCInitStructure);
		}
			break;

		case _PWM_ch4:
		{
			TIM_OC1Init(_PWM_1_TIM, &mTIM_OCInitStructure);
		}
			break;

		case _PWM_ch5:
		{
			TIM_OC2Init(_PWM_1_TIM, &mTIM_OCInitStructure);
		}
			break;

		case _PWM_ch6:
		{
			TIM_OC3Init(_PWM_1_TIM, &mTIM_OCInitStructure);
		}
			break;

		case _PWM_ch7:
		{
			TIM_OC4Init(_PWM_1_TIM, &mTIM_OCInitStructure);
		}
			break;

		case _PWM_ch8:
		{
			TIM_OC1Init(_PWM_2_TIM, &mTIM_OCInitStructure);
		}
			break;

		case _PWM_ch9:
		{
			TIM_OC2Init(_PWM_2_TIM, &mTIM_OCInitStructure);
		}
			break;

		case _PWM_ch10:
		{
			TIM_OC3Init(_PWM_2_TIM, &mTIM_OCInitStructure);
		}
			break;

		case _PWM_ch11:
		{
			TIM_OC4Init(_PWM_2_TIM, &mTIM_OCInitStructure);
		}
			break;
		
	}

	return ret;

}

void PWMLowLevel_initialize( void )
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	//!	Confgiure PWM peripheral clock @{
	RCC_APB1PeriphClockCmd( (_PMW_0_CLK| _PMW_1_CLK| _PMW_2_CLK), ENABLE);
	RCC_APB2PeriphClockCmd( (RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO), ENABLE);
	//!	@}


	/* GPIOA Configuration: Channel 1, 2, 3 and 4 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = _PWM_0_CH1_PA0 | _PWM_0_CH2_PA1 | _PWM_0_CH3_PA2 | _PWM_0_CH4_PA3| _PWM_1_CH1_PA6| _PWM_1_CH2_PA7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* GPIOB Configuration: Channel 1, 2, 3 and 4 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = _PWM_1_CH3_PB0 | _PWM_1_CH4_PB1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* GPIOD Configuration: Channel 1, 2, 3 and 4 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = _PWM_2_CH1_PD12 | _PWM_2_CH2_PD13| _PWM_2_CH3_PD14| _PWM_2_CH4_PD15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); 
	/* TIM1 Full remapping pins */
	
	/* TIM2-4 Configuration ---------------------------------------------------
	  Generate 7 PWM signals with 4 different duty cycles:
	  TIM2-4CLK = SystemCoreClock, Prescaler = 0, TIM1 counter clock = SystemCoreClock
	  SystemCoreClock is set to 72 MHz for Low-density, Medium-density, High-density
	  and Connectivity line devices and to 24 MHz for Low-Density Value line and
	  Medium-Density Value line devices
	  
	  The objective is to generate 7 PWM signal at 17.57 KHz:
		- TIM1_Period = (SystemCoreClock / 17570) - 1
	  The channel 1 and channel 1N duty cycle is set to 50%
	  The channel 2 and channel 2N duty cycle is set to 37.5%
	  The channel 3 and channel 3N duty cycle is set to 25%
	  The channel 4 duty cycle is set to 12.5%
	  The Timer pulse is calculated as follows:
		- ChannelxPulse = DutyCycle * (TIM1_Period - 1) / 100
	 ----------------------------------------------------------------------- */
	 /* Compute the value to be set in ARR regiter to generate signal frequency at 17.57 Khz */
	 mTimerPeriod = (SystemCoreClock / 17570 ) - 1;

#if 0	
	 /* Compute CCR1 value to generate a duty cycle at 50% for channel 1 and 1N */
	 Channel1Pulse = (uint16_t) (((uint32_t) 5 * (mTimerPeriod - 1)) / 10);
	 /* Compute CCR2 value to generate a duty cycle at 37.5%  for channel 2 and 2N */
	 Channel2Pulse = (uint16_t) (((uint32_t) 375 * (mTimerPeriod - 1)) / 1000);
	 /* Compute CCR3 value to generate a duty cycle at 25%	for channel 3 and 3N */
	 Channel3Pulse = (uint16_t) (((uint32_t) 25 * (mTimerPeriod - 1)) / 100);
	 /* Compute CCR4 value to generate a duty cycle at 12.5%  for channel 4 */
	 Channel4Pulse = (uint16_t) (((uint32_t) 125 * (mTimerPeriod- 1)) / 1000);
#endif 	 
	
	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = mTimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(_PWM_0_TIM, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(_PWM_1_TIM, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(_PWM_2_TIM, &TIM_TimeBaseStructure);

	/* Channel 1, 2, 3 and 4 Configuration in PWM mode */
	mTIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	mTIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	mTIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	mTIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	mTIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	mTIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	mTIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_Cmd( _PWM_0_TIM , ENABLE);
	TIM_Cmd( _PWM_1_TIM , ENABLE);
	TIM_Cmd( _PWM_2_TIM , ENABLE);

	TIM_CtrlPWMOutputs( _PWM_0_TIM, ENABLE);
	TIM_CtrlPWMOutputs( _PWM_1_TIM, ENABLE);
	TIM_CtrlPWMOutputs( _PWM_2_TIM, ENABLE);
}

void PWMLowLevel_Deinitialize( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* TIM1 Main Output Enable */
	TIM_CtrlPWMOutputs( _PWM_0_TIM, DISABLE);
	TIM_CtrlPWMOutputs( _PWM_1_TIM, DISABLE);
	TIM_CtrlPWMOutputs( _PWM_2_TIM, DISABLE);

	/* TIM1 counter enable */
	TIM_Cmd( _PWM_0_TIM , DISABLE);
	TIM_Cmd( _PWM_1_TIM , DISABLE);
	TIM_Cmd( _PWM_2_TIM , DISABLE);

	//!	Confgiure PWM peripheral clock @{
	RCC_APB1PeriphClockCmd( (_PMW_0_CLK| _PMW_1_CLK| _PMW_2_CLK), DISABLE);
	RCC_APB2PeriphClockCmd( (RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOD), DISABLE);
	//!	@}

	/* GPIOA Configuration: Channel 1, 2, 3 and 4 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = _PWM_0_CH1_PA0 | _PWM_0_CH2_PA1 | _PWM_0_CH3_PA2 | _PWM_0_CH4_PA3| _PWM_1_CH1_PA6| _PWM_1_CH2_PA7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* GPIOB Configuration: Channel 1, 2, 3 and 4 as alternate function push-pull */
	//GPIO_InitStructure.GPIO_Pin = _PWM_1_CH3_PB0 | _PWM_1_CH4_PB1| _PWM_2_CH1_PB6 | _PWM_2_CH2_PB7| _PWM_2_CH3_PB8| _PWM_2_CH4_PB9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//GPIO_Init(GPIOB, &GPIO_InitStructure);

}

#endif //__PWM_ENABLE__