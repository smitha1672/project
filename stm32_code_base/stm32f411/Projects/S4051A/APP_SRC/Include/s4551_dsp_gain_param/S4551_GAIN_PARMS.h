#ifndef __S4551_GAIN_PARAMS_H__
#define __S4551_GAIN_PARAMS_H__

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

/*Tony modify 2013/12/19*/
const static uint32 BALANCE_PCM_GAIN[GAIN_STEP_MAX]=
{
    0x1FD93C1F,
    0x28185086,
    0x327A01A4,
    0x3F8BD79D,
    0x50000000,
    0x64B6CADC,
    0x7ECA9CD2,//
    0x64B6CADC,
    0x50000000,
    0x3F8BD79D,
    0x327A01A4,
    0x28185086,
    0x1FD93C1F
};

/*Tony modify 2013/12/19*/
const static uint32 BALANCE_AC3_GAIN[GAIN_STEP_MAX]=
{
    0x0A12477C,
    0x0CADDC7B,
    0x0FF64C16,
    0x141857E9,
    0x194C583A,
    0x1FD93C1F,
    0x28185058,
    0x1FD93C1F,
    0x194C583A,
    0x141857E9,
    0x0FF64C16,
    0x0CADDC7B,
    0x0A12477C
};

/*Tony modify 2013/12/19*/
const static uint32 BALANCE_DTS_GAIN[GAIN_STEP_MAX]=
{
    0x0A12477C,
    0x0CADDC7B,
    0x0FF64C16,
    0x141857E9,
    0x194C583A,
    0x1FD93C1F,
    0x28185058,
    0x1FD93C1F,
    0x194C583A,
    0x141857E9,
    0x0FF64C16,
    0x0CADDC7B,
    0x0A12477C
};

/*Tony modify 2013/12/19*/
const static uint32 CENTER_GAIN[GAIN_STEP_MAX]=
{
	0x02026F30,
	0x0287A26C,
	0x032F52CF,
	0x04026E73,
	0x050C335D,
	0x065AC8C2,
	0x08000000,
	0x0A12477C,
	0x0CADDC7B,
	0x0FF64C16,
	0x141857E9,
	0x194C583A,
	0x1FD93C1F  
};

/*Tony modify 2013/12/19*/
const static uint32 LS_RS_PCM51_GAIN[GAIN_STEP_MAX]=
{
    0x032F52CF,
    0x04026E73,
    0x050C335D,
    0x065AC8C2,
    0x08000000,
    0x0A12477C,
    0xcaddc61,
    0x0FF64C16,
    0x141857E9,
    0x194C583A,
    0x1FD93C1F,
    0x28185086,
    0x327A01A4
};

/*Tony modify 2013/12/19*/
const static uint32 LS_RS_DTS_GAIN[GAIN_STEP_MAX]=
{
    0x02026F30,
    0x0287A26C,
    0x032F52CF,
    0x04026E73,
    0x050C335D,
    0x065AC8C2,
    0x8000000,
    0x0A12477C,
    0x0CADDC7B,
    0x0FF64C16,
    0x141857E9,
    0x194C583A,
    0x1FD93C1F
};

/*Tony modify 2013/12/19*/
const static uint32 LS_RS_AC3_GAIN[GAIN_STEP_MAX]=
{
    0x02026F30,
    0x0287A26C,
    0x032F52CF,
    0x04026E73,
    0x050C335D,
    0x065AC8C2,
    0x8000000,
    0x0A12477C,
    0x0CADDC7B,
    0x0FF64C16,
    0x141857E9,
    0x194C583A,
    0x1FD93C1F
};


/*Tony modify 2013/12/19*/
const static uint32 SUB_PCM21_GAIN[GAIN_STEP_MAX]=
{
    0x02D6A866,
    0x0392CED8,
    0x047FACCF,
    0x05A9DF7A,
    0x0721482B,
    0x08F9E4CF,
    0xb4ce0b9,
    0x0FF64C16,
    0x168C0C59,
    0x1FD93C1F,
    0x2CFCC016,
    0x3F8BD79D,
    0x59C2F01D
};

/*Tony modify 2013/12/19*/
const static uint32 SUB_PCM51_GAIN[GAIN_STEP_MAX]=
{
    0x02D6A866,
    0x0392CED8,
    0x047FACCF,
    0x05A9DF7A,
    0x0721482B,
    0x08F9E4CF,
    0xb4ce0b9,
    0x0FF64C16,
    0x168C0C59,
    0x1FD93C1F,
    0x2CFCC016,
    0x3F8BD79D,
    0x59C2F01D,
};

/*Tony modify 2013/12/19*/
const static uint32 SUB_AC321_GAIN[GAIN_STEP_MAX]=
{
    0x02D6A866,
    0x0392CED8,
    0x047FACCF,
    0x05A9DF7A,
    0x0721482B,
    0x08F9E4CF,
    0xb4ce0b9,
    0x0FF64C16,
    0x168C0C59,
    0x1FD93C1F,
    0x2CFCC016,
    0x3F8BD79D,
    0x59C2F01D
};
	
/*Tony modify 2013/12/19*/
const static uint32 SUB_DTS21_GAIN[GAIN_STEP_MAX]=
{
    0x02D6A866,
    0x0392CED8,
    0x047FACCF,
    0x05A9DF7A,
    0x0721482B,
    0x08F9E4CF,
    0xb4ce0b9,
    0x0FF64C16,
    0x168C0C59,
    0x1FD93C1F,
    0x2CFCC016,
    0x3F8BD79D,
    0x59C2F01D
};

/*Tony modify 2013/12/19*/
const static uint32 SUB_AC351_GAIN[GAIN_STEP_MAX]=
{
    0x02D6A866,
    0x0392CED8,
    0x047FACCF,
    0x05A9DF7A,
    0x0721482B,
    0x08F9E4CF,
    0xb4ce0b9,
    0x0FF64C16,
    0x168C0C59,
    0x1FD93C1F,
    0x2CFCC016,
    0x3F8BD79D,
    0x59C2F01D
};
	
/*Tony modify 2013/12/19*/
const static uint32 SUB_DTS51_GAIN[GAIN_STEP_MAX]=
{
    0x02D6A866,
    0x0392CED8,
    0x047FACCF,
    0x05A9DF7A,
    0x0721482B,
    0x08F9E4CF,
    0xb4ce0b9,
    0x0FF64C16,
    0x168C0C59,
    0x1FD93C1F,
    0x2CFCC016,
    0x3F8BD79D,
    0x59C2F01D
};

/*
Bass / Treb Value for the Left Surround Channel. Configurable from +18dB to -18dB
(0x00000012 to 0xffffffee) †
Default* = 0x00000000
*/
/*smith implemented with audio jerry on 12/31*/
const static uint32 Treb_table [GAIN_STEP_MAX]=
{
    0xFFFFFFF6,
    0xFFFFFFF8,
    0xFFFFFFF9,
    0xFFFFFFFB,
    0xFFFFFFFD,
    0xFFFFFFFF,
    0x00000000,
    0x00000002,
    0x00000004,
    0x00000006,
    0x00000008,
    0x00000009,
    0x0000000B
};
	
/*smith implemented with audio jerry on 12/31*/	
const static uint32 Bass_table [GAIN_STEP_MAX]=
{
    0xFFFFFFEE,
    0xFFFFFFF2,
    0xFFFFFFF5,
    0xFFFFFFF7,
    0xFFFFFFF9,
    0xFFFFFFFB,
    0xFFFFFFFD,
    0xFFFFFFFF,
    0x00000001,
    0x00000003,
    0x00000005,
    0x00000007,
    0x00000009
};

#endif /*__S3851_GAIN_PARAMS_H__*/
