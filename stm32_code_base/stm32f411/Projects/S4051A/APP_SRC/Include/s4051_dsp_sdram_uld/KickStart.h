#ifndef __KICK_START_H__
#define __KICK_START_H__

/***********************************************
This file is created by cirrus dsp tool on 08/12/14 11:08:37
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



/***********************************************
This file is created by cirrus dsp tool on 12/27/14 18:06:27
This tool was written by XuShoucai on 06/01/09
************************************************/
uint32 code ATMOS_KICKSTART_CFG [] = 
{
/* KickStart.cfg */
// ----------
// cs49844 (cs49844) config
// ----------
// Kickstart dspd first
  0x82000000, 0x00000008, 
  0x81000000, 0x00000001, 
// Kickstart dspc
  0x82000000, 0x00000004, 
  0x81000000, 0x00000001, 
// Kickstart dspb
  0x82000000, 0x00000002, 
  0x81000000, 0x00000001, 
// Kickstart dspa 
  0x82000000, 0x00000000, 
  0x81000000, 0x00000001, 
};
#define ATMOS_Bytes_of_KickStart_cfg  64   //(bytes)

#endif /*__KICK_START_H__*/
