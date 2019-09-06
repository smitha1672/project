#ifndef __PRE_KICK_START_H__
#define __PRE_KICK_START_H__

/***********************************************
This file is created by cirrus dsp tool on 08/12/14 11:08:37
This tool was written by XuShoucai on 06/01/09
************************************************/
static const uint32 code PREKICKSTART_CFG [] = 
{
/* preKickStart.cfg */
//include preKickStart\system_pcm.cfg
// ----------
// PCM (system_pcm) config
// ----------
// input_mode=2
// lfe_enable=0
  0x9b000012, 0x00000002, 
//include preKickStart\cs495314.cfg
// ----------
// CS495314 (cs495314) config
// ----------
  0x82000000, 0x00000002, 
// Enable/Disable SDRAM
  0x8100005c, 0x00000001, 
// Offsets 0x6E
  0x8100006e, 0x00000280, 
  0x82000000, 0x00000000, 
//  Set PLL Speed 
  0x81000054, 0x00000005, 
//include preKickStart\dsp_dai.cfg
// ----------
// DAI (dsp_dai) config
// ----------
//   DAI parameter A
//       A0 - I2S format (default)
  0x81800020, 0xFFFF0000, 
  0x81400020, 0x01001F00, 
  0x81800021, 0xFFFF0000, 
  0x81400021, 0x01001F00, 
  0x81800022, 0xFFFF0000, 
  0x81400022, 0x01001F00, 
  0x81800023, 0xFFFF0000, 
  0x81400023, 0x01001F00, 
  0x81800024, 0xFFFF0000, 
  0x81400024, 0x01001F00, 
//    DAI parameter B
//       B0 - rising edge (default)
  0x81800020, 0xFFDFFFFF, 
  0x81800021, 0xFFDFFFFF, 
  0x81800022, 0xFFDFFFFF, 
  0x81800023, 0xFFDFFFFF, 
  0x81800024, 0xFFDFFFFF, 
//    DAI parameter C
//       C1 - low == right channel (channel 1) (default)
  0x81400020, 0x00100000, 
  0x81400021, 0x00100000, 
  0x81400022, 0x00100000, 
  0x81400023, 0x00100000, 
  0x81400024, 0x00100000, 
//    DAI parameter D
//       D0 (default)
  0x81000025, 0x1008d110, 
//include preKickStart\crd_spdif_rx.cfg
// ----------
// SPDIF (crd_spdif_rx) config
// ----------
// Set port to detect and calculate sample rate
  0x81000027, 0x00001000, 
//include preKickStart\dsp_dao.cfg
// ----------
// DAO (dsp_dao) config
// ----------
// Configure DAO
  0x82000000, 0x00000002, 
//    DAO parameter A
//       A1 - master output clocks
  0x8180002C, 0xFFFFDFFF, 
  0x8180002D, 0xFFFFDFFF, 
//    DAO parameter B
//       B0 - dependent (default)
  0x8140002B, 0x00002000, 
// MCLK/SCLK ratio 256Fs/64Fs
//    DAO parameter C
//       C0
  0x8100003D, 0x00007711, 
  0x8100003E, 0x00007711, 
  0x8180002C, 0xFFFFFF8F, 
  0x8140002C, 0x00000020, 
//    DAO parameter D
//       D0 - I2S format (default)
  0x81000030, 0x00000001, 
  0x81000031, 0x00000001, 
  0x81000032, 0x00000001, 
  0x81000033, 0x00000001, 
  0x81000034, 0x00000001, 
  0x81000035, 0x00000001, 
  0x81000036, 0x00000001, 
  0x81000037, 0x00000001, 
//    DAO parameter E
//       E0 - low == left channel (default)
  0x8140002C, 0x00000700, 
  0x8140002D, 0x00000700, 
//    DAO parameter F
//       F0 - rising edge == valid data (default)
  0x8180002C, 0xFFFFEFFF, 
  0x8180002D, 0xFFFFEFFF, 
  0x82000000, 0x00000000, 
//include preKickStart\mpm_b_tshd4.cfg
// ----------
// TSHD4 (mpm_b_tshd4) config
// ----------
// Enable TruSurroundHD4 
  0xb3000000, 0x00000001, 
//include preKickStart\mpm_b_wow.cfg
// ----------
// WOW HD (mpm_b_wow) config
// ----------
// Enable SRS WOW 
  0xb4000000, 0x00000001, 
//include preKickStart\eq_proc.cfg
// ----------
// PEQ (eq_proc) config
// ----------
// Pre-kickstart code for PEQ
  0xd5000001, 0x00000001, 
// Set Athena EQ Properties
  0xd5000002, 0x0000009f, 
  0xd5000003, 0x0000000b, 
//include preKickStart\delay_proc.cfg
// ----------
// Delay Proc. (delay_proc) config
// ----------
// set max delay values for delay_proc
  0xd900000a, 0x0fa00000, 
  0xd900000b, 0x0fa00000, 
  0xd900000c, 0x0fa00000, 
  0xd900000d, 0x0fa00000, 
  0xd900000e, 0x0fa00000, 
  0xd900000f, 0x00000000, 
  0xd9000010, 0x00000000, 
  0xd9000011, 0x0fa00000, 
// external memory for delay_proc
  0xd9000012, 0x00000000, 
  0xd9000013, 0x00200000, 
// configure external-internal memory switch
  0xd9000000, 0x00000100, 
// configure units property
  0xd9000001, 0x00000000, 
//include preKickStart\custom.cfg
};
#define Bytes_of_preKickStart_cfg  528   //(bytes)











/***********************************************
This file is created by cirrus dsp tool on 12/27/14 18:06:27
This tool was written by XuShoucai on 06/01/09
************************************************/
uint32 code ATMOS_PREKICKSTART_CFG [] = 
{
/* preKickStart.cfg */
//include preKickStart\cs49844.cfg
// ----------
// cs49844 (cs49844) config
// ----------
// OS decimator=disabled
  0x86000000, 0x00000000, 
  0x82000000, 0x00000008, 
// Enable/Disable SDRAM
  0x8100005c, 0x00000001, 
// Dynamic Refresh
  0x81000061, 0x00000075, 
// Offsets 0x62-0x6F
  0x81000062, 0x00000002, 
  0x81000063, 0x00000004, 
  0x81000064, 0x00000009, 
  0x81000065, 0x00000000, 
  0x81000066, 0x00000005, 
  0x81000067, 0x00000001, 
  0x81000068, 0x00000007, 
  0x81000069, 0x00000007, 
  0x8100006a, 0x00000009, 
  0x8100006b, 0x00000001, 
  0x8100006c, 0x00000000, 
  0x8100006e, 0x00000280, 
  0x8100006f, 0x00000303, 
  0x82000000, 0x00000000, 
//  Set PLL Speed 
//include preKickStart\dsp_dai.cfg
// ----------
// Remap, DAI (dsp_dai) config
// ----------
//   DAI parameter A
//       A0 - I2S format (default)
  0x81800020, 0xFFFF0000, 
  0x81400020, 0x01001F00, 
  0x81800021, 0xFFFF0000, 
  0x81400021, 0x01001F00, 
  0x81800022, 0xFFFF0000, 
  0x81400022, 0x01001F00, 
  0x81800023, 0xFFFF0000, 
  0x81400023, 0x01001F00, 
  0x81800024, 0xFFFF0000, 
  0x81400024, 0x01001F00, 
//    DAI parameter B
//       B0 - rising edge (default)
  0x81800020, 0xFFDFFFFF, 
  0x81800021, 0xFFDFFFFF, 
  0x81800022, 0xFFDFFFFF, 
  0x81800023, 0xFFDFFFFF, 
  0x81800024, 0xFFDFFFFF, 
//    DAI parameter C
//       C1 - low == right channel (channel 1) (default)
  0x81400020, 0x00100000, 
  0x81400021, 0x00100000, 
  0x81400022, 0x00100000, 
  0x81400023, 0x00100000, 
  0x81400024, 0x00100000, 
//    DAI parameter D
//       D0 (default)
  0x81000025, 0x1008d110, 
// // io_buff_0_source : unsigned : 32.0 format
  0x9b000001, 0x00000000, 
// // io_buff_1_source : unsigned : 32.0 format
  0x9b000002, 0x00000004, 
// // io_buff_2_source : unsigned : 32.0 format
  0x9b000003, 0x00000001, 
// // io_buff_3_source : unsigned : 32.0 format
  0x9b000004, 0x00000002, 
// // io_buff_4_source : unsigned : 32.0 format
  0x9b000005, 0x00000003, 
// // io_buff_5_source : unsigned : 32.0 format
  0x9b000006, 0x00000006, 
// // io_buff_6_source : unsigned : 32.0 format
  0x9b000007, 0x00000007, 
// // io_buff_7_source : unsigned : 32.0 format
  0x9b000008, 0x00000005, 
// // io_buff_8_source : unsigned : 32.0 format
  0x9b000009, 0x08000000, 
// // io_buff_9_source : unsigned : 32.0 format
  0x9b00000a, 0x08000000, 
// // io_buff_10_source : unsigned : 32.0 format
  0x9b00000b, 0x08000000, 
// // io_buff_11_source : unsigned : 32.0 format
  0x9b00000c, 0x08000000, 
// // io_buff_12_source : unsigned : 32.0 format
  0x9b00000d, 0x08000000, 
// // io_buff_13_source : unsigned : 32.0 format
  0x9b00000e, 0x08000000, 
// // io_buff_14_source : unsigned : 32.0 format
  0x9b00000f, 0x08000000, 
// // io_buff_15_source : unsigned : 32.0 format
  0x9b000010, 0x08000000, 
//include preKickStart\system_pcm.cfg
// ----------
// PCM (system_pcm) config
// ----------
// PCM_Decoder_Input_Channel_Mask
  0x9b000012, 0x00000005, 
//include preKickStart\crd_hdmi_rx_498xx.cfg
// ----------
// HDMI (crd_hdmi_rx_498xx) config
// ----------
// prekick for HDMI
// 4 line HD decoder
  0x81000001, 0x00080104, 
//include preKickStart\dsp_dao32_498xx.cfg
// ----------
// Remap, DAO (dsp_dao32_498xx) config
// ----------
// Configure DAO
  0x82000000, 0x00000008, 
// Enable SPDIF OUT
  0x8100002f, 0x00005080, 
// Set clock dividers to bring MCLK down from 512 to 256 for SPDIF
  0x8180003C, 0xFFFFFF8F, 
  0x8140003C, 0x00000010, 
//    DAO parameter A
//       A1 - master output clocks
  0x8180002C, 0xFFFFDFFF, 
  0x8180002D, 0xFFFFDFFF, 
//    DAO parameter B
//       B0 - dependent (default)
  0x8140002B, 0x00002000, 
// MCLK/SCLK ratio 512Fs/ 64Fs
//    DAO parameter C
//       C5
  0x8100003D, 0x00007713, 
  0x8100003E, 0x00007713, 
  0x8180002C, 0xFFFFFF8F, 
  0x8140002C, 0x00000020, 
//    DAO parameter D
//       D0 - I2S format (default)
  0x81000030, 0x00000001, 
  0x81000031, 0x00000001, 
  0x81000032, 0x00000001, 
  0x81000033, 0x00000001, 
  0x81000034, 0x00000001, 
  0x81000035, 0x00000001, 
  0x81000036, 0x00000001, 
  0x81000037, 0x00000001, 
//    DAO parameter E
//       E0 - low == left channel (default)
  0x8140002C, 0x00000700, 
  0x8140002D, 0x00000700, 
//    DAO parameter F
//       F0 - rising edge == valid data (default)
  0x8180002C, 0xFFFFEFFF, 
  0x8180002D, 0xFFFFEFFF, 
  0x82000000, 0x00000000, 
//include preKickStart\crd_dac32.cfg
// ----------
// Audio Out (crd_dac32) config
// ----------
// // (AUDIO OUT 1) DAO1_D0: AOUT Left port. unsigned : 32.0 format
  0x83000022, 0x00000003, 
// // (AUDIO OUT 2) DAO1_D0: AOUT Right port. unsigned : 32.0 format
  0x83000023, 0x00000004, 
// // (AUDIO OUT 3) DAO1_D1: AOUT Ls port. unsigned : 32.0 format
  0x83000024, 0x00000015, 
// // (AUDIO OUT 4) DAO1_D1: AOUT Rs port. unsigned : 32.0 format
  0x83000025, 0x00000016, 
// // (AUDIO OUT 5) DAO1_D2: AOUT Center port. unsigned : 32.0 format
  0x83000026, 0x00000007, 
// // (AUDIO OUT 6) DAO1_D2: AOUT Sub port. unsigned : 32.0 format
  0x83000027, 0x00000007, 
// // (AUDIO OUT 7) DAO1_D3: XMTA Left (AOUT Sbl port). unsigned : 32.0 format
  0x83000028, 0x00000005, 
// // (AUDIO OUT 8) DAO1_D3: XMTA Right (AOUT Sbr port). unsigned : 32.0 format
  0x83000029, 0x00000006, 
// // (AUDIO OUT 9) DAO1_D4: Left. unsigned : 32.0 format
  0x8300002A, 0x00000000, 
// // (AUDIO OUT 10) DAO1_D4: Right. unsigned : 32.0 format
  0x8300002B, 0x0000000d, 
// // (AUDIO OUT 11) DAO1_D5: Left. unsigned : 32.0 format
  0x8300002C, 0x00000002, 
// // (AUDIO OUT 12) DAO1_D5: Right. unsigned : 32.0 format
  0x8300002D, 0x0000000c, 
// // (AUDIO OUT 13) DAO1_D6: Left. unsigned : 32.0 format
  0x8300002E, 0x00000001, 
// // (AUDIO OUT 14) DAO1_D6: Right. unsigned : 32.0 format
  0x8300002F, 0x00000001, 
// // (AUDIO OUT 15) DAO1_D7: XMTB Left. unsigned : 32.0 format
  0x83000030, 0x0000000e, 
// // (AUDIO OUT 16) DAO1_D7: XMTB Right. unsigned : 32.0 format
  0x83000031, 0x0000000f, 
//include preKickStart\decoder_true_hd_atmos_cs498XX.cfg
// ----------
// TrueHD (decoder_true_hd_atmos_cs498XX) config
// ----------
// Dolby Required Values
  0x9d000007, 0x00000000, 
// h/w workaround
// 8100008D00000000
// X_VY_HD_OS_HOST_FIFO0_Size
  0x8100007f, 0x00002000, 
// X_VY_HD_OS_HOST_FIFO0_nm
  0x81000080, 0x0000c000, 
// X_VY_HD_OS_HOST_FIFO0_Base
  0x81000081, 0x00006000, 
// X_VY_HD_OS_HOST_FIFO1_Size
  0x81000082, 0x00000000, 
// X_VY_HD_OS_HOST_FIFO1_nm
  0x81000083, 0x00000000, 
// X_VY_HD_OS_HOST_FIFO1_Base
  0x81000084, 0x00000000, 
//include preKickStart\decimator_498xx.cfg
// ----------
// Decimator (decimator_498xx) config
// ----------
  0x86000000, 0x00000000, 
//include preKickStart\mpm_oar.cfg
// ----------
// Dolby OAR (mpm_oar) config
// ----------
  0xb9000001, 0x00000200, 
  0xb9000002, 0x0000111f, 
  0xb9000003, 0x00000200, 
  0xb9000004, 0x00000010, 
  0xb9000005, 0x00000002, 
  0xb9000006, 0x00000008, 
// Remap the upper 16 channels
  0x9b000018, 0x00000010, 
  0x9b000019, 0x00000011, 
  0x9b00001a, 0x00000012, 
  0x9b00001b, 0x00000013, 
  0x9b00001c, 0x00000014, 
  0x9b00001d, 0x00000015, 
  0x9b00001e, 0x00000016, 
  0x9b00001f, 0x00000017, 
  0x9b000020, 0x00000018, 
  0x9b000021, 0x00000019, 
  0x9b000022, 0x0000001a, 
  0x9b000023, 0x0000001b, 
  0x9b000024, 0x0000001c, 
  0x9b000025, 0x0000001d, 
  0x9b000026, 0x0000001e, 
  0x9b000027, 0x0000001f, 
// Set the OAR syncword
  0x9b000015, 0xf8720000, 
//include preKickStart\bass_manager_498XX.cfg
// ----------
// Bass mgr (bass_manager_498XX) config
// ----------
// Pre-kickstart code for the bass manager
  0xd7000003, 0x00000006, 
  0xd7000004, 0x00000006, 
  0xd7000005, 0x00000006, 
  0xd7000006, 0x00000006, 
  0xd7000007, 0x00000006, 
  0xd7000008, 0x00000006, 
  0xd7000009, 0x00000006, 
  0xd700000a, 0x00000006, 
  0xd700000b, 0x00000006, 
  0xd700000c, 0x00000006, 
  0xd700000d, 0x00000006, 
  0xd700000e, 0x00000006, 
  0xd700000f, 0x00000006, 
  0xd7000010, 0x00000006, 
  0xd7000011, 0x00000006, 
  0xd7000012, 0x00000006, 
  0xd7000013, 0x00000006, 
  0xd7000014, 0x00000006, 
  0xd7000015, 0x00000006, 
//include preKickStart\eq_proc.cfg
// ----------
// PEQ (eq_proc) config
// ----------
// Pre-kickstart code for PEQ
  0xd5000001, 0x00000001, 
// Set Athena EQ Properties
  0xd5000002, 0x0000ffff, 
  0xd5000003, 0x0000000f, 
//include preKickStart\custom.cfg
};
#define ATMOS_Bytes_of_preKickStart_cfg  1200   //(bytes)

#endif /*__PRE_KICK_START_H__*/
