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
    // mpm_b_tshd4 (TSHD4) 
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x7fffffff, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000004
};

static const uint32 code AC3_2_1_TVOLOFF [] = 
{
    /* ac3-2.1-TvolOFF_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000000, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000001, 
    // mpm_b_tshd4 (TSHD4) 
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x7fffffff, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000004
};

static const uint32 code AC3_5_1_TVOLON [] = 
{
    /* ac3-5.1-TvolON_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000001, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000000, 
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x00000000, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000008
};

static const uint32 code AC3_5_1_TVOLOFF [] = 
{
    /* ac3-2.1-TvolOFF_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000000, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000001, 
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x00000000, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000008
};

static const uint32 code DTS_2_1_TVOLOFF [] = 
{
    /* dts-2.1-TvolOFF_1218.cfg */
    // TVHDMC_ENABLE : unsigned : 32.0 format
    0xe0000000, 0x00000000, 
    // TVHDMC_BYPASS : unsigned : 32.0 format
    0xe0000001, 0x00000001, 
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x7fffffff, 
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
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x7fffffff, 
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
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x00000000, 
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
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x00000000, 
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
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x7fffffff, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000004, 
    // FRONT_TRUBASS_CONTROL : signed : 1.31 format
    0xb300000e, 0x2ea59840, 
    // SUB_TRUBASS_SPEAKER_SIZE : unsigned : 32.0 format
    0xb3000010, 0x00000001, 
};

static const uint32 code _2_1_TRUVOLON[] = 
{
    /* pcm-2.1-TvolON_1218.cfg */
    // tv_enable : unsigned : 32.0 format
    0xe0000000, 0x00000001, 
    // tv_bypass : unsigned : 32.0 format
    0xe0000001, 0x00000000,
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x7fffffff, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000004, 
    // FRONT_TRUBASS_CONTROL : signed : 1.31 format
    0xb300000e, 0x2ea59840, 
    // SUB_TRUBASS_SPEAKER_SIZE : unsigned : 32.0 format
    0xb3000010, 0x00000001, 
};

static const uint32 code _5_1_TRUVOLON[] = 
{
    // tv_2_1_4953X (SRS TV)
    //
    // tv_enable : unsigned : 32.0 format
    0xe0000000, 0x00000001, 
    // tv_bypass : unsigned : 32.0 format
    0xe0000001, 0x00000000, 
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x00000000, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000008, 
    // FRONT_TRUBASS_CONTROL : signed : 1.31 format
    0xb300000e, 0x2e978d40, 
    // SUB_TRUBASS_SPEAKER_SIZE : unsigned : 32.0 format
    0xb3000010, 0x00000000, 
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
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x00000000, 
    // _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
    0xb3000005, 0x00000008, 
    // FRONT_TRUBASS_CONTROL : signed : 1.31 format
    0xb300000e, 0x2e978d40, 
    // SUB_TRUBASS_SPEAKER_SIZE : unsigned : 32.0 format
    0xb3000010, 0x00000000, 
};

#endif /* __S3851_DSP_TVHDMC_PARMS_H__ */
