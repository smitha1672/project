/***********************************************
This file is created by cirrus dsp tool on 12/27/14 18:06:27
This tool was written by XuShoucai on 06/01/09
************************************************/
uint32 code ATMOS_DEMO_1227_CES_CFG [] = 
{
/* ATMOS_Demo_1227_CES.cfg */
// Initial control values
// Created : 
//include ATMOS_Demo_1227_CES\cs49844.cfg
//
// cs49844 (cs49844)
//
// output_mode : unsigned : 32.0 format
  0x81000002, 0x000000ff, 
// sample_rate : unsigned : 32.0 format
  0x81000003, 0x00000000, 
// autodetect_config : unsigned : 32.0 format
  0x81000005, 0x00000005, 
// autoswitch_enable : unsigned : 32.0 format
  0x81000006, 0x00000001, 
// silence_threshold : unsigned : 32.0 format
  0x81000008, 0x0000bb80, 
//include ATMOS_Demo_1227_CES\system_pcm.cfg
//
// system_pcm (PCM)
//
// pcm_enable : unsigned : 32.0 format
  0x9b000000, 0x00000001, 
//include ATMOS_Demo_1227_CES\decoder_true_hd_atmos_cs498XX.cfg
//
// decoder_true_hd_atmos_cs498XX (TrueHD)
//
// truehd_control : unsigned : 32.0 format
  0x9d000000, 0x00000001, 
// decode_mode_control : unsigned : 32.0 format
  0x9d000001, 0x00000000, 
// drc_control : unsigned : 32.0 format
  0x9d000002, 0x00000001, 
// drc_cut_scalefactor : unsigned : 1.31 format
  0x9d000003, 0x80000000, 
// drc_boost_scalefactor : unsigned : 1.31 format
  0x9d000004, 0x80000000, 
//include ATMOS_Demo_1227_CES\mpm_oar.cfg
//
// mpm_oar (Dolby OAR)
//
// OAR_CONTROL : unsigned : 32.0 format
  0xb9000000, 0x00000001, 
//include ATMOS_Demo_1227_CES\router_NxM_basic.cfg
//
// router_NxM_basic (6x6 Router)
//
// ramp : signed : 1.31 format
  0xf00000a5, 0x2448b3b7, 
// output_select_1 : signed : 32.0 format
  0xf00000a6, 0x00000001, 
// output_select_2 : signed : 32.0 format
  0xf00000a7, 0x00000002, 
// output_select_3 : signed : 32.0 format
  0xf00000a8, 0x00000003, 
// output_select_4 : signed : 32.0 format
  0xf00000a9, 0x00000004, 
// output_select_5 : signed : 32.0 format
  0xf00000aa, 0x00000005, 
// output_select_6 : signed : 32.0 format
  0xf00000ab, 0x00000006, 
//
// router_NxM_basic (8x8 Router)
//
// ramp : signed : 1.31 format
  0xf0000074, 0x2448b3b7, 
// output_select_1 : signed : 32.0 format
  0xf0000075, 0x00000001, 
// output_select_2 : signed : 32.0 format
  0xf0000076, 0x00000002, 
// output_select_3 : signed : 32.0 format
  0xf0000077, 0x00000003, 
// output_select_4 : signed : 32.0 format
  0xf0000078, 0x00000004, 
// output_select_5 : signed : 32.0 format
  0xf0000079, 0x00000005, 
// output_select_6 : signed : 32.0 format
  0xf000007a, 0x00000006, 
// output_select_7 : signed : 32.0 format
  0xf000007b, 0x00000007, 
// output_select_8 : signed : 32.0 format
  0xf000007c, 0x00000008, 
//include ATMOS_Demo_1227_CES\gain_multi_channel_plus24.cfg
//
// gain_multi_channel_plus24 (LFH/RFH Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000ac, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000ad, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000ae, 0x141857af, 
// g_2 : signed : 5.27 format
  0xf00000af, 0x141857af, 
//
// gain_multi_channel_plus24 (LFH/RFH Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000d6, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000d7, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000d8, 0x08000000, 
// g_2 : signed : 5.27 format
  0xf00000d9, 0x08000000, 
//
// gain_multi_channel_plus24 (FL/FR Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000b8, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000b9, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000ba, 0x0ff64bec, 
// g_2 : signed : 5.27 format
  0xf00000bb, 0x0ff64bec, 
//
// gain_multi_channel_plus24 (FL/FR Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000b2, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000b3, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000b4, 0x08000000, 
// g_2 : signed : 5.27 format
  0xf00000b5, 0x08000000, 
//
// gain_multi_channel_plus24 (Ls / Rs Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000c4, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000c5, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000c6, 0x0e39ea57, 
// g_2 : signed : 5.27 format
  0xf00000c7, 0x0e39ea57, 
//
// gain_multi_channel_plus24 (Ls /Rs  Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000be, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000bf, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000c0, 0x08000000, 
// g_2 : signed : 5.27 format
  0xf00000c1, 0x08000000, 
//
// gain_multi_channel_plus24 (Subwoofer Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000f6, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000f7, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000f8, 0x7eca9cdc, 
//
// gain_multi_channel_plus24 (Subwoofer Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000f2, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000f3, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000f4, 0x0b4ce0b9, 
//
// gain_multi_channel_plus24 (Lrb/Rrb Gain 1)
//
// hold_count : signed : 32.0 format
  0xf00000d0, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000d1, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000d2, 0x0e39ea57, 
// g_2 : signed : 5.27 format
  0xf00000d3, 0x0e39ea57, 
//
// gain_multi_channel_plus24 (Lrb/Rrb Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000ca, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000cb, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000cc, 0x08000000, 
// g_2 : signed : 5.27 format
  0xf00000cd, 0x08000000, 
//
// gain_multi_channel_plus24 (Center Gain 1)
//
// hold_count : signed : 32.0 format
  0xf0000102, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf0000103, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf0000104, 0x05a9df97, 
//
// gain_multi_channel_plus24 (Center Gain 2)
//
// hold_count : signed : 32.0 format
  0xf00000fa, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000fb, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf00000fc, 0x08000000, 
//
// gain_multi_channel_plus24 (Master Volume)
//
// hold_count : signed : 32.0 format
  0xf0000001, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf0000002, 0x2448b3b7, 
// g_1 : signed : 5.27 format
  0xf0000003, 0x08f9e492, 
// g_2 : signed : 5.27 format
  0xf0000004, 0x08f9e492, 
// g_3 : signed : 5.27 format
  0xf0000005, 0x08f9e492, 
// g_4 : signed : 5.27 format
  0xf0000006, 0x08f9e492, 
// g_5 : signed : 5.27 format
  0xf0000007, 0x08f9e492, 
// g_6 : signed : 5.27 format
  0xf0000008, 0x08f9e492, 
// g_7 : signed : 5.27 format
  0xf0000009, 0x08f9e492, 
// g_8 : signed : 5.27 format
  0xf000000a, 0x08f9e492, 
//include ATMOS_Demo_1227_CES\mixer_NxM.cfg
//
// mixer_NxM (Rs/Rb 2x1 Mixer)
//
// hold_count : signed : 32.0 format
  0xf00000dc, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000dd, 0x2448b3b7, 
// input_1_output_1_send : signed : 3.29 format
  0xf00000de, 0x18464406, 
// input_2_output_1_send : signed : 3.29 format
  0xf00000df, 0x18464406, 
//
// mixer_NxM (Ls/Lb 2x1 Mixer)
//
// hold_count : signed : 32.0 format
  0xf00000e1, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf00000e2, 0x2448b3b7, 
// input_1_output_1_send : signed : 3.29 format
  0xf00000e3, 0x18464406, 
// input_2_output_1_send : signed : 3.29 format
  0xf00000e4, 0x18464406, 
//include ATMOS_Demo_1227_CES\detector_signal_presence.cfg
//
// detector_signal_presence (Presence)
//
// threshold : signed : 1.31 format
  0xf00000fe, 0x00033df4, 
// infinite_hold : signed : 32.0 format
  0xf00000ff, 0x00000000, 
// hold_time : signed : 32.0 format
  0xf0000100, 0x000005dc, 
//
// detector_signal_presence (Presence)
//
// threshold : signed : 1.31 format
  0xf0000106, 0x00033df4, 
// infinite_hold : signed : 32.0 format
  0xf0000107, 0x00000000, 
// hold_time : signed : 32.0 format
  0xf0000108, 0x000005dc, 
//include ATMOS_Demo_1227_CES\filter_highpass_transposed_direct_II.cfg
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf0000024, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf0000025, 0x2448b3b7, 
// k : signed : 2.30 format
  0xf0000026, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf0000027, 0x7e27e481, 
// b_1 : signed : 1.31 format
  0xf0000028, 0x7ffefd46, 
// c_1 : signed : 32.0 format
  0xf0000029, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf000002a, 0x7f3b9e35, 
// b_2 : signed : 1.31 format
  0xf000002b, 0x7ffefd46, 
// c_2 : signed : 32.0 format
  0xf000002c, 0x00000001, 
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf000002e, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf000002f, 0x2448b3b7, 
// k : signed : 2.30 format
  0xf0000030, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf0000031, 0x7e27e481, 
// b_1 : signed : 1.31 format
  0xf0000032, 0x7ffefd46, 
// c_1 : signed : 32.0 format
  0xf0000033, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf0000034, 0x7f3b9e35, 
// b_2 : signed : 1.31 format
  0xf0000035, 0x7ffefd46, 
// c_2 : signed : 32.0 format
  0xf0000036, 0x00000001, 
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf0000038, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf0000039, 0x2448b3b7, 
// k : signed : 2.30 format
  0xf000003a, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf000003b, 0x7cf0ede4, 
// b_1 : signed : 1.31 format
  0xf000003c, 0x7ffd3153, 
// c_1 : signed : 32.0 format
  0xf000003d, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf000003e, 0x7eb95aec, 
// b_2 : signed : 1.31 format
  0xf000003f, 0x7ffd3153, 
// c_2 : signed : 32.0 format
  0xf0000040, 0x00000001, 
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf0000042, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf0000043, 0x2448b3b7, 
// k : signed : 2.30 format
  0xf0000044, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf0000045, 0x7cf0ede4, 
// b_1 : signed : 1.31 format
  0xf0000046, 0x7ffd3153, 
// c_1 : signed : 32.0 format
  0xf0000047, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf0000048, 0x7eb95aec, 
// b_2 : signed : 1.31 format
  0xf0000049, 0x7ffd3153, 
// c_2 : signed : 32.0 format
  0xf000004a, 0x00000001, 
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf000004c, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf000004d, 0x2448b3b7, 
// k : signed : 2.30 format
  0xf000004e, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf000004f, 0x7cf0ede4, 
// b_1 : signed : 1.31 format
  0xf0000050, 0x7ffd3153, 
// c_1 : signed : 32.0 format
  0xf0000051, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf0000052, 0x7eb95aec, 
// b_2 : signed : 1.31 format
  0xf0000053, 0x7ffd3153, 
// c_2 : signed : 32.0 format
  0xf0000054, 0x00000001, 
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf0000056, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf0000057, 0x2448b3b7, 
// k : signed : 2.30 format
  0xf0000058, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf0000059, 0x7d8c094e, 
// b_1 : signed : 1.31 format
  0xf000005a, 0x7ffe340c, 
// c_1 : signed : 32.0 format
  0xf000005b, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf000005c, 0x7efa6bb3, 
// b_2 : signed : 1.31 format
  0xf000005d, 0x7ffe340c, 
// c_2 : signed : 32.0 format
  0xf000005e, 0x00000001, 
//
// filter_highpass_transposed_direct_II (High Pass)
//
// hold_count : signed : 32.0 format
  0xf0000060, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf0000061, 0x2448b3b7, 
// k : signed : 2.30 format
  0xf0000062, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf0000063, 0x7d8c094e, 
// b_1 : signed : 1.31 format
  0xf0000064, 0x7ffe340c, 
// c_1 : signed : 32.0 format
  0xf0000065, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf0000066, 0x7efa6bb3, 
// b_2 : signed : 1.31 format
  0xf0000067, 0x7ffe340c, 
// c_2 : signed : 32.0 format
  0xf0000068, 0x00000001, 
//include ATMOS_Demo_1227_CES\detector_peak_square.cfg
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000e6, 0x00000000, 
// input_disable_2 : signed : 32.0 format
  0xf00000e7, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000e8, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000ee, 0x00000000, 
// input_disable_2 : signed : 32.0 format
  0xf00000ef, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000f0, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf000010a, 0x00000000, 
// tc : signed : 1.31 format
  0xf000010b, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf00000ea, 0x00000000, 
// input_disable_2 : signed : 32.0 format
  0xf00000eb, 0x00000000, 
// tc : signed : 1.31 format
  0xf00000ec, 0x04324349, 
//
// detector_peak_square (Peak Square Detector)
//
// input_disable_1 : signed : 32.0 format
  0xf000010d, 0x00000000, 
// tc : signed : 1.31 format
  0xf000010e, 0x04324349, 
//include ATMOS_Demo_1227_CES\dynamic_hard_limiter2.cfg
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf0000085, 0x07999998, 
// soft_knee : signed : 9.23 format
  0xf0000086, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf0000087, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf0000088, 0x04324349, 
// dsp_release : signed : 1.31 format
  0xf0000089, 0x00fa8a7d, 
// dsp_bypass : signed : 32.0 format
  0xf000008a, 0x00000000, 
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf0000095, 0x07999998, 
// soft_knee : signed : 9.23 format
  0xf0000096, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf0000097, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf0000098, 0x04324349, 
// dsp_release : signed : 1.31 format
  0xf0000099, 0x00fa8a7d, 
// dsp_bypass : signed : 32.0 format
  0xf000009a, 0x00000000, 
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf000007d, 0x03800000, 
// soft_knee : signed : 9.23 format
  0xf000007e, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf000007f, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf0000080, 0x04324349, 
// dsp_release : signed : 1.31 format
  0xf0000081, 0x00fa8a7d, 
// dsp_bypass : signed : 32.0 format
  0xf0000082, 0x00000000, 
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf000008d, 0x07999998, 
// soft_knee : signed : 9.23 format
  0xf000008e, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf000008f, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf0000090, 0x04324349, 
// dsp_release : signed : 1.31 format
  0xf0000091, 0x00fa8a7d, 
// dsp_bypass : signed : 32.0 format
  0xf0000092, 0x00000000, 
//
// dynamic_hard_limiter2 (Hard Limiter)
//
// threshold : signed : 9.23 format
  0xf000009d, 0x0b400000, 
// soft_knee : signed : 9.23 format
  0xf000009e, 0x00000000, 
// dsp_ratio : signed : 1.31 format
  0xf000009f, 0x8147ae15, 
// dsp_attack : signed : 1.31 format
  0xf00000a0, 0x02d02583, 
// dsp_release : signed : 1.31 format
  0xf00000a1, 0x0974f10a, 
// dsp_bypass : signed : 32.0 format
  0xf00000a2, 0x00000000, 
//include ATMOS_Demo_1227_CES\filter_lowpass_transposed_direct_II.cfg
//
// filter_lowpass_transposed_direct_II (Low Pass)
//
// hold_count : signed : 32.0 format
  0xf000006a, 0x00000bb8, 
// ramp : signed : 1.31 format
  0xf000006b, 0x2448b3b7, 
// k : signed : 2.30 format
  0xf000006c, 0x40000000, 
// a_1 : signed : 1.31 format
  0xf000006d, 0x7cf0ede4, 
// b_1 : signed : 1.31 format
  0xf000006e, 0x7ffd3153, 
// c_1 : signed : 32.0 format
  0xf000006f, 0x00000001, 
// a_2 : signed : 1.31 format
  0xf0000070, 0x7eb95aec, 
// b_2 : signed : 1.31 format
  0xf0000071, 0x7ffd3153, 
// c_2 : signed : 32.0 format
  0xf0000072, 0x00000001, 
//include ATMOS_Demo_1227_CES\remove_24dB_headroom.cfg
//
// remove_24dB_headroom (24dB Gain)
//
// hold_count : signed : 32.0 format
  0xf0000013, 0x00000bb8, 
//include ATMOS_Demo_1227_CES\bass_manager_498XX.cfg
//
// bass_manager_498XX (Bass mgr)
//
// control_word_0 : unsigned : 32.0 format
  0xd7000000, 0x00000029, 
// xover_control_word_1 : unsigned : 32.0 format
  0xd7000001, 0x000000bf, 
// hpf_control_word_2 : unsigned : 32.0 format
  0xd7000002, 0x00000000, 
// _input_level_l : signed : 1.31 format
  0xd7000016, 0x5a9df980, 
// _input_level_c : signed : 1.31 format
  0xd7000017, 0x5a9df980, 
// _input_level_r : signed : 1.31 format
  0xd7000018, 0x5a9df980, 
// _input_level_ls : signed : 1.31 format
  0xd7000019, 0x5a9df980, 
// _input_level_rs : signed : 1.31 format
  0xd700001a, 0x5a9df980, 
// _input_level_lb : signed : 1.31 format
  0xd700001b, 0x5a9df980, 
// _input_level_rb : signed : 1.31 format
  0xd700001c, 0x5a9df980, 
// _input_level_lh : signed : 1.31 format
  0xd700001d, 0x5a9df980, 
// _input_level_rh : signed : 1.31 format
  0xd700001e, 0x5a9df980, 
// _input_level_lw : signed : 1.31 format
  0xd700001f, 0x5a9df980, 
// _input_level_rw : signed : 1.31 format
  0xd7000020, 0x5a9df980, 
// _front_sum_output_level_l : signed : 1.31 format
  0xd7000021, 0x101d4300, 
// _front_sum_output_level_c : signed : 1.31 format
  0xd7000022, 0x101d4300, 
// _front_sum_output_level_r : signed : 1.31 format
  0xd7000023, 0x101d4300, 
// _back_sum_output_level_ls : signed : 1.31 format
  0xd7000024, 0x101d4300, 
// _back_sum_output_level_rs : signed : 1.31 format
  0xd7000025, 0x101d4300, 
// _back_sum_output_level_lb : signed : 1.31 format
  0xd7000026, 0x101d4300, 
// _back_sum_output_level_rb : signed : 1.31 format
  0xd7000027, 0x101d4300, 
// _front_sum_output_level_lh : signed : 1.31 format
  0xd7000028, 0x101d4300, 
// _front_sum_output_level_rh : signed : 1.31 format
  0xd7000029, 0x101d4300, 
// _front_sum_output_level_lw : signed : 1.31 format
  0xd700002a, 0x101d4300, 
// _front_sum_output_level_rw : signed : 1.31 format
  0xd700002b, 0x101d4300, 
// _bass_mgr_lr_lpf_input_level_l : signed : 1.31 format
  0xd700002c, 0x2026f300, 
// _bass_mgr_lr_lpf_input_level_r : signed : 1.31 format
  0xd700002d, 0x2026f300, 
// _bass_mgr_c_lpf_input_level_c : signed : 1.31 format
  0xd700002e, 0x2d6a8680, 
// _bass_mgr_lsrs_lpf_input_level_ls : signed : 1.31 format
  0xd700002f, 0x2026f300, 
// _bass_mgr_lsrs_lpf_input_level_rs : signed : 1.31 format
  0xd7000030, 0x2026f300, 
// _bass_mgr_lbrb_lpf_input_level_lb : signed : 1.31 format
  0xd7000031, 0x2026f300, 
// _bass_mgr_lbrb_lpf_input_level_rb : signed : 1.31 format
  0xd7000032, 0x2026f300, 
// _bass_mgr_lhrh_lpf_input_level_lh : signed : 1.31 format
  0xd7000033, 0x2026f300, 
// _bass_mgr_lhrh_lpf_input_level_rh : signed : 1.31 format
  0xd7000034, 0x2026f300, 
// _bass_mgr_lwrw_lpf_input_level_lw : signed : 1.31 format
  0xd7000035, 0x2026f300, 
// _bass_mgr_lwrw_lpf_input_level_rw : signed : 1.31 format
  0xd7000036, 0x2026f300, 
// _bass_mgr_all_lpf_input_level_l : signed : 1.31 format
  0xd7000037, 0x00000000, 
// _bass_mgr_all_lpf_input_level_c : signed : 1.31 format
  0xd7000038, 0x00000000, 
// _bass_mgr_all_lpf_input_level_r : signed : 1.31 format
  0xd7000039, 0x00000000, 
// _bass_mgr_all_lpf_input_level_ls : signed : 1.31 format
  0xd700003a, 0x00000000, 
// _bass_mgr_all_lpf_input_level_rs : signed : 1.31 format
  0xd700003b, 0x00000000, 
// _bass_mgr_all_lpf_input_level_lb : signed : 1.31 format
  0xd700003c, 0x00000000, 
// _bass_mgr_all_lpf_input_level_rb : signed : 1.31 format
  0xd700003d, 0x00000000, 
// _bass_mgr_all_lpf_input_level_lh : signed : 1.31 format
  0xd700003e, 0x00000000, 
// _bass_mgr_all_lpf_input_level_rh : signed : 1.31 format
  0xd700003f, 0x00000000, 
// _bass_mgr_all_lpf_input_level_lw : signed : 1.31 format
  0xd7000040, 0x00000000, 
// _bass_mgr_all_lpf_input_level_rw : signed : 1.31 format
  0xd7000041, 0x00000000, 
// _bass_mgr_all_lpf_input_level_lfe : signed : 1.31 format
  0xd7000042, 0x00000000, 
// _bass_mgr_lfe_lpf_input_level_lfe : signed : 1.31 format
  0xd7000043, 0x2d6a8680, 
// _front_sum_in_level_lr : signed : 1.31 format
  0xd7000044, 0x2d6a8680, 
// _front_sum_in_level_c : signed : 1.31 format
  0xd7000045, 0x2d6a8680, 
// _front_sum_in_level_lsrs : signed : 1.31 format
  0xd7000046, 0x101d3f20, 
// _front_sum_in_level_lbrb : signed : 1.31 format
  0xd7000047, 0x101d3f20, 
// _front_sum_in_level_lhrh : signed : 1.31 format
  0xd7000048, 0x101d3f20, 
// _front_sum_in_level_lwrw : signed : 1.31 format
  0xd7000049, 0x00000000, 
// _front_sum_in_level_all : signed : 1.31 format
  0xd700004a, 0x00000000, 
// _front_sum_in_level_lfe : signed : 1.31 format
  0xd700004b, 0x00000000, 
// _back_sum_in_level_lr : signed : 1.31 format
  0xd700004c, 0x2d6a8680, 
// _back_sum_in_level_c : signed : 1.31 format
  0xd700004d, 0x2d6a8680, 
// _back_sum_in_level_lsrs : signed : 1.31 format
  0xd700004e, 0x101d3f20, 
// _back_sum_in_level_lbrb : signed : 1.31 format
  0xd700004f, 0x101d3f20, 
// _back_sum_in_level_lhrh : signed : 1.31 format
  0xd7000050, 0x101d3f20, 
// _back_sum_in_level_lwrw : signed : 1.31 format
  0xd7000051, 0x00000000, 
// _back_sum_in_level_all : signed : 1.31 format
  0xd7000052, 0x00000000, 
// _back_sum_in_level_lfe : signed : 1.31 format
  0xd7000053, 0x00000000, 
// _input_lfe_sub_level : signed : 1.31 format
  0xd7000054, 0x16c310e0, 
// _front_sum_output_sub_level : signed : 1.31 format
  0xd7000055, 0x16c310e0, 
// _back_sum_output_sub_level : signed : 1.31 format
  0xd7000056, 0x16c310e0, 
//include ATMOS_Demo_1227_CES\eq_proc.cfg
//
// eq_proc (PEQ)
//
// eq_control : unsigned : 32.0 format
  0xd5000000, 0x800030ff, 
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
// eq_chlb_preattenuation : signed : 1.31 format
  0xd500000f, 0x80000000, 
// eq_chlb_postgain : signed : 9.23 format
  0xd5000010, 0xff800000, 
// eq_chrb_preattenuation : signed : 1.31 format
  0xd5000011, 0x80000000, 
// eq_chrb_postgain : signed : 9.23 format
  0xd5000012, 0xff800000, 
// eq_chlfe0_preattenuation : signed : 1.31 format
  0xd5000013, 0x80000000, 
// eq_chlfe0_postgain : signed : 9.23 format
  0xd5000014, 0xff800000, 
// eq_chlfe1_preattenuation : signed : 1.31 format
  0xd5000015, 0x80000000, 
// eq_chlfe1_postgain : signed : 9.23 format
  0xd5000016, 0xff800000, 
// eq_chlfe2_preattenuation : signed : 1.31 format
  0xd5000017, 0x80000000, 
// eq_chlfe2_postgain : signed : 9.23 format
  0xd5000018, 0xff800000, 
// eq_chlfe3_preattenuation : signed : 1.31 format
  0xd5000019, 0x80000000, 
// eq_chlfe3_postgain : signed : 9.23 format
  0xd500001a, 0xff800000, 
// eq_chres_preattenuation : signed : 1.31 format
  0xd500001b, 0x80000000, 
// eq_chres_postgain : signed : 9.23 format
  0xd500001c, 0xff800000, 
// eq_chlz_preattenuation : signed : 1.31 format
  0xd500001d, 0x80000000, 
// eq_chlz_postgain : signed : 9.23 format
  0xd500001e, 0xff800000, 
// eq_chrz_preattenuation : signed : 1.31 format
  0xd500001f, 0x80000000, 
// eq_chrz_postgain : signed : 9.23 format
  0xd5000020, 0xff800000, 
// eq_chlk_preattenuation : signed : 1.31 format
  0xd5000021, 0x80000000, 
// eq_chlk_postgain : signed : 9.23 format
  0xd5000022, 0xff800000, 
// eq_chrk_preattenuation : signed : 1.31 format
  0xd5000023, 0x80000000, 
// eq_chrk_postgain : signed : 9.23 format
  0xd5000024, 0xff800000, 
// eq_channel_l_band_1_fc : unsigned : 20.12 format
  0xd5000025, 0x8295e000, 
// eq_channel_l_band_1_q : unsigned : 32.0 format
  0xd5000026, 0x20000000, 
// eq_channel_l_band_1_gain : signed : 6.26 format
  0xd5000027, 0xf8000000, 
// eq_channel_l_band_2_fc : unsigned : 20.12 format
  0xd5000028, 0x801a4000, 
// eq_channel_l_band_2_q : unsigned : 32.0 format
  0xd5000029, 0x10000000, 
// eq_channel_l_band_2_gain : signed : 6.26 format
  0xd500002a, 0x10000000, 
// eq_channel_l_band_3_fc : unsigned : 20.12 format
  0xd500002b, 0x800c8000, 
// eq_channel_l_band_3_q : unsigned : 32.0 format
  0xd500002c, 0x0c000000, 
// eq_channel_l_band_3_gain : signed : 6.26 format
  0xd500002d, 0x0c000000, 
// eq_channel_l_band_4_fc : unsigned : 20.12 format
  0xd500002e, 0x80582000, 
// eq_channel_l_band_4_q : unsigned : 32.0 format
  0xd500002f, 0x10000000, 
// eq_channel_l_band_4_gain : signed : 6.26 format
  0xd5000030, 0xf0000000, 
// eq_channel_l_band_5_fc : unsigned : 20.12 format
  0xd5000031, 0x80000000, 
// eq_channel_l_band_5_q : unsigned : 32.0 format
  0xd5000032, 0x00000000, 
// eq_channel_l_band_5_gain : signed : 6.26 format
  0xd5000033, 0x00000000, 
// eq_channel_l_band_6_fc : unsigned : 20.12 format
  0xd5000034, 0x80dde000, 
// eq_channel_l_band_6_q : unsigned : 32.0 format
  0xd5000035, 0x08000000, 
// eq_channel_l_band_6_gain : signed : 6.26 format
  0xd5000036, 0x14000000, 
// eq_channel_l_band_7_fc : unsigned : 20.12 format
  0xd5000037, 0x81b58000, 
// eq_channel_l_band_7_q : unsigned : 32.0 format
  0xd5000038, 0x12000000, 
// eq_channel_l_band_7_gain : signed : 6.26 format
  0xd5000039, 0x10000000, 
// eq_channel_l_band_8_fc : unsigned : 20.12 format
  0xd500003a, 0x81f40000, 
// eq_channel_l_band_8_q : unsigned : 32.0 format
  0xd500003b, 0x04000000, 
// eq_channel_l_band_8_gain : signed : 6.26 format
  0xd500003c, 0x0c000000, 
// eq_channel_l_band_9_fc : unsigned : 20.12 format
  0xd500003d, 0x83732000, 
// eq_channel_l_band_9_q : unsigned : 32.0 format
  0xd500003e, 0x04000000, 
// eq_channel_l_band_9_gain : signed : 6.26 format
  0xd500003f, 0x0c000000, 
// eq_channel_l_band_10_fc : unsigned : 20.12 format
  0xd5000040, 0x808c0000, 
// eq_channel_l_band_10_q : unsigned : 32.0 format
  0xd5000041, 0x08000000, 
// eq_channel_l_band_10_gain : signed : 6.26 format
  0xd5000042, 0xf4000000, 
// eq_channel_l_band_11_fc : unsigned : 20.12 format
  0xd5000043, 0x84844000, 
// eq_channel_l_band_11_q : unsigned : 32.0 format
  0xd5000044, 0x08000000, 
// eq_channel_l_band_11_gain : signed : 6.26 format
  0xd5000045, 0x18000000, 
// eq_channel_l_band_12_fc : unsigned : 20.12 format
  0xd5000046, 0x830d4000, 
// eq_channel_l_band_12_q : unsigned : 32.0 format
  0xd5000047, 0x18000000, 
// eq_channel_l_band_12_gain : signed : 6.26 format
  0xd5000048, 0xec000000, 
// eq_channel_l_band_13_fc : unsigned : 20.12 format
  0xd5000049, 0x81388000, 
// eq_channel_l_band_13_q : unsigned : 32.0 format
  0xd500004a, 0x08000000, 
// eq_channel_l_band_13_gain : signed : 6.26 format
  0xd500004b, 0x0c000000, 
// eq_channel_l_band_14_fc : unsigned : 20.12 format
  0xd500004c, 0x80000000, 
// eq_channel_l_band_14_q : unsigned : 32.0 format
  0xd500004d, 0x00000000, 
// eq_channel_l_band_14_gain : signed : 6.26 format
  0xd500004e, 0x00000000, 
// eq_channel_l_band_15_fc : unsigned : 20.12 format
  0xd500004f, 0x80212000, 
// eq_channel_l_band_15_q : unsigned : 32.0 format
  0xd5000050, 0x04000000, 
// eq_channel_l_band_15_gain : signed : 6.26 format
  0xd5000051, 0x08000000, 
// eq_channel_c_band_1_fc : unsigned : 20.12 format
  0xd5000052, 0x80000000, 
// eq_channel_c_band_1_q : unsigned : 32.0 format
  0xd5000053, 0x00000000, 
// eq_channel_c_band_1_gain : signed : 6.26 format
  0xd5000054, 0x00000000, 
// eq_channel_c_band_2_fc : unsigned : 20.12 format
  0xd5000055, 0x800e1000, 
// eq_channel_c_band_2_q : unsigned : 32.0 format
  0xd5000056, 0x14000000, 
// eq_channel_c_band_2_gain : signed : 6.26 format
  0xd5000057, 0xf0000000, 
// eq_channel_c_band_3_fc : unsigned : 20.12 format
  0xd5000058, 0x80177000, 
// eq_channel_c_band_3_q : unsigned : 32.0 format
  0xd5000059, 0x10000000, 
// eq_channel_c_band_3_gain : signed : 6.26 format
  0xd500005a, 0x0c000000, 
// eq_channel_c_band_4_fc : unsigned : 20.12 format
  0xd500005b, 0x80582000, 
// eq_channel_c_band_4_q : unsigned : 32.0 format
  0xd500005c, 0x08000000, 
// eq_channel_c_band_4_gain : signed : 6.26 format
  0xd500005d, 0xf0000000, 
// eq_channel_c_band_5_fc : unsigned : 20.12 format
  0xd500005e, 0x80348000, 
// eq_channel_c_band_5_q : unsigned : 32.0 format
  0xd500005f, 0x0e000000, 
// eq_channel_c_band_5_gain : signed : 6.26 format
  0xd5000060, 0xf0000000, 
// eq_channel_c_band_6_fc : unsigned : 20.12 format
  0xd5000061, 0x80000000, 
// eq_channel_c_band_6_q : unsigned : 32.0 format
  0xd5000062, 0x00000000, 
// eq_channel_c_band_6_gain : signed : 6.26 format
  0xd5000063, 0x00000000, 
// eq_channel_c_band_7_fc : unsigned : 20.12 format
  0xd5000064, 0x807d0000, 
// eq_channel_c_band_7_q : unsigned : 32.0 format
  0xd5000065, 0x10000000, 
// eq_channel_c_band_7_gain : signed : 6.26 format
  0xd5000066, 0xe8000000, 
// eq_channel_c_band_8_fc : unsigned : 20.12 format
  0xd5000067, 0x80a64000, 
// eq_channel_c_band_8_q : unsigned : 32.0 format
  0xd5000068, 0x20000000, 
// eq_channel_c_band_8_gain : signed : 6.26 format
  0xd5000069, 0xe4000000, 
// eq_channel_c_band_9_fc : unsigned : 20.12 format
  0xd500006a, 0x8189c000, 
// eq_channel_c_band_9_q : unsigned : 32.0 format
  0xd500006b, 0x08000000, 
// eq_channel_c_band_9_gain : signed : 6.26 format
  0xd500006c, 0x0c000000, 
// eq_channel_c_band_10_fc : unsigned : 20.12 format
  0xd500006d, 0x8295e000, 
// eq_channel_c_band_10_q : unsigned : 32.0 format
  0xd500006e, 0x20000000, 
// eq_channel_c_band_10_gain : signed : 6.26 format
  0xd500006f, 0xec000000, 
// eq_channel_c_band_11_fc : unsigned : 20.12 format
  0xd5000070, 0x83a70000, 
// eq_channel_c_band_11_q : unsigned : 32.0 format
  0xd5000071, 0x10000000, 
// eq_channel_c_band_11_gain : signed : 6.26 format
  0xd5000072, 0x0c000000, 
// eq_channel_c_band_12_fc : unsigned : 20.12 format
  0xd5000073, 0x80460000, 
// eq_channel_c_band_12_q : unsigned : 32.0 format
  0xd5000074, 0x03333334, 
// eq_channel_c_band_12_gain : signed : 6.26 format
  0xd5000075, 0x08000000, 
// eq_channel_c_band_13_fc : unsigned : 20.12 format
  0xd5000076, 0x81388000, 
// eq_channel_c_band_13_q : unsigned : 32.0 format
  0xd5000077, 0x088f5c30, 
// eq_channel_c_band_13_gain : signed : 6.26 format
  0xd5000078, 0x04000000, 
// eq_channel_c_band_14_fc : unsigned : 20.12 format
  0xd5000079, 0x81f40000, 
// eq_channel_c_band_14_q : unsigned : 32.0 format
  0xd500007a, 0x088f5c30, 
// eq_channel_c_band_14_gain : signed : 6.26 format
  0xd500007b, 0x00000000, 
// eq_channel_c_band_15_fc : unsigned : 20.12 format
  0xd500007c, 0x830d4000, 
// eq_channel_c_band_15_q : unsigned : 32.0 format
  0xd500007d, 0x088f5c30, 
// eq_channel_c_band_15_gain : signed : 6.26 format
  0xd500007e, 0x00000000, 
// eq_channel_r_band_1_fc : unsigned : 20.12 format
  0xd500007f, 0x8274e000, 
// eq_channel_r_band_1_q : unsigned : 32.0 format
  0xd5000080, 0x20000000, 
// eq_channel_r_band_1_gain : signed : 6.26 format
  0xd5000081, 0xec000000, 
// eq_channel_r_band_2_fc : unsigned : 20.12 format
  0xd5000082, 0x80177000, 
// eq_channel_r_band_2_q : unsigned : 32.0 format
  0xd5000083, 0x04000000, 
// eq_channel_r_band_2_gain : signed : 6.26 format
  0xd5000084, 0x10000000, 
// eq_channel_r_band_3_fc : unsigned : 20.12 format
  0xd5000085, 0x80000000, 
// eq_channel_r_band_3_q : unsigned : 32.0 format
  0xd5000086, 0x00000000, 
// eq_channel_r_band_3_gain : signed : 6.26 format
  0xd5000087, 0x00000000, 
// eq_channel_r_band_4_fc : unsigned : 20.12 format
  0xd5000088, 0x80582000, 
// eq_channel_r_band_4_q : unsigned : 32.0 format
  0xd5000089, 0x10000000, 
// eq_channel_r_band_4_gain : signed : 6.26 format
  0xd500008a, 0xf0000000, 
// eq_channel_r_band_5_fc : unsigned : 20.12 format
  0xd500008b, 0x80000000, 
// eq_channel_r_band_5_q : unsigned : 32.0 format
  0xd500008c, 0x00000000, 
// eq_channel_r_band_5_gain : signed : 6.26 format
  0xd500008d, 0x00000000, 
// eq_channel_r_band_6_fc : unsigned : 20.12 format
  0xd500008e, 0x81176000, 
// eq_channel_r_band_6_q : unsigned : 32.0 format
  0xd500008f, 0x04ccccd0, 
// eq_channel_r_band_6_gain : signed : 6.26 format
  0xd5000090, 0x14000000, 
// eq_channel_r_band_7_fc : unsigned : 20.12 format
  0xd5000091, 0x800df000, 
// eq_channel_r_band_7_q : unsigned : 32.0 format
  0xd5000092, 0x20000000, 
// eq_channel_r_band_7_gain : signed : 6.26 format
  0xd5000093, 0xf4000000, 
// eq_channel_r_band_8_fc : unsigned : 20.12 format
  0xd5000094, 0x81f40000, 
// eq_channel_r_band_8_q : unsigned : 32.0 format
  0xd5000095, 0x03333334, 
// eq_channel_r_band_8_gain : signed : 6.26 format
  0xd5000096, 0x10000000, 
// eq_channel_r_band_9_fc : unsigned : 20.12 format
  0xd5000097, 0x8394a000, 
// eq_channel_r_band_9_q : unsigned : 32.0 format
  0xd5000098, 0x0c000000, 
// eq_channel_r_band_9_gain : signed : 6.26 format
  0xd5000099, 0x10000000, 
// eq_channel_r_band_10_fc : unsigned : 20.12 format
  0xd500009a, 0x808c0000, 
// eq_channel_r_band_10_q : unsigned : 32.0 format
  0xd500009b, 0x08000000, 
// eq_channel_r_band_10_gain : signed : 6.26 format
  0xd500009c, 0xf4000000, 
// eq_channel_r_band_11_fc : unsigned : 20.12 format
  0xd500009d, 0x84d29000, 
// eq_channel_r_band_11_q : unsigned : 32.0 format
  0xd500009e, 0x08000000, 
// eq_channel_r_band_11_gain : signed : 6.26 format
  0xd500009f, 0x18000000, 
// eq_channel_r_band_12_fc : unsigned : 20.12 format
  0xd50000a0, 0x830d4000, 
// eq_channel_r_band_12_q : unsigned : 32.0 format
  0xd50000a1, 0x18000000, 
// eq_channel_r_band_12_gain : signed : 6.26 format
  0xd50000a2, 0xf0000000, 
// eq_channel_r_band_13_fc : unsigned : 20.12 format
  0xd50000a3, 0x81388000, 
// eq_channel_r_band_13_q : unsigned : 32.0 format
  0xd50000a4, 0x088f5c30, 
// eq_channel_r_band_13_gain : signed : 6.26 format
  0xd50000a5, 0x00000000, 
// eq_channel_r_band_14_fc : unsigned : 20.12 format
  0xd50000a6, 0x81f40000, 
// eq_channel_r_band_14_q : unsigned : 32.0 format
  0xd50000a7, 0x088f5c30, 
// eq_channel_r_band_14_gain : signed : 6.26 format
  0xd50000a8, 0x00000000, 
// eq_channel_r_band_15_fc : unsigned : 20.12 format
  0xd50000a9, 0x830d4000, 
// eq_channel_r_band_15_q : unsigned : 32.0 format
  0xd50000aa, 0x088f5c30, 
// eq_channel_r_band_15_gain : signed : 6.26 format
  0xd50000ab, 0x00000000, 
// eq_channel_ls_band_1_fc : unsigned : 20.12 format
  0xd50000ac, 0x80140000, 
// eq_channel_ls_band_1_q : unsigned : 32.0 format
  0xd50000ad, 0x10000000, 
// eq_channel_ls_band_1_gain : signed : 6.26 format
  0xd50000ae, 0x20000000, 
// eq_channel_ls_band_2_fc : unsigned : 20.12 format
  0xd50000af, 0x80190000, 
// eq_channel_ls_band_2_q : unsigned : 32.0 format
  0xd50000b0, 0x10000000, 
// eq_channel_ls_band_2_gain : signed : 6.26 format
  0xd50000b1, 0x0c000000, 
// eq_channel_ls_band_3_fc : unsigned : 20.12 format
  0xd50000b2, 0x800dc000, 
// eq_channel_ls_band_3_q : unsigned : 32.0 format
  0xd50000b3, 0x12000000, 
// eq_channel_ls_band_3_gain : signed : 6.26 format
  0xd50000b4, 0xf8000000, 
// eq_channel_ls_band_4_fc : unsigned : 20.12 format
  0xd50000b5, 0x8037a000, 
// eq_channel_ls_band_4_q : unsigned : 32.0 format
  0xd50000b6, 0x10000000, 
// eq_channel_ls_band_4_gain : signed : 6.26 format
  0xd50000b7, 0xf8000000, 
// eq_channel_ls_band_5_fc : unsigned : 20.12 format
  0xd50000b8, 0x808c0000, 
// eq_channel_ls_band_5_q : unsigned : 32.0 format
  0xd50000b9, 0x0c000000, 
// eq_channel_ls_band_5_gain : signed : 6.26 format
  0xd50000ba, 0xe8000000, 
// eq_channel_ls_band_6_fc : unsigned : 20.12 format
  0xd50000bb, 0x80690000, 
// eq_channel_ls_band_6_q : unsigned : 32.0 format
  0xd50000bc, 0x20000000, 
// eq_channel_ls_band_6_gain : signed : 6.26 format
  0xd50000bd, 0xf4000000, 
// eq_channel_ls_band_7_fc : unsigned : 20.12 format
  0xd50000be, 0x80b04000, 
// eq_channel_ls_band_7_q : unsigned : 32.0 format
  0xd50000bf, 0x10000000, 
// eq_channel_ls_band_7_gain : signed : 6.26 format
  0xd50000c0, 0xf0000000, 
// eq_channel_ls_band_8_fc : unsigned : 20.12 format
  0xd50000c1, 0x81388000, 
// eq_channel_ls_band_8_q : unsigned : 32.0 format
  0xd50000c2, 0x0c000000, 
// eq_channel_ls_band_8_gain : signed : 6.26 format
  0xd50000c3, 0x18000000, 
// eq_channel_ls_band_9_fc : unsigned : 20.12 format
  0xd50000c4, 0x83a70000, 
// eq_channel_ls_band_9_q : unsigned : 32.0 format
  0xd50000c5, 0x08000000, 
// eq_channel_ls_band_9_gain : signed : 6.26 format
  0xd50000c6, 0x0c000000, 
// eq_channel_ls_band_10_fc : unsigned : 20.12 format
  0xd50000c7, 0x84e20000, 
// eq_channel_ls_band_10_q : unsigned : 32.0 format
  0xd50000c8, 0x04000000, 
// eq_channel_ls_band_10_gain : signed : 6.26 format
  0xd50000c9, 0x08000000, 
// eq_channel_ls_band_11_fc : unsigned : 20.12 format
  0xd50000ca, 0x8295e000, 
// eq_channel_ls_band_11_q : unsigned : 32.0 format
  0xd50000cb, 0x20000000, 
// eq_channel_ls_band_11_gain : signed : 6.26 format
  0xd50000cc, 0xe4000000, 
// eq_channel_ls_band_12_fc : unsigned : 20.12 format
  0xd50000cd, 0x81f40000, 
// eq_channel_ls_band_12_q : unsigned : 32.0 format
  0xd50000ce, 0x08000000, 
// eq_channel_ls_band_12_gain : signed : 6.26 format
  0xd50000cf, 0x08000000, 
// eq_channel_ls_band_13_fc : unsigned : 20.12 format
  0xd50000d0, 0x804ec000, 
// eq_channel_ls_band_13_q : unsigned : 32.0 format
  0xd50000d1, 0x18000000, 
// eq_channel_ls_band_13_gain : signed : 6.26 format
  0xd50000d2, 0xf4000000, 
// eq_channel_ls_band_14_fc : unsigned : 20.12 format
  0xd50000d3, 0x80064000, 
// eq_channel_ls_band_14_q : unsigned : 32.0 format
  0xd50000d4, 0x08000000, 
// eq_channel_ls_band_14_gain : signed : 6.26 format
  0xd50000d5, 0x08000000, 
// eq_channel_ls_band_15_fc : unsigned : 20.12 format
  0xd50000d6, 0x830d4000, 
// eq_channel_ls_band_15_q : unsigned : 32.0 format
  0xd50000d7, 0x088f5c30, 
// eq_channel_ls_band_15_gain : signed : 6.26 format
  0xd50000d8, 0x00000000, 
// eq_channel_rs_band_1_fc : unsigned : 20.12 format
  0xd50000d9, 0x80140000, 
// eq_channel_rs_band_1_q : unsigned : 32.0 format
  0xd50000da, 0x10000000, 
// eq_channel_rs_band_1_gain : signed : 6.26 format
  0xd50000db, 0x20000000, 
// eq_channel_rs_band_2_fc : unsigned : 20.12 format
  0xd50000dc, 0x80190000, 
// eq_channel_rs_band_2_q : unsigned : 32.0 format
  0xd50000dd, 0x10000000, 
// eq_channel_rs_band_2_gain : signed : 6.26 format
  0xd50000de, 0x0c000000, 
// eq_channel_rs_band_3_fc : unsigned : 20.12 format
  0xd50000df, 0x800dc000, 
// eq_channel_rs_band_3_q : unsigned : 32.0 format
  0xd50000e0, 0x12000000, 
// eq_channel_rs_band_3_gain : signed : 6.26 format
  0xd50000e1, 0xf8000000, 
// eq_channel_rs_band_4_fc : unsigned : 20.12 format
  0xd50000e2, 0x8037a000, 
// eq_channel_rs_band_4_q : unsigned : 32.0 format
  0xd50000e3, 0x10000000, 
// eq_channel_rs_band_4_gain : signed : 6.26 format
  0xd50000e4, 0xf8000000, 
// eq_channel_rs_band_5_fc : unsigned : 20.12 format
  0xd50000e5, 0x808c0000, 
// eq_channel_rs_band_5_q : unsigned : 32.0 format
  0xd50000e6, 0x0c000000, 
// eq_channel_rs_band_5_gain : signed : 6.26 format
  0xd50000e7, 0xe8000000, 
// eq_channel_rs_band_6_fc : unsigned : 20.12 format
  0xd50000e8, 0x80690000, 
// eq_channel_rs_band_6_q : unsigned : 32.0 format
  0xd50000e9, 0x20000000, 
// eq_channel_rs_band_6_gain : signed : 6.26 format
  0xd50000ea, 0xf4000000, 
// eq_channel_rs_band_7_fc : unsigned : 20.12 format
  0xd50000eb, 0x80b04000, 
// eq_channel_rs_band_7_q : unsigned : 32.0 format
  0xd50000ec, 0x10000000, 
// eq_channel_rs_band_7_gain : signed : 6.26 format
  0xd50000ed, 0xf0000000, 
// eq_channel_rs_band_8_fc : unsigned : 20.12 format
  0xd50000ee, 0x81388000, 
// eq_channel_rs_band_8_q : unsigned : 32.0 format
  0xd50000ef, 0x12000000, 
// eq_channel_rs_band_8_gain : signed : 6.26 format
  0xd50000f0, 0x18000000, 
// eq_channel_rs_band_9_fc : unsigned : 20.12 format
  0xd50000f1, 0x83a70000, 
// eq_channel_rs_band_9_q : unsigned : 32.0 format
  0xd50000f2, 0x08000000, 
// eq_channel_rs_band_9_gain : signed : 6.26 format
  0xd50000f3, 0x10000000, 
// eq_channel_rs_band_10_fc : unsigned : 20.12 format
  0xd50000f4, 0x84e20000, 
// eq_channel_rs_band_10_q : unsigned : 32.0 format
  0xd50000f5, 0x04000000, 
// eq_channel_rs_band_10_gain : signed : 6.26 format
  0xd50000f6, 0x0e000000, 
// eq_channel_rs_band_11_fc : unsigned : 20.12 format
  0xd50000f7, 0x8295e000, 
// eq_channel_rs_band_11_q : unsigned : 32.0 format
  0xd50000f8, 0x20000000, 
// eq_channel_rs_band_11_gain : signed : 6.26 format
  0xd50000f9, 0xe0000000, 
// eq_channel_rs_band_12_fc : unsigned : 20.12 format
  0xd50000fa, 0x81f40000, 
// eq_channel_rs_band_12_q : unsigned : 32.0 format
  0xd50000fb, 0x08000000, 
// eq_channel_rs_band_12_gain : signed : 6.26 format
  0xd50000fc, 0x08000000, 
// eq_channel_rs_band_13_fc : unsigned : 20.12 format
  0xd50000fd, 0x804ec000, 
// eq_channel_rs_band_13_q : unsigned : 32.0 format
  0xd50000fe, 0x18000000, 
// eq_channel_rs_band_13_gain : signed : 6.26 format
  0xd50000ff, 0xf4000000, 
// eq_channel_rs_band_14_fc : unsigned : 20.12 format
  0xd5000100, 0x80064000, 
// eq_channel_rs_band_14_q : unsigned : 32.0 format
  0xd5000101, 0x08000000, 
// eq_channel_rs_band_14_gain : signed : 6.26 format
  0xd5000102, 0x08000000, 
// eq_channel_rs_band_15_fc : unsigned : 20.12 format
  0xd5000103, 0x80fa0000, 
// eq_channel_rs_band_15_q : unsigned : 32.0 format
  0xd5000104, 0x18000000, 
// eq_channel_rs_band_15_gain : signed : 6.26 format
  0xd5000105, 0xf8000000, 
// eq_channel_lb_band_1_fc : unsigned : 20.12 format
  0xd5000106, 0x80140000, 
// eq_channel_lb_band_1_q : unsigned : 32.0 format
  0xd5000107, 0x10000000, 
// eq_channel_lb_band_1_gain : signed : 6.26 format
  0xd5000108, 0x20000000, 
// eq_channel_lb_band_2_fc : unsigned : 20.12 format
  0xd5000109, 0x80190000, 
// eq_channel_lb_band_2_q : unsigned : 32.0 format
  0xd500010a, 0x10000000, 
// eq_channel_lb_band_2_gain : signed : 6.26 format
  0xd500010b, 0x0c000000, 
// eq_channel_lb_band_3_fc : unsigned : 20.12 format
  0xd500010c, 0x800dc000, 
// eq_channel_lb_band_3_q : unsigned : 32.0 format
  0xd500010d, 0x12000000, 
// eq_channel_lb_band_3_gain : signed : 6.26 format
  0xd500010e, 0xf8000000, 
// eq_channel_lb_band_4_fc : unsigned : 20.12 format
  0xd500010f, 0x8037a000, 
// eq_channel_lb_band_4_q : unsigned : 32.0 format
  0xd5000110, 0x10000000, 
// eq_channel_lb_band_4_gain : signed : 6.26 format
  0xd5000111, 0xf8000000, 
// eq_channel_lb_band_5_fc : unsigned : 20.12 format
  0xd5000112, 0x808c0000, 
// eq_channel_lb_band_5_q : unsigned : 32.0 format
  0xd5000113, 0x0c000000, 
// eq_channel_lb_band_5_gain : signed : 6.26 format
  0xd5000114, 0xe8000000, 
// eq_channel_lb_band_6_fc : unsigned : 20.12 format
  0xd5000115, 0x80690000, 
// eq_channel_lb_band_6_q : unsigned : 32.0 format
  0xd5000116, 0x20000000, 
// eq_channel_lb_band_6_gain : signed : 6.26 format
  0xd5000117, 0xf4000000, 
// eq_channel_lb_band_7_fc : unsigned : 20.12 format
  0xd5000118, 0x80b04000, 
// eq_channel_lb_band_7_q : unsigned : 32.0 format
  0xd5000119, 0x10000000, 
// eq_channel_lb_band_7_gain : signed : 6.26 format
  0xd500011a, 0xf0000000, 
// eq_channel_lb_band_8_fc : unsigned : 20.12 format
  0xd500011b, 0x81388000, 
// eq_channel_lb_band_8_q : unsigned : 32.0 format
  0xd500011c, 0x0c000000, 
// eq_channel_lb_band_8_gain : signed : 6.26 format
  0xd500011d, 0x18000000, 
// eq_channel_lb_band_9_fc : unsigned : 20.12 format
  0xd500011e, 0x83a70000, 
// eq_channel_lb_band_9_q : unsigned : 32.0 format
  0xd500011f, 0x08000000, 
// eq_channel_lb_band_9_gain : signed : 6.26 format
  0xd5000120, 0x0c000000, 
// eq_channel_lb_band_10_fc : unsigned : 20.12 format
  0xd5000121, 0x84e20000, 
// eq_channel_lb_band_10_q : unsigned : 32.0 format
  0xd5000122, 0x04000000, 
// eq_channel_lb_band_10_gain : signed : 6.26 format
  0xd5000123, 0x08000000, 
// eq_channel_lb_band_11_fc : unsigned : 20.12 format
  0xd5000124, 0x8295e000, 
// eq_channel_lb_band_11_q : unsigned : 32.0 format
  0xd5000125, 0x20000000, 
// eq_channel_lb_band_11_gain : signed : 6.26 format
  0xd5000126, 0xe4000000, 
// eq_channel_lb_band_12_fc : unsigned : 20.12 format
  0xd5000127, 0x81f40000, 
// eq_channel_lb_band_12_q : unsigned : 32.0 format
  0xd5000128, 0x08000000, 
// eq_channel_lb_band_12_gain : signed : 6.26 format
  0xd5000129, 0x08000000, 
// eq_channel_lb_band_13_fc : unsigned : 20.12 format
  0xd500012a, 0x804ec000, 
// eq_channel_lb_band_13_q : unsigned : 32.0 format
  0xd500012b, 0x18000000, 
// eq_channel_lb_band_13_gain : signed : 6.26 format
  0xd500012c, 0xf4000000, 
// eq_channel_lb_band_14_fc : unsigned : 20.12 format
  0xd500012d, 0x80064000, 
// eq_channel_lb_band_14_q : unsigned : 32.0 format
  0xd500012e, 0x08000000, 
// eq_channel_lb_band_14_gain : signed : 6.26 format
  0xd500012f, 0x08000000, 
// eq_channel_lb_band_15_fc : unsigned : 20.12 format
  0xd5000130, 0x830d4000, 
// eq_channel_lb_band_15_q : unsigned : 32.0 format
  0xd5000131, 0x088f5c30, 
// eq_channel_lb_band_15_gain : signed : 6.26 format
  0xd5000132, 0x00000000, 
// eq_channel_rb_band_1_fc : unsigned : 20.12 format
  0xd5000133, 0x80140000, 
// eq_channel_rb_band_1_q : unsigned : 32.0 format
  0xd5000134, 0x10000000, 
// eq_channel_rb_band_1_gain : signed : 6.26 format
  0xd5000135, 0x20000000, 
// eq_channel_rb_band_2_fc : unsigned : 20.12 format
  0xd5000136, 0x80190000, 
// eq_channel_rb_band_2_q : unsigned : 32.0 format
  0xd5000137, 0x10000000, 
// eq_channel_rb_band_2_gain : signed : 6.26 format
  0xd5000138, 0x0c000000, 
// eq_channel_rb_band_3_fc : unsigned : 20.12 format
  0xd5000139, 0x800dc000, 
// eq_channel_rb_band_3_q : unsigned : 32.0 format
  0xd500013a, 0x12000000, 
// eq_channel_rb_band_3_gain : signed : 6.26 format
  0xd500013b, 0xf8000000, 
// eq_channel_rb_band_4_fc : unsigned : 20.12 format
  0xd500013c, 0x8037a000, 
// eq_channel_rb_band_4_q : unsigned : 32.0 format
  0xd500013d, 0x10000000, 
// eq_channel_rb_band_4_gain : signed : 6.26 format
  0xd500013e, 0xf8000000, 
// eq_channel_rb_band_5_fc : unsigned : 20.12 format
  0xd500013f, 0x808c0000, 
// eq_channel_rb_band_5_q : unsigned : 32.0 format
  0xd5000140, 0x0c000000, 
// eq_channel_rb_band_5_gain : signed : 6.26 format
  0xd5000141, 0xe8000000, 
// eq_channel_rb_band_6_fc : unsigned : 20.12 format
  0xd5000142, 0x80690000, 
// eq_channel_rb_band_6_q : unsigned : 32.0 format
  0xd5000143, 0x20000000, 
// eq_channel_rb_band_6_gain : signed : 6.26 format
  0xd5000144, 0xf4000000, 
// eq_channel_rb_band_7_fc : unsigned : 20.12 format
  0xd5000145, 0x80b04000, 
// eq_channel_rb_band_7_q : unsigned : 32.0 format
  0xd5000146, 0x10000000, 
// eq_channel_rb_band_7_gain : signed : 6.26 format
  0xd5000147, 0xf0000000, 
// eq_channel_rb_band_8_fc : unsigned : 20.12 format
  0xd5000148, 0x81388000, 
// eq_channel_rb_band_8_q : unsigned : 32.0 format
  0xd5000149, 0x12000000, 
// eq_channel_rb_band_8_gain : signed : 6.26 format
  0xd500014a, 0x18000000, 
// eq_channel_rb_band_9_fc : unsigned : 20.12 format
  0xd500014b, 0x83a70000, 
// eq_channel_rb_band_9_q : unsigned : 32.0 format
  0xd500014c, 0x08000000, 
// eq_channel_rb_band_9_gain : signed : 6.26 format
  0xd500014d, 0x10000000, 
// eq_channel_rb_band_10_fc : unsigned : 20.12 format
  0xd500014e, 0x84e20000, 
// eq_channel_rb_band_10_q : unsigned : 32.0 format
  0xd500014f, 0x04000000, 
// eq_channel_rb_band_10_gain : signed : 6.26 format
  0xd5000150, 0x0e000000, 
// eq_channel_rb_band_11_fc : unsigned : 20.12 format
  0xd5000151, 0x8295e000, 
// eq_channel_rb_band_11_q : unsigned : 32.0 format
  0xd5000152, 0x20000000, 
// eq_channel_rb_band_11_gain : signed : 6.26 format
  0xd5000153, 0xe0000000, 
// eq_channel_rb_band_12_fc : unsigned : 20.12 format
  0xd5000154, 0x81f40000, 
// eq_channel_rb_band_12_q : unsigned : 32.0 format
  0xd5000155, 0x08000000, 
// eq_channel_rb_band_12_gain : signed : 6.26 format
  0xd5000156, 0x08000000, 
// eq_channel_rb_band_13_fc : unsigned : 20.12 format
  0xd5000157, 0x804ec000, 
// eq_channel_rb_band_13_q : unsigned : 32.0 format
  0xd5000158, 0x18000000, 
// eq_channel_rb_band_13_gain : signed : 6.26 format
  0xd5000159, 0xf4000000, 
// eq_channel_rb_band_14_fc : unsigned : 20.12 format
  0xd500015a, 0x80064000, 
// eq_channel_rb_band_14_q : unsigned : 32.0 format
  0xd500015b, 0x08000000, 
// eq_channel_rb_band_14_gain : signed : 6.26 format
  0xd500015c, 0x08000000, 
// eq_channel_rb_band_15_fc : unsigned : 20.12 format
  0xd500015d, 0x80fa0000, 
// eq_channel_rb_band_15_q : unsigned : 32.0 format
  0xd500015e, 0x18000000, 
// eq_channel_rb_band_15_gain : signed : 6.26 format
  0xd500015f, 0xf8000000, 
// eq_channel_lfe0_band_1_fc : unsigned : 20.12 format
  0xd5000160, 0x80014000, 
// eq_channel_lfe0_band_1_q : unsigned : 32.0 format
  0xd5000161, 0x04000000, 
// eq_channel_lfe0_band_1_gain : signed : 6.26 format
  0xd5000162, 0x08000000, 
// eq_channel_lfe0_band_2_fc : unsigned : 20.12 format
  0xd5000163, 0x80021800, 
// eq_channel_lfe0_band_2_q : unsigned : 32.0 format
  0xd5000164, 0x28000000, 
// eq_channel_lfe0_band_2_gain : signed : 6.26 format
  0xd5000165, 0xe0000000, 
// eq_channel_lfe0_band_3_fc : unsigned : 20.12 format
  0xd5000166, 0x8004e83c, 
// eq_channel_lfe0_band_3_q : unsigned : 32.0 format
  0xd5000167, 0x06ccccd0, 
// eq_channel_lfe0_band_3_gain : signed : 6.26 format
  0xd5000168, 0x10000000, 
// eq_channel_lfe0_band_4_fc : unsigned : 20.12 format
  0xd5000169, 0x8006c000, 
// eq_channel_lfe0_band_4_q : unsigned : 32.0 format
  0xd500016a, 0x04000000, 
// eq_channel_lfe0_band_4_gain : signed : 6.26 format
  0xd500016b, 0x0c000000, 
// eq_channel_lfe0_band_5_fc : unsigned : 20.12 format
  0xd500016c, 0x8007d000, 
// eq_channel_lfe0_band_5_q : unsigned : 32.0 format
  0xd500016d, 0x088f5c30, 
// eq_channel_lfe0_band_5_gain : signed : 6.26 format
  0xd500016e, 0x00000000, 
// eq_channel_lfe0_band_6_fc : unsigned : 20.12 format
  0xd500016f, 0x800c8000, 
// eq_channel_lfe0_band_6_q : unsigned : 32.0 format
  0xd5000170, 0x088f5c30, 
// eq_channel_lfe0_band_6_gain : signed : 6.26 format
  0xd5000171, 0x00000000, 
// eq_channel_lfe0_band_7_fc : unsigned : 20.12 format
  0xd5000172, 0x8013b000, 
// eq_channel_lfe0_band_7_q : unsigned : 32.0 format
  0xd5000173, 0x088f5c30, 
// eq_channel_lfe0_band_7_gain : signed : 6.26 format
  0xd5000174, 0x00000000, 
// eq_channel_lfe0_band_8_fc : unsigned : 20.12 format
  0xd5000175, 0x801f4000, 
// eq_channel_lfe0_band_8_q : unsigned : 32.0 format
  0xd5000176, 0x088f5c30, 
// eq_channel_lfe0_band_8_gain : signed : 6.26 format
  0xd5000177, 0x00000000, 
// eq_channel_lfe0_band_9_fc : unsigned : 20.12 format
  0xd5000178, 0x80320000, 
// eq_channel_lfe0_band_9_q : unsigned : 32.0 format
  0xd5000179, 0x088f5c30, 
// eq_channel_lfe0_band_9_gain : signed : 6.26 format
  0xd500017a, 0x00000000, 
// eq_channel_lfe0_band_10_fc : unsigned : 20.12 format
  0xd500017b, 0x804e2000, 
// eq_channel_lfe0_band_10_q : unsigned : 32.0 format
  0xd500017c, 0x088f5c30, 
// eq_channel_lfe0_band_10_gain : signed : 6.26 format
  0xd500017d, 0x00000000, 
// eq_channel_lfe0_band_11_fc : unsigned : 20.12 format
  0xd500017e, 0x807d0000, 
// eq_channel_lfe0_band_11_q : unsigned : 32.0 format
  0xd500017f, 0x088f5c30, 
// eq_channel_lfe0_band_11_gain : signed : 6.26 format
  0xd5000180, 0x00000000, 
// eq_channel_lfe0_band_12_fc : unsigned : 20.12 format
  0xd5000181, 0x80c4e000, 
// eq_channel_lfe0_band_12_q : unsigned : 32.0 format
  0xd5000182, 0x088f5c30, 
// eq_channel_lfe0_band_12_gain : signed : 6.26 format
  0xd5000183, 0x00000000, 
// eq_channel_lfe0_band_13_fc : unsigned : 20.12 format
  0xd5000184, 0x81388000, 
// eq_channel_lfe0_band_13_q : unsigned : 32.0 format
  0xd5000185, 0x088f5c30, 
// eq_channel_lfe0_band_13_gain : signed : 6.26 format
  0xd5000186, 0x00000000, 
// eq_channel_lfe0_band_14_fc : unsigned : 20.12 format
  0xd5000187, 0x81f40000, 
// eq_channel_lfe0_band_14_q : unsigned : 32.0 format
  0xd5000188, 0x088f5c30, 
// eq_channel_lfe0_band_14_gain : signed : 6.26 format
  0xd5000189, 0x00000000, 
// eq_channel_lfe0_band_15_fc : unsigned : 20.12 format
  0xd500018a, 0x830d4000, 
// eq_channel_lfe0_band_15_q : unsigned : 32.0 format
  0xd500018b, 0x088f5c30, 
// eq_channel_lfe0_band_15_gain : signed : 6.26 format
  0xd500018c, 0x00000000, 
// eq_channel_lfe1_band_1_fc : unsigned : 20.12 format
  0xd500018d, 0x00096000, 
// eq_channel_lfe1_band_1_q : unsigned : 32.0 format
  0xd500018e, 0x00000000, 
// eq_channel_lfe1_band_1_gain : signed : 6.26 format
  0xd500018f, 0x00000000, 
// eq_channel_lfe1_band_2_fc : unsigned : 20.12 format
  0xd5000190, 0x000d2000, 
// eq_channel_lfe1_band_2_q : unsigned : 32.0 format
  0xd5000191, 0x00000000, 
// eq_channel_lfe1_band_2_gain : signed : 6.26 format
  0xd5000192, 0x00000000, 
// eq_channel_lfe1_band_3_fc : unsigned : 20.12 format
  0xd5000193, 0x000fa000, 
// eq_channel_lfe1_band_3_q : unsigned : 32.0 format
  0xd5000194, 0x00000000, 
// eq_channel_lfe1_band_3_gain : signed : 6.26 format
  0xd5000195, 0x00000000, 
// eq_channel_lfe1_band_4_fc : unsigned : 20.12 format
  0xd5000196, 0x0037a000, 
// eq_channel_lfe1_band_4_q : unsigned : 32.0 format
  0xd5000197, 0x00000000, 
// eq_channel_lfe1_band_4_gain : signed : 6.26 format
  0xd5000198, 0x00000000, 
// eq_channel_lfe1_band_5_fc : unsigned : 20.12 format
  0xd5000199, 0x008c0000, 
// eq_channel_lfe1_band_5_q : unsigned : 32.0 format
  0xd500019a, 0x00000000, 
// eq_channel_lfe1_band_5_gain : signed : 6.26 format
  0xd500019b, 0x00000000, 
// eq_channel_lfe1_band_6_fc : unsigned : 20.12 format
  0xd500019c, 0x00fa0000, 
// eq_channel_lfe1_band_6_q : unsigned : 32.0 format
  0xd500019d, 0x00000000, 
// eq_channel_lfe1_band_6_gain : signed : 6.26 format
  0xd500019e, 0x00000000, 
// eq_channel_lfe1_band_7_fc : unsigned : 20.12 format
  0xd500019f, 0x04844000, 
// eq_channel_lfe1_band_7_q : unsigned : 32.0 format
  0xd50001a0, 0x00000000, 
// eq_channel_lfe1_band_7_gain : signed : 6.26 format
  0xd50001a1, 0x00000000, 
// eq_channel_lfe1_band_8_fc : unsigned : 20.12 format
  0xd50001a2, 0x022ce000, 
// eq_channel_lfe1_band_8_q : unsigned : 32.0 format
  0xd50001a3, 0x00000000, 
// eq_channel_lfe1_band_8_gain : signed : 6.26 format
  0xd50001a4, 0x00000000, 
// eq_channel_lfe1_band_9_fc : unsigned : 20.12 format
  0xd50001a5, 0x02710000, 
// eq_channel_lfe1_band_9_q : unsigned : 32.0 format
  0xd50001a6, 0x00000000, 
// eq_channel_lfe1_band_9_gain : signed : 6.26 format
  0xd50001a7, 0x00000000, 
// eq_channel_lfe1_band_10_fc : unsigned : 20.12 format
  0xd50001a8, 0x002ee000, 
// eq_channel_lfe1_band_10_q : unsigned : 32.0 format
  0xd50001a9, 0x00000000, 
// eq_channel_lfe1_band_10_gain : signed : 6.26 format
  0xd50001aa, 0x00000000, 
// eq_channel_lfe1_band_11_fc : unsigned : 20.12 format
  0xd50001ab, 0x01b58000, 
// eq_channel_lfe1_band_11_q : unsigned : 32.0 format
  0xd50001ac, 0x00000000, 
// eq_channel_lfe1_band_11_gain : signed : 6.26 format
  0xd50001ad, 0x00000000, 
// eq_channel_lfe1_band_12_fc : unsigned : 20.12 format
  0xd50001ae, 0x000df000, 
// eq_channel_lfe1_band_12_q : unsigned : 32.0 format
  0xd50001af, 0x00000000, 
// eq_channel_lfe1_band_12_gain : signed : 6.26 format
  0xd50001b0, 0x00000000, 
// eq_channel_lfe1_band_13_fc : unsigned : 20.12 format
  0xd50001b1, 0x0003c000, 
// eq_channel_lfe1_band_13_q : unsigned : 32.0 format
  0xd50001b2, 0x00000000, 
// eq_channel_lfe1_band_13_gain : signed : 6.26 format
  0xd50001b3, 0x00000000, 
// eq_channel_lfe1_band_14_fc : unsigned : 20.12 format
  0xd50001b4, 0x01f40000, 
// eq_channel_lfe1_band_14_q : unsigned : 32.0 format
  0xd50001b5, 0x00000000, 
// eq_channel_lfe1_band_14_gain : signed : 6.26 format
  0xd50001b6, 0x00000000, 
// eq_channel_lfe1_band_15_fc : unsigned : 20.12 format
  0xd50001b7, 0x030d4000, 
// eq_channel_lfe1_band_15_q : unsigned : 32.0 format
  0xd50001b8, 0x00000000, 
// eq_channel_lfe1_band_15_gain : signed : 6.26 format
  0xd50001b9, 0x00000000, 
// eq_channel_lfe2_band_1_fc : unsigned : 20.12 format
  0xd50001ba, 0x00096000, 
// eq_channel_lfe2_band_1_q : unsigned : 32.0 format
  0xd50001bb, 0x00000000, 
// eq_channel_lfe2_band_1_gain : signed : 6.26 format
  0xd50001bc, 0x00000000, 
// eq_channel_lfe2_band_2_fc : unsigned : 20.12 format
  0xd50001bd, 0x000d2000, 
// eq_channel_lfe2_band_2_q : unsigned : 32.0 format
  0xd50001be, 0x00000000, 
// eq_channel_lfe2_band_2_gain : signed : 6.26 format
  0xd50001bf, 0x00000000, 
// eq_channel_lfe2_band_3_fc : unsigned : 20.12 format
  0xd50001c0, 0x000fa000, 
// eq_channel_lfe2_band_3_q : unsigned : 32.0 format
  0xd50001c1, 0x00000000, 
// eq_channel_lfe2_band_3_gain : signed : 6.26 format
  0xd50001c2, 0x00000000, 
// eq_channel_lfe2_band_4_fc : unsigned : 20.12 format
  0xd50001c3, 0x0037a000, 
// eq_channel_lfe2_band_4_q : unsigned : 32.0 format
  0xd50001c4, 0x00000000, 
// eq_channel_lfe2_band_4_gain : signed : 6.26 format
  0xd50001c5, 0x00000000, 
// eq_channel_lfe2_band_5_fc : unsigned : 20.12 format
  0xd50001c6, 0x008c0000, 
// eq_channel_lfe2_band_5_q : unsigned : 32.0 format
  0xd50001c7, 0x00000000, 
// eq_channel_lfe2_band_5_gain : signed : 6.26 format
  0xd50001c8, 0x00000000, 
// eq_channel_lfe2_band_6_fc : unsigned : 20.12 format
  0xd50001c9, 0x00fa0000, 
// eq_channel_lfe2_band_6_q : unsigned : 32.0 format
  0xd50001ca, 0x00000000, 
// eq_channel_lfe2_band_6_gain : signed : 6.26 format
  0xd50001cb, 0x00000000, 
// eq_channel_lfe2_band_7_fc : unsigned : 20.12 format
  0xd50001cc, 0x04844000, 
// eq_channel_lfe2_band_7_q : unsigned : 32.0 format
  0xd50001cd, 0x00000000, 
// eq_channel_lfe2_band_7_gain : signed : 6.26 format
  0xd50001ce, 0x00000000, 
// eq_channel_lfe2_band_8_fc : unsigned : 20.12 format
  0xd50001cf, 0x022ce000, 
// eq_channel_lfe2_band_8_q : unsigned : 32.0 format
  0xd50001d0, 0x00000000, 
// eq_channel_lfe2_band_8_gain : signed : 6.26 format
  0xd50001d1, 0x00000000, 
// eq_channel_lfe2_band_9_fc : unsigned : 20.12 format
  0xd50001d2, 0x02710000, 
// eq_channel_lfe2_band_9_q : unsigned : 32.0 format
  0xd50001d3, 0x00000000, 
// eq_channel_lfe2_band_9_gain : signed : 6.26 format
  0xd50001d4, 0x00000000, 
// eq_channel_lfe2_band_10_fc : unsigned : 20.12 format
  0xd50001d5, 0x002ee000, 
// eq_channel_lfe2_band_10_q : unsigned : 32.0 format
  0xd50001d6, 0x00000000, 
// eq_channel_lfe2_band_10_gain : signed : 6.26 format
  0xd50001d7, 0x00000000, 
// eq_channel_lfe2_band_11_fc : unsigned : 20.12 format
  0xd50001d8, 0x01b58000, 
// eq_channel_lfe2_band_11_q : unsigned : 32.0 format
  0xd50001d9, 0x00000000, 
// eq_channel_lfe2_band_11_gain : signed : 6.26 format
  0xd50001da, 0x00000000, 
// eq_channel_lfe2_band_12_fc : unsigned : 20.12 format
  0xd50001db, 0x000df000, 
// eq_channel_lfe2_band_12_q : unsigned : 32.0 format
  0xd50001dc, 0x00000000, 
// eq_channel_lfe2_band_12_gain : signed : 6.26 format
  0xd50001dd, 0x00000000, 
// eq_channel_lfe2_band_13_fc : unsigned : 20.12 format
  0xd50001de, 0x0003c000, 
// eq_channel_lfe2_band_13_q : unsigned : 32.0 format
  0xd50001df, 0x00000000, 
// eq_channel_lfe2_band_13_gain : signed : 6.26 format
  0xd50001e0, 0x00000000, 
// eq_channel_lfe2_band_14_fc : unsigned : 20.12 format
  0xd50001e1, 0x01f40000, 
// eq_channel_lfe2_band_14_q : unsigned : 32.0 format
  0xd50001e2, 0x00000000, 
// eq_channel_lfe2_band_14_gain : signed : 6.26 format
  0xd50001e3, 0x00000000, 
// eq_channel_lfe2_band_15_fc : unsigned : 20.12 format
  0xd50001e4, 0x030d4000, 
// eq_channel_lfe2_band_15_q : unsigned : 32.0 format
  0xd50001e5, 0x00000000, 
// eq_channel_lfe2_band_15_gain : signed : 6.26 format
  0xd50001e6, 0x00000000, 
// eq_channel_lfe3_band_1_fc : unsigned : 20.12 format
  0xd50001e7, 0x00014000, 
// eq_channel_lfe3_band_1_q : unsigned : 32.0 format
  0xd50001e8, 0x00000000, 
// eq_channel_lfe3_band_1_gain : signed : 6.26 format
  0xd50001e9, 0x00000000, 
// eq_channel_lfe3_band_2_fc : unsigned : 20.12 format
  0xd50001ea, 0x0001f800, 
// eq_channel_lfe3_band_2_q : unsigned : 32.0 format
  0xd50001eb, 0x00000000, 
// eq_channel_lfe3_band_2_gain : signed : 6.26 format
  0xd50001ec, 0x00000000, 
// eq_channel_lfe3_band_3_fc : unsigned : 20.12 format
  0xd50001ed, 0x00032000, 
// eq_channel_lfe3_band_3_q : unsigned : 32.0 format
  0xd50001ee, 0x00000000, 
// eq_channel_lfe3_band_3_gain : signed : 6.26 format
  0xd50001ef, 0x00000000, 
// eq_channel_lfe3_band_4_fc : unsigned : 20.12 format
  0xd50001f0, 0x00050000, 
// eq_channel_lfe3_band_4_q : unsigned : 32.0 format
  0xd50001f1, 0x00000000, 
// eq_channel_lfe3_band_4_gain : signed : 6.26 format
  0xd50001f2, 0x00000000, 
// eq_channel_lfe3_band_5_fc : unsigned : 20.12 format
  0xd50001f3, 0x0007d000, 
// eq_channel_lfe3_band_5_q : unsigned : 32.0 format
  0xd50001f4, 0x00000000, 
// eq_channel_lfe3_band_5_gain : signed : 6.26 format
  0xd50001f5, 0x00000000, 
// eq_channel_lfe3_band_6_fc : unsigned : 20.12 format
  0xd50001f6, 0x000c8000, 
// eq_channel_lfe3_band_6_q : unsigned : 32.0 format
  0xd50001f7, 0x00000000, 
// eq_channel_lfe3_band_6_gain : signed : 6.26 format
  0xd50001f8, 0x00000000, 
// eq_channel_lfe3_band_7_fc : unsigned : 20.12 format
  0xd50001f9, 0x0013b000, 
// eq_channel_lfe3_band_7_q : unsigned : 32.0 format
  0xd50001fa, 0x00000000, 
// eq_channel_lfe3_band_7_gain : signed : 6.26 format
  0xd50001fb, 0x00000000, 
// eq_channel_lfe3_band_8_fc : unsigned : 20.12 format
  0xd50001fc, 0x001f4000, 
// eq_channel_lfe3_band_8_q : unsigned : 32.0 format
  0xd50001fd, 0x00000000, 
// eq_channel_lfe3_band_8_gain : signed : 6.26 format
  0xd50001fe, 0x00000000, 
// eq_channel_lfe3_band_9_fc : unsigned : 20.12 format
  0xd50001ff, 0x00320000, 
// eq_channel_lfe3_band_9_q : unsigned : 32.0 format
  0xd5000200, 0x00000000, 
// eq_channel_lfe3_band_9_gain : signed : 6.26 format
  0xd5000201, 0x00000000, 
// eq_channel_lfe3_band_10_fc : unsigned : 20.12 format
  0xd5000202, 0x004e2000, 
// eq_channel_lfe3_band_10_q : unsigned : 32.0 format
  0xd5000203, 0x00000000, 
// eq_channel_lfe3_band_10_gain : signed : 6.26 format
  0xd5000204, 0x00000000, 
// eq_channel_lfe3_band_11_fc : unsigned : 20.12 format
  0xd5000205, 0x007d0000, 
// eq_channel_lfe3_band_11_q : unsigned : 32.0 format
  0xd5000206, 0x00000000, 
// eq_channel_lfe3_band_11_gain : signed : 6.26 format
  0xd5000207, 0x00000000, 
// eq_channel_lfe3_band_12_fc : unsigned : 20.12 format
  0xd5000208, 0x00c4e000, 
// eq_channel_lfe3_band_12_q : unsigned : 32.0 format
  0xd5000209, 0x00000000, 
// eq_channel_lfe3_band_12_gain : signed : 6.26 format
  0xd500020a, 0x00000000, 
// eq_channel_lfe3_band_13_fc : unsigned : 20.12 format
  0xd500020b, 0x01388000, 
// eq_channel_lfe3_band_13_q : unsigned : 32.0 format
  0xd500020c, 0x00000000, 
// eq_channel_lfe3_band_13_gain : signed : 6.26 format
  0xd500020d, 0x00000000, 
// eq_channel_lfe3_band_14_fc : unsigned : 20.12 format
  0xd500020e, 0x01f40000, 
// eq_channel_lfe3_band_14_q : unsigned : 32.0 format
  0xd500020f, 0x00000000, 
// eq_channel_lfe3_band_14_gain : signed : 6.26 format
  0xd5000210, 0x00000000, 
// eq_channel_lfe3_band_15_fc : unsigned : 20.12 format
  0xd5000211, 0x030d4000, 
// eq_channel_lfe3_band_15_q : unsigned : 32.0 format
  0xd5000212, 0x00000000, 
// eq_channel_lfe3_band_15_gain : signed : 6.26 format
  0xd5000213, 0x00000000, 
// eq_channel_res_band_1_fc : unsigned : 20.12 format
  0xd5000214, 0x00014000, 
// eq_channel_res_band_1_q : unsigned : 32.0 format
  0xd5000215, 0x00000000, 
// eq_channel_res_band_1_gain : signed : 6.26 format
  0xd5000216, 0x00000000, 
// eq_channel_res_band_2_fc : unsigned : 20.12 format
  0xd5000217, 0x0001f800, 
// eq_channel_res_band_2_q : unsigned : 32.0 format
  0xd5000218, 0x00000000, 
// eq_channel_res_band_2_gain : signed : 6.26 format
  0xd5000219, 0x00000000, 
// eq_channel_res_band_3_fc : unsigned : 20.12 format
  0xd500021a, 0x00032000, 
// eq_channel_res_band_3_q : unsigned : 32.0 format
  0xd500021b, 0x00000000, 
// eq_channel_res_band_3_gain : signed : 6.26 format
  0xd500021c, 0x00000000, 
// eq_channel_res_band_4_fc : unsigned : 20.12 format
  0xd500021d, 0x00050000, 
// eq_channel_res_band_4_q : unsigned : 32.0 format
  0xd500021e, 0x00000000, 
// eq_channel_res_band_4_gain : signed : 6.26 format
  0xd500021f, 0x00000000, 
// eq_channel_res_band_5_fc : unsigned : 20.12 format
  0xd5000220, 0x0007d000, 
// eq_channel_res_band_5_q : unsigned : 32.0 format
  0xd5000221, 0x00000000, 
// eq_channel_res_band_5_gain : signed : 6.26 format
  0xd5000222, 0x00000000, 
// eq_channel_res_band_6_fc : unsigned : 20.12 format
  0xd5000223, 0x000c8000, 
// eq_channel_res_band_6_q : unsigned : 32.0 format
  0xd5000224, 0x00000000, 
// eq_channel_res_band_6_gain : signed : 6.26 format
  0xd5000225, 0x00000000, 
// eq_channel_res_band_7_fc : unsigned : 20.12 format
  0xd5000226, 0x0013b000, 
// eq_channel_res_band_7_q : unsigned : 32.0 format
  0xd5000227, 0x00000000, 
// eq_channel_res_band_7_gain : signed : 6.26 format
  0xd5000228, 0x00000000, 
// eq_channel_res_band_8_fc : unsigned : 20.12 format
  0xd5000229, 0x001f4000, 
// eq_channel_res_band_8_q : unsigned : 32.0 format
  0xd500022a, 0x00000000, 
// eq_channel_res_band_8_gain : signed : 6.26 format
  0xd500022b, 0x00000000, 
// eq_channel_res_band_9_fc : unsigned : 20.12 format
  0xd500022c, 0x00320000, 
// eq_channel_res_band_9_q : unsigned : 32.0 format
  0xd500022d, 0x00000000, 
// eq_channel_res_band_9_gain : signed : 6.26 format
  0xd500022e, 0x00000000, 
// eq_channel_res_band_10_fc : unsigned : 20.12 format
  0xd500022f, 0x004e2000, 
// eq_channel_res_band_10_q : unsigned : 32.0 format
  0xd5000230, 0x00000000, 
// eq_channel_res_band_10_gain : signed : 6.26 format
  0xd5000231, 0x00000000, 
// eq_channel_res_band_11_fc : unsigned : 20.12 format
  0xd5000232, 0x007d0000, 
// eq_channel_res_band_11_q : unsigned : 32.0 format
  0xd5000233, 0x00000000, 
// eq_channel_res_band_11_gain : signed : 6.26 format
  0xd5000234, 0x00000000, 
// eq_channel_res_band_12_fc : unsigned : 20.12 format
  0xd5000235, 0x00c4e000, 
// eq_channel_res_band_12_q : unsigned : 32.0 format
  0xd5000236, 0x00000000, 
// eq_channel_res_band_12_gain : signed : 6.26 format
  0xd5000237, 0x00000000, 
// eq_channel_res_band_13_fc : unsigned : 20.12 format
  0xd5000238, 0x01388000, 
// eq_channel_res_band_13_q : unsigned : 32.0 format
  0xd5000239, 0x00000000, 
// eq_channel_res_band_13_gain : signed : 6.26 format
  0xd500023a, 0x00000000, 
// eq_channel_res_band_14_fc : unsigned : 20.12 format
  0xd500023b, 0x01f40000, 
// eq_channel_res_band_14_q : unsigned : 32.0 format
  0xd500023c, 0x00000000, 
// eq_channel_res_band_14_gain : signed : 6.26 format
  0xd500023d, 0x00000000, 
// eq_channel_res_band_15_fc : unsigned : 20.12 format
  0xd500023e, 0x030d4000, 
// eq_channel_res_band_15_q : unsigned : 32.0 format
  0xd500023f, 0x00000000, 
// eq_channel_res_band_15_gain : signed : 6.26 format
  0xd5000240, 0x00000000, 
// eq_channel_lz_band_1_fc : unsigned : 20.12 format
  0xd5000241, 0x8012c000, 
// eq_channel_lz_band_1_q : unsigned : 32.0 format
  0xd5000242, 0x08000000, 
// eq_channel_lz_band_1_gain : signed : 6.26 format
  0xd5000243, 0x10000000, 
// eq_channel_lz_band_2_fc : unsigned : 20.12 format
  0xd5000244, 0x8002523d, 
// eq_channel_lz_band_2_q : unsigned : 32.0 format
  0xd5000245, 0x03999998, 
// eq_channel_lz_band_2_gain : signed : 6.26 format
  0xd5000246, 0x13d0f520, 
// eq_channel_lz_band_3_fc : unsigned : 20.12 format
  0xd5000247, 0x807d0000, 
// eq_channel_lz_band_3_q : unsigned : 32.0 format
  0xd5000248, 0x08000000, 
// eq_channel_lz_band_3_gain : signed : 6.26 format
  0xd5000249, 0xf0000000, 
// eq_channel_lz_band_4_fc : unsigned : 20.12 format
  0xd500024a, 0x80000000, 
// eq_channel_lz_band_4_q : unsigned : 32.0 format
  0xd500024b, 0x00000000, 
// eq_channel_lz_band_4_gain : signed : 6.26 format
  0xd500024c, 0x00000000, 
// eq_channel_lz_band_5_fc : unsigned : 20.12 format
  0xd500024d, 0x814be000, 
// eq_channel_lz_band_5_q : unsigned : 32.0 format
  0xd500024e, 0x18000000, 
// eq_channel_lz_band_5_gain : signed : 6.26 format
  0xd500024f, 0xec000000, 
// eq_channel_lz_band_6_fc : unsigned : 20.12 format
  0xd5000250, 0x80000000, 
// eq_channel_lz_band_6_q : unsigned : 32.0 format
  0xd5000251, 0x00000000, 
// eq_channel_lz_band_6_gain : signed : 6.26 format
  0xd5000252, 0x00000000, 
// eq_channel_lz_band_7_fc : unsigned : 20.12 format
  0xd5000253, 0x80b04000, 
// eq_channel_lz_band_7_q : unsigned : 32.0 format
  0xd5000254, 0x18000000, 
// eq_channel_lz_band_7_gain : signed : 6.26 format
  0xd5000255, 0xf8000000, 
// eq_channel_lz_band_8_fc : unsigned : 20.12 format
  0xd5000256, 0x80e10000, 
// eq_channel_lz_band_8_q : unsigned : 32.0 format
  0xd5000257, 0x12000000, 
// eq_channel_lz_band_8_gain : signed : 6.26 format
  0xd5000258, 0x0c000000, 
// eq_channel_lz_band_9_fc : unsigned : 20.12 format
  0xd5000259, 0x83a70000, 
// eq_channel_lz_band_9_q : unsigned : 32.0 format
  0xd500025a, 0x04ccccd0, 
// eq_channel_lz_band_9_gain : signed : 6.26 format
  0xd500025b, 0x18000000, 
// eq_channel_lz_band_10_fc : unsigned : 20.12 format
  0xd500025c, 0x82710000, 
// eq_channel_lz_band_10_q : unsigned : 32.0 format
  0xd500025d, 0x20000000, 
// eq_channel_lz_band_10_gain : signed : 6.26 format
  0xd500025e, 0xe4000000, 
// eq_channel_lz_band_11_fc : unsigned : 20.12 format
  0xd500025f, 0x80050805, 
// eq_channel_lz_band_11_q : unsigned : 32.0 format
  0xd5000260, 0x04000000, 
// eq_channel_lz_band_11_gain : signed : 6.26 format
  0xd5000261, 0x151c3380, 
// eq_channel_lz_band_12_fc : unsigned : 20.12 format
  0xd5000262, 0x84e20000, 
// eq_channel_lz_band_12_q : unsigned : 32.0 format
  0xd5000263, 0x03333334, 
// eq_channel_lz_band_12_gain : signed : 6.26 format
  0xd5000264, 0x18000000, 
// eq_channel_lz_band_13_fc : unsigned : 20.12 format
  0xd5000265, 0x800a0000, 
// eq_channel_lz_band_13_q : unsigned : 32.0 format
  0xd5000266, 0x0c000000, 
// eq_channel_lz_band_13_gain : signed : 6.26 format
  0xd5000267, 0x0c000000, 
// eq_channel_lz_band_14_fc : unsigned : 20.12 format
  0xd5000268, 0x81f40000, 
// eq_channel_lz_band_14_q : unsigned : 32.0 format
  0xd5000269, 0x088f5c30, 
// eq_channel_lz_band_14_gain : signed : 6.26 format
  0xd500026a, 0x00000000, 
// eq_channel_lz_band_15_fc : unsigned : 20.12 format
  0xd500026b, 0x830d4000, 
// eq_channel_lz_band_15_q : unsigned : 32.0 format
  0xd500026c, 0x088f5c30, 
// eq_channel_lz_band_15_gain : signed : 6.26 format
  0xd500026d, 0x00000000, 
// eq_channel_rz_band_1_fc : unsigned : 20.12 format
  0xd500026e, 0x8012c000, 
// eq_channel_rz_band_1_q : unsigned : 32.0 format
  0xd500026f, 0x08000000, 
// eq_channel_rz_band_1_gain : signed : 6.26 format
  0xd5000270, 0x10000000, 
// eq_channel_rz_band_2_fc : unsigned : 20.12 format
  0xd5000271, 0x80025199, 
// eq_channel_rz_band_2_q : unsigned : 32.0 format
  0xd5000272, 0x03999998, 
// eq_channel_rz_band_2_gain : signed : 6.26 format
  0xd5000273, 0x13ccccc0, 
// eq_channel_rz_band_3_fc : unsigned : 20.12 format
  0xd5000274, 0x80a64000, 
// eq_channel_rz_band_3_q : unsigned : 32.0 format
  0xd5000275, 0x18000000, 
// eq_channel_rz_band_3_gain : signed : 6.26 format
  0xd5000276, 0xf0000000, 
// eq_channel_rz_band_4_fc : unsigned : 20.12 format
  0xd5000277, 0x80690000, 
// eq_channel_rz_band_4_q : unsigned : 32.0 format
  0xd5000278, 0x04000000, 
// eq_channel_rz_band_4_gain : signed : 6.26 format
  0xd5000279, 0xf4000000, 
// eq_channel_rz_band_5_fc : unsigned : 20.12 format
  0xd500027a, 0x814be000, 
// eq_channel_rz_band_5_q : unsigned : 32.0 format
  0xd500027b, 0x04000000, 
// eq_channel_rz_band_5_gain : signed : 6.26 format
  0xd500027c, 0xf4000000, 
// eq_channel_rz_band_6_fc : unsigned : 20.12 format
  0xd500027d, 0x80000000, 
// eq_channel_rz_band_6_q : unsigned : 32.0 format
  0xd500027e, 0x00000000, 
// eq_channel_rz_band_6_gain : signed : 6.26 format
  0xd500027f, 0x00000000, 
// eq_channel_rz_band_7_fc : unsigned : 20.12 format
  0xd5000280, 0x80000000, 
// eq_channel_rz_band_7_q : unsigned : 32.0 format
  0xd5000281, 0x00000000, 
// eq_channel_rz_band_7_gain : signed : 6.26 format
  0xd5000282, 0x00000000, 
// eq_channel_rz_band_8_fc : unsigned : 20.12 format
  0xd5000283, 0x80eb0000, 
// eq_channel_rz_band_8_q : unsigned : 32.0 format
  0xd5000284, 0x10000000, 
// eq_channel_rz_band_8_gain : signed : 6.26 format
  0xd5000285, 0x0c000000, 
// eq_channel_rz_band_9_fc : unsigned : 20.12 format
  0xd5000286, 0x84e20000, 
// eq_channel_rz_band_9_q : unsigned : 32.0 format
  0xd5000287, 0x03999998, 
// eq_channel_rz_band_9_gain : signed : 6.26 format
  0xd5000288, 0x18000000, 
// eq_channel_rz_band_10_fc : unsigned : 20.12 format
  0xd5000289, 0x82710000, 
// eq_channel_rz_band_10_q : unsigned : 32.0 format
  0xd500028a, 0x20000000, 
// eq_channel_rz_band_10_gain : signed : 6.26 format
  0xd500028b, 0xe8000000, 
// eq_channel_rz_band_11_fc : unsigned : 20.12 format
  0xd500028c, 0x80050800, 
// eq_channel_rz_band_11_q : unsigned : 32.0 format
  0xd500028d, 0x04000000, 
// eq_channel_rz_band_11_gain : signed : 6.26 format
  0xd500028e, 0x151eb860, 
// eq_channel_rz_band_12_fc : unsigned : 20.12 format
  0xd500028f, 0x83e28000, 
// eq_channel_rz_band_12_q : unsigned : 32.0 format
  0xd5000290, 0x06000000, 
// eq_channel_rz_band_12_gain : signed : 6.26 format
  0xd5000291, 0x12000000, 
// eq_channel_rz_band_13_fc : unsigned : 20.12 format
  0xd5000292, 0x800a0000, 
// eq_channel_rz_band_13_q : unsigned : 32.0 format
  0xd5000293, 0x0c000000, 
// eq_channel_rz_band_13_gain : signed : 6.26 format
  0xd5000294, 0x0c000000, 
// eq_channel_rz_band_14_fc : unsigned : 20.12 format
  0xd5000295, 0x81f40000, 
// eq_channel_rz_band_14_q : unsigned : 32.0 format
  0xd5000296, 0x088f5c30, 
// eq_channel_rz_band_14_gain : signed : 6.26 format
  0xd5000297, 0x00000000, 
// eq_channel_rz_band_15_fc : unsigned : 20.12 format
  0xd5000298, 0x830d4000, 
// eq_channel_rz_band_15_q : unsigned : 32.0 format
  0xd5000299, 0x088f5c30, 
// eq_channel_rz_band_15_gain : signed : 6.26 format
  0xd500029a, 0x00000000, 
// eq_channel_lk_band_1_fc : unsigned : 20.12 format
  0xd500029b, 0x002bc000, 
// eq_channel_lk_band_1_q : unsigned : 32.0 format
  0xd500029c, 0x00000000, 
// eq_channel_lk_band_1_gain : signed : 6.26 format
  0xd500029d, 0x00000000, 
// eq_channel_lk_band_2_fc : unsigned : 20.12 format
  0xd500029e, 0x0001f800, 
// eq_channel_lk_band_2_q : unsigned : 32.0 format
  0xd500029f, 0x00000000, 
// eq_channel_lk_band_2_gain : signed : 6.26 format
  0xd50002a0, 0x00000000, 
// eq_channel_lk_band_3_fc : unsigned : 20.12 format
  0xd50002a1, 0x00032000, 
// eq_channel_lk_band_3_q : unsigned : 32.0 format
  0xd50002a2, 0x00000000, 
// eq_channel_lk_band_3_gain : signed : 6.26 format
  0xd50002a3, 0x00000000, 
// eq_channel_lk_band_4_fc : unsigned : 20.12 format
  0xd50002a4, 0x00050000, 
// eq_channel_lk_band_4_q : unsigned : 32.0 format
  0xd50002a5, 0x00000000, 
// eq_channel_lk_band_4_gain : signed : 6.26 format
  0xd50002a6, 0x00000000, 
// eq_channel_lk_band_5_fc : unsigned : 20.12 format
  0xd50002a7, 0x0007d000, 
// eq_channel_lk_band_5_q : unsigned : 32.0 format
  0xd50002a8, 0x00000000, 
// eq_channel_lk_band_5_gain : signed : 6.26 format
  0xd50002a9, 0x00000000, 
// eq_channel_lk_band_6_fc : unsigned : 20.12 format
  0xd50002aa, 0x000c8000, 
// eq_channel_lk_band_6_q : unsigned : 32.0 format
  0xd50002ab, 0x00000000, 
// eq_channel_lk_band_6_gain : signed : 6.26 format
  0xd50002ac, 0x00000000, 
// eq_channel_lk_band_7_fc : unsigned : 20.12 format
  0xd50002ad, 0x0013b000, 
// eq_channel_lk_band_7_q : unsigned : 32.0 format
  0xd50002ae, 0x00000000, 
// eq_channel_lk_band_7_gain : signed : 6.26 format
  0xd50002af, 0x00000000, 
// eq_channel_lk_band_8_fc : unsigned : 20.12 format
  0xd50002b0, 0x001f4000, 
// eq_channel_lk_band_8_q : unsigned : 32.0 format
  0xd50002b1, 0x00000000, 
// eq_channel_lk_band_8_gain : signed : 6.26 format
  0xd50002b2, 0x00000000, 
// eq_channel_lk_band_9_fc : unsigned : 20.12 format
  0xd50002b3, 0x00320000, 
// eq_channel_lk_band_9_q : unsigned : 32.0 format
  0xd50002b4, 0x00000000, 
// eq_channel_lk_band_9_gain : signed : 6.26 format
  0xd50002b5, 0x00000000, 
// eq_channel_lk_band_10_fc : unsigned : 20.12 format
  0xd50002b6, 0x004e2000, 
// eq_channel_lk_band_10_q : unsigned : 32.0 format
  0xd50002b7, 0x00000000, 
// eq_channel_lk_band_10_gain : signed : 6.26 format
  0xd50002b8, 0x00000000, 
// eq_channel_lk_band_11_fc : unsigned : 20.12 format
  0xd50002b9, 0x007d0000, 
// eq_channel_lk_band_11_q : unsigned : 32.0 format
  0xd50002ba, 0x00000000, 
// eq_channel_lk_band_11_gain : signed : 6.26 format
  0xd50002bb, 0x00000000, 
// eq_channel_lk_band_12_fc : unsigned : 20.12 format
  0xd50002bc, 0x00c4e000, 
// eq_channel_lk_band_12_q : unsigned : 32.0 format
  0xd50002bd, 0x00000000, 
// eq_channel_lk_band_12_gain : signed : 6.26 format
  0xd50002be, 0x00000000, 
// eq_channel_lk_band_13_fc : unsigned : 20.12 format
  0xd50002bf, 0x01388000, 
// eq_channel_lk_band_13_q : unsigned : 32.0 format
  0xd50002c0, 0x00000000, 
// eq_channel_lk_band_13_gain : signed : 6.26 format
  0xd50002c1, 0x00000000, 
// eq_channel_lk_band_14_fc : unsigned : 20.12 format
  0xd50002c2, 0x01f40000, 
// eq_channel_lk_band_14_q : unsigned : 32.0 format
  0xd50002c3, 0x00000000, 
// eq_channel_lk_band_14_gain : signed : 6.26 format
  0xd50002c4, 0x00000000, 
// eq_channel_lk_band_15_fc : unsigned : 20.12 format
  0xd50002c5, 0x030d4000, 
// eq_channel_lk_band_15_q : unsigned : 32.0 format
  0xd50002c6, 0x00000000, 
// eq_channel_lk_band_15_gain : signed : 6.26 format
  0xd50002c7, 0x00000000, 
// eq_channel_rk_band_1_fc : unsigned : 20.12 format
  0xd50002c8, 0x00014000, 
// eq_channel_rk_band_1_q : unsigned : 32.0 format
  0xd50002c9, 0x00000000, 
// eq_channel_rk_band_1_gain : signed : 6.26 format
  0xd50002ca, 0x00000000, 
// eq_channel_rk_band_2_fc : unsigned : 20.12 format
  0xd50002cb, 0x0001f800, 
// eq_channel_rk_band_2_q : unsigned : 32.0 format
  0xd50002cc, 0x00000000, 
// eq_channel_rk_band_2_gain : signed : 6.26 format
  0xd50002cd, 0x00000000, 
// eq_channel_rk_band_3_fc : unsigned : 20.12 format
  0xd50002ce, 0x00032000, 
// eq_channel_rk_band_3_q : unsigned : 32.0 format
  0xd50002cf, 0x00000000, 
// eq_channel_rk_band_3_gain : signed : 6.26 format
  0xd50002d0, 0x00000000, 
// eq_channel_rk_band_4_fc : unsigned : 20.12 format
  0xd50002d1, 0x00050000, 
// eq_channel_rk_band_4_q : unsigned : 32.0 format
  0xd50002d2, 0x00000000, 
// eq_channel_rk_band_4_gain : signed : 6.26 format
  0xd50002d3, 0x00000000, 
// eq_channel_rk_band_5_fc : unsigned : 20.12 format
  0xd50002d4, 0x0007d000, 
// eq_channel_rk_band_5_q : unsigned : 32.0 format
  0xd50002d5, 0x00000000, 
// eq_channel_rk_band_5_gain : signed : 6.26 format
  0xd50002d6, 0x00000000, 
// eq_channel_rk_band_6_fc : unsigned : 20.12 format
  0xd50002d7, 0x000c8000, 
// eq_channel_rk_band_6_q : unsigned : 32.0 format
  0xd50002d8, 0x00000000, 
// eq_channel_rk_band_6_gain : signed : 6.26 format
  0xd50002d9, 0x00000000, 
// eq_channel_rk_band_7_fc : unsigned : 20.12 format
  0xd50002da, 0x0013b000, 
// eq_channel_rk_band_7_q : unsigned : 32.0 format
  0xd50002db, 0x00000000, 
// eq_channel_rk_band_7_gain : signed : 6.26 format
  0xd50002dc, 0x00000000, 
// eq_channel_rk_band_8_fc : unsigned : 20.12 format
  0xd50002dd, 0x001f4000, 
// eq_channel_rk_band_8_q : unsigned : 32.0 format
  0xd50002de, 0x00000000, 
// eq_channel_rk_band_8_gain : signed : 6.26 format
  0xd50002df, 0x00000000, 
// eq_channel_rk_band_9_fc : unsigned : 20.12 format
  0xd50002e0, 0x00320000, 
// eq_channel_rk_band_9_q : unsigned : 32.0 format
  0xd50002e1, 0x00000000, 
// eq_channel_rk_band_9_gain : signed : 6.26 format
  0xd50002e2, 0x00000000, 
// eq_channel_rk_band_10_fc : unsigned : 20.12 format
  0xd50002e3, 0x004e2000, 
// eq_channel_rk_band_10_q : unsigned : 32.0 format
  0xd50002e4, 0x00000000, 
// eq_channel_rk_band_10_gain : signed : 6.26 format
  0xd50002e5, 0x00000000, 
// eq_channel_rk_band_11_fc : unsigned : 20.12 format
  0xd50002e6, 0x007d0000, 
// eq_channel_rk_band_11_q : unsigned : 32.0 format
  0xd50002e7, 0x00000000, 
// eq_channel_rk_band_11_gain : signed : 6.26 format
  0xd50002e8, 0x00000000, 
// eq_channel_rk_band_12_fc : unsigned : 20.12 format
  0xd50002e9, 0x00c4e000, 
// eq_channel_rk_band_12_q : unsigned : 32.0 format
  0xd50002ea, 0x00000000, 
// eq_channel_rk_band_12_gain : signed : 6.26 format
  0xd50002eb, 0x00000000, 
// eq_channel_rk_band_13_fc : unsigned : 20.12 format
  0xd50002ec, 0x01388000, 
// eq_channel_rk_band_13_q : unsigned : 32.0 format
  0xd50002ed, 0x00000000, 
// eq_channel_rk_band_13_gain : signed : 6.26 format
  0xd50002ee, 0x00000000, 
// eq_channel_rk_band_14_fc : unsigned : 20.12 format
  0xd50002ef, 0x01f40000, 
// eq_channel_rk_band_14_q : unsigned : 32.0 format
  0xd50002f0, 0x00000000, 
// eq_channel_rk_band_14_gain : signed : 6.26 format
  0xd50002f1, 0x00000000, 
// eq_channel_rk_band_15_fc : unsigned : 20.12 format
  0xd50002f2, 0x030d4000, 
// eq_channel_rk_band_15_q : unsigned : 32.0 format
  0xd50002f3, 0x00000000, 
// eq_channel_rk_band_15_gain : signed : 6.26 format
  0xd50002f4, 0x00000000, 
//include ATMOS_Demo_1227_CES\audio_manager_32.cfg
//
// audio_manager_32 (Audio Mgr 32)
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
// lh_trim : unsigned : 1.31 format
  0x8300000a, 0x80000000, 
// rh_trim : unsigned : 1.31 format
  0x8300000b, 0x80000000, 
// lw_trim : unsigned : 1.31 format
  0x8300000c, 0x80000000, 
// rw_trim : unsigned : 1.31 format
  0x8300000d, 0x80000000, 
// lt_trim : unsigned : 1.31 format
  0x8300000e, 0x80000000, 
// rt_trim : unsigned : 1.31 format
  0x8300000f, 0x80000000, 
// lfe1_trim : unsigned : 1.31 format
  0x83000010, 0x80000000, 
// res_trim : unsigned : 1.31 format
  0x83000011, 0x80000000, 
// ch16_trim : unsigned : 1.31 format
  0x83000012, 0x80000000, 
// ch17_trim : unsigned : 1.31 format
  0x83000013, 0x80000000, 
// ch18_trim : unsigned : 1.31 format
  0x83000014, 0x80000000, 
// ch19_trim : unsigned : 1.31 format
  0x83000015, 0x80000000, 
// ch20_trim : unsigned : 1.31 format
  0x83000016, 0x80000000, 
// ch21_trim : unsigned : 1.31 format
  0x83000017, 0x80000000, 
// ch22_trim : unsigned : 1.31 format
  0x83000018, 0x80000000, 
// ch23_trim : unsigned : 1.31 format
  0x83000019, 0x80000000, 
// ch24_trim : unsigned : 1.31 format
  0x8300001a, 0x80000000, 
// ch25_trim : unsigned : 1.31 format
  0x8300001b, 0x80000000, 
// ch26_trim : unsigned : 1.31 format
  0x8300001c, 0x80000000, 
// ch27_trim : unsigned : 1.31 format
  0x8300001d, 0x80000000, 
// ch28_trim : unsigned : 1.31 format
  0x8300001e, 0x80000000, 
// ch29_trim : unsigned : 1.31 format
  0x8300001f, 0x80000000, 
// ch30_trim : unsigned : 1.31 format
  0x83000020, 0x80000000, 
// ch31_trim : unsigned : 1.31 format
  0x83000021, 0x80000000, 
//include ATMOS_Demo_1227_CES\upby2.cfg
//
// upby2 (Up by 2)
//
// enable : unsigned : 32.0 format
  0x85000000, 0x00000000, 
};
#define Bytes_of_ATMOS_Demo_1227_CES_cfg  8552   //(bytes)