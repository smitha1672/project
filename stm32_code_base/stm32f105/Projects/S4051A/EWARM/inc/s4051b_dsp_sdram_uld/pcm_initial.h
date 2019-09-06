#ifndef PCM_51_INITIAL_H__
#define PCM_51_INITIAL_H__

/***********************************************
This file is created by cirrus dsp tool on 01/07/15 15:17:56
This tool was written by XuShoucai on 06/01/09
************************************************/
static const uint32 code PCM_INITIAL[] = 
{
/* pcm-5.1-TvolOFF_0107.cfg */
// Initial control values
// Created : 
//include pcm-5.1-TvolOFF_0107\system_pcm.cfg
//
// system_pcm (PCM)
//
// pcm_enable : unsigned : 32.0 format
  0x9b000000, 0x00000001, 
// io_buff_0_source : unsigned : 32.0 format
  0x9b000001, 0x00000000, 
// io_buff_1_source : unsigned : 32.0 format
  0x9b000002, 0x00000004, 
// io_buff_2_source : unsigned : 32.0 format
  0x9b000003, 0x00000001, 
// io_buff_3_source : unsigned : 32.0 format
  0x9b000004, 0x00000002, 
// io_buff_4_source : unsigned : 32.0 format
  0x9b000005, 0x00000003, 
// io_buff_5_source : unsigned : 32.0 format
  0x9b000006, 0x00000006, 
// io_buff_6_source : unsigned : 32.0 format
  0x9b000007, 0x00000007, 
// io_buff_7_source : unsigned : 32.0 format
  0x9b000008, 0x00000005, 
// io_buff_8_source : unsigned : 32.0 format
  0x9b000009, 0x08000000, 
// io_buff_9_source : unsigned : 32.0 format
  0x9b00000a, 0x08000000, 
// io_buff_10_source : unsigned : 32.0 format
  0x9b00000b, 0x08000000, 
// io_buff_11_source : unsigned : 32.0 format
  0x9b00000c, 0x08000000, 
// io_buff_12_source : unsigned : 32.0 format
  0x9b00000d, 0x08000000, 
// io_buff_13_source : unsigned : 32.0 format
  0x9b00000e, 0x08000000, 
// io_buff_14_source : unsigned : 32.0 format
  0x9b00000f, 0x00000000, 
// io_buff_15_source : unsigned : 32.0 format
  0x9b000010, 0x00000001, 
//include pcm-5.1-TvolOFF_0107\cs495314.cfg
//
// cs495314 (CS495314)
//
// output_mode : unsigned : 32.0 format
  0x81000002, 0x00000007, 
// sample_rate : unsigned : 32.0 format
  0x81000003, 0x00000000, 
// autodetect_config : unsigned : 32.0 format
  0x81000005, 0x00000001, 
// autoswitch_enable : unsigned : 32.0 format
  0x81000006, 0x00000001, 
// silence_threshold : unsigned : 32.0 format
  0x81000008, 0x0000bb80,  /*Smith 23 July, 13 fix ps3 click issue thershold value is 90 seconds (0x0083d600) or 2.5 (0x0000bb80) seconds*/
// output_mode_b : unsigned : 32.0 format
  0x8100000a, 0x00000007, 
// dspb_output_mode_enable : unsigned : 32.0 format
  0x8100000b, 0x00000000, 
// dao1_chan_0_remap : unsigned : 32.0 format
  0x83000012, 0x00000000, 
// dao1_chan_1_remap : unsigned : 32.0 format
  0x83000013, 0x00000002, 
// dao1_chan_2_remap : unsigned : 32.0 format
  0x83000014, 0x00000001, 
// dao1_chan_3_remap : unsigned : 32.0 format
  0x83000015, 0x00000001, 
// dao1_chan_4_remap : unsigned : 32.0 format
  0x83000016, 0x00000007, 
// dao1_chan_5_remap : unsigned : 32.0 format
  0x83000017, 0x00000007, 
// dao1_chan_6_remap : unsigned : 32.0 format
  0x83000018, 0x00000003, 
// dao1_chan_7_remap : unsigned : 32.0 format
  0x83000019, 0x00000004, 
// dao2_chan_0_remap : unsigned : 32.0 format
  0x8300001a, 0x0000000c, 
// dao2_chan_1_remap : unsigned : 32.0 format
  0x8300001b, 0x0000000d, 
// dao2_chan_2_remap : unsigned : 32.0 format
  0x8300001c, 0x00000008, 
// dao2_chan_3_remap : unsigned : 32.0 format
  0x8300001d, 0x00000009, 
// dao2_chan_4_remap : unsigned : 32.0 format
  0x8300001e, 0x0000000a, 
// dao2_chan_5_remap : unsigned : 32.0 format
  0x8300001f, 0x0000000b, 
// dao2_chan_6_remap : unsigned : 32.0 format
  0x83000020, 0x0000000e, 
// dao2_chan_7_remap : unsigned : 32.0 format
  0x83000021, 0x0000000f, 
//include pcm-5.1-TvolOFF_0107\tv_2_1_4953X.cfg
//
// tv_2_1_4953X (SRS TV)
//
// tv_enable : unsigned : 32.0 format
  0xe0000000, 0x00000000, 
// tv_bypass : unsigned : 32.0 format
  0xe0000001, 0x00000001, 
// tv_in_gain : signed : 6.26 format
  0xe0000002, 0x04000000, 
// tv_out_gain : signed : 6.26 format
  0xe0000003, 0x013126ea, 
// tv_bypass_gain : signed : 1.31 format
  0xe0000004, 0x7fffffff, 
// channel_mode : unsigned : 32.0 format
  0xe0000006, 0x00000000, 
//include pcm-5.1-TvolOFF_0107\mpm_a_cs2_cs4953X.cfg
//
// mpm_a_cs2_cs4953X (CS II)
//
// control : unsigned : 32.0 format
  0xb7000000, 0x00000001, 
// mode : unsigned : 32.0 format
  0xb7000001, 0x00000000, 
// centerfb : unsigned : 32.0 format
  0xb7000002, 0x00000001, 
// mode_525 : unsigned : 32.0 format
  0xb7000003, 0x00000001, 
// sub_enable : unsigned : 32.0 format
  0xb7000004, 0x00000000, 
// front_enable : unsigned : 32.0 format
  0xb7000005, 0x00000000, 
// focus_enable : unsigned : 32.0 format
  0xb7000006, 0x00000000, 
// focus_elevation : signed : 1.31 format
  0xb7000007, 0x7fffffff, 
// rear_centerfb : unsigned : 32.0 format
  0xb7000008, 0x00000001, 
// input_gain : signed : 1.31 format
  0xb700000a, 0x7fffffff, 
// trubass_mix : signed : 1.31 format
  0xb700000b, 0x199999a0, 
// speaker_size : unsigned : 32.0 format
  0xb700000c, 0x00000001, 
//include pcm-5.1-TvolOFF_0107\mpm_b_tshd4.cfg
//
// mpm_b_tshd4 (TSHD4)
//
// tshd4_enable : unsigned : 32.0 format
  0xb3000000, 0x00000001, 
// tshd4_bypass : unsigned : 32.0 format
  0xb3000001, 0x00000000, 
// TSHD_LEVEL_CONTROL : signed : 1.31 format
  0xb3000002, 0x00000000, 
// TSHD_INPUT_MODE : unsigned : 32.0 format
  0xb3000003, 0x00000006, 
// TRUBASS_CROSSOVER_FREQ : unsigned : 32.0 format
  0xb3000004, 0x00000000, 
// _TSHD_TSHD4_OUTPUT_MODE : unsigned : 32.0 format
  0xb3000005, 0x00000008, 
// DIALOG_CLARITY_ENABLE : unsigned : 32.0 format
  0xb3000006, 0x00000001, 
// DIALOG_CLARITY_CONTROL : signed : 1.31 format
  0xb3000007, 0x1b645a20, 
// FRONT_DEFINITION_ENABLE : unsigned : 32.0 format
  0xb3000008, 0x00000001, 
// FRONT_DEFINITION_CONTROL : signed : 1.31 format
  0xb3000009, 0x083126f0, 
// CENTER_DEFINITION_ENABLE : unsigned : 32.0 format
  0xb300000a, 0x00000001, 
// CENTER_DEFINITION_CONTROL : signed : 1.31 format
  0xb300000b, 0x09168730, 
// FRONT_TRUBASS_ENABLE : unsigned : 32.0 format
  0xb300000c, 0x00000001, 
// FRONT_TRUBASS_SPEAKER_SIZE : unsigned : 32.0 format
  0xb300000d, 0x00000002, 
// FRONT_TRUBASS_CONTROL : signed : 1.31 format
  0xb300000e, 0x2e978d40, 
// SUB_TRUBASS_ENABLE : unsigned : 32.0 format
  0xb300000f, 0x00000001, 
// SUB_TRUBASS_SPEAKER_SIZE : unsigned : 32.0 format
  0xb3000010, 0x00000000, 
// SUB_TRUBASS_CONTROL : signed : 1.31 format
  0xb3000011, 0x33333340, 
// input_gain : signed : 1.31 format
  0xb3000012, 0x33333340, 
// output_gain : signed : 1.31 format
  0xb3000013, 0x7fffffff, 
// bypass_gain : signed : 1.31 format
  0xb3000014, 0x40000000, 
// MONO_2_STEREO : unsigned : 32.0 format
  0xb3000015, 0x00000000, 
// focus_enable : unsigned : 32.0 format
  0xb3000016, 0x00000000, 
// focus_bypass : unsigned : 32.0 format
  0xb3000017, 0x00000001, 
// focus_enable_ch0 : unsigned : 32.0 format
  0xb3000018, 0x00000001, 
// focus_enable_ch1 : unsigned : 32.0 format
  0xb3000019, 0x00000001, 
// focus_enable_ch2 : unsigned : 32.0 format
  0xb300001a, 0x00000001, 
// focus_enable_ch3 : unsigned : 32.0 format
  0xb300001b, 0x00000001, 
// focus_enable_ch4 : unsigned : 32.0 format
  0xb300001c, 0x00000001, 
// focus_input_gain : signed : 1.31 format
  0xb300001d, 0x40000000, 
// focus_output_gain : signed : 1.31 format
  0xb300001e, 0x7fffffff, 
// focus_bypass_gain : signed : 1.31 format
  0xb300001f, 0x5a7efa00, 
// focus_control : signed : 1.31 format
  0xb3000020, 0x00000000, 
// X_VY_HOST_SRS_TRU_BASS_MODE : unsigned : 32.0 format
  0xb3000021, 0x00000000, 
// TSHD_NO_VIRTUALISATION : unsigned : 32.0 format
  0xb3000023, 0x00000000, 
//include pcm-5.1-TvolOFF_0107\mpm_b_wow.cfg
//
// mpm_b_wow (WOW HD)
//
// wow_enable : unsigned : 32.0 format
  0xb4000000, 0x00000001, 
// wow_high_bitrate : unsigned : 32.0 format
  0xb4000001, 0x00000001, 
// wow_mode : unsigned : 32.0 format
  0xb4000002, 0x00000002, 
// wow_center_control : signed : 1.31 format
  0xb4000003, 0x40000000, 
// wow_space_control : signed : 1.31 format
  0xb4000004, 0x66666680, 
// limiter_enable : unsigned : 32.0 format
  0xb4000005, 0x00000001, 
// wow_limiter_control : signed : 1.31 format
  0xb4000006, 0x60000000, 
// focus_enable : unsigned : 32.0 format
  0xb4000007, 0x00000001, 
// wow_focus_control : signed : 1.31 format
  0xb4000008, 0x33333340, 
//include pcm-5.1-TvolOFF_0107\eq_proc.cfg
//
// eq_proc (PEQ)
//
// eq_control : unsigned : 32.0 format
  0xd5000000, 0x8000009f, 
// eq_flash : unsigned : 32.0 format
  0xd5000004, 0x00000000, 
// eq_chl_preattenuation : signed : 1.31 format
  0xd5000005, 0x80000000, 
// eq_chl_postgain : signed : 9.23 format
  0xd5000006, 0xff800000, 
// eq_chc_preattenuation : signed : 1.31 format
  0xd5000007, 0x80000000, 
// eq_chc_postgain : signed : 9.23 format
  0xd5000008, 0xff800000, 
// eq_chr_preattenuation : signed : 1.31 format
  0xd5000009, 0x80000000, 
// eq_chr_postgain : signed : 9.23 format
  0xd500000a, 0xff800000, 
// eq_chls_preattenuation : signed : 1.31 format
  0xd500000b, 0x80000000, 
// eq_chls_postgain : signed : 9.23 format
  0xd500000c, 0xff800000, 
// eq_chrs_preattenuation : signed : 1.31 format
  0xd500000d, 0x80000000, 
// eq_chrs_postgain : signed : 9.23 format
  0xd500000e, 0xff800000, 
// eq_chlfe0_preattenuation : signed : 1.31 format
  0xd500000f, 0x80000000, 
// eq_chlfe0_postgain : signed : 9.23 format
  0xd5000010, 0xff800000, 
// eq_channel_l_band_1_fc : unsigned : 20.12 format
  0xd5000025, 0x81b58000, 
// eq_channel_l_band_1_q : unsigned : 32.0 format
  0xd5000026, 0x04000000, 
// eq_channel_l_band_1_gain : signed : 6.26 format
  0xd5000027, 0x10000000, 
// eq_channel_l_band_2_fc : unsigned : 20.12 format
  0xd5000028, 0x83138000, 
// eq_channel_l_band_2_q : unsigned : 32.0 format
  0xd5000029, 0x0c000000, 
// eq_channel_l_band_2_gain : signed : 6.26 format
  0xd500002a, 0xec000000, 
// eq_channel_l_band_3_fc : unsigned : 20.12 format
  0xd500002b, 0x805dc000, 
// eq_channel_l_band_3_q : unsigned : 32.0 format
  0xd500002c, 0x08000000, 
// eq_channel_l_band_3_gain : signed : 6.26 format
  0xd500002d, 0x00000000, 
// eq_channel_l_band_4_fc : unsigned : 20.12 format
  0xd500002e, 0x80384000, 
// eq_channel_l_band_4_q : unsigned : 32.0 format
  0xd500002f, 0x03333334, 
// eq_channel_l_band_4_gain : signed : 6.26 format
  0xd5000030, 0xf0000000, 
// eq_channel_l_band_5_fc : unsigned : 20.12 format
  0xd5000031, 0x80ce4000, 
// eq_channel_l_band_5_q : unsigned : 32.0 format
  0xd5000032, 0x08000000, 
// eq_channel_l_band_5_gain : signed : 6.26 format
  0xd5000033, 0xf4000000, 
// eq_channel_l_band_6_fc : unsigned : 20.12 format
  0xd5000034, 0x800c8000, 
// eq_channel_l_band_6_q : unsigned : 32.0 format
  0xd5000035, 0x04000000, 
// eq_channel_l_band_6_gain : signed : 6.26 format
  0xd5000036, 0x0c000000, 
// eq_channel_l_band_7_fc : unsigned : 20.12 format
  0xd5000037, 0x800c8000, 
// eq_channel_l_band_7_q : unsigned : 32.0 format
  0xd5000038, 0x14000000, 
// eq_channel_l_band_7_gain : signed : 6.26 format
  0xd5000039, 0xe8000000, 
// eq_channel_l_band_8_fc : unsigned : 20.12 format
  0xd500003a, 0x80000000, 
// eq_channel_l_band_8_q : unsigned : 32.0 format
  0xd500003b, 0x00000000, 
// eq_channel_l_band_8_gain : signed : 6.26 format
  0xd500003c, 0x00000000, 
// eq_channel_l_band_9_fc : unsigned : 20.12 format
  0xd500003d, 0x80000000, 
// eq_channel_l_band_9_q : unsigned : 32.0 format
  0xd500003e, 0x00000000, 
// eq_channel_l_band_9_gain : signed : 6.26 format
  0xd500003f, 0x00000000, 
// eq_channel_l_band_10_fc : unsigned : 20.12 format
  0xd5000040, 0x80000000, 
// eq_channel_l_band_10_q : unsigned : 32.0 format
  0xd5000041, 0x00000000, 
// eq_channel_l_band_10_gain : signed : 6.26 format
  0xd5000042, 0x00000000, 
// eq_channel_l_band_11_fc : unsigned : 20.12 format
  0xd5000043, 0x80000000, 
// eq_channel_l_band_11_q : unsigned : 32.0 format
  0xd5000044, 0x00000000, 
// eq_channel_l_band_11_gain : signed : 6.26 format
  0xd5000045, 0x00000000, 
// eq_channel_c_band_1_fc : unsigned : 20.12 format
  0xd5000046, 0x81b58000, 
// eq_channel_c_band_1_q : unsigned : 32.0 format
  0xd5000047, 0x04000000, 
// eq_channel_c_band_1_gain : signed : 6.26 format
  0xd5000048, 0x10000000, 
// eq_channel_c_band_2_fc : unsigned : 20.12 format
  0xd5000049, 0x83138000, 
// eq_channel_c_band_2_q : unsigned : 32.0 format
  0xd500004a, 0x0c000000, 
// eq_channel_c_band_2_gain : signed : 6.26 format
  0xd500004b, 0xec000000, 
// eq_channel_c_band_3_fc : unsigned : 20.12 format
  0xd500004c, 0x805dc000, 
// eq_channel_c_band_3_q : unsigned : 32.0 format
  0xd500004d, 0x08000000, 
// eq_channel_c_band_3_gain : signed : 6.26 format
  0xd500004e, 0x00000000, 
// eq_channel_c_band_4_fc : unsigned : 20.12 format
  0xd500004f, 0x80384000, 
// eq_channel_c_band_4_q : unsigned : 32.0 format
  0xd5000050, 0x03333334, 
// eq_channel_c_band_4_gain : signed : 6.26 format
  0xd5000051, 0xec000000, 
// eq_channel_c_band_5_fc : unsigned : 20.12 format
  0xd5000052, 0x80ce4000, 
// eq_channel_c_band_5_q : unsigned : 32.0 format
  0xd5000053, 0x08000000, 
// eq_channel_c_band_5_gain : signed : 6.26 format
  0xd5000054, 0xec000000, 
// eq_channel_c_band_6_fc : unsigned : 20.12 format
  0xd5000055, 0x800c8000, 
// eq_channel_c_band_6_q : unsigned : 32.0 format
  0xd5000056, 0x04000000, 
// eq_channel_c_band_6_gain : signed : 6.26 format
  0xd5000057, 0x0c000000, 
// eq_channel_c_band_7_fc : unsigned : 20.12 format
  0xd5000058, 0x800c8000, 
// eq_channel_c_band_7_q : unsigned : 32.0 format
  0xd5000059, 0x14000000, 
// eq_channel_c_band_7_gain : signed : 6.26 format
  0xd500005a, 0xe8000000, 
// eq_channel_c_band_8_fc : unsigned : 20.12 format
  0xd500005b, 0x80000000, 
// eq_channel_c_band_8_q : unsigned : 32.0 format
  0xd500005c, 0x00000000, 
// eq_channel_c_band_8_gain : signed : 6.26 format
  0xd500005d, 0x00000000, 
// eq_channel_c_band_9_fc : unsigned : 20.12 format
  0xd500005e, 0x80000000, 
// eq_channel_c_band_9_q : unsigned : 32.0 format
  0xd500005f, 0x00000000, 
// eq_channel_c_band_9_gain : signed : 6.26 format
  0xd5000060, 0x00000000, 
// eq_channel_c_band_10_fc : unsigned : 20.12 format
  0xd5000061, 0x80000000, 
// eq_channel_c_band_10_q : unsigned : 32.0 format
  0xd5000062, 0x00000000, 
// eq_channel_c_band_10_gain : signed : 6.26 format
  0xd5000063, 0x00000000, 
// eq_channel_c_band_11_fc : unsigned : 20.12 format
  0xd5000064, 0x80000000, 
// eq_channel_c_band_11_q : unsigned : 32.0 format
  0xd5000065, 0x00000000, 
// eq_channel_c_band_11_gain : signed : 6.26 format
  0xd5000066, 0x00000000, 
// eq_channel_r_band_1_fc : unsigned : 20.12 format
  0xd5000067, 0x81b58000, 
// eq_channel_r_band_1_q : unsigned : 32.0 format
  0xd5000068, 0x04000000, 
// eq_channel_r_band_1_gain : signed : 6.26 format
  0xd5000069, 0x10000000, 
// eq_channel_r_band_2_fc : unsigned : 20.12 format
  0xd500006a, 0x83138000, 
// eq_channel_r_band_2_q : unsigned : 32.0 format
  0xd500006b, 0x0c000000, 
// eq_channel_r_band_2_gain : signed : 6.26 format
  0xd500006c, 0xec000000, 
// eq_channel_r_band_3_fc : unsigned : 20.12 format
  0xd500006d, 0x805dc000, 
// eq_channel_r_band_3_q : unsigned : 32.0 format
  0xd500006e, 0x08000000, 
// eq_channel_r_band_3_gain : signed : 6.26 format
  0xd500006f, 0x00000000, 
// eq_channel_r_band_4_fc : unsigned : 20.12 format
  0xd5000070, 0x80384000, 
// eq_channel_r_band_4_q : unsigned : 32.0 format
  0xd5000071, 0x03333334, 
// eq_channel_r_band_4_gain : signed : 6.26 format
  0xd5000072, 0xf0000000, 
// eq_channel_r_band_5_fc : unsigned : 20.12 format
  0xd5000073, 0x80ce4000, 
// eq_channel_r_band_5_q : unsigned : 32.0 format
  0xd5000074, 0x08000000, 
// eq_channel_r_band_5_gain : signed : 6.26 format
  0xd5000075, 0xf4000000, 
// eq_channel_r_band_6_fc : unsigned : 20.12 format
  0xd5000076, 0x800c8000, 
// eq_channel_r_band_6_q : unsigned : 32.0 format
  0xd5000077, 0x04000000, 
// eq_channel_r_band_6_gain : signed : 6.26 format
  0xd5000078, 0x0c000000, 
// eq_channel_r_band_7_fc : unsigned : 20.12 format
  0xd5000079, 0x800c8000, 
// eq_channel_r_band_7_q : unsigned : 32.0 format
  0xd500007a, 0x14000000, 
// eq_channel_r_band_7_gain : signed : 6.26 format
  0xd500007b, 0xe8000000, 
// eq_channel_r_band_8_fc : unsigned : 20.12 format
  0xd500007c, 0x80000000, 
// eq_channel_r_band_8_q : unsigned : 32.0 format
  0xd500007d, 0x00000000, 
// eq_channel_r_band_8_gain : signed : 6.26 format
  0xd500007e, 0x00000000, 
// eq_channel_r_band_9_fc : unsigned : 20.12 format
  0xd500007f, 0x80000000, 
// eq_channel_r_band_9_q : unsigned : 32.0 format
  0xd5000080, 0x00000000, 
// eq_channel_r_band_9_gain : signed : 6.26 format
  0xd5000081, 0x00000000, 
// eq_channel_r_band_10_fc : unsigned : 20.12 format
  0xd5000082, 0x80000000, 
// eq_channel_r_band_10_q : unsigned : 32.0 format
  0xd5000083, 0x00000000, 
// eq_channel_r_band_10_gain : signed : 6.26 format
  0xd5000084, 0x00000000, 
// eq_channel_r_band_11_fc : unsigned : 20.12 format
  0xd5000085, 0x80000000, 
// eq_channel_r_band_11_q : unsigned : 32.0 format
  0xd5000086, 0x00000000, 
// eq_channel_r_band_11_gain : signed : 6.26 format
  0xd5000087, 0x00000000, 
// eq_channel_ls_band_1_fc : unsigned : 20.12 format
  0xd5000088, 0x80c80000, 
// eq_channel_ls_band_1_q : unsigned : 32.0 format
  0xd5000089, 0x06000000, 
// eq_channel_ls_band_1_gain : signed : 6.26 format
  0xd500008a, 0xe8000000, 
// eq_channel_ls_band_2_fc : unsigned : 20.12 format
  0xd500008b, 0x83714000, 
// eq_channel_ls_band_2_q : unsigned : 32.0 format
  0xd500008c, 0x08000000, 
// eq_channel_ls_band_2_gain : signed : 6.26 format
  0xd500008d, 0xd0000000, 
// eq_channel_ls_band_3_fc : unsigned : 20.12 format
  0xd500008e, 0x800df000, 
// eq_channel_ls_band_3_q : unsigned : 32.0 format
  0xd500008f, 0x08000000, 
// eq_channel_ls_band_3_gain : signed : 6.26 format
  0xd5000090, 0xf0000000, 
// eq_channel_ls_band_4_fc : unsigned : 20.12 format
  0xd5000091, 0x803e8000, 
// eq_channel_ls_band_4_q : unsigned : 32.0 format
  0xd5000092, 0x04000000, 
// eq_channel_ls_band_4_gain : signed : 6.26 format
  0xd5000093, 0xf0000000, 
// eq_channel_ls_band_5_fc : unsigned : 20.12 format
  0xd5000094, 0x834bc000, 
// eq_channel_ls_band_5_q : unsigned : 32.0 format
  0xd5000095, 0x10000000, 
// eq_channel_ls_band_5_gain : signed : 6.26 format
  0xd5000096, 0xe0000000, 
// eq_channel_ls_band_6_fc : unsigned : 20.12 format
  0xd5000097, 0x80000000, 
// eq_channel_ls_band_6_q : unsigned : 32.0 format
  0xd5000098, 0x00000000, 
// eq_channel_ls_band_6_gain : signed : 6.26 format
  0xd5000099, 0x00000000, 
// eq_channel_ls_band_7_fc : unsigned : 20.12 format
  0xd500009a, 0x80000000, 
// eq_channel_ls_band_7_q : unsigned : 32.0 format
  0xd500009b, 0x00000000, 
// eq_channel_ls_band_7_gain : signed : 6.26 format
  0xd500009c, 0x00000000, 
// eq_channel_ls_band_8_fc : unsigned : 20.12 format
  0xd500009d, 0x80000000, 
// eq_channel_ls_band_8_q : unsigned : 32.0 format
  0xd500009e, 0x00000000, 
// eq_channel_ls_band_8_gain : signed : 6.26 format
  0xd500009f, 0x00000000, 
// eq_channel_ls_band_9_fc : unsigned : 20.12 format
  0xd50000a0, 0x80000000, 
// eq_channel_ls_band_9_q : unsigned : 32.0 format
  0xd50000a1, 0x00000000, 
// eq_channel_ls_band_9_gain : signed : 6.26 format
  0xd50000a2, 0x00000000, 
// eq_channel_ls_band_10_fc : unsigned : 20.12 format
  0xd50000a3, 0x80000000, 
// eq_channel_ls_band_10_q : unsigned : 32.0 format
  0xd50000a4, 0x00000000, 
// eq_channel_ls_band_10_gain : signed : 6.26 format
  0xd50000a5, 0x00000000, 
// eq_channel_ls_band_11_fc : unsigned : 20.12 format
  0xd50000a6, 0x80000000, 
// eq_channel_ls_band_11_q : unsigned : 32.0 format
  0xd50000a7, 0x00000000, 
// eq_channel_ls_band_11_gain : signed : 6.26 format
  0xd50000a8, 0x00000000, 
// eq_channel_rs_band_1_fc : unsigned : 20.12 format
  0xd50000a9, 0x80c80000, 
// eq_channel_rs_band_1_q : unsigned : 32.0 format
  0xd50000aa, 0x06000000, 
// eq_channel_rs_band_1_gain : signed : 6.26 format
  0xd50000ab, 0xe8000000, 
// eq_channel_rs_band_2_fc : unsigned : 20.12 format
  0xd50000ac, 0x83714000, 
// eq_channel_rs_band_2_q : unsigned : 32.0 format
  0xd50000ad, 0x08000000, 
// eq_channel_rs_band_2_gain : signed : 6.26 format
  0xd50000ae, 0xd0000000, 
// eq_channel_rs_band_3_fc : unsigned : 20.12 format
  0xd50000af, 0x800df000, 
// eq_channel_rs_band_3_q : unsigned : 32.0 format
  0xd50000b0, 0x08000000, 
// eq_channel_rs_band_3_gain : signed : 6.26 format
  0xd50000b1, 0xf0000000, 
// eq_channel_rs_band_4_fc : unsigned : 20.12 format
  0xd50000b2, 0x803e8000, 
// eq_channel_rs_band_4_q : unsigned : 32.0 format
  0xd50000b3, 0x04000000, 
// eq_channel_rs_band_4_gain : signed : 6.26 format
  0xd50000b4, 0xf0000000, 
// eq_channel_rs_band_5_fc : unsigned : 20.12 format
  0xd50000b5, 0x834bc000, 
// eq_channel_rs_band_5_q : unsigned : 32.0 format
  0xd50000b6, 0x10000000, 
// eq_channel_rs_band_5_gain : signed : 6.26 format
  0xd50000b7, 0xe0000000, 
// eq_channel_rs_band_6_fc : unsigned : 20.12 format
  0xd50000b8, 0x80000000, 
// eq_channel_rs_band_6_q : unsigned : 32.0 format
  0xd50000b9, 0x00000000, 
// eq_channel_rs_band_6_gain : signed : 6.26 format
  0xd50000ba, 0x00000000, 
// eq_channel_rs_band_7_fc : unsigned : 20.12 format
  0xd50000bb, 0x80000000, 
// eq_channel_rs_band_7_q : unsigned : 32.0 format
  0xd50000bc, 0x00000000, 
// eq_channel_rs_band_7_gain : signed : 6.26 format
  0xd50000bd, 0x00000000, 
// eq_channel_rs_band_8_fc : unsigned : 20.12 format
  0xd50000be, 0x80000000, 
// eq_channel_rs_band_8_q : unsigned : 32.0 format
  0xd50000bf, 0x00000000, 
// eq_channel_rs_band_8_gain : signed : 6.26 format
  0xd50000c0, 0x00000000, 
// eq_channel_rs_band_9_fc : unsigned : 20.12 format
  0xd50000c1, 0x80000000, 
// eq_channel_rs_band_9_q : unsigned : 32.0 format
  0xd50000c2, 0x00000000, 
// eq_channel_rs_band_9_gain : signed : 6.26 format
  0xd50000c3, 0x00000000, 
// eq_channel_rs_band_10_fc : unsigned : 20.12 format
  0xd50000c4, 0x80000000, 
// eq_channel_rs_band_10_q : unsigned : 32.0 format
  0xd50000c5, 0x00000000, 
// eq_channel_rs_band_10_gain : signed : 6.26 format
  0xd50000c6, 0x00000000, 
// eq_channel_rs_band_11_fc : unsigned : 20.12 format
  0xd50000c7, 0x80000000, 
// eq_channel_rs_band_11_q : unsigned : 32.0 format
  0xd50000c8, 0x00000000, 
// eq_channel_rs_band_11_gain : signed : 6.26 format
  0xd50000c9, 0x00000000, 
// eq_channel_lfe0_band_1_fc : unsigned : 20.12 format
  0xd50000ca, 0x80032000, 
// eq_channel_lfe0_band_1_q : unsigned : 32.0 format
  0xd50000cb, 0x07333330, 
// eq_channel_lfe0_band_1_gain : signed : 6.26 format
  0xd50000cc, 0x10000000, 
// eq_channel_lfe0_band_2_fc : unsigned : 20.12 format
  0xd50000cd, 0x80050000, 
// eq_channel_lfe0_band_2_q : unsigned : 32.0 format
  0xd50000ce, 0x10000000, 
// eq_channel_lfe0_band_2_gain : signed : 6.26 format
  0xd50000cf, 0xf0000000, 
// eq_channel_lfe0_band_3_fc : unsigned : 20.12 format
  0xd50000d0, 0x80000000, 
// eq_channel_lfe0_band_3_q : unsigned : 32.0 format
  0xd50000d1, 0x00000000, 
// eq_channel_lfe0_band_3_gain : signed : 6.26 format
  0xd50000d2, 0x00000000, 
// eq_channel_lfe0_band_4_fc : unsigned : 20.12 format
  0xd50000d3, 0x80000000, 
// eq_channel_lfe0_band_4_q : unsigned : 32.0 format
  0xd50000d4, 0x00000000, 
// eq_channel_lfe0_band_4_gain : signed : 6.26 format
  0xd50000d5, 0x00000000, 
// eq_channel_lfe0_band_5_fc : unsigned : 20.12 format
  0xd50000d6, 0x80000000, 
// eq_channel_lfe0_band_5_q : unsigned : 32.0 format
  0xd50000d7, 0x00000000, 
// eq_channel_lfe0_band_5_gain : signed : 6.26 format
  0xd50000d8, 0x00000000, 
// eq_channel_lfe0_band_6_fc : unsigned : 20.12 format
  0xd50000d9, 0x80000000, 
// eq_channel_lfe0_band_6_q : unsigned : 32.0 format
  0xd50000da, 0x00000000, 
// eq_channel_lfe0_band_6_gain : signed : 6.26 format
  0xd50000db, 0x00000000, 
// eq_channel_lfe0_band_7_fc : unsigned : 20.12 format
  0xd50000dc, 0x80000000, 
// eq_channel_lfe0_band_7_q : unsigned : 32.0 format
  0xd50000dd, 0x00000000, 
// eq_channel_lfe0_band_7_gain : signed : 6.26 format
  0xd50000de, 0x00000000, 
// eq_channel_lfe0_band_8_fc : unsigned : 20.12 format
  0xd50000df, 0x80000000, 
// eq_channel_lfe0_band_8_q : unsigned : 32.0 format
  0xd50000e0, 0x00000000, 
// eq_channel_lfe0_band_8_gain : signed : 6.26 format
  0xd50000e1, 0x00000000, 
// eq_channel_lfe0_band_9_fc : unsigned : 20.12 format
  0xd50000e2, 0x80000000, 
// eq_channel_lfe0_band_9_q : unsigned : 32.0 format
  0xd50000e3, 0x00000000, 
// eq_channel_lfe0_band_9_gain : signed : 6.26 format
  0xd50000e4, 0x00000000, 
// eq_channel_lfe0_band_10_fc : unsigned : 20.12 format
  0xd50000e5, 0x80000000, 
// eq_channel_lfe0_band_10_q : unsigned : 32.0 format
  0xd50000e6, 0x00000000, 
// eq_channel_lfe0_band_10_gain : signed : 6.26 format
  0xd50000e7, 0x00000000, 
// eq_channel_lfe0_band_11_fc : unsigned : 20.12 format
  0xd50000e8, 0x80000000, 
// eq_channel_lfe0_band_11_q : unsigned : 32.0 format
  0xd50000e9, 0x00000000, 
// eq_channel_lfe0_band_11_gain : signed : 6.26 format
  0xd50000ea, 0x00000000, 
//include pcm-5.1-TvolOFF_0107\tone_control_proc.cfg
//
// tone_control_proc (Tone Control)
//
// control_word_0 : unsigned : 32.0 format
  0xd4000000, 0x0000003f, 
// _treble_level_left : signed : 32.0 format
  0xd4000001, 0xfffffffe, 
// _treble_level_center : signed : 32.0 format
  0xd4000002, 0xfffffffe, 
// _treble_level_right : signed : 32.0 format
  0xd4000003, 0xfffffffe, 
// _treble_level_ls : signed : 32.0 format
  0xd4000004, 0xfffffffe, 
// _treble_level_rs : signed : 32.0 format
  0xd4000005, 0xfffffffe, 
// _treble_level_lb : signed : 32.0 format
  0xd4000006, 0x00000000, 
// _treble_level_rb : signed : 32.0 format
  0xd4000007, 0x00000000, 
// _bass_level_left : signed : 32.0 format
  0xd4000008, 0x00000000, 
// _bass_level_center : signed : 32.0 format
  0xd4000009, 0x00000000, 
// _bass_level_right : signed : 32.0 format
  0xd400000a, 0x00000000, 
// _bass_level_ls : signed : 32.0 format
  0xd400000b, 0x00000000, 
// _bass_level_rs : signed : 32.0 format
  0xd400000c, 0x00000000, 
// _bass_level_lb : signed : 32.0 format
  0xd400000d, 0x00000000, 
// _bass_level_rb : signed : 32.0 format
  0xd400000e, 0x00000000, 
// _pre_attenuation_left : signed : 1.31 format
  0xd400000f, 0x7fffffff, 
// _pre_attenuation_center : signed : 1.31 format
  0xd4000010, 0x7fffffff, 
// _pre_attenuation_right : signed : 1.31 format
  0xd4000011, 0x7fffffff, 
// _pre_attenuation_ls : signed : 1.31 format
  0xd4000012, 0x7fffffff, 
// _pre_attenuation_rs : signed : 1.31 format
  0xd4000013, 0x7fffffff, 
// _pre_attenuation_lb : signed : 1.31 format
  0xd4000014, 0x00000000, 
// _pre_attenuation_rb : signed : 1.31 format
  0xd4000015, 0x00000000, 
//include pcm-5.1-TvolOFF_0107\delay_proc.cfg
//
// delay_proc (Delay Proc.)
//
// control_word_0 : unsigned : 32.0 format
  0xd9000000, 0x00000101, 
// delay_value_left : unsigned : 15.17 format
  0xd9000002, 0x00333333, 
// delay_value_center : unsigned : 15.17 format
  0xd9000003, 0x00333333, 
// delay_value_right : unsigned : 15.17 format
  0xd9000004, 0x00333333, 
// delay_value_ls : unsigned : 15.17 format
  0xd9000005, 0x00146666, 
// delay_value_rs : unsigned : 15.17 format
  0xd9000006, 0x00146666, 
// delay_value_lb : unsigned : 15.17 format
  0xd9000007, 0x00000000, 
// delay_value_rb : unsigned : 15.17 format
  0xd9000008, 0x00000000, 
// delay_value_lfe : unsigned : 15.17 format
  0xd9000009, 0x00000000, 
//include pcm-5.1-TvolOFF_0107\gain_multi_channel_plus24.cfg
//
// gain_multi_channel_plus24 (Subwoofer Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000c8, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000c9, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000ca, 0x7eca9cdc, 
//
// gain_multi_channel_plus24 (Subwoofer Gain 1_1)
//
// hold_count : signed : 32.0 format
  0xf00000c0, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000c1, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000c2, 0x0caddc61, 
//
// gain_multi_channel_plus24 (Subwoofer Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000c4, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000c5, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000c6, 0x08000000, 
//
// gain_multi_channel_plus24 (Ls-Rs Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000b4, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000b5, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000b6, 0x7eca9cdc, 
// g_2 : signed : 5.27 format
  0xf00000b7, 0x7eca9cdc, 
//
// gain_multi_channel_plus24 (Ls-Rs Gain 1_1)
//
// hold_count : signed : 32.0 format
  0xf00000a8, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000a9, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000aa, 0x09820d56, 
// g_2 : signed : 5.27 format
  0xf00000ab, 0x09820d56, 
//
// gain_multi_channel_plus24 (Ls-Rs Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000ae, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000af, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000b0, 0x08000000, 
// g_2 : signed : 5.27 format
  0xf00000b1, 0x08000000, 
//
// gain_multi_channel_plus24 (Center Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000d4, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000d5, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000d6, 0x1c629414, 
//
// gain_multi_channel_plus24 (Center Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000cc, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000cd, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000ce, 0x08000000, 
//
// gain_multi_channel_plus24 (L/R Gain (+24dB))
//
// hold_count : signed : 32.0 format
  0xf00000ba, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000bb, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000bc, 0x50000000, 
// g_2 : signed : 5.27 format
  0xf00000bd, 0x50000000, 
//
// gain_multi_channel_plus24 (Master Volume)
//
// hold_count : signed : 32.0 format
  0xf000002e, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf000002f, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf0000030, 0x00000000, /*SMITH SET GAIN TO ZERO*/
// g_2 : signed : 5.27 format
  0xf0000031, 0x00000000, 	/*SMITH SET GAIN TO ZERO*/
// g_3 : signed : 5.27 format
  0xf0000032, 0x00000000, 	/*SMITH SET GAIN TO ZERO*/
// g_4 : signed : 5.27 format
  0xf0000033, 0x00000000, 	/*SMITH SET GAIN TO ZERO*/
// g_5 : signed : 5.27 format
  0xf0000034, 0x00000000, 	/*SMITH SET GAIN TO ZERO*/
// g_6 : signed : 5.27 format
  0xf0000035, 0x00000000, 	/*SMITH SET GAIN TO ZERO*/
//include pcm-5.1-TvolOFF_0107\detector_signal_presence.cfg
//
// detector_signal_presence (Presence)
//
// threshold : signed : 1.31 format
  0xf00000d0, 0x00092322, /*Threhold Setting merge by Smith*/
// infinite_hold : signed : 32.0 format
  0xf00000d1, 0x00000000, 
// hold_time : signed : 32.0 format
  0xf00000d2, 0x000005dc, 
//
// detector_signal_presence (Presence)
//
// threshold : signed : 1.31 format
  0xf00000d8, 0x00092322, /*Threhold Setting merge by Smith*/
// infinite_hold : signed : 32.0 format
  0xf00000d9, 0x00000000, 
// hold_time : signed : 32.0 format
  0xf00000da, 0x000005dc, 
//include pcm-5.1-TvolOFF_0107\filter_highpass_transposed_direct_II.cfg
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf000003c, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf000003d, 0x00914ff9, 
// k : signed : 2.30 format
  0xf000003e, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf000003f, 0x7d8c094e, 
// b_1 : signed : 1.31 format
  0xf0000040, 0x7ffe340c, 
// c_1 : signed : 32.0 format
  0xf0000041, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf0000042, 0x7efa6bb3, 
// b_2 : signed : 1.31 format
  0xf0000043, 0x7ffe340c, 
// c_2 : signed : 32.0 format
  0xf0000044, 0x00000001, 
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf0000046, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf0000047, 0x00914ff9, 
// k : signed : 2.30 format
  0xf0000048, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf0000049, 0x7d8c094e, 
// b_1 : signed : 1.31 format
  0xf000004a, 0x7ffe340c, 
// c_1 : signed : 32.0 format
  0xf000004b, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf000004c, 0x7efa6bb3, 
// b_2 : signed : 1.31 format
  0xf000004d, 0x7ffe340c, 
// c_2 : signed : 32.0 format
  0xf000004e, 0x00000001, 
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf0000050, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf0000051, 0x00914ff9, 
// k : signed : 2.30 format
  0xf0000052, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf0000053, 0x7d8c094e, 
// b_1 : signed : 1.31 format
  0xf0000054, 0x7ffe340c, 
// c_1 : signed : 32.0 format
  0xf0000055, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf0000056, 0x7efa6bb3, 
// b_2 : signed : 1.31 format
  0xf0000057, 0x7ffe340c, 
// c_2 : signed : 32.0 format
  0xf0000058, 0x00000001, 
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf000005a, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf000005b, 0x00914ff9, 
// k : signed : 2.30 format
  0xf000005c, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf000005d, 0x7d8c094e, 
// b_1 : signed : 1.31 format
  0xf000005e, 0x7ffe340c, 
// c_1 : signed : 32.0 format
  0xf000005f, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf0000060, 0x7efa6bb3, 
// b_2 : signed : 1.31 format
  0xf0000061, 0x7ffe340c, 
// c_2 : signed : 32.0 format
  0xf0000062, 0x00000001, 
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf0000064, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf0000065, 0x00914ff9, 
// k : signed : 2.30 format
  0xf0000066, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf0000067, 0x7d8c094e, 
// b_1 : signed : 1.31 format
  0xf0000068, 0x7ffe340c, 
// c_1 : signed : 32.0 format
  0xf0000069, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf000006a, 0x7efa6bb3, 
// b_2 : signed : 1.31 format
  0xf000006b, 0x7ffe340c, 
// c_2 : signed : 32.0 format
  0xf000006c, 0x00000001, 
//include pcm-5.1-TvolOFF_0107\detector_peak_square.cfg
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000eb, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000ec, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000e8, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000e9, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000e5, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000e6, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000df, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000e0, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000e2, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000e3, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000dc, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000dd, 0x04324349, 
//include pcm-5.1-TvolOFF_0107\dynamic_hard_limiter2.cfg
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf00000a0, 0x07333330, 
// soft_knee : signed : 9.23 format
  0xf00000a1, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf00000a2, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf00000a3, 0x08414ea2, 
// dsp_release : signed : 1.31 format
  0xf00000a4, 0x00fa8a7d, 
// dsp_bypass : signed : 32.0 format
  0xf00000a5, 0x00000000, 
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf0000098, 0x07333330, 
// soft_knee : signed : 9.23 format
  0xf0000099, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf000009a, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf000009b, 0x08414ea2, 
// dsp_release : signed : 1.31 format
  0xf000009c, 0x00fa8a7d, 
// dsp_bypass : signed : 32.0 format
  0xf000009d, 0x00000000, 
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf0000090, 0x03999998, 
// soft_knee : signed : 9.23 format
  0xf0000091, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf0000092, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf0000093, 0x08414ea2, 
// dsp_release : signed : 1.31 format
  0xf0000094, 0x00fa8a7d, 
// dsp_bypass : signed : 32.0 format
  0xf0000095, 0x00000000, 
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf0000080, 0x01266666, 
// soft_knee : signed : 9.23 format
  0xf0000081, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf0000082, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf0000083, 0x08414ea2, 
// dsp_release : signed : 1.31 format
  0xf0000084, 0x00fa8a7d, 
// dsp_bypass : signed : 32.0 format
  0xf0000085, 0x00000000, 
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf0000088, 0x01266666, 
// soft_knee : signed : 9.23 format
  0xf0000089, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf000008a, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf000008b, 0x08414ea2, 
// dsp_release : signed : 1.31 format
  0xf000008c, 0x00fa8a7d, 
// dsp_bypass : signed : 32.0 format
  0xf000008d, 0x00000000, 
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf0000078, 0x0199999a, 
// soft_knee : signed : 9.23 format
  0xf0000079, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf000007a, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf000007b, 0x02d02583, 
// dsp_release : signed : 1.31 format
  0xf000007c, 0x0974f10a, 
// dsp_bypass : signed : 32.0 format
  0xf000007d, 0x00000000, 
//include pcm-5.1-TvolOFF_0107\filter_lowpass_transposed_direct_II.cfg
//
// filter_lowpass_transposed_direct_II (Low Pass)
//
// hold_count : signed : 32.0 format
  0xf000006e, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf000006f, 0x00914ff9, 
// k : signed : 2.30 format
  0xf0000070, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf0000071, 0x7cf0ede4, 
// b_1 : signed : 1.31 format
  0xf0000072, 0x7ffd3153, 
// c_1 : signed : 32.0 format
  0xf0000073, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf0000074, 0x7eb95aec, 
// b_2 : signed : 1.31 format
  0xf0000075, 0x7ffd3153, 
// c_2 : signed : 32.0 format
  0xf0000076, 0x00000001, 
//include pcm-5.1-TvolOFF_0107\meter_N_true_peak_and_RMS.cfg
//
// meter_N_true_peak_and_RMS (Meters)
//
// coeff_rms : signed : 1.31 format
  0xf0000001, 0x002ba95c, 
// coeff_peak : signed : 1.31 format
  0xf0000002, 0x7fea2975, 
// hold_count : signed : 32.0 format
  0xf0000003, 0x00000bb8, 
// infinite_peak_hold : signed : 32.0 format
  0xf0000004, 0x00000000, 
//include pcm-5.1-TvolOFF_0107\remove_24dB_headroom.cfg
//
// remove_24dB_headroom (24dB Gain)
//
// hold_count : signed : 32.0 format
  0xf000001d, 0x00000bb8, 
//include pcm-5.1-TvolOFF_0107\audio_manager.cfg
//
// audio_manager (Audio Mgr)
//
// _gain : signed : 5.27 format
  0x83000000, 0x08000000, 
// mute : unsigned : 32.0 format
  0x83000001, 0x00000000, 
// l_trim : unsigned : 1.31 format
  0x83000002, 0x80000000, 
// c_trim : unsigned : 1.31 format
  0x83000003, 0x80000000, 
// r_trim : unsigned : 1.31 format
  0x83000004, 0x80000000, 
// ls_trim : unsigned : 1.31 format
  0x83000005, 0x80000000, 
// rs_trim : unsigned : 1.31 format
  0x83000006, 0x80000000, 
// sbl_trim : unsigned : 1.31 format
  0x83000007, 0x80000000, 
// sbr_trim : unsigned : 1.31 format
  0x83000008, 0x80000000, 
// lfe0_trim : unsigned : 1.31 format
  0x83000009, 0x80000000, 
// lfe1_trim : unsigned : 1.31 format
  0x8300000a, 0x80000000, 
// lfe2_trim : unsigned : 1.31 format
  0x8300000b, 0x80000000, 
// lfe3_trim : unsigned : 1.31 format
  0x8300000c, 0x80000000, 
// lt_trim : unsigned : 1.31 format
  0x8300000e, 0x80000000, 
// rt_trim : unsigned : 1.31 format
  0x8300000f, 0x80000000, 
// lk_trim : unsigned : 1.31 format
  0x83000010, 0x80000000, 
// rk_trim : unsigned : 1.31 format
  0x83000011, 0x80000000, 
};
//#define Bytes_of_pcm-5_1-TvolOFF_1024  4112    //(bytes)
#endif
