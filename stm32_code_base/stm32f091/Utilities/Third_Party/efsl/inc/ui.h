

#ifndef __UI_H__
#define __UI_H__

/*****************************************************************************/
#include "stdint.h"

#if defined ( STM32F10X_CL )
#include "stm32f10x.h"
#endif 

#if defined ( STM32F40_41xxx )
#include "stm32f4xx.h"
#endif

#include "fs.h"
#include "types.h"
#include "fat.h"
#include "dir.h"
#include "config.h"
/*****************************************************************************/

short listFiles(FileSystem *fs, char *dir);
esint16 rmfile(FileSystem *fs,euint8* filename);
esint8 mkdir(FileSystem *fs,eint8* dirname);

#endif
