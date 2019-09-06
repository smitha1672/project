#ifndef PCM_51_INITIAL_H__
#define PCM_51_INITIAL_H__

/***********************************************
This file is created by cirrus dsp tool on 06/25/15 15:24:28
This tool was written by XuShoucai on 06/01/09
************************************************/
static const uint32 code PCM_INITIAL[] = 
{
/* pcm-5.1-150625.cfg */
// Initial control values
// Created : 
//include pcm-5.1-150625\system_pcm.cfg
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
//include pcm-5.1-150625\cs495314.cfg
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
  0x81000008, 0x0000bb80, 
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
//include pcm-5.1-150625\router_NxM_basic.cfg
//
// router_NxM_basic (6x6 Router)
//
// ramp : signed : 1.31 format
  0xf00000a8, 0x00914ff9, 
// output_select_1 : signed : 32.0 format
  0xf00000a9, 0x00000001, 
// output_select_2 : signed : 32.0 format
  0xf00000aa, 0x00000002, 
// output_select_3 : signed : 32.0 format
  0xf00000ab, 0x00000001, 
// output_select_4 : signed : 32.0 format
  0xf00000ac, 0x00000002, 
// output_select_5 : signed : 32.0 format
  0xf00000ad, 0x00000001, 
// output_select_6 : signed : 32.0 format
  0xf00000ae, 0x00000002, 
//include pcm-5.1-150625\gain_multi_channel_plus24.cfg
//
// gain_multi_channel_plus24 (Subwoofer Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000cf, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000d0, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000d1, 0x7eca9cdc, 
//
// gain_multi_channel_plus24 (Subwoofer Gain 1_1)
//
// hold_count : signed : 32.0 format
  0xf00000c7, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000c8, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000c9, 0x0caddc61, 
//
// gain_multi_channel_plus24 (Subwoofer Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000cb, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000cc, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000cd, 0x08000000, 
//
// gain_multi_channel_plus24 (Ls-Rs Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000bb, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000bc, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000bd, 0x7100c49b, 
// g_2 : signed : 5.27 format
  0xf00000be, 0x7100c49b, 
//
// gain_multi_channel_plus24 (Ls-Rs Gain 1_1)
//
// hold_count : signed : 32.0 format
  0xf00000af, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000b0, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000b1, 0x08000000, 
// g_2 : signed : 5.27 format
  0xf00000b2, 0x08000000, 
//
// gain_multi_channel_plus24 (Ls-Rs Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000b5, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000b6, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000b7, 0x08000000, 
// g_2 : signed : 5.27 format
  0xf00000b8, 0x08000000, 
//
// gain_multi_channel_plus24 (Center Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000db, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000dc, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000dd, 0x168c0c62, 
//
// gain_multi_channel_plus24 (Center Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000d3, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000d4, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000d5, 0x08000000, 
//
// gain_multi_channel_plus24 (L/R Gain (+24dB))
//
// hold_count : signed : 32.0 format
  0xf00000c1, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000c2, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf00000c3, 0x59c2f05a, 
// g_2 : signed : 5.27 format
  0xf00000c4, 0x59c2f05a, 
//
// gain_multi_channel_plus24 (Master Volume)
//
// hold_count : signed : 32.0 format
  0xf000002e, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf000002f, 0x00914ff9, 
// g_1 : signed : 5.27 format
  0xf0000030, 0x025481b2, 
// g_2 : signed : 5.27 format
  0xf0000031, 0x025481b2, 
// g_3 : signed : 5.27 format
  0xf0000032, 0x025481b2, 
// g_4 : signed : 5.27 format
  0xf0000033, 0x025481b2, 
// g_5 : signed : 5.27 format
  0xf0000034, 0x025481b2, 
// g_6 : signed : 5.27 format
  0xf0000035, 0x025481b2, 
//include pcm-5.1-150625\filter_highpass_transposed_direct_II.cfg
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
  0xf000003f, 0x7dd9dedd, 
// b_1 : signed : 1.31 format
  0xf0000040, 0x7ffe9fd9, 
// c_1 : signed : 32.0 format
  0xf0000041, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf0000042, 0x7f1b00bd, 
// b_2 : signed : 1.31 format
  0xf0000043, 0x7ffe9fd9, 
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
  0xf0000049, 0x7dd9dedd, 
// b_1 : signed : 1.31 format
  0xf000004a, 0x7ffe9fd9, 
// c_1 : signed : 32.0 format
  0xf000004b, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf000004c, 0x7f1b00bd, 
// b_2 : signed : 1.31 format
  0xf000004d, 0x7ffe9fd9, 
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
//include pcm-5.1-150625\detector_peak_square.cfg
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000f2, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000f3, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000ef, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000f0, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000ec, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000ed, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000e6, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000e7, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000e9, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000ea, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000e3, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000e4, 0x04324349, 
//include pcm-5.1-150625\dynamic_hard_limiter2.cfg
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
  0xf000007c, 0x00fa8a7d, 
// dsp_bypass : signed : 32.0 format
  0xf000007d, 0x00000000, 
//include pcm-5.1-150625\filter_lowpass_transposed_direct_II.cfg
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
//include pcm-5.1-150625\meter_N_true_peak_and_RMS.cfg
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
//include pcm-5.1-150625\remove_24dB_headroom.cfg
//
// remove_24dB_headroom (24dB Gain)
//
// hold_count : signed : 32.0 format
  0xf000001d, 0x00000bb8, 
//include pcm-5.1-150625\detector_signal_presence.cfg
//
// detector_signal_presence (Presence)
//
// threshold : signed : 1.31 format
  0xf00000d7, 0x00033df4, 
// infinite_hold : signed : 32.0 format
  0xf00000d8, 0x00000000, 
// hold_time : signed : 32.0 format
  0xf00000d9, 0x000005dc, 
//
// detector_signal_presence (Presence)
//
// threshold : signed : 1.31 format
  0xf00000df, 0x00033df4, 
// infinite_hold : signed : 32.0 format
  0xf00000e0, 0x00000000, 
// hold_time : signed : 32.0 format
  0xf00000e1, 0x000005dc, 
//include pcm-5.1-150625\audio_manager.cfg
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
#define Bytes_of_pcm-5_1-150625  1728   //(bytes)

#endif