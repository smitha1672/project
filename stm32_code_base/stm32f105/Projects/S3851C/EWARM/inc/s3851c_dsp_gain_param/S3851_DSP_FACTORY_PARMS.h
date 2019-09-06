#ifndef __S3851_DSP_FACTORY_PARMS_H__
#define __S3851_DSP_FACTORY_PARMS_H__

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
    // mpm_a_cs2_cs4953X (CS II)
    //
    // control : unsigned : 32.0 format
    0xb7000000, 0x00000001, 
    // mode : unsigned : 32.0 format
    0xb7000001, 0x00000002, 
    // centerfb : unsigned : 32.0 format
    0xb7000002, 0x00000001, 
    // mode_525 : unsigned : 32.0 format
    0xb7000003, 0x00000001, 
    // mpm_b_tshd4 (TSHD4)
    // tshd4_enable : unsigned : 32.0 format
    0xb3000000, 0x00000000, 
    // tshd4_bypass : unsigned : 32.0 format
    0xb3000001, 0x00000001, 
    // eq_proc (PEQ)
    // eq_control : unsigned : 32.0 format
    0xd5000000, 0x0000009f     
};

/*retrun to normal mode */
static const uint32 code PCM_51_DIS_BPS_PARMS[]=
{
    // mpm_a_cs2_cs4953X (CS II)
    //
    // control : unsigned : 32.0 format
    0xb7000000, 0x00000001, 
    // mode : unsigned : 32.0 format
    0xb7000001, 0x00000002, 
    // centerfb : unsigned : 32.0 format
    0xb7000002, 0x00000000, 
    // mode_525 : unsigned : 32.0 format
    0xb7000003, 0x00000000, 
    // mpm_b_tshd4 (TSHD4)
    // tshd4_enable : unsigned : 32.0 format
    0xb3000000, 0x00000001, 
    // tshd4_bypass : unsigned : 32.0 format
    0xb3000001, 0x00000000, 
    // eq_proc (PEQ)
    // eq_control : unsigned : 32.0 format
    0xd5000000, 0x8000009f
};


#endif /*__S3851_DSP_FACTORY_PARMS_H__*/
