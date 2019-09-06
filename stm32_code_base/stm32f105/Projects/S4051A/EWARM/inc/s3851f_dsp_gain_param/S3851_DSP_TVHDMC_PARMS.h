#ifndef __S3851_DSP_TVHDMC_PARMS_H__
#define __S3851_DSP_TVHDMC_PARMS_H__

/***********************************************
This file is created by cirrus dsp tool on 01/29/13 04:32:59
This tool was written by XuShoucai on 06/01/09
************************************************/
static const uint32 code AC3_2_1_TVOLON [] = 
{
    /* ac3-2.1-TvolON_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000001, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000000, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000004, 
    // SUB_TRUBASS_CONTROL : signed : 1.31 format
    0xb3000011, 0x33333340, 
};

static const uint32 code AC3_2_1_TVOLOFF [] = 
{
    /* ac3-2.1-TvolOFF_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000000, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000001, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000004, 
    // SUB_TRUBASS_CONTROL : signed : 1.31 format
    0xb3000011, 0x33333340, 
};

static const uint32 code AC3_5_1_TVOLON [] = 
{
    /* ac3-5.1-TvolON_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000001, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000000, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000008, 
    // SUB_TRUBASS_CONTROL : signed : 1.31 format
    0xb3000011, 0x26666680, 
};

static const uint32 code AC3_5_1_TVOLOFF [] = 
{
    /* ac3-2.1-TvolOFF_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000000, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000001, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000008, 
    // SUB_TRUBASS_CONTROL : signed : 1.31 format
    0xb3000011, 0x26666680, 
};

static const uint32 code DTS_2_1_TVOLOFF [] = 
{
    /* dts-2.1-TvolOFF_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000000, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000001, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000004, 
};


static const uint32 code DTS_2_1_TVOLON [] = 
{
    /* dts-2.1-TvolON_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000001, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000000, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000004, 
};


static const uint32 code DTS_5_1_TVOLOFF [] = 
{
    /* dts-5.1-TvolOFF_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000000, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000001, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000008, 
};


static const uint32 code DTS_5_1_TVOLON [] = 
{
    /* dts-5.1-TvolON_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000001, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000000, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000008, 
};

/***********************************************
This file is created by cirrus dsp tool on 01/29/13 06:26:43
This tool was written by XuShoucai on 06/01/09
************************************************/

static const uint32 code _2_1_TRUVOLOFF[] = 
{
    /* pcm-2.1-TvolOFF_1218.cfg */
    // tv_enable : unsigned : 32.0 format
    0xe0000000, 0x00000000, 
    // tv_bypass : unsigned : 32.0 format
    0xe0000001, 0x00000001,
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000004, 
    // eq_proc (PEQ)
    // eq_control : unsigned : 32.0 format
    0xd5000000, 0x8000009f, 
    // eq_channel_rs_band_1_fc : unsigned : 20.12 format
    0xd50000a9, 0x80d7a000, 
    // eq_channel_rs_band_1_q : unsigned : 32.0 format
    0xd50000aa, 0x10000000, 
    // eq_channel_rs_band_1_gain : signed : 6.26 format
    0xd50000ab, 0xf0000000, 
    // eq_channel_rs_band_2_fc : unsigned : 20.12 format
    0xd50000ac, 0x805dc000, 
    // eq_channel_rs_band_2_q : unsigned : 32.0 format
    0xd50000ad, 0x04000000, 
    // eq_channel_rs_band_2_gain : signed : 6.26 format
    0xd50000ae, 0xf4000000, 
    // eq_channel_rs_band_3_fc : unsigned : 20.12 format
    0xd50000af, 0x8023f000, 
    // eq_channel_rs_band_3_q : unsigned : 32.0 format
    0xd50000b0, 0x08000000, 
    // eq_channel_rs_band_3_gain : signed : 6.26 format
    0xd50000b1, 0x18000000, 
    // eq_channel_rs_band_4_fc : unsigned : 20.12 format
    0xd50000b2, 0x80078000, 
    // eq_channel_rs_band_4_q : unsigned : 32.0 format
    0xd50000b3, 0x04000000, 
    // eq_channel_rs_band_4_gain : signed : 6.26 format
    0xd50000b4, 0x0c000000, 
    // eq_channel_rs_band_5_fc : unsigned : 20.12 format
    0xd50000b5, 0x84394000, 
    // eq_channel_rs_band_5_q : unsigned : 32.0 format
    0xd50000b6, 0x08000000, 
    // eq_channel_rs_band_5_gain : signed : 6.26 format
    0xd50000b7, 0xdc000000, 
    // eq_channel_rs_band_6_fc : unsigned : 20.12 format
    0xd50000b8, 0x82ee0000, 
    // eq_channel_rs_band_6_q : unsigned : 32.0 format
    0xd50000b9, 0x02000000, 
    // eq_channel_rs_band_6_gain : signed : 6.26 format
    0xd50000ba, 0xf8000000, 
    // eq_channel_rs_band_7_fc : unsigned : 20.12 format
    0xd50000bb, 0x80000000, 
    // eq_channel_rs_band_8_fc : unsigned : 20.12 format
    0xd50000be, 0x81770000, 
    // eq_channel_rs_band_8_q : unsigned : 32.0 format
    0xd50000bf, 0x04000000, 
    // eq_channel_rs_band_8_gain : signed : 6.26 format
    0xd50000c0, 0x08000000, 
    // eq_channel_rs_band_9_fc : unsigned : 20.12 format
    0xd50000c1, 0x8003c000, 
    // eq_channel_rs_band_9_q : unsigned : 32.0 format
    0xd50000c2, 0x04000000, 
    // eq_channel_rs_band_9_gain : signed : 6.26 format
    0xd50000c3, 0x0c000000, 
    // eq_channel_rs_band_10_fc : unsigned : 20.12 format
    0xd50000c4, 0x80000000, 
    // eq_channel_rs_band_11_fc : unsigned : 20.12 format
    0xd50000c7, 0x80000000, 
    // gain_multi_channel_plus24 (L/R Gain (+24dB))
    // g_1 : signed : 5.27 format
    0xf00000b6, 0x194c57e2, 
    // dynamic_hard_limiter2 (Hard Limiter) 
    // dsp_attack : signed : 1.31 format
    0xf0000093, 0x04324349, 
    // dsp_attack : signed : 1.31 format
    0xf0000083, 0x04324349, 
    // dsp_release : signed : 1.31 format
    0xf0000084, 0x00fa8a7d, 
    // dsp_attack : signed : 1.31 format
    0xf000008b, 0x04324349, 
    // dsp_release : signed : 1.31 format
    0xf000007c, 0x00fa8a7d, 
    // filter_lowpass_transposed_direct_II (Low Pass)
    // a_1 : signed : 1.31 format
    0xf0000071, 0x7c569125, 
    // a_2 : signed : 1.31 format
    0xf0000074, 0x7e786be3, 
};

static const uint32 code _2_1_TRUVOLON[] = 
{
    /* pcm-2.1-TvolON_1218.cfg */
    // tv_enable : unsigned : 32.0 format
    0xe0000000, 0x00000001, 
    // tv_bypass : unsigned : 32.0 format
    0xe0000001, 0x00000000,
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000004, 
    // eq_proc (PEQ)
    // eq_control : unsigned : 32.0 format
    0xd5000000, 0x8000009f, 
    // eq_channel_rs_band_1_fc : unsigned : 20.12 format
    0xd50000a9, 0x80d7a000, 
    // eq_channel_rs_band_1_q : unsigned : 32.0 format
    0xd50000aa, 0x10000000, 
    // eq_channel_rs_band_1_gain : signed : 6.26 format
    0xd50000ab, 0xf0000000, 
    // eq_channel_rs_band_2_fc : unsigned : 20.12 format
    0xd50000ac, 0x805dc000, 
    // eq_channel_rs_band_2_q : unsigned : 32.0 format
    0xd50000ad, 0x04000000, 
    // eq_channel_rs_band_2_gain : signed : 6.26 format
    0xd50000ae, 0xf4000000, 
    // eq_channel_rs_band_3_fc : unsigned : 20.12 format
    0xd50000af, 0x8023f000, 
    // eq_channel_rs_band_3_q : unsigned : 32.0 format
    0xd50000b0, 0x08000000, 
    // eq_channel_rs_band_3_gain : signed : 6.26 format
    0xd50000b1, 0x18000000, 
    // eq_channel_rs_band_4_fc : unsigned : 20.12 format
    0xd50000b2, 0x80078000, 
    // eq_channel_rs_band_4_q : unsigned : 32.0 format
    0xd50000b3, 0x04000000, 
    // eq_channel_rs_band_4_gain : signed : 6.26 format
    0xd50000b4, 0x0c000000, 
    // eq_channel_rs_band_5_fc : unsigned : 20.12 format
    0xd50000b5, 0x84394000, 
    // eq_channel_rs_band_5_q : unsigned : 32.0 format
    0xd50000b6, 0x08000000, 
    // eq_channel_rs_band_5_gain : signed : 6.26 format
    0xd50000b7, 0xdc000000, 
    // eq_channel_rs_band_6_fc : unsigned : 20.12 format
    0xd50000b8, 0x82ee0000, 
    // eq_channel_rs_band_6_q : unsigned : 32.0 format
    0xd50000b9, 0x02000000, 
    // eq_channel_rs_band_6_gain : signed : 6.26 format
    0xd50000ba, 0xf8000000, 
    // eq_channel_rs_band_7_fc : unsigned : 20.12 format
    0xd50000bb, 0x80000000, 
    // eq_channel_rs_band_8_fc : unsigned : 20.12 format
    0xd50000be, 0x81770000, 
    // eq_channel_rs_band_8_q : unsigned : 32.0 format
    0xd50000bf, 0x04000000, 
    // eq_channel_rs_band_8_gain : signed : 6.26 format
    0xd50000c0, 0x08000000, 
    // eq_channel_rs_band_9_fc : unsigned : 20.12 format
    0xd50000c1, 0x8003c000, 
    // eq_channel_rs_band_9_q : unsigned : 32.0 format
    0xd50000c2, 0x04000000, 
    // eq_channel_rs_band_9_gain : signed : 6.26 format
    0xd50000c3, 0x0c000000, 
    // eq_channel_rs_band_10_fc : unsigned : 20.12 format
    0xd50000c4, 0x80000000, 
    // eq_channel_rs_band_11_fc : unsigned : 20.12 format
    0xd50000c7, 0x80000000, 
    // gain_multi_channel_plus24 (L/R Gain (+24dB))
    // g_1 : signed : 5.27 format
    0xf00000b6, 0x194c57e2, 
    // dynamic_hard_limiter2 (Hard Limiter) 
    // dsp_attack : signed : 1.31 format
    0xf0000093, 0x04324349, 
    // dsp_attack : signed : 1.31 format
    0xf0000083, 0x04324349, 
    // dsp_release : signed : 1.31 format
    0xf0000084, 0x00fa8a7d, 
    // dsp_attack : signed : 1.31 format
    0xf000008b, 0x04324349, 
    // dsp_release : signed : 1.31 format
    0xf000007c, 0x00fa8a7d, 
    // filter_lowpass_transposed_direct_II (Low Pass)
    // a_1 : signed : 1.31 format
    0xf0000071, 0x7c569125, 
    // a_2 : signed : 1.31 format
    0xf0000074, 0x7e786be3, 
};

static const uint32 code _5_1_TRUVOLON[] = 
{
    // tv_2_1_4953X (SRS TV)
    //
    // tv_enable : unsigned : 32.0 format
    0xe0000000, 0x00000001, 
    // tv_bypass : unsigned : 32.0 format
    0xe0000001, 0x00000000, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000008, 
    // eq_proc (PEQ)
    // eq_control : unsigned : 32.0 format
    0xd5000000, 0x8000008f, 
    // eq_channel_rs_band_1_fc : unsigned : 20.12 format
    0xd50000a9, 0x00d7a000, 
    // eq_channel_rs_band_1_q : unsigned : 32.0 format
    0xd50000aa, 0x00000000, 
    // eq_channel_rs_band_1_gain : signed : 6.26 format
    0xd50000ab, 0x00000000, 
    // eq_channel_rs_band_2_fc : unsigned : 20.12 format
    0xd50000ac, 0x005dc000, 
    // eq_channel_rs_band_2_q : unsigned : 32.0 format
    0xd50000ad, 0x00000000, 
    // eq_channel_rs_band_2_gain : signed : 6.26 format
    0xd50000ae, 0x00000000, 
    // eq_channel_rs_band_3_fc : unsigned : 20.12 format
    0xd50000af, 0x0023f000, 
    // eq_channel_rs_band_3_q : unsigned : 32.0 format
    0xd50000b0, 0x00000000, 
    // eq_channel_rs_band_3_gain : signed : 6.26 format
    0xd50000b1, 0x00000000, 
    // eq_channel_rs_band_4_fc : unsigned : 20.12 format
    0xd50000b2, 0x00078000, 
    // eq_channel_rs_band_4_q : unsigned : 32.0 format
    0xd50000b3, 0x00000000, 
    // eq_channel_rs_band_4_gain : signed : 6.26 format
    0xd50000b4, 0x00000000, 
    // eq_channel_rs_band_5_fc : unsigned : 20.12 format
    0xd50000b5, 0x04394000, 
    // eq_channel_rs_band_5_q : unsigned : 32.0 format
    0xd50000b6, 0x00000000, 
    // eq_channel_rs_band_5_gain : signed : 6.26 format
    0xd50000b7, 0x00000000, 
    // eq_channel_rs_band_6_fc : unsigned : 20.12 format
    0xd50000b8, 0x02ee0000, 
    // eq_channel_rs_band_6_q : unsigned : 32.0 format
    0xd50000b9, 0x00000000, 
    // eq_channel_rs_band_6_gain : signed : 6.26 format
    0xd50000ba, 0x00000000, 
    // eq_channel_rs_band_7_fc : unsigned : 20.12 format
    0xd50000bb, 0x00226000, 
    // eq_channel_rs_band_8_fc : unsigned : 20.12 format
    0xd50000be, 0x01770000, 
    // eq_channel_rs_band_8_q : unsigned : 32.0 format
    0xd50000bf, 0x00000000, 
    // eq_channel_rs_band_8_gain : signed : 6.26 format
    0xd50000c0, 0x00000000, 
    // eq_channel_rs_band_9_fc : unsigned : 20.12 format
    0xd50000c1, 0x0003c000, 
    // eq_channel_rs_band_9_q : unsigned : 32.0 format
    0xd50000c2, 0x00000000, 
    // eq_channel_rs_band_9_gain : signed : 6.26 format
    0xd50000c3, 0x00000000, 
    // eq_channel_rs_band_10_fc : unsigned : 20.12 format
    0xd50000c4, 0x04394000, 
    // eq_channel_rs_band_11_fc : unsigned : 20.12 format
    0xd50000c7, 0x007d0000, 

    // gain_multi_channel_plus24 (L/R Gain (+24dB)) 
    // g_1 : signed : 5.27 format
    0xf00000b6, 0x194c5868, 
    // dynamic_hard_limiter2 (Hard Limiter)
    // dsp_attack : signed : 1.31 format
    0xf0000093, 0x08414ea2, 
    // dsp_attack : signed : 1.31 format
    0xf0000083, 0x08414ea2, 
    // dsp_release : signed : 1.31 format
    0xf0000084, 0x0974f10a, 
    // dsp_attack : signed : 1.31 format
    0xf000008b, 0x08414ea2, 
    // dsp_release : signed : 1.31 format
    0xf000007c, 0x0974f10a, 
    // filter_lowpass_transposed_direct_II (Low Pass)
    // a_1 : signed : 1.31 format
    0xf0000071, 0x7c569121, 
    // a_2 : signed : 1.31 format
    0xf0000074, 0x7e786be1,
};


static const uint32 code _5_1_TRUVOLOFF[] = 
{
    /* pcm-5.1-TvolOFF_1218.cfg */
    // tv_2_1_4953X (SRS TV)
    //
    // tv_enable : unsigned : 32.0 format
    0xe0000000, 0x00000000, 
    // tv_bypass : unsigned : 32.0 format
    0xe0000001, 0x00000001,
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000008, 
    // eq_proc (PEQ)
    // eq_control : unsigned : 32.0 format
    0xd5000000, 0x8000008f, 
    // eq_channel_rs_band_1_fc : unsigned : 20.12 format
    0xd50000a9, 0x00d7a000, 
    // eq_channel_rs_band_1_q : unsigned : 32.0 format
    0xd50000aa, 0x00000000, 
    // eq_channel_rs_band_1_gain : signed : 6.26 format
    0xd50000ab, 0x00000000, 
    // eq_channel_rs_band_2_fc : unsigned : 20.12 format
    0xd50000ac, 0x005dc000, 
    // eq_channel_rs_band_2_q : unsigned : 32.0 format
    0xd50000ad, 0x00000000, 
    // eq_channel_rs_band_2_gain : signed : 6.26 format
    0xd50000ae, 0x00000000, 
    // eq_channel_rs_band_3_fc : unsigned : 20.12 format
    0xd50000af, 0x0023f000, 
    // eq_channel_rs_band_3_q : unsigned : 32.0 format
    0xd50000b0, 0x00000000, 
    // eq_channel_rs_band_3_gain : signed : 6.26 format
    0xd50000b1, 0x00000000, 
    // eq_channel_rs_band_4_fc : unsigned : 20.12 format
    0xd50000b2, 0x00078000, 
    // eq_channel_rs_band_4_q : unsigned : 32.0 format
    0xd50000b3, 0x00000000, 
    // eq_channel_rs_band_4_gain : signed : 6.26 format
    0xd50000b4, 0x00000000, 
    // eq_channel_rs_band_5_fc : unsigned : 20.12 format
    0xd50000b5, 0x04394000, 
    // eq_channel_rs_band_5_q : unsigned : 32.0 format
    0xd50000b6, 0x00000000, 
    // eq_channel_rs_band_5_gain : signed : 6.26 format
    0xd50000b7, 0x00000000, 
    // eq_channel_rs_band_6_fc : unsigned : 20.12 format
    0xd50000b8, 0x02ee0000, 
    // eq_channel_rs_band_6_q : unsigned : 32.0 format
    0xd50000b9, 0x00000000, 
    // eq_channel_rs_band_6_gain : signed : 6.26 format
    0xd50000ba, 0x00000000, 
    // eq_channel_rs_band_7_fc : unsigned : 20.12 format
    0xd50000bb, 0x00226000, 
    // eq_channel_rs_band_8_fc : unsigned : 20.12 format
    0xd50000be, 0x01770000, 
    // eq_channel_rs_band_8_q : unsigned : 32.0 format
    0xd50000bf, 0x00000000, 
    // eq_channel_rs_band_8_gain : signed : 6.26 format
    0xd50000c0, 0x00000000, 
    // eq_channel_rs_band_9_fc : unsigned : 20.12 format
    0xd50000c1, 0x0003c000, 
    // eq_channel_rs_band_9_q : unsigned : 32.0 format
    0xd50000c2, 0x00000000, 
    // eq_channel_rs_band_9_gain : signed : 6.26 format
    0xd50000c3, 0x00000000, 
    // eq_channel_rs_band_10_fc : unsigned : 20.12 format
    0xd50000c4, 0x04394000, 
    // eq_channel_rs_band_11_fc : unsigned : 20.12 format
    0xd50000c7, 0x007d0000, 
    // gain_multi_channel_plus24 (L/R Gain (+24dB)) 
    // g_1 : signed : 5.27 format
    0xf00000b6, 0x194c5868, 
    // dynamic_hard_limiter2 (Hard Limiter)
    // dsp_attack : signed : 1.31 format
    0xf0000093, 0x08414ea2, 
    // dsp_attack : signed : 1.31 format
    0xf0000083, 0x08414ea2, 
    // dsp_release : signed : 1.31 format
    0xf0000084, 0x0974f10a, 
    // dsp_attack : signed : 1.31 format
    0xf000008b, 0x08414ea2, 
    // dsp_release : signed : 1.31 format
    0xf000007c, 0x0974f10a, 
    // filter_lowpass_transposed_direct_II (Low Pass)
    // a_1 : signed : 1.31 format
    0xf0000071, 0x7c569121, 
    // a_2 : signed : 1.31 format
    0xf0000074, 0x7e786be1,
};



#endif /* __S3851_DSP_TVHDMC_PARMS_H__ */
