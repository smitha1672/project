#ifndef __S4051_GAIN_PARAMS_H__
#define __S4051_GAIN_PARAMS_H__

#define GAIN_STEP_MAX 13
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


//! < This part is for 24 step value curve for VIZIO UI specification  @{
/*Modify with VIZIO JASON volume curve*/

const static uint32 MASTER_GAIN_TABLE[24]=
{
	0x00000000,
	0x00022D46,
	0x0005F7CF,
	0x000CAFA7,
	0x00172A35,
	0x00263934,//0x02026f60, //0x00263934,
	0x003AB9D6,
	0x00557923,
	0x00775A21,
	0x00A12FDD,
	0x00D3C0D0,
	0x010FD9B2,
	0x0156A233,
	0x01A8740D,
	0x020698D1,
	0x0271A5D9,
	0x02EA2813,
	0x0371B7EF,
	0x04085B6B,
	0x04AF079C,
	0x05689E30,
	0x06321906,
	0x070E7777,
	0x08000000 
};

/*Tony modify 2014/12/19*/
const static uint32 BALANCE_PCM_GAIN[GAIN_STEP_MAX]=
{
    0x02d6a832,
    0x0392cf0f,
    0x047faca3,
    0x05a9df97,
    0x07214834,
    0x08f9e492,
    0x0b4ce0b9,//
    0x08f9e492,
    0x07214834,
    0x05a9df97,
    0x047faca3,
    0x0392cf0f,
    0x02d6a832
};

/*Tony modify 2014/08/12*/
const static uint32 BALANCE_AC3_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,//
    0x065ac8a3,
    0x050c332f,
    0x04026e54,
    0x032f52b9,
    0x0287a28b,
    0x02026f60
};

/*Tony modify 2014/08/12*/
const static uint32 BALANCE_DTS_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,//
    0x065ac8a3,
    0x050c332f,
    0x04026e54,
    0x032f52b9,
    0x0287a28b,
    0x02026f60
};

/*Tony modify 2014/08/12*/
const static uint32 CENTER_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,
    0x0a124745,
    0x0caddc61,
    0x0ff64bec,
    0x141857af,
    0x194c5868,
    0x1fd93c46
};

/*Tony modify 2014/08/12*/
const static uint32 LS_RS_PCM51_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,
    0x0a124745,
    0x0caddc61,
    0x0ff64bec,
    0x141857af,
    0x194c5868,
    0x1fd93c46
};

/*Tony modify 2014/08/12*/
const static uint32 LS_RS_DTS_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,
    0x0a124745,
    0x0caddc61,
    0x0ff64bec,
    0x141857af,
    0x194c5868,
    0x1fd93c46
};

/*Tony modify 2014/08/12*/
const static uint32 LS_RS_AC3_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,
    0x0a124745,
    0x0caddc61,
    0x0ff64bec,
    0x141857af,
    0x194c5868,
    0x1fd93c46
};


/*Tony modify 2014/08/12*/
const static uint32 SUB_PCM21_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,
    0x0b4ce0b9,
    0x0ff64bec,
    0x168c0c62,
    0x1fd93c46,
    0x2cfcbff4,
    0x3f8bd76e
};

/*Tony modify 2014/08/12*/
const static uint32 SUB_PCM51_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,
    0x0b4ce0b9,
    0x0ff64bec,
    0x168c0c62,
    0x1fd93c46,
    0x2cfcbff4,
    0x3f8bd76e
};

/*Tony modify 2014/08/12*/
const static uint32 SUB_AC321_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,
    0x0b4ce0b9,
    0x0ff64bec,
    0x168c0c62,
    0x1fd93c46,
    0x2cfcbff4,
    0x3f8bd76e
};
	
/*Tony modify 2014/08/12*/
const static uint32 SUB_DTS21_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,
    0x0b4ce0b9,
    0x0ff64bec,
    0x168c0c62,
    0x1fd93c46,
    0x2cfcbff4,
    0x3f8bd76e
};

/*Tony modify 2014/08/12*/
const static uint32 SUB_AC351_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,
    0x0b4ce0b9,
    0x0ff64bec,
    0x168c0c62,
    0x1fd93c46,
    0x2cfcbff4,
    0x3f8bd76e
};
	
/*Tony modify 2014/08/12*/
const static uint32 SUB_DTS51_GAIN[GAIN_STEP_MAX]=
{
    0x02026f60,
    0x0287a28b,
    0x032f52b9,
    0x04026e54,
    0x050c332f,
    0x065ac8a3,
    0x08000000,
    0x0b4ce0b9,
    0x0ff64bec,
    0x168c0c62,
    0x1fd93c46,
    0x2cfcbff4,
    0x3f8bd76e
};

/*
Bass / Treb Value for the Left Surround Channel. Configurable from +18dB to -18dB
(0x00000012 to 0xffffffee) †
Default* = 0x00000000
*/
/*smith implemented with audio jerry on 12/31*/
const static uint32 Treb_table [GAIN_STEP_MAX]=
{
    0xfffffff6,
    0xfffffff8,
    0xfffffffa,
    0xfffffffb,
    0xfffffffd,
    0xfffffffe,
    0x00000000,
    0x00000002,
    0x00000004,
    0x00000006,
    0x00000007,
    0x00000009,
    0x0000000b
};
	
/*Tony modify 2014/08/12*/
const static uint32 Bass_table [GAIN_STEP_MAX]=
{
    0xfffffff4,
    0xfffffff7,
    0xfffffff9,
    0xfffffffb,
    0xfffffffc,
    0xfffffffe,
    0x00000000,
    0x00000003,
    0x00000005,
    0x00000007,
    0x00000009,
    0x0000000b,
    0x0000000c
};

#endif /*__S4051_GAIN_PARAMS_H__*/
