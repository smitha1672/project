/** \file api_doc.h
*  Type definitions that describe the primitive application programming interface (API) *
*  The api definitions are created by DSP composer *
*/


#ifndef __COMPOSER_API_DOC_H__
#define __COMPOSER_API_DOC_H__



/** 'Meters' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _coeff_rms_Meters127 0x0001          /*signed 1.31 format*/
#define _coeff_peak_Meters127 0x0002          /*signed 1.31 format*/
#define _hold_count_Meters127 0x0003          /*signed integer*/
#define _infinite_peak_hold_Meters127 0x0004          /*signed integer*/
#define _ms_0_Meters127 0x0005          /*READ-ONLY: signed 1.31 format*/
#define _pk_0_Meters127 0x0006          /*READ-ONLY: signed 1.31 format*/
#define _ms_hold_0_Meters127 0x0007          /*READ-ONLY: signed 1.31 format*/
#define _pk_hold_0_Meters127 0x0008          /*READ-ONLY: signed 1.31 format*/
#define _ms_1_Meters127 0x0009          /*READ-ONLY: signed 1.31 format*/
#define _pk_1_Meters127 0x000a          /*READ-ONLY: signed 1.31 format*/
#define _ms_hold_1_Meters127 0x000b          /*READ-ONLY: signed 1.31 format*/
#define _pk_hold_1_Meters127 0x000c          /*READ-ONLY: signed 1.31 format*/
#define _ms_2_Meters127 0x000d          /*READ-ONLY: signed 1.31 format*/
#define _pk_2_Meters127 0x000e          /*READ-ONLY: signed 1.31 format*/
#define _ms_hold_2_Meters127 0x000f          /*READ-ONLY: signed 1.31 format*/
#define _pk_hold_2_Meters127 0x0010          /*READ-ONLY: signed 1.31 format*/
#define _ms_3_Meters127 0x0011          /*READ-ONLY: signed 1.31 format*/
#define _pk_3_Meters127 0x0012          /*READ-ONLY: signed 1.31 format*/
#define _ms_hold_3_Meters127 0x0013          /*READ-ONLY: signed 1.31 format*/
#define _pk_hold_3_Meters127 0x0014          /*READ-ONLY: signed 1.31 format*/
#define _ms_4_Meters127 0x0015          /*READ-ONLY: signed 1.31 format*/
#define _pk_4_Meters127 0x0016          /*READ-ONLY: signed 1.31 format*/
#define _ms_hold_4_Meters127 0x0017          /*READ-ONLY: signed 1.31 format*/
#define _pk_hold_4_Meters127 0x0018          /*READ-ONLY: signed 1.31 format*/
#define _ms_5_Meters127 0x0019          /*READ-ONLY: signed 1.31 format*/
#define _pk_5_Meters127 0x001a          /*READ-ONLY: signed 1.31 format*/
#define _ms_hold_5_Meters127 0x001b          /*READ-ONLY: signed 1.31 format*/
#define _pk_hold_5_Meters127 0x001c          /*READ-ONLY: signed 1.31 format*/

/** '24dB Gain'  (Master Vol Hard LimiterCustom PPM)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x001d          /*signed integer*/
#define _clip_0_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x001e          /*READ-ONLY: signed integer*/
#define _clip_1_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x001f          /*READ-ONLY: signed integer*/
#define _clip_2_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x0020          /*READ-ONLY: signed integer*/
#define _clip_3_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x0021          /*READ-ONLY: signed integer*/
#define _clip_4_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x0022          /*READ-ONLY: signed integer*/
#define _clip_5_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x0023          /*READ-ONLY: signed integer*/
#define _clip_6_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x0024          /*READ-ONLY: signed integer*/
#define _clip_7_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x0025          /*READ-ONLY: signed integer*/
#define _clip_8_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x0026          /*READ-ONLY: signed integer*/
#define _clip_9_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x0027          /*READ-ONLY: signed integer*/
#define _clip_10_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x0028          /*READ-ONLY: signed integer*/
#define _clip_11_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x0029          /*READ-ONLY: signed integer*/
#define _clip_12_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x002a          /*READ-ONLY: signed integer*/
#define _clip_13_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x002b          /*READ-ONLY: signed integer*/
#define _clip_14_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x002c          /*READ-ONLY: signed integer*/
#define _clip_15_24dB_Gain_Master_Vol_Hard_LimiterCustom_PPM35 0x002d          /*READ-ONLY: signed integer*/

/** 'Master Volume' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_Master_Volume131 0x002e          /*signed integer*/
#define _ramp_Master_Volume131 0x002f          /*signed 1.31 format*/
#define _g_0_Master_Volume131 0x0030          /*signed 5.27 format*/
#define _g_1_Master_Volume131 0x0031          /*signed 5.27 format*/
#define _g_2_Master_Volume131 0x0032          /*signed 5.27 format*/
#define _g_3_Master_Volume131 0x0033          /*signed 5.27 format*/
#define _g_4_Master_Volume131 0x0034          /*signed 5.27 format*/
#define _g_5_Master_Volume131 0x0035          /*signed 5.27 format*/
#define _clip_0_Master_Volume131 0x0036          /*READ-ONLY: signed integer*/
#define _clip_1_Master_Volume131 0x0037          /*READ-ONLY: signed integer*/
#define _clip_2_Master_Volume131 0x0038          /*READ-ONLY: signed integer*/
#define _clip_3_Master_Volume131 0x0039          /*READ-ONLY: signed integer*/
#define _clip_4_Master_Volume131 0x003a          /*READ-ONLY: signed integer*/
#define _clip_5_Master_Volume131 0x003b          /*READ-ONLY: signed integer*/

/** 'High Pass' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_High_Pass124 0x003c          /*signed integer*/
#define _ramp_High_Pass124 0x003d          /*signed 1.31 format*/
#define _k_High_Pass124 0x003e          /*signed 2.30 format*/
#define _a_0_High_Pass124 0x003f          /*signed 1.31 format*/
#define _b_0_High_Pass124 0x0040          /*signed 1.31 format*/
#define _c_0_High_Pass124 0x0041          /*signed integer*/
#define _a_1_High_Pass124 0x0042          /*signed 1.31 format*/
#define _b_1_High_Pass124 0x0043          /*signed 1.31 format*/
#define _c_1_High_Pass124 0x0044          /*signed integer*/
#define _clip_High_Pass124 0x0045          /*READ-ONLY: signed integer*/

/** 'High Pass' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_High_Pass123 0x0046          /*signed integer*/
#define _ramp_High_Pass123 0x0047          /*signed 1.31 format*/
#define _k_High_Pass123 0x0048          /*signed 2.30 format*/
#define _a_0_High_Pass123 0x0049          /*signed 1.31 format*/
#define _b_0_High_Pass123 0x004a          /*signed 1.31 format*/
#define _c_0_High_Pass123 0x004b          /*signed integer*/
#define _a_1_High_Pass123 0x004c          /*signed 1.31 format*/
#define _b_1_High_Pass123 0x004d          /*signed 1.31 format*/
#define _c_1_High_Pass123 0x004e          /*signed integer*/
#define _clip_High_Pass123 0x004f          /*READ-ONLY: signed integer*/

/** 'High Pass' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_High_Pass121 0x0050          /*signed integer*/
#define _ramp_High_Pass121 0x0051          /*signed 1.31 format*/
#define _k_High_Pass121 0x0052          /*signed 2.30 format*/
#define _a_0_High_Pass121 0x0053          /*signed 1.31 format*/
#define _b_0_High_Pass121 0x0054          /*signed 1.31 format*/
#define _c_0_High_Pass121 0x0055          /*signed integer*/
#define _a_1_High_Pass121 0x0056          /*signed 1.31 format*/
#define _b_1_High_Pass121 0x0057          /*signed 1.31 format*/
#define _c_1_High_Pass121 0x0058          /*signed integer*/
#define _clip_High_Pass121 0x0059          /*READ-ONLY: signed integer*/

/** 'High Pass' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_High_Pass117 0x005a          /*signed integer*/
#define _ramp_High_Pass117 0x005b          /*signed 1.31 format*/
#define _k_High_Pass117 0x005c          /*signed 2.30 format*/
#define _a_0_High_Pass117 0x005d          /*signed 1.31 format*/
#define _b_0_High_Pass117 0x005e          /*signed 1.31 format*/
#define _c_0_High_Pass117 0x005f          /*signed integer*/
#define _a_1_High_Pass117 0x0060          /*signed 1.31 format*/
#define _b_1_High_Pass117 0x0061          /*signed 1.31 format*/
#define _c_1_High_Pass117 0x0062          /*signed integer*/
#define _clip_High_Pass117 0x0063          /*READ-ONLY: signed integer*/

/** 'High Pass' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_High_Pass116 0x0064          /*signed integer*/
#define _ramp_High_Pass116 0x0065          /*signed 1.31 format*/
#define _k_High_Pass116 0x0066          /*signed 2.30 format*/
#define _a_0_High_Pass116 0x0067          /*signed 1.31 format*/
#define _b_0_High_Pass116 0x0068          /*signed 1.31 format*/
#define _c_0_High_Pass116 0x0069          /*signed integer*/
#define _a_1_High_Pass116 0x006a          /*signed 1.31 format*/
#define _b_1_High_Pass116 0x006b          /*signed 1.31 format*/
#define _c_1_High_Pass116 0x006c          /*signed integer*/
#define _clip_High_Pass116 0x006d          /*READ-ONLY: signed integer*/

/** 'Low Pass' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_Low_Pass115 0x006e          /*signed integer*/
#define _ramp_Low_Pass115 0x006f          /*signed 1.31 format*/
#define _k_Low_Pass115 0x0070          /*signed 2.30 format*/
#define _a_0_Low_Pass115 0x0071          /*signed 1.31 format*/
#define _b_0_Low_Pass115 0x0072          /*signed 1.31 format*/
#define _c_0_Low_Pass115 0x0073          /*signed integer*/
#define _a_1_Low_Pass115 0x0074          /*signed 1.31 format*/
#define _b_1_Low_Pass115 0x0075          /*signed 1.31 format*/
#define _c_1_Low_Pass115 0x0076          /*signed integer*/
#define _clip_Low_Pass115 0x0077          /*READ-ONLY: signed integer*/

/** 'Hard Limiter'  (SUB-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _threshold_Hard_Limiter_SUB_Hard_Limiter107 0x0078          /*signed 9.23 format*/
#define _knee_size_Hard_Limiter_SUB_Hard_Limiter107 0x0079          /*signed 9.23 format*/
#define _dsp_compress_ratio_Hard_Limiter_SUB_Hard_Limiter107 0x007a          /*signed 1.31 format*/
#define _dsp_attack_coef_Hard_Limiter_SUB_Hard_Limiter107 0x007b          /*signed 1.31 format*/
#define _dsp_release_coef_Hard_Limiter_SUB_Hard_Limiter107 0x007c          /*signed 1.31 format*/
#define _dsp_bypass_Hard_Limiter_SUB_Hard_Limiter107 0x007d          /*signed integer*/
#define _region_Hard_Limiter_SUB_Hard_Limiter107 0x007e          /*READ-ONLY: signed integer signal indicator, 0 - below; 1 - knee; 2 - above*/
#define _gain_reduction_Hard_Limiter_SUB_Hard_Limiter107 0x007f          /*READ-ONLY: signed 1.31 format*/

/** 'Hard Limiter'  (RS-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _threshold_Hard_Limiter_RS_Hard_Limiter103 0x0080          /*signed 9.23 format*/
#define _knee_size_Hard_Limiter_RS_Hard_Limiter103 0x0081          /*signed 9.23 format*/
#define _dsp_compress_ratio_Hard_Limiter_RS_Hard_Limiter103 0x0082          /*signed 1.31 format*/
#define _dsp_attack_coef_Hard_Limiter_RS_Hard_Limiter103 0x0083          /*signed 1.31 format*/
#define _dsp_release_coef_Hard_Limiter_RS_Hard_Limiter103 0x0084          /*signed 1.31 format*/
#define _dsp_bypass_Hard_Limiter_RS_Hard_Limiter103 0x0085          /*signed integer*/
#define _region_Hard_Limiter_RS_Hard_Limiter103 0x0086          /*READ-ONLY: signed integer signal indicator, 0 - below; 1 - knee; 2 - above*/
#define _gain_reduction_Hard_Limiter_RS_Hard_Limiter103 0x0087          /*READ-ONLY: signed 1.31 format*/

/** 'Hard Limiter'  (LS-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _threshold_Hard_Limiter_LS_Hard_Limiter99 0x0088          /*signed 9.23 format*/
#define _knee_size_Hard_Limiter_LS_Hard_Limiter99 0x0089          /*signed 9.23 format*/
#define _dsp_compress_ratio_Hard_Limiter_LS_Hard_Limiter99 0x008a          /*signed 1.31 format*/
#define _dsp_attack_coef_Hard_Limiter_LS_Hard_Limiter99 0x008b          /*signed 1.31 format*/
#define _dsp_release_coef_Hard_Limiter_LS_Hard_Limiter99 0x008c          /*signed 1.31 format*/
#define _dsp_bypass_Hard_Limiter_LS_Hard_Limiter99 0x008d          /*signed integer*/
#define _region_Hard_Limiter_LS_Hard_Limiter99 0x008e          /*READ-ONLY: signed integer signal indicator, 0 - below; 1 - knee; 2 - above*/
#define _gain_reduction_Hard_Limiter_LS_Hard_Limiter99 0x008f          /*READ-ONLY: signed 1.31 format*/

/** 'Hard Limiter'  (C-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _threshold_Hard_Limiter_C_Hard_Limiter95 0x0090          /*signed 9.23 format*/
#define _knee_size_Hard_Limiter_C_Hard_Limiter95 0x0091          /*signed 9.23 format*/
#define _dsp_compress_ratio_Hard_Limiter_C_Hard_Limiter95 0x0092          /*signed 1.31 format*/
#define _dsp_attack_coef_Hard_Limiter_C_Hard_Limiter95 0x0093          /*signed 1.31 format*/
#define _dsp_release_coef_Hard_Limiter_C_Hard_Limiter95 0x0094          /*signed 1.31 format*/
#define _dsp_bypass_Hard_Limiter_C_Hard_Limiter95 0x0095          /*signed integer*/
#define _region_Hard_Limiter_C_Hard_Limiter95 0x0096          /*READ-ONLY: signed integer signal indicator, 0 - below; 1 - knee; 2 - above*/
#define _gain_reduction_Hard_Limiter_C_Hard_Limiter95 0x0097          /*READ-ONLY: signed 1.31 format*/

/** 'Hard Limiter'  (R-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _threshold_Hard_Limiter_R_Hard_Limiter91 0x0098          /*signed 9.23 format*/
#define _knee_size_Hard_Limiter_R_Hard_Limiter91 0x0099          /*signed 9.23 format*/
#define _dsp_compress_ratio_Hard_Limiter_R_Hard_Limiter91 0x009a          /*signed 1.31 format*/
#define _dsp_attack_coef_Hard_Limiter_R_Hard_Limiter91 0x009b          /*signed 1.31 format*/
#define _dsp_release_coef_Hard_Limiter_R_Hard_Limiter91 0x009c          /*signed 1.31 format*/
#define _dsp_bypass_Hard_Limiter_R_Hard_Limiter91 0x009d          /*signed integer*/
#define _region_Hard_Limiter_R_Hard_Limiter91 0x009e          /*READ-ONLY: signed integer signal indicator, 0 - below; 1 - knee; 2 - above*/
#define _gain_reduction_Hard_Limiter_R_Hard_Limiter91 0x009f          /*READ-ONLY: signed 1.31 format*/

/** 'Hard Limiter'  (L-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _threshold_Hard_Limiter_L_Hard_Limiter87 0x00a0          /*signed 9.23 format*/
#define _knee_size_Hard_Limiter_L_Hard_Limiter87 0x00a1          /*signed 9.23 format*/
#define _dsp_compress_ratio_Hard_Limiter_L_Hard_Limiter87 0x00a2          /*signed 1.31 format*/
#define _dsp_attack_coef_Hard_Limiter_L_Hard_Limiter87 0x00a3          /*signed 1.31 format*/
#define _dsp_release_coef_Hard_Limiter_L_Hard_Limiter87 0x00a4          /*signed 1.31 format*/
#define _dsp_bypass_Hard_Limiter_L_Hard_Limiter87 0x00a5          /*signed integer*/
#define _region_Hard_Limiter_L_Hard_Limiter87 0x00a6          /*READ-ONLY: signed integer signal indicator, 0 - below; 1 - knee; 2 - above*/
#define _gain_reduction_Hard_Limiter_L_Hard_Limiter87 0x00a7          /*READ-ONLY: signed 1.31 format*/

/** 'Ls-Rs Gain2' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_Ls_Rs_Gain2151 0x00a8          /*signed integer*/
#define _ramp_Ls_Rs_Gain2151 0x00a9          /*signed 1.31 format*/
#define _g_0_Ls_Rs_Gain2151 0x00aa          /*signed 5.27 format*/
#define _g_1_Ls_Rs_Gain2151 0x00ab          /*signed 5.27 format*/
#define _clip_0_Ls_Rs_Gain2151 0x00ac          /*READ-ONLY: signed integer*/
#define _clip_1_Ls_Rs_Gain2151 0x00ad          /*READ-ONLY: signed integer*/

/** 'Ls-Rs Gain 1' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_Ls_Rs_Gain_1119 0x00ae          /*signed integer*/
#define _ramp_Ls_Rs_Gain_1119 0x00af          /*signed 1.31 format*/
#define _g_0_Ls_Rs_Gain_1119  0x00b0          /*signed 5.27 format*/
#define _g_1_Ls_Rs_Gain_1119 0x00b1          /*signed 5.27 format*/
#define _clip_0_Ls_Rs_Gain_1119 0x00b2          /*READ-ONLY: signed integer*/
#define _clip_1_Ls_Rs_Gain_1119 0x00b3          /*READ-ONLY: signed integer*/

/** 'L/R Gain (+24dB)' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_L_R_Gain___24dB_114 0x00b4          /*signed integer*/
#define _ramp_L_R_Gain___24dB_114 0x00b5          /*signed 1.31 format*/
#define _g_0_L_R_Gain___24dB_114 0x00b6          /*signed 5.27 format*/
#define _g_1_L_R_Gain___24dB_114 0x00b7          /*signed 5.27 format*/
#define _clip_0_L_R_Gain___24dB_114 0x00b8          /*READ-ONLY: signed integer*/
#define _clip_1_L_R_Gain___24dB_114 0x00b9          /*READ-ONLY: signed integer*/

/** 'Subwoofer Gain 2' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_Subwoofer_Gain_2132 0x00ba          /*signed integer*/
#define _ramp_Subwoofer_Gain_2132 0x00bb          /*signed 1.31 format*/
#define _g_0_Subwoofer_Gain_2132 0x00bc          /*signed 5.27 format*/
#define _clip_0_Subwoofer_Gain_2132 0x00bd          /*READ-ONLY: signed integer*/

/** 'Subwoofer Gain 1' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_Subwoofer_Gain_1120 0x00be          /*signed integer*/
#define _ramp_Subwoofer_Gain_1120 0x00bf          /*signed 1.31 format*/
#define _g_0_Subwoofer_Gain_1120 0x00c0          /*signed 5.27 format*/
#define _clip_0_Subwoofer_Gain_1120 0x00c1          /*READ-ONLY: signed integer*/

/** 'Center Gain 2' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_Center_Gain_2152 0x00c2          /*signed integer*/
#define _ramp_Center_Gain_2152 0x00c3          /*signed 1.31 format*/
#define _g_0_Center_Gain_2152 0x00c4          /*signed 5.27 format*/
#define _clip_0_Center_Gain_2152 0x00c5          /*READ-ONLY: signed integer*/

/** 'Presence' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _threshold_Presence122 0x00c6          /*signed 1.31 format*/
#define _infinite_hold_Presence122 0x00c7          /*signed integer*/
#define _hold_count_Presence122 0x00c8          /*signed integer*/
#define _presence_Presence122 0x00c9          /*READ-ONLY: signed integer*/

/** 'Center Gain 1' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _hold_count_Center_Gain_1118 0x00ca          /*signed integer*/
#define _ramp_Center_Gain_1118 0x00cb          /*signed 1.31 format*/
#define _g_0_Center_Gain_1118 0x00cc          /*signed 5.27 format*/
#define _clip_0_Center_Gain_1118 0x00cd          /*READ-ONLY: signed integer*/

/** 'Presence' 
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _threshold_Presence73 0x00ce          /*signed 1.31 format*/
#define _infinite_hold_Presence73 0x00cf          /*signed integer*/
#define _hold_count_Presence73 0x00d0          /*signed integer*/
#define _presence_Presence73 0x00d1          /*READ-ONLY: signed integer*/

/** 'Peak Square Detector'  (SUB-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _disable_0_Peak_Square_Detector_SUB_Hard_Limiter106 0x00d2          /*signed integer*/
#define _tc_Peak_Square_Detector_SUB_Hard_Limiter106 0x00d3          /*signed 1.31 format*/
#define _max_peak_Peak_Square_Detector_SUB_Hard_Limiter106 0x00d4          /*READ-ONLY: signed 1.31 format*/

/** 'Peak Square Detector'  (RS-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _disable_0_Peak_Square_Detector_RS_Hard_Limiter102 0x00d5          /*signed integer*/
#define _tc_Peak_Square_Detector_RS_Hard_Limiter102 0x00d6          /*signed 1.31 format*/
#define _max_peak_Peak_Square_Detector_RS_Hard_Limiter102 0x00d7          /*READ-ONLY: signed 1.31 format*/

/** 'Peak Square Detector'  (LS-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _disable_0_Peak_Square_Detector_LS_Hard_Limiter98 0x00d8          /*signed integer*/
#define _tc_Peak_Square_Detector_LS_Hard_Limiter98 0x00d9          /*signed 1.31 format*/
#define _max_peak_Peak_Square_Detector_LS_Hard_Limiter98 0x00da          /*READ-ONLY: signed 1.31 format*/

/** 'Peak Square Detector'  (C-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _disable_0_Peak_Square_Detector_C_Hard_Limiter94 0x00db          /*signed integer*/
#define _tc_Peak_Square_Detector_C_Hard_Limiter94 0x00dc          /*signed 1.31 format*/
#define _max_peak_Peak_Square_Detector_C_Hard_Limiter94 0x00dd          /*READ-ONLY: signed 1.31 format*/

/** 'Peak Square Detector'  (R-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _disable_0_Peak_Square_Detector_R_Hard_Limiter90 0x00de          /*signed integer*/
#define _tc_Peak_Square_Detector_R_Hard_Limiter90 0x00df          /*signed 1.31 format*/
#define _max_peak_Peak_Square_Detector_R_Hard_Limiter90 0x00e0          /*READ-ONLY: signed 1.31 format*/

/** 'Peak Square Detector'  (L-Hard Limiter)
*
* Write Message = 0xf000HHHH 0xhhhhhhhh
* Read Request Message = 0xf0C0HHHH
* Read ResponseMessage = 0x70C0HHHH 0xhhhhhhhh
* 0xHHHH = index. 0xhhhhhhhh = Data Value
*/
#define _disable_0_Peak_Square_Detector_L_Hard_Limiter86 0x00e1          /*signed integer*/
#define _tc_Peak_Square_Detector_L_Hard_Limiter86 0x00e2          /*signed 1.31 format*/
#define _max_peak_Peak_Square_Detector_L_Hard_Limiter86 0x00e3          /*READ-ONLY: signed 1.31 format*/
#endif /*__COMPOSER_API_DOC_H__*/
