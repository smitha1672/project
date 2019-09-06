#ifndef __AUDIO_GAIN_PARAMETERS_H__
#define __AUDIO_GAIN_PARAMETERS_H__

#if defined ( S4051A )
#include "../App_SRC/Include/s4051_dsp_gain_param/S4051_GAIN_PARMS.h"
#include "../App_SRC/Include/s4051_dsp_gain_param/S4051_DSP_FACTORY_PARMS.h"
#include "../App_SRC/Include/s4051_dsp_gain_param/S4051_DSP_TVHDMC_PARMS.h"

#elif defined ( S4551A ) 
#include "../App_SRC/Include/s4551_dsp_gain_param/S4551_GAIN_PARMS.h"
#include "../App_SRC/Include/s4551_dsp_gain_param/S4551_DSP_FACTORY_PARMS.h"
#include "../App_SRC/Include/s4551_dsp_gain_param/S4551_DSP_TVHDMC_PARMS.h"

#else
#error "AUDIO GAIN PARAMETERS HAS NOT BEEN INCLUDED !! "
#endif /*S4051A*/

#endif /* __AUDIO_GAIN_PARAMETERS_H__ */
