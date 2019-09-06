#ifndef __S3851_GAIN_PARAMS_H__
#define __S3851_GAIN_PARAMS_H__

#define GAIN_STEP_MAX 25
#define SubWoofer_GAIN_24dB 0x81356324
#define SubWoofer_GAIN_14dB 0x28185058

/*
How to calcute gain value:
Step 1: value = (2^27)X(10^(gain/20))
Step 2: dec to hex

Excel
 hex to gain    =20*LOG((HEX2DEC(A1))/(2^27),10)
 gain to hex    =DEC2HEX(((2^27)*(10^(A1/20))))
*/


//! < This part is for 36 step value curve for VIZIO UI specification  @{
/*Modify with Jerry volume curve 2014/09/02*/

const static uint32 MASTER_GAIN_TABLE[36]=
{
    0x00000863,
    0x0000d0aa,
    0x00024c40,
    0x000679cc,
    0x000a43bb,
    0x000e7f6f,
    0x00147ae1,
    0x001ced91,
    0x00246b69,
    0x002dd93c,
    0x0039b888,
    0x0048aa43,
    0x005b7ae5,
    0x00732acf,
    0x0090fcf8,
    0x00ac5197,
    0x00cccccc,
    0x00f367a0,
    0x012149c6,
    0x0157d178,
    0x0198a116,
    0x01e5a815,
    0x02413448,
    0x02ae0221,
    0x0301b6d4,
    0x035fa2ad,
    0x03c90364,
    0x043f401c,
    0x04c3ea70,
    0x0558c4bd,
    0x05ffc86c,
    0x065ac8a3,
    0x06bb2d4d,
    0x07214834,
    0x078d6fda,
    0x08000000
};

/*Smith maintains 2015/5/27*/
const static uint32 BALANCE_PCM_GAIN[GAIN_STEP_MAX]=
{
    0x1fd93c46, /*-12dB*/
    0x23bc1483,
    0x28185058,
    0x2cfcbff4,
    0x327a0168,
    0x38a2bae0,
    0x3f8bd76e,
    0x474cd184,
    0x50000000,
    0x59c2f05a,
    0x64b6cacd,
    0x7100c49b,
    0x7eca9cdc,
    0x7100c49b,
    0x64b6cacd,
    0x59c2f05a,
    0x50000000,
    0x474cd184,
    0x3f8bd76e,
    0x38a2bae0,
    0x327a0168,
    0x2cfcbff4,
    0x28185058,
    0x23bc1483,
    0x1fd93c46  /*-12dB*/
};

/*Smith maintains 2015/5/27*/
const static uint32 BALANCE_AC3_GAIN[GAIN_STEP_MAX]=
{
    0x050c332f, /*-4dB*/
    0x05a9df97,
    0x065ac8a3,
    0x07214834,
    0x08000000,
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x11e8e68e,
    0x0ff64bec,
    0x0e39ea57,
    0x0caddc61,
    0x0b4ce0b9,
    0x0a124745,
    0x08f9e492,
    0x08000000,
    0x07214834,
    0x065ac8a3,
    0x05a9df97,
    0x050c332f  /*-4dB*/
};

/*Smith maintains 2015/5/27*/
const static uint32 BALANCE_DTS_GAIN[GAIN_STEP_MAX]=
{
    0x050c332f,  /*-4dB*/
    0x05a9df97,
    0x065ac8a3,
    0x07214834,
    0x08000000,
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,//
    0x11e8e68e,
    0x0ff64bec,
    0x0e39ea57,
    0x0caddc61,
    0x0b4ce0b9,
    0x0a124745,
    0x08f9e492,
    0x08000000,
    0x07214834,
    0x065ac8a3,
    0x05a9df97,
    0x050c332f  /*-4dB*/
};

/*Smith maintains 2015/5/27*/
const static uint32 CENTER_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,  /*-12dB*/
    0x02413448,
    0x0287a28b,
    0x02d6a832,
    0x032f52b9,
    0x0392cf0f,
    0x04026e54,
    0x047faca3,
    0x050c332f,
    0x05a9df97,
    0x065ac8a3,
    0x07214834,
    0x08000000,//
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x168c0c62,
    0x194c5868,
    0x1c629414,
    0x1fd93c46  /*+12dB*/
};

/*Smith maintains 2015/5/27*/
const static uint32 LS_RS_PCM51_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60, /*-12dB*/
    0x02413448,
    0x0287a28b,
    0x02d6a832,
    0x032f52b9,
    0x0392cf0f,
    0x04026e54,
    0x047faca3,
    0x050c332f,
    0x05a9df97,
    0x065ac8a3,
    0x07214834,
    0x08000000,//
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x168c0c62,
    0x194c5868,
    0x1c629414,
    0x1fd93c46  /*-12dB*/
};

/*Smith maintains 2015/5/27*/
const static uint32 LS_RS_DTS_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x02413448,
    0x0287a28b,
    0x02d6a832,
    0x032f52b9,
    0x0392cf0f,
    0x04026e54,
    0x047faca3,
    0x050c332f,
    0x05a9df97,
    0x065ac8a3,
    0x07214834,
    0x08000000,//
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x168c0c62,
    0x194c5868,
    0x1c629414,
    0x1fd93c46
};

/*Smith maintains 2015/5/27*/
const static uint32 LS_RS_AC3_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60, /*-12dB*/
    0x02413448,
    0x0287a28b,
    0x02d6a832,
    0x032f52b9,
    0x0392cf0f,
    0x04026e54,
    0x047faca3,
    0x050c332f,
    0x05a9df97,
    0x065ac8a3,
    0x07214834,
    0x08000000,//
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x168c0c62,
    0x194c5868,
    0x1c629414,
    0x1fd93c46  /*+12dB*/
};


/*Smith maintains 2015/5/27*/
const static uint32 SUB_PCM21_GAIN[GAIN_STEP_MAX]=
{
    0x0000346d, /*-80dB*/
    0x00a2adc4,
    0x00cccccc,
    0x0101d430,
    0x014495e1,
    0x0198a116,
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,//
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x168c0c62,
    0x194c5868,
    0x1c629414,
    0x1fd93c46  /*+12dB*/
};

/*Smith maintains 2015/5/27*/
const static uint32 SUB_PCM51_GAIN[GAIN_STEP_MAX]=
{
    0x0000346d,  /*-80dB*/
    0x00a2adc4,
    0x00cccccc,
    0x0101d430,
    0x014495e1,
    0x0198a116,
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,//
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x168c0c62,
    0x194c5868,
    0x1c629414,
    0x1fd93c46  /*+12dB*/
};

/*Smith maintains 2015/5/27*/
const static uint32 SUB_AC321_GAIN[GAIN_STEP_MAX]=
{
    0x0000346d, /*-80dB*/
    0x00a2adc4,
    0x00cccccc,
    0x0101d430,
    0x014495e1,
    0x0198a116,
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,//
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x168c0c62,
    0x194c5868,
    0x1c629414,
    0x1fd93c46  /*+12dB*/
};
	
/*Smith maintains 2015/5/27*/
const static uint32 SUB_DTS21_GAIN[GAIN_STEP_MAX]=
{
    0x0000346d,  /*-80dB*/
    0x00a2adc4,
    0x00cccccc,
    0x0101d430,
    0x014495e1,
    0x0198a116,
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,//
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x168c0c62,
    0x194c5868,
    0x1c629414,
    0x1fd93c46  /*+12dB*/
};

/*Smith maintains 2015/5/27*/
const static uint32 SUB_AC351_GAIN[GAIN_STEP_MAX]=
{
    0x0000346d,  /*-80dB*/
    0x00a2adc4,
    0x00cccccc,
    0x0101d430,
    0x014495e1,
    0x0198a116,
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,//
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x168c0c62,
    0x194c5868,
    0x1c629414,
    0x1fd93c46  /*+12dB*/
};
	
/*Smith maintains 2015/5/27*/
const static uint32 SUB_DTS51_GAIN[GAIN_STEP_MAX]=
{
    0x0000346d,  /*-80dB*/
    0x00a2adc4,
    0x00cccccc,
    0x0101d430,
    0x014495e1,
    0x0198a116,
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,//
    0x08f9e492,
    0x0a124745,
    0x0b4ce0b9,
    0x0caddc61,
    0x0e39ea57,
    0x0ff64bec,
    0x11e8e68e,
    0x141857af,
    0x168c0c62,
    0x194c5868,
    0x1c629414,
    0x1fd93c46  /*+12dB*/
};

/*
Bass / Treb Value for the Left Surround Channel. Configurable from +18dB to -18dB
(0x00000012 to 0xffffffee) ?
Default* = 0x00000000
*/
/*Smith maintains 2015/5/27*/
const static uint32 Treb_table [GAIN_STEP_MAX]=
{
    0xfffffff2,  /*-14dB*/
    0xfffffff3,
    0xfffffff4,
    0xfffffff5,
    0xfffffff6,
    0xfffffff7,
    0xfffffff8, 
    0xfffffff9,
    0xfffffffa,
    0xfffffffb,
    0xfffffffc,
    0xfffffffd, 
    0xfffffffe,
    0xffffffff,     
    0x00000000,
    0x00000001,
    0x00000002,
    0x00000003,
    0x00000004,
    0x00000005,
    0x00000006,
    0x00000007,
    0x00000008,
    0x00000009,
    0x0000000a  /*+10dB*/
};
	
/*Smith maintains 2015/5/27*/
const static uint32 Bass_table [GAIN_STEP_MAX]=
{
    0xffffffee,  /*-18dB*/
    0xfffffff3,
    0xfffffff4,
    0xfffffff6,
    0xfffffff7,
    0xfffffff8, 
    0xfffffff9,
    0xfffffffa,
    0xfffffffb,
    0xfffffffc,
    0xfffffffd,
    0xfffffffe,      
    0x00000000,
    0x00000001,
    0x00000002,
    0x00000003,
    0x00000004,
    0x00000005,
    0x00000006,
    0x00000007,
    0x00000008,
    0x00000009,
    0x0000000a, 
    0x0000000b,
    0x0000000c  /*+12dB*/
};

const static uint32 Bass_Night_table [GAIN_STEP_MAX]=
{
    0xFFFFFFEE,//-18
    0xFFFFFFF0,//-16
    0xFFFFFFF2,//-14
    0xFFFFFFF5,//-11
    0xFFFFFFF7,//-9
    0xFFFFFFF9,//-7
    0xFFFFFFFB,//-5
    0xFFFFFFFD,//-3
    0xFFFFFFFF,//-1
    0x00000001,//1
    0x00000003,//3
    0x00000005,//5
    0x00000007,//7
};


#endif /*__S3851_GAIN_PARAMS_H__*/
