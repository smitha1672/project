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
    // cs495314 (CS495314)
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x00000000, 
    // DIALOG_CLARITY_ENABLE : unsigned : 32.0 format
    0xb3000006, 0x00000000, 
    // DIALOG_CLARITY_CONTROL : signed : 1.31 format
    0xb3000007, 0x00000000, 
    // FRONT_DEFINITION_ENABLE : unsigned : 32.0 format
    0xb3000008, 0x00000000, 
    // FRONT_DEFINITION_CONTROL : signed : 1.31 format
    0xb3000009, 0x00000000, 
    // CENTER_DEFINITION_ENABLE : unsigned : 32.0 format
    0xb300000a, 0x00000000, 
    // CENTER_DEFINITION_CONTROL : signed : 1.31 format
    0xb300000b, 0x00000000, 
    // FRONT_TRUBASS_ENABLE : unsigned : 32.0 format
    0xb300000c, 0x00000000, 
    // FRONT_TRUBASS_SPEAKER_SIZE : unsigned : 32.0 format
    0xb300000d, 0x00000000, 
    // FRONT_TRUBASS_CONTROL : signed : 1.31 format
    0xb300000e, 0x00000000, 
    // SUB_TRUBASS_ENABLE : unsigned : 32.0 format
    0xb300000f, 0x00000000, 
    // SUB_TRUBASS_CONTROL : signed : 1.31 format
    0xb3000011, 0x00000000, 
    // eq_proc (PEQ)
    // eq_control : unsigned : 32.0 format
    0xd5000000, 0x0000009f, 
    // gain_multi_channel_plus24 (Subwoofer Gain 1_1)
    // g_1 : signed : 5.27 format
    0xf00000c2, 0x1fd93c46, 
    // gain_multi_channel_plus24 (Center Gain 1)
    // g_1 : signed : 5.27 format
    0xf00000d6, 0x15492a30, 
    // gain_multi_channel_plus24 (L/R Gain (+24dB))
    // g_1 : signed : 5.27 format
    0xf00000bc, 0x30346bac, 
    // g_2 : signed : 5.27 format
    0xf00000bd, 0x30346bac
};

/*retrun to normal mode */
static const uint32 code PCM_51_DIS_BPS_PARMS[]=
{
    // cs495314 (CS495314)
    // mpm_b_tshd4 (TSHD4)
    // TSHD_LEVEL_CONTROL : signed : 1.31 format
    0xb3000002, 0x26666680, 
    // DIALOG_CLARITY_ENABLE : unsigned : 32.0 format
    0xb3000006, 0x00000001, 
    // DIALOG_CLARITY_CONTROL : signed : 1.31 format
    0xb3000007, 0x1b645a20, 
    // FRONT_DEFINITION_ENABLE : unsigned : 32.0 format
    0xb3000008, 0x00000001, 
    // FRONT_DEFINITION_CONTROL : signed : 1.31 format
    0xb3000009, 0x083126f0, 
    // CENTER_DEFINITION_ENABLE : unsigned : 32.0 format
    0xb300000a, 0x00000001, 
    // CENTER_DEFINITION_CONTROL : signed : 1.31 format
    0xb300000b, 0x09168730, 
    // FRONT_TRUBASS_ENABLE : unsigned : 32.0 format
    0xb300000c, 0x00000001, 
    // FRONT_TRUBASS_SPEAKER_SIZE : unsigned : 32.0 format
    0xb300000d, 0x00000002, 
    // FRONT_TRUBASS_CONTROL : signed : 1.31 format
    0xb300000e, 0x2be76c80, 
    // SUB_TRUBASS_ENABLE : unsigned : 32.0 format
    0xb300000f, 0x00000001, 
    // SUB_TRUBASS_CONTROL : signed : 1.31 format
    0xb3000011, 0x33333340, 
    // eq_proc (PEQ)
    // eq_control : unsigned : 32.0 format
    0xd5000000, 0x8000009f, 
    // gain_multi_channel_plus24 (Subwoofer Gain 1_1)
    // g_1 : signed : 5.27 format
    0xf00000c2, 0x0caddc61, 
    // gain_multi_channel_plus24 (Center Gain 1)
    // g_1 : signed : 5.27 format
    0xf00000d6, 0x1c629414, 
    // gain_multi_channel_plus24 (L/R Gain (+24dB))
    // g_1 : signed : 5.27 format
    0xf00000bc, 0x50000000, 
    // g_2 : signed : 5.27 format
    0xf00000bd, 0x50000000
};


#endif /*__S3851_DSP_FACTORY_PARMS_H__*/
