#ifndef __AUDIO_GAIN_PARAMETERS_H__
#define __AUDIO_GAIN_PARAMETERS_H__

#if defined ( S3851C )
#include "../inc/s3851c_dsp_gain_param/S3851_GAIN_PARMS.h"
#include "../inc/s3851c_dsp_gain_param/S3851_DSP_FACTORY_PARMS.h"
#include "../inc/s3851c_dsp_gain_param/S3851_DSP_TVHDMC_PARMS.h"

#elif defined ( S3851D ) 
#include "../inc/s3851d_dsp_gain_param/S3851_GAIN_PARMS.h"
#include "../inc/s3851d_dsp_gain_param/S3851_DSP_FACTORY_PARMS.h"
#include "../inc/s3851d_dsp_gain_param/S3851_DSP_TVHDMC_PARMS.h"

#else
#warning "AUDIO GAIN PARAMETERS HAS NOT BEEN INCLUDED !! "
#include "../inc/s4451i_dsp_gain_param/S3851_GAIN_PARMS.h"
#include "../inc/s4451i_dsp_gain_param/S3851_DSP_FACTORY_PARMS.h"
#include "../inc/s4451i_dsp_gain_param/S3851_DSP_TVHDMC_PARMS.h"
#endif /*S3851C*/

#endif /* __AUDIO_GAIN_PARAMETERS_H__ */
