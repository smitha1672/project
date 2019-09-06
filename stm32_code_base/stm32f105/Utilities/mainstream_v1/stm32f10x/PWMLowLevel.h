#ifndef __PWM_LOW_LEVEL_H__
#define __PWM_LOW_LEVEL_H__

#include "Defs.h"


//! PWM Private typedef _________________________________________________________________________________@{
typedef enum { 
	_DUTY_OFF = 0, 
	_DUTY_25 = 1, 
	_DUTY_75 = 3,
	_DUTY_100 = 4
} DutySetup;

	
typedef enum { 
	_PWM_ch0 = 0, 
	_PWM_ch1 = 1, 
	_PWM_ch2 = 2,
	_PWM_ch3 = 3, 
	_PWM_ch4 = 4,
	_PWM_ch5 = 5,
	_PWM_ch6 = 6, 
	_PWM_ch7 = 7,
	_PWM_ch8 = 8,
	_PWM_ch9 = 9,
	_PWM_ch10 = 10, 
	_PWM_ch11 = 11 
} VirtualChannel;

//!	@}

int PWMLowLevel_DutySetting( VirtualChannel ch, DutySetup duty );



#endif /*__PWM_LOW_LEVEL_H__*/
