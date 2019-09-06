#ifndef __AUDIO_DSP_ULD_H__
#define __AUDIO_DSP_ULD_H__

#include "Defs.h"
#include "device_config.h"


#if defined ( S3851C )

#if( ( configAPP_AUDIO_321 == 1 ) && ( configDSP_SRAM == 1 ) )
#include "../inc/s3851c_dsp_sdram_uld/KickStart.h"
#include "../inc/s3851c_dsp_sdram_uld/preKickStart.h"
#include "../inc/s3851c_dsp_sdram_uld/ac3_initial.h"
#include "../inc/s3851c_dsp_sdram_uld/dts_initial.h"
#include "../inc/s3851c_dsp_sdram_uld/pcm_initial.h"
#include "../inc/s3851c_dsp_sdram_uld/api_doc.h"
#include "../inc/s3851c_dsp_sdram_uld/ULD_Location.h"
#endif

#elif defined ( S3851D ) 

#if( ( configAPP_AUDIO_321 == 1 ) && ( configDSP_SRAM == 1 ) )
#include "../inc/s3851d_dsp_sdram_uld/KickStart.h"
#include "../inc/s3851d_dsp_sdram_uld/preKickStart.h"
#include "../inc/s3851d_dsp_sdram_uld/ac3_initial.h"
#include "../inc/s3851d_dsp_sdram_uld/dts_initial.h"
#include "../inc/s3851d_dsp_sdram_uld/pcm_initial.h"
#include "../inc/s3851d_dsp_sdram_uld/api_doc.h"
#include "../inc/s3851d_dsp_sdram_uld/ULD_Location.h"
#endif

#else
#warning "AUDIO ULD FILE HAS NOT BEEN INCLUDED !! "
#if 1
#include "../inc/s4451i_dsp_uld/KickStart.h"
#include "../inc/s4451i_dsp_uld/preKickStart.h"
#include "../inc/s4451i_dsp_uld/os_hd2_rc_x.h"
#include "../inc/s4451i_dsp_uld/pcm_in_gain.h"
#include "../inc/s4451i_dsp_uld/pcm_initial.h"
#include "../inc/s4451i_dsp_uld/api_doc.h"
#else
#include "../inc/s4451i_dsp_uld/KickStart.h"
#include "../inc/s4451i_dsp_uld/preKickStart.h"
#include "../inc/s4451i_dsp_uld/os_hd2_rc_x.h"
#include "../inc/s4451i_dsp_uld/ac3_rc23.h"
#include "../inc/s4451i_dsp_uld/dts_rc25.h"
#include "../inc/s4451i_dsp_uld/pcm_tvcsii.h"
#include "../inc/s4451i_dsp_uld/ac3_in_gain.h"
#include "../inc/s4451i_dsp_uld/dts_in_gain.h"
#include "../inc/s4451i_dsp_uld/pcm_in_gain.h"
#include "../inc/s4451i_dsp_uld/ac3_initial.h"
#include "../inc/s4451i_dsp_uld/dts_initial.h"
#include "../inc/s4451i_dsp_uld/pcm_initial.h"
#include "../inc/s4451i_dsp_uld/api_doc.h"
#endif

#endif 


#endif /*__AUDIO_DSP_ULD_H__*/
