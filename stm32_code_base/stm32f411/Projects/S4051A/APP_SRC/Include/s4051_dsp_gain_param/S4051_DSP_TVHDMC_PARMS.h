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
    // eq_channel_l_band_6_fc : unsigned : 20.12 format
    0xd5000034, 0x82ffd000, 
    // eq_channel_l_band_6_gain : signed : 6.26 format
    0xd5000036, 0xdeccccc0, 
    // eq_channel_l_band_7_fc : unsigned : 20.12 format
    0xd5000037, 0x823ac000, 
    // eq_channel_c_band_1_fc : unsigned : 20.12 format
    0xd5000046, 0x800e4000, 
    // eq_channel_c_band_1_gain : signed : 6.26 format
    0xd5000048, 0x10ccccc0, 
    // eq_channel_c_band_2_fc : unsigned : 20.12 format
    0xd5000049, 0x801ee000, 
    // eq_channel_c_band_5_fc : unsigned : 20.12 format
    0xd5000052, 0x80096000, 
    // eq_channel_c_band_5_gain : signed : 6.26 format
    0xd5000054, 0x08a3d710, 
    // eq_channel_c_band_7_fc : unsigned : 20.12 format
    0xd5000058, 0x81190000, 
    // eq_channel_c_band_7_gain : signed : 6.26 format
    0xd500005a, 0xf2eb8520, 
    // eq_channel_c_band_8_fc : unsigned : 20.12 format
    0xd500005b, 0x82232000, 
    // eq_channel_c_band_8_gain : signed : 6.26 format
    0xd500005d, 0xd928f5c0, 
    // eq_channel_c_band_10_fc : unsigned : 20.12 format
    0xd5000061, 0x848bf000, 
    // eq_channel_c_band_10_gain : signed : 6.26 format
    0xd5000063, 0xe8a3d700, 
    // filter_highpass_transposed_direct_II (High Pass)
    // a_1 : signed : 1.31 format
    0xf000003f, 0x7e27e481, 
    // a_2 : signed : 1.31 format
    0xf0000042, 0x7f3b9e35, 
    // c_2 : signed : 32.0 format
    0xf0000044, 0x00000001, 
    // a_1 : signed : 1.31 format
    0xf0000049, 0x7e27e481, 
    // a_2 : signed : 1.31 format
    0xf000004c, 0x7f3b9e35, 
    // c_2 : signed : 32.0 format
    0xf000004e, 0x00000001, 
    // a_1 : signed : 1.31 format
    0xf0000053, 0x7dd9dedd, 
    // a_2 : signed : 1.31 format
    0xf0000056, 0x7f1b00bd, 
    // c_2 : signed : 32.0 format
    0xf0000058, 0x00000001, 
    // a_1 : signed : 1.31 format
    0xf000005d, 0x7e27e481, 
    // a_2 : signed : 1.31 format
    0xf0000060, 0x7f3b9e35, 
    // c_2 : signed : 32.0 format
    0xf0000062, 0x00000001, 
    // a_1 : signed : 1.31 format
    0xf0000067, 0x7e27e481, 
    // a_2 : signed : 1.31 format
    0xf000006a, 0x7f3b9e35, 
    // c_2 : signed : 32.0 format
    0xf000006c, 0x00000001, 
    // dynamic_hard_limiter2 (Hard Limiter)
    // dsp_attack : signed : 1.31 format
    0xf0000093, 0x04324349, 
    // dsp_attack : signed : 1.31 format
    0xf0000083, 0x04324349, 
    // dsp_attack : signed : 1.31 format
    0xf000008b, 0x04324349,     
    // threshold : signed : 9.23 format
    0xf0000078, 0x0199999a, 
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
    // eq_channel_l_band_6_fc : unsigned : 20.12 format
    0xd5000034, 0x82ffd000, 
    // eq_channel_l_band_6_gain : signed : 6.26 format
    0xd5000036, 0xdeccccc0, 
    // eq_channel_l_band_7_fc : unsigned : 20.12 format
    0xd5000037, 0x823ac000, 
    // eq_channel_c_band_1_fc : unsigned : 20.12 format
    0xd5000046, 0x800e4000, 
    // eq_channel_c_band_1_gain : signed : 6.26 format
    0xd5000048, 0x10ccccc0, 
    // eq_channel_c_band_2_fc : unsigned : 20.12 format
    0xd5000049, 0x801ee000, 
    // eq_channel_c_band_5_fc : unsigned : 20.12 format
    0xd5000052, 0x80096000, 
    // eq_channel_c_band_5_gain : signed : 6.26 format
    0xd5000054, 0x08a3d710, 
    // eq_channel_c_band_7_fc : unsigned : 20.12 format
    0xd5000058, 0x81190000, 
    // eq_channel_c_band_7_gain : signed : 6.26 format
    0xd500005a, 0xf2eb8520, 
    // eq_channel_c_band_8_fc : unsigned : 20.12 format
    0xd500005b, 0x82232000, 
    // eq_channel_c_band_8_gain : signed : 6.26 format
    0xd500005d, 0xd928f5c0, 
    // eq_channel_c_band_10_fc : unsigned : 20.12 format
    0xd5000061, 0x848bf000, 
    // eq_channel_c_band_10_gain : signed : 6.26 format
    0xd5000063, 0xe8a3d700, 
    // filter_highpass_transposed_direct_II (High Pass)
    // a_1 : signed : 1.31 format
    0xf000003f, 0x7e27e481, 
    // a_2 : signed : 1.31 format
    0xf0000042, 0x7f3b9e35, 
    // c_2 : signed : 32.0 format
    0xf0000044, 0x00000001, 
    // a_1 : signed : 1.31 format
    0xf0000049, 0x7e27e481, 
    // a_2 : signed : 1.31 format
    0xf000004c, 0x7f3b9e35, 
    // c_2 : signed : 32.0 format
    0xf000004e, 0x00000001, 
    // a_1 : signed : 1.31 format
    0xf0000053, 0x7dd9dedd, 
    // a_2 : signed : 1.31 format
    0xf0000056, 0x7f1b00bd, 
    // c_2 : signed : 32.0 format
    0xf0000058, 0x00000001, 
    // a_1 : signed : 1.31 format
    0xf000005d, 0x7e27e481, 
    // a_2 : signed : 1.31 format
    0xf0000060, 0x7f3b9e35, 
    // c_2 : signed : 32.0 format
    0xf0000062, 0x00000001, 
    // a_1 : signed : 1.31 format
    0xf0000067, 0x7e27e481, 
    // a_2 : signed : 1.31 format
    0xf000006a, 0x7f3b9e35, 
    // c_2 : signed : 32.0 format
    0xf000006c, 0x00000001, 
    // dynamic_hard_limiter2 (Hard Limiter)
    // dsp_attack : signed : 1.31 format
    0xf0000093, 0x04324349, 
    // dsp_attack : signed : 1.31 format
    0xf0000083, 0x04324349, 
    // dsp_attack : signed : 1.31 format
    0xf000008b, 0x04324349, 
    // threshold : signed : 9.23 format
    0xf0000078, 0x0199999a, 
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
    // eq_channel_l_band_6_fc : unsigned : 20.12 format
    0xd5000034, 0x82ffce68, 
    // eq_channel_l_band_6_gain : signed : 6.26 format
    0xd5000036, 0xdec8e8c0, 
    // eq_channel_l_band_7_fc : unsigned : 20.12 format
    0xd5000037, 0x823abd20, 
    // eq_channel_c_band_1_fc : unsigned : 20.12 format
    0xd5000046, 0x800e3e35, 
    // eq_channel_c_band_1_gain : signed : 6.26 format
    0xd5000048, 0x10cf6400, 
    // eq_channel_c_band_2_fc : unsigned : 20.12 format
    0xd5000049, 0x801ede35, 
    // eq_channel_c_band_5_fc : unsigned : 20.12 format
    0xd5000052, 0x80096737, 
    // eq_channel_c_band_5_gain : signed : 6.26 format
    0xd5000054, 0x08a7a630, 
    // eq_channel_c_band_7_fc : unsigned : 20.12 format
    0xd5000058, 0x8119023e,  
    // eq_channel_c_band_7_gain : signed : 6.26 format
    0xd500005a, 0xf2f09420, 
    // eq_channel_c_band_8_fc : unsigned : 20.12 format
    0xd500005b, 0x82231c28, 
    // eq_channel_c_band_8_gain : signed : 6.26 format
    0xd500005d, 0xd92a2080, 
    // eq_channel_c_band_10_fc : unsigned : 20.12 format
    0xd5000061, 0x848be800, 
    // eq_channel_c_band_10_gain : signed : 6.26 format
    0xd5000063, 0xe8a64ec0, 
    // filter_highpass_transposed_direct_II (High Pass)
    // a_1 : signed : 1.31 format
    0xf000003f, 0x7e960d88, 
    // a_2 : signed : 1.31 format
    0xf0000042, 0x00000000, 
    // c_2 : signed : 32.0 format
    0xf0000044, 0xffffffff, 
    // a_1 : signed : 1.31 format
    0xf0000049, 0x7e960d88, 
    // a_2 : signed : 1.31 format
    0xf000004c, 0x00000000, 
    // c_2 : signed : 32.0 format
    0xf000004e, 0xffffffff, 
    // a_1 : signed : 1.31 format
    0xf0000053, 0x7e5a1e44, 
    // a_2 : signed : 1.31 format
    0xf0000056, 0x00000000, 
    // c_2 : signed : 32.0 format
    0xf0000058, 0xffffffff, 
    // a_1 : signed : 1.31 format
    0xf000005d, 0x7e960d88, 
    // a_2 : signed : 1.31 format
    0xf0000060, 0x00000000, 
    // c_2 : signed : 32.0 format
    0xf0000062, 0xffffffff, 
    // a_1 : signed : 1.31 format
    0xf0000067, 0x7e960d88, 
    // a_2 : signed : 1.31 format
    0xf000006a, 0x00000000, 
    // c_2 : signed : 32.0 format
    0xf000006c, 0xffffffff, 
    // dynamic_hard_limiter2 (Hard Limiter)
    // dsp_attack : signed : 1.31 format
    0xf0000093, 0x08414ea2, 
    // dsp_attack : signed : 1.31 format
    0xf0000083, 0x08414ea2, 
    // dsp_attack : signed : 1.31 format
    0xf000008b, 0x08414ea2,     
    // threshold : signed : 9.23 format
    0xf0000078, 0x01999980,     
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
    // eq_channel_l_band_6_fc : unsigned : 20.12 format
    0xd5000034, 0x82ffce68, 
    // eq_channel_l_band_6_gain : signed : 6.26 format
    0xd5000036, 0xdec8e8c0, 
    // eq_channel_l_band_7_fc : unsigned : 20.12 format
    0xd5000037, 0x823abd20, 
    // eq_channel_c_band_1_fc : unsigned : 20.12 format
    0xd5000046, 0x800e3e35, 
    // eq_channel_c_band_1_gain : signed : 6.26 format
    0xd5000048, 0x10cf6400, 
    // eq_channel_c_band_2_fc : unsigned : 20.12 format
    0xd5000049, 0x801ede35, 
    // eq_channel_c_band_5_fc : unsigned : 20.12 format
    0xd5000052, 0x80096737, 
    // eq_channel_c_band_5_gain : signed : 6.26 format
    0xd5000054, 0x08a7a630, 
    // eq_channel_c_band_7_fc : unsigned : 20.12 format
    0xd5000058, 0x8119023e,  
    // eq_channel_c_band_7_gain : signed : 6.26 format
    0xd500005a, 0xf2f09420, 
    // eq_channel_c_band_8_fc : unsigned : 20.12 format
    0xd500005b, 0x82231c28, 
    // eq_channel_c_band_8_gain : signed : 6.26 format
    0xd500005d, 0xd92a2080, 
    // eq_channel_c_band_10_fc : unsigned : 20.12 format
    0xd5000061, 0x848be800, 
    // eq_channel_c_band_10_gain : signed : 6.26 format
    0xd5000063, 0xe8a64ec0, 
    // filter_highpass_transposed_direct_II (High Pass)
    // a_1 : signed : 1.31 format
    0xf000003f, 0x7e960d88, 
    // a_2 : signed : 1.31 format
    0xf0000042, 0x00000000, 
    // c_2 : signed : 32.0 format
    0xf0000044, 0xffffffff, 
    // a_1 : signed : 1.31 format
    0xf0000049, 0x7e960d88, 
    // a_2 : signed : 1.31 format
    0xf000004c, 0x00000000, 
    // c_2 : signed : 32.0 format
    0xf000004e, 0xffffffff, 
    // a_1 : signed : 1.31 format
    0xf0000053, 0x7e5a1e44, 
    // a_2 : signed : 1.31 format
    0xf0000056, 0x00000000, 
    // c_2 : signed : 32.0 format
    0xf0000058, 0xffffffff, 
    // a_1 : signed : 1.31 format
    0xf000005d, 0x7e960d88, 
    // a_2 : signed : 1.31 format
    0xf0000060, 0x00000000, 
    // c_2 : signed : 32.0 format
    0xf0000062, 0xffffffff, 
    // a_1 : signed : 1.31 format
    0xf0000067, 0x7e960d88, 
    // a_2 : signed : 1.31 format
    0xf000006a, 0x00000000, 
    // c_2 : signed : 32.0 format
    0xf000006c, 0xffffffff, 
    // dynamic_hard_limiter2 (Hard Limiter)
    // dsp_attack : signed : 1.31 format
    0xf0000093, 0x08414ea2, 
    // dsp_attack : signed : 1.31 format
    0xf0000083, 0x08414ea2, 
    // dsp_attack : signed : 1.31 format
    0xf000008b, 0x08414ea2, 
    // threshold : signed : 9.23 format
    0xf0000078, 0x01999980,  
};

#endif /* __S3851_DSP_TVHDMC_PARMS_H__ */
