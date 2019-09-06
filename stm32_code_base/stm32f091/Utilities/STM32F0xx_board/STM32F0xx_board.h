#ifndef __STM32F0XX_BOARD_H_
#define __STM32F0XX_BOARD_H_

#if defined ( SB50514 )
 #ifndef STM32_IAP
 #include "SB50514_C0/STM32F0xx_SB50514_board.h"
 #else
 #include "SB50514_C0/STM32F0xx_SB50514_boot_board.h"
 #endif
#else
#error "__STM32F0XX_BOARD_H_ HAS NOT BEEN INCLUDED !! "
#endif 

#endif /* __STM32F0XX_BOARD_H_ */
