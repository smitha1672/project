#ifndef __AUDIO_DSP_ULD_H__
#define __AUDIO_DSP_ULD_H__

#include "Defs.h"
#include "device_config.h"


#if defined ( S4051A )

#if( ( configAPP_AUDIO_321 == 1 ) && ( configDSP_SRAM == 1 ) )
#include "../inc/s4051a_dsp_sdram_uld/KickStart.h"
#include "../inc/s4051a_dsp_sdram_uld/preKickStart.h"
#include "../inc/s4051a_dsp_sdram_uld/ac3_initial.h"
#include "../inc/s4051a_dsp_sdram_uld/dts_initial.h"
#include "../inc/s4051a_dsp_sdram_uld/pcm_initial.h"
#include "../inc/s4051a_dsp_sdram_uld/api_doc.h"
#include "../inc/s4051a_dsp_sdram_uld/ULD_Location.h"
#endif

#elif defined ( S4051B ) 

#if( ( configAPP_AUDIO_321 == 1 ) && ( configDSP_SRAM == 1 ) )
#include "../inc/s4051b_dsp_sdram_uld/KickStart.h"
#include "../inc/s4051b_dsp_sdram_uld/preKickStart.h"
#include "../inc/s4051b_dsp_sdram_uld/ac3_initial.h"
#include "../inc/s4051b_dsp_sdram_uld/dts_initial.h"
#include "../inc/s4051b_dsp_sdram_uld/pcm_initial.h"
#include "../inc/s4051b_dsp_sdram_uld/api_doc.h"
#include "../inc/s4051b_dsp_sdram_uld/ULD_Location.h"
#endif

#elif defined ( S3851E ) 

#if( ( configAPP_AUDIO_321 == 1 ) && ( configDSP_SRAM == 1 ) )
#include "../inc/s3851e_dsp_sdram_uld/KickStart.h"
#include "../inc/s3851e_dsp_sdram_uld/preKickStart.h"
#include "../inc/s3851e_dsp_sdram_uld/ac3_initial.h"
#include "../inc/s3851e_dsp_sdram_uld/dts_initial.h"
#include "../inc/s3851e_dsp_sdram_uld/pcm_initial.h"
#include "../inc/s3851e_dsp_sdram_uld/api_doc.h"
#include "../inc/s3851e_dsp_sdram_uld/ULD_Location.h"
#endif

#elif defined ( S3851F ) 

#if( ( configAPP_AUDIO_321 == 1 ) && ( configDSP_SRAM == 1 ) )
#include "../inc/s3851f_dsp_sdram_uld/KickStart.h"
#include "../inc/s3851f_dsp_sdram_uld/preKickStart.h"
#include "../inc/s3851f_dsp_sdram_uld/ac3_initial.h"
#include "../inc/s3851f_dsp_sdram_uld/dts_initial.h"
#include "../inc/s3851f_dsp_sdram_uld/pcm_initial.h"
#include "../inc/s3851f_dsp_sdram_uld/api_doc.h"
#include "../inc/s3851f_dsp_sdram_uld/ULD_Location.h"
#endif

#elif defined ( S4551A ) 

#if( ( configAPP_AUDIO_321 == 1 ) && ( configDSP_SRAM == 1 ) )
#include "../inc/s4551a_dsp_sdram_uld/KickStart.h"
#include "../inc/s4551a_dsp_sdram_uld/preKickStart.h"
#include "../inc/s4551a_dsp_sdram_uld/ac3_initial.h"
#include "../inc/s4551a_dsp_sdram_uld/dts_initial.h"
#include "../inc/s4551a_dsp_sdram_uld/pcm_initial.h"
#include "../inc/s4551a_dsp_sdram_uld/api_doc.h"
#include "../inc/s4551a_dsp_sdram_uld/ULD_Location.h"
#endif

#else
#error "AUDIO ULD FILE HAS NOT BEEN INCLUDED !! "
#endif /*S4051A*/


#endif /*__AUDIO_DSP_ULD_H__*/
