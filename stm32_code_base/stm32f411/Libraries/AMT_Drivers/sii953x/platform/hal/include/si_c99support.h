/*
********************************************************************
  Copyright (c) 2011, Silicon Image, Inc.  All rights reserved.
  No part of this work may be reproduced, modified, distributed, transmitted,
  transcribed, or translated into any language or computer format, in any form
  or by any means without written permission of: Silicon Image, Inc.,
  1060 East Arques Avenue, Sunnyvale, California 94085
******************************************************************
*/

#include "device_config.h"

#if ( configSII_DEV_953x_PORTING == 0 )
/* C99 defined data types.  */
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;

typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed long    int32_t;

#ifndef NULL
 #define NULL ((void *) 0)
#endif

    /* Emulate C99/C++ bool type    */

#ifdef __cplusplus
typedef bool bool_t;
#else
typedef enum
{
    false   = 0,
    true    = !(false)
} bool_t;

typedef char BOOL;

#endif 

#endif // __cplusplus
