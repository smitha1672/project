#include "GPIOMiddleLevel.h"

//!	< External Application ___________________________________________________________________________________@{
//These applicatons is only used on here.

extern void GPIOLowLevel_Set( VirtualPin pin );

extern void GPIOLowLevel_Clr( VirtualPin pin );

extern uint8_t GPIOLowLevel_RinputPinBit( VirtualPin pin );

extern uint8_t GPIOLowLevel_RoutputPinBit( VirtualPin pin );
//!	@}

/*________________________________________________________________________________________________________*/
void GPIOMiddleLevel_Set( VirtualPin pin )
{
	GPIOLowLevel_Set( pin );
}

void GPIOMiddleLevel_Clr( VirtualPin pin )
{
	GPIOLowLevel_Clr( pin );
}

uint8 GPIOMiddleLevel_Read_InBit( VirtualPin pin )
{
	uint8 bitstatus = 0x00; ;

	bitstatus = GPIOLowLevel_RinputPinBit( pin );

	return bitstatus;
}

uint8 GPIOMiddleLevel_Read_OutBit( VirtualPin pin )
{
	uint8 bitstatus = 0x00; ;

	bitstatus = GPIOLowLevel_RoutputPinBit( pin );

	return bitstatus;
}


