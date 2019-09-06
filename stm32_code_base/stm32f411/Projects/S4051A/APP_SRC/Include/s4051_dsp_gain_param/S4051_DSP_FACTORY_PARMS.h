#ifndef __S4051_DSP_FACTORY_PARMS_H__
#define __S4051_DSP_FACTORY_PARMS_H__

/*DSP_DAO_ROUTER_INITIAL*/
static const uint32 DSP_DAO_ROUTER_INITIAL [] = 
{
    // dao1_chan_0_remap : unsigned : 32.0 format
    0x83000012, 0x00000000, 
    // dao1_chan_1_remap : unsigned : 32.0 format
    0x83000013, 0x00000002, 
    // dao1_chan_2_remap : unsigned : 32.0 format
    0x83000014, 0x00000001, 
    // dao1_chan_3_remap : unsigned : 32.0 format
    0x83000015, 0x00000001, 
    // dao1_chan_4_remap : unsigned : 32.0 format
    0x83000016, 0x00000007, 
    // dao1_chan_5_remap : unsigned : 32.0 format
    0x83000017, 0x00000007, 
    // dao1_chan_6_remap : unsigned : 32.0 format
    0x83000018, 0x00000003, 
    // dao1_chan_7_remap : unsigned : 32.0 format
    0x83000019, 0x00000004
};

/*bypass tshd4, eq for factory test*/
static const uint32 code PCM_51_BPS_PARMS[]=
{
    // DIALOG_CLARITY_ENABLE : unsigned : 32.0 format
    0xb3000006, 0x00000000, 
    // FRONT_DEFINITION_ENABLE : unsigned : 32.0 format
    0xb3000008, 0x00000000, 
    // FRONT_TRUBASS_ENABLE : unsigned : 32.0 format
    0xb300000c, 0x00000000, 
    // SUB_TRUBASS_ENABLE : unsigned : 32.0 format
    0xb300000f, 0x00000000, 
    // eq_proc (PEQ)
    // eq_control : unsigned : 32.0 format
    0xd5000000, 0x0000009f, 
    // gain_multi_channel_plus24 (Center Gain 1)
    // g_1 : signed : 5.27 format
    0xf00000d6, 0x141857af, 
    // gain_multi_channel_plus24 (L/R Gain (+24dB))
    // g_1 : signed : 5.27 format
    0xf00000bc, 0x2fa728e9, 
    // g_2 : signed : 5.27 format
    0xf00000bd, 0x2fa728e9
};

/*retrun to normal mode */
static const uint32 code PCM_51_DIS_BPS_PARMS[]=
{
    // DIALOG_CLARITY_ENABLE : unsigned : 32.0 format
    0xb3000006, 0x00000001, 
    // FRONT_DEFINITION_ENABLE : unsigned : 32.0 format
    0xb3000008, 0x00000001, 
    // FRONT_TRUBASS_ENABLE : unsigned : 32.0 format
    0xb300000c, 0x00000001, 
    // SUB_TRUBASS_ENABLE : unsigned : 32.0 format
    0xb300000f, 0x00000001, 
    // eq_proc (PEQ)
    // eq_control : unsigned : 32.0 format
    0xd5000000, 0x8000009f, 
    // gain_multi_channel_plus24 (Center Gain 1)
    // g_1 : signed : 5.27 format
    0xf00000d6, 0x0caddc61, 
    // gain_multi_channel_plus24 (L/R Gain (+24dB))
    // g_1 : signed : 5.27 format
    0xf00000bc, 0x2a788333, 
    // g_2 : signed : 5.27 format
    0xf00000bd, 0x2a788333, 
};


#endif /*__S3851_DSP_FACTORY_PARMS_H__*/
