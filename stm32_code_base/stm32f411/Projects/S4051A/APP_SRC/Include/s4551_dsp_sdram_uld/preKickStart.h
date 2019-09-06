#ifndef __PRE_KICK_START_H__
#define __PRE_KICK_START_H__

/***********************************************
This file is created by cirrus dsp tool on 06/26/14 12:22:24
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

#endif /*__PRE_KICK_START_H__*/
