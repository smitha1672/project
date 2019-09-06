//***************************************************************************
//!file     si_osd_config.h
//!brief    System-specific configuration information for OSD
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SI_CONFIG_OSD_H__
#define __SI_CONFIG_OSD_H__

//------------------------------------------------------------------------------
//  OSD Component SiI953x-specific
//------------------------------------------------------------------------------

// The following must match the default font in the SPI flash (first font in file)
#define OSD_DEFAULT_FONT_WIDTH      16      // default font is 720p
#define OSD_DEFAULT_FONT_HEIGHT     24      // default font is 720p
#define OSD_DEFAULT_SCREEN_WIDTH    1280    // default screen is 720p
#define OSD_DEFAULT_SCREEN_HEIGHT   720     // default screen is 720p

#define INC_OSD_SLIDER          1
#define INC_OSD_LISTBOX         1
#define INC_OSD_EDITBOX         1
#define INC_OSD_MESSAGEBOX      1
#define INC_OSD_MENU            1
#define INC_OSD_BASE_HANDLER    1   // Must be set to 1 if any of the above are set to 1

#endif  //__SI_CONFIG_OSD_H__
