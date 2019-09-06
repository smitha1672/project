#include "Defs.h"

/* It need be modify with "stm32f10x_application.icf" */
/*Smith fixes: *.bin loses one byte with IAR 7.1*/
const uint8 EEpromDefaultParams[] @ "USER_PARMS" ={
    0x00, 0x00
};

