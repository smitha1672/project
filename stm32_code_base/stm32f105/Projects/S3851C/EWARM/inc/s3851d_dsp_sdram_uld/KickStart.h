#ifndef __KICK_START_H__
#define __KICK_START_H__

/***********************************************
This file is created by cirrus dsp tool on 10/21/14 11:58:34
This tool was written by XuShoucai on 06/01/09
************************************************/
static const uint32 code KICKSTART_CFG [] = 
{
/* KickStart.cfg */
// ----------
// CS495314 (cs495314) config
// ----------
// Kickstart dspb first
  0x82000000, 0x00000002, 
  0x81000000, 0x00000001, 
// Kickstart dspa 
  0x82000000, 0x00000000, 
  0x81000000, 0x00000001, 
};
#define Bytes_of_KickStart_cfg  32   //(bytes)

#endif /*__KICK_START_H__*/
