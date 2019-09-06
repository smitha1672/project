#ifndef __AUDIO_GAIN_PARAMETERS_H__
#define __AUDIO_GAIN_PARAMETERS_H__

#if defined ( S4051A )
#include "../inc/s4051a_dsp_gain_param/S4051_GAIN_PARMS.h"
#include "../inc/s4051a_dsp_gain_param/S4051_DSP_FACTORY_PARMS.h"
#include "../inc/s4051a_dsp_gain_param/S4051_DSP_TVHDMC_PARMS.h"

#elif defined ( S4051B )
#include "../inc/s4051b_dsp_gain_param/S4051_GAIN_PARMS.h"
#include "../inc/s4051b_dsp_gain_param/S4051_DSP_FACTORY_PARMS.h"
#include "../inc/s4051b_dsp_gain_param/S4051_DSP_TVHDMC_PARMS.h"

#elif defined ( S3851E )
#include "../inc/s3851e_dsp_gain_param/S3851_GAIN_PARMS.h"
#include "../inc/s3851e_dsp_gain_param/S3851_DSP_FACTORY_PARMS.h"
#include "../inc/s3851e_dsp_gain_param/S3851_DSP_TVHDMC_PARMS.h"

#elif defined ( S3851F )
#include "../inc/s3851f_dsp_gain_param/S3851_GAIN_PARMS.h"
#include "../inc/s3851f_dsp_gain_param/S3851_DSP_FACTORY_PARMS.h"
#include "../inc/s3851f_dsp_gain_param/S3851_DSP_TVHDMC_PARMS.h"

#elif defined ( S4551A ) 
#include "../inc/s4551a_dsp_gain_param/S4551_GAIN_PARMS.h"
#include "../inc/s4551a_dsp_gain_param/S4551_DSP_FACTORY_PARMS.h"
#include "../inc/s4551a_dsp_gain_param/S4551_DSP_TVHDMC_PARMS.h"

#else
#error "AUDIO GAIN PARAMETERS HAS NOT BEEN INCLUDED !! "
#endif /*S4051A*/

#endif /* __AUDIO_GAIN_PARAMETERS_H__ */
