#include "Defs.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "api_typedef.h"

#include "Debug.h"
#include "IRLowLevel.h"
#include "IRCmdHandler.h"

//____________________________________________________________________________________________________________
#define IR_TIME_BASE_GAP 5
#define IR_GAP(X) ( X )

#define IR_NEC_1T_P_MIN ( ( (560-280)/IR_TIME_BASE ) |0x8000 )
#define IR_NEC_1T_P_MAX ( ( (560+280)/IR_TIME_BASE ) |0x8000 )
#define IR_NEC_LEAD_P_MIN ( ( (9000-280)/IR_TIME_BASE ) |0x8000 )
#define IR_NEC_LEAD_P_MAX ( ( (9000+280)/IR_TIME_BASE ) |0x8000 )
#define IR_NEC_LEAD_S ( ( (4500)/IR_TIME_BASE ) |0x0000 )
#define IR_NEC_LEAD_S_MIN ( ( (4500-280)/IR_TIME_BASE ) |0x0000 )
#define IR_NEC_LEAD_S_MAX ( ( (4500+280)/IR_TIME_BASE ) |0x0000 )
#define IR_NEC_REPEAT_S_MIN ( ( (2250-280)/IR_TIME_BASE ) |0x0000 )
#define IR_NEC_REPEAT_S_MAX ( ( (2250+280)/IR_TIME_BASE ) |0x0000 )
#define IR_NEC_LG1_S_MIN ( ( (1690-280)/IR_TIME_BASE ) |0x0000 )
#define IR_NEC_LG1_S_MAX ( ( (1690+280)/IR_TIME_BASE ) |0x0000 )
#define IR_NEC_LG0_S_MIN ( ( (560-280)/IR_TIME_BASE ) |0x0000 )
#define IR_NEC_LG0_S_MAX ( ( (560+280)/IR_TIME_BASE ) |0x0000 )

#define IR_JVC_1T_P_MIN ( ( (527-263)/IR_TIME_BASE ) |0x8000 )
#define IR_JVC_1T_P_MAX ( ( (527+263)/IR_TIME_BASE ) |0x8000 )
#define IR_JVC_LEAD_P_MIN ( ( (8440-263)/IR_TIME_BASE ) |0x8000 )
#define IR_JVC_LEAD_P_MAX ( ( (8440+263)/IR_TIME_BASE ) |0x8000 )
#define IR_JVC_LEAD_S_MIN ( ( (4220-263)/IR_TIME_BASE ) |0x0000 )
#define IR_JVC_LEAD_S_MAX ( ( (4220+263)/IR_TIME_BASE ) |0x0000 )
#define IR_JVC_REPEAT_S_MIN ( ( (12130-280)/IR_TIME_BASE ) |0x0000 )
#define IR_JVC_REPEAT_S_MAX ( ( (12130+280)/IR_TIME_BASE ) |0x0000 )
#define IR_JVC_LG1_S_MIN ( ( (1538-280)/IR_TIME_BASE ) |0x0000 )
#define IR_JVC_LG1_S_MAX ( ( (1538+280)/IR_TIME_BASE ) |0x0000 )
#define IR_JVC_LG0_S_MIN ( ( (527-280)/IR_TIME_BASE ) |0x0000 )
#define IR_JVC_LG0_S_MAX ( ( (527+280)/IR_TIME_BASE ) |0x0000 )

#define IR_SONY_5T_S ( ( (3000-0)/IR_TIME_BASE ) |0x0000 )
#define IR_SONY_1T_S_MIN ( ( (600-300)/IR_TIME_BASE ) |0x0000 )
#define IR_SONY_1T_S_MAX ( ( (600+300)/IR_TIME_BASE ) |0x0000 )
#define IR_SONY_LEAD_P_MIN ( ( (2400-300)/IR_TIME_BASE ) |0x8000 )
#define IR_SONY_LEAD_P_MAX ( ( (2400+300)/IR_TIME_BASE ) |0x8000 )
#define IR_SONY_LG1_P_MIN ( ( (1200-300)/IR_TIME_BASE ) |0x8000 )
#define IR_SONY_LG1_P_MAX ( ( (1200+300)/IR_TIME_BASE ) |0x8000 )
#define IR_SONY_LG0_P_MIN ( ( (600-300)/IR_TIME_BASE ) |0x8000 )
#define IR_SONY_LG0_P_MAX ( ( (600+300)/IR_TIME_BASE ) |0x8000 )

#define IR_SHARP_1T_P_MIN ( ( (320-160)/IR_TIME_BASE ) |0x8000 )
#define IR_SHARP_1T_P_MAX ( ( (320+160)/IR_TIME_BASE ) |0x8000 )
#define IR_SHARP_LG0_S_MIN ( ( (680-320)/IR_TIME_BASE ) |0x0000 )
#define IR_SHARP_LG0_S_MAX ( ( (680+320)/IR_TIME_BASE ) |0x0000 )
#define IR_SHARP_LG1_S_MIN ( ( (1680-320)/IR_TIME_BASE ) |0x0000 )
#define IR_SHARP_LG1_S_MAX ( ( (1680+320)/IR_TIME_BASE ) |0x0000 )
#define IR_SHARP_INV_S_MIN ( ( (40000-10000)/IR_TIME_BASE ) |0x0000 )
#define IR_SHARP_INV_S_MAX ( ( (40000+10000)/IR_TIME_BASE ) |0x0000 )


#define IR_RC5_SHORT_PULSE  ( (889/IR_TIME_BASE) | 0x8000 )
#define IR_RC5_SHORT_PULSE_MIN ( (444/IR_TIME_BASE ) | 0x8000 )
#define IR_RC5_SHORT_PULSE_MAX ( (1333/IR_TIME_BASE ) | 0x8000 )

#define IR_RC5_SHORT_SPACE  ( (889/IR_TIME_BASE) )
#define IR_RC5_SHORT_SPACE_MIN ( (444/IR_TIME_BASE )  )
#define IR_RC5_SHORT_SPACE_MAX ( (1333/IR_TIME_BASE )  )

#define IR_RC5_LONG_PULSE ( ((889*2)/IR_TIME_BASE)|0x8000 )
#define IR_RC5_LONG_PULSE_MIN ( (1334/IR_TIME_BASE )|0x8000 )
#define IR_RC5_LONG_PULSE_MAX ( (2222/IR_TIME_BASE )|0x8000 )

#define IR_RC5_LONG_SPACE ( ((889*2)/IR_TIME_BASE) )
#define IR_RC5_LONG_SPACE_MIN ( (1334/IR_TIME_BASE ) )
#define IR_RC5_LONG_SPACE_MAX ( (2222/IR_TIME_BASE ) )

#define IR_RC6_SHORT_PULSE ( (444/IR_TIME_BASE) |0x8000 )
#define IR_RC6_SHORT_PULSE_MIN ( (222/IR_TIME_BASE) |0x8000 )
#define IR_RC6_SHORT_PULSE_MAX ( (666/IR_TIME_BASE) |0x8000 )

#define IR_RC6_LONG_PULSE ( (444*2/IR_TIME_BASE) |0x8000 )
#define IR_RC6_LONG_PULSE_MIN ( (666/IR_TIME_BASE) |0x8000 )
#define IR_RC6_LONG_PULSE_MAX ( (1100/IR_TIME_BASE) |0x8000 )

#define IR_RC6_LONG_SPACE ( (444*2/IR_TIME_BASE) )
#define IR_RC6_LONG_SPACE_MIN ( (666/IR_TIME_BASE) )
#define IR_RC6_LONG_SPACE_MAX ( (1100/IR_TIME_BASE) )

#define IR_RC6_LEADER_P ( (2666/IR_TIME_BASE) |0x8000 )
#define IR_RC6_LEADER_P_MIN ( (2444/IR_TIME_BASE) |0x8000 )
#define IR_RC6_LEADER_P_MAX ( (2888/IR_TIME_BASE) |0x8000 )

#define IR_RC6_LEADER_S ( (889/IR_TIME_BASE) )
#define IR_RC6_LEADER_S_MIN ( (667/IR_TIME_BASE) )
#define IR_RC6_LEADER_S_MAX ( (1111/IR_TIME_BASE) )


#define IR_RC6_1T (444)
#define IR_RC6_2T (444*2)
#define IR_RC6_3T (444*3)

#define IR_RC6_1T_P ( ( (IR_RC6_1T)/IR_TIME_BASE ) |0x8000 )
#define IR_RC6_1T_P_MIN ( ( (IR_RC6_1T-222)/IR_TIME_BASE ) |0x8000 )
#define IR_RC6_1T_P_MAX ( ( (IR_RC6_1T+222)/IR_TIME_BASE ) |0x8000 )
#define IR_RC6_1T_S ( ( (IR_RC6_1T)/IR_TIME_BASE ) |0x0000 )
#define IR_RC6_1T_S_MIN ( ( (IR_RC6_1T-222)/IR_TIME_BASE ) |0x0000 )
#define IR_RC6_1T_S_MAX ( ( (IR_RC6_1T+222)/IR_TIME_BASE ) |0x0000 )
#define IR_RC6_2T_P_MIN ( ( (IR_RC6_2T-222)/IR_TIME_BASE ) |0x8000 )
#define IR_RC6_2T_P_MAX ( ( (IR_RC6_2T+222)/IR_TIME_BASE ) |0x8000 )
#define IR_RC6_2T_S_MIN ( ( (IR_RC6_2T-222)/IR_TIME_BASE ) |0x0000 )
#define IR_RC6_2T_S_MAX ( ( (IR_RC6_2T+222)/IR_TIME_BASE ) |0x0000 )
#define IR_RC6_3T_P_MIN ( ( (IR_RC6_3T-222)/IR_TIME_BASE ) |0x8000 )
#define IR_RC6_3T_P_MAX ( ( (IR_RC6_3T+222)/IR_TIME_BASE ) |0x8000 )
#define IR_RC6_3T_S_MIN ( ( (IR_RC6_3T-222)/IR_TIME_BASE ) |0x0000 )
#define IR_RC6_3T_S_MAX ( ( (IR_RC6_3T+222)/IR_TIME_BASE ) |0x0000 )

//____________________________________________________________________________________________________________
#define IR_CMD_REPEAT_TIME 220

//____________________________________________________________________________________________________________
extern IR_LOWLEVEL_OBJECT *pIR_LowLevel;
extern IR_CMD_HANDLER_OBJECT *pIRCmdHandle_ObjCtrl;
//____________________________________________________________________________________________________________
static portTickType xIrDecoderTimeTick = 0;

uint16 ir_rowdata[IR_ROW_DATA_MAX];
uint16 ir_queue_number = 0;
IR_PROTOCOL_PAYLOAD mIrPayload;


//____________________________________________________________________________________________________________
static uint32 IRLowLevelDecoder_BitsReverse(uint32 value )
{
	uint32 x = value;

    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return((x >> 16) | (x << 16));

}

static IR_PROTOCOL_PAYLOAD IRLowLevelDecoder_RC5_Decode( const uint16 *pIrRowData, uint16 length )
{
	/*Smith refer to state machine: http://www.clearwater.com.au/code/rc5*/
	uint8 state_mach = 0;
	uint8 i = 0;
	uint8 data_cnt = 0;
	IR_PROTOCOL_PAYLOAD ir_payload;

	ir_payload.payload	 = 0;
	ir_payload.info.byte = 0;
	if ( pIrRowData == NULL )
		return ir_payload;

	state_mach = 0;
	for( i = 0; i < length; i++ )
	{
		switch( state_mach )
		{
			case 0:
			{
				if ( ( *( pIrRowData+i ) > IR_RC5_SHORT_PULSE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_SHORT_PULSE_MAX ) )
				{
					state_mach = 2;
				}
				else if ( ( *( pIrRowData+i ) > IR_RC5_LONG_PULSE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_LONG_PULSE_MAX ) )
				{
					// zero
					ir_payload.payload <<= 1; 
					data_cnt ++;
					state_mach = 1;
				}
				
			}
				break;

			case 2:
			{
				if ( ( *( pIrRowData+i ) > IR_RC5_LONG_SPACE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_LONG_SPACE_MAX ) )
				{
					// one	
					ir_payload.payload <<= 1;
					ir_payload.payload |=1;
					data_cnt ++; 
					state_mach = 0;
				}
				else if ( ( *( pIrRowData+i ) > IR_RC5_SHORT_SPACE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_SHORT_SPACE_MAX ) )
				{
					//one
					ir_payload.payload <<= 1;
					ir_payload.payload |=1;
					data_cnt ++;
					state_mach = 0;
				}
			}
				break;

			case 1:
			{
				if ( ( *( pIrRowData+i ) > IR_RC5_SHORT_SPACE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_SHORT_SPACE_MAX ) )
				{
					state_mach = 3;
				}
				else if ( ( *( pIrRowData+i ) > IR_RC5_LONG_SPACE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_LONG_SPACE_MAX ) )
				{
					ir_payload.payload <<= 1;
					ir_payload.payload |=1;
					data_cnt ++;
					
					state_mach = 0;
				}
			}
				break;

			case 3:
			{
				if ( ( *( pIrRowData+i ) > IR_RC5_SHORT_PULSE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_SHORT_PULSE_MAX ) )
				{
					ir_payload.payload <<= 1;
					data_cnt ++;
					state_mach = 1;
				}
				else if ( ( *( pIrRowData+i ) > IR_RC5_LONG_PULSE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_LONG_PULSE_MAX ) )
				{
					ir_payload.payload <<= 1;
					data_cnt ++;
					state_mach = 1;
				}

			}
				break;
		}

		if ( data_cnt >=13)
		{
			ir_payload.info.bits.protocol_type = IR_PROTOCOL_36K_RC5;
			break;
		}
			
	}

#if 0
	TRACE_DEBUG((0, "rc5 h = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));
	TRACE_DEBUG((0, "ir_cnt = %d", data_cnt ));
#endif 	
	
	return ir_payload;
}

static IR_PROTOCOL_PAYLOAD IRLowLevelDecoder_Sharp_Decode( const uint16 *pIrRowData, uint16 length )
{

	int8 state_mach = 0;
	uint8 i = 0;
	uint8 data_cnt = 0;
	IR_PROTOCOL_PAYLOAD ir_payload;

	uint16 frame1 = 0;
	uint16 frame2 = 0;

	ir_payload.payload	 = 0;
	ir_payload.info.byte = 0;
	
	if ( pIrRowData == NULL )
		return ir_payload;

	state_mach = 0;
	for( i = 0; i < length; i++ )
	{
		switch( state_mach )
		{
			case 0: // decode data
			{
				if ((*( pIrRowData+i) > IR_SHARP_LG1_S_MIN ) && (*( pIrRowData+i) < IR_SHARP_LG1_S_MAX ))
				{
					if ((*( pIrRowData+i-1) > IR_SHARP_1T_P_MIN ) && (*( pIrRowData+i-1) < IR_SHARP_1T_P_MAX ))
					{
						frame1 <<= 1;
						frame1 |= 1;
						data_cnt++;
					}
				}
				else if ((*( pIrRowData+i) > IR_SHARP_LG0_S_MIN ) && (*( pIrRowData+i) < IR_SHARP_LG0_S_MAX ))
				{
					if ((*( pIrRowData+i-1) > IR_SHARP_1T_P_MIN ) && (*( pIrRowData+i-1) < IR_SHARP_1T_P_MAX ))
					{
						frame1 <<= 1;
						data_cnt++;
					}
				}

				if ( data_cnt >= 15 ) /*address 5 bits + command 8 bits + 2 ext code*/
				{
					state_mach = 1;
				}
				
			}
				break;

			case 1:
			{
				if ((*( pIrRowData+i) > IR_SHARP_INV_S_MIN ) && (*( pIrRowData+i) < IR_SHARP_INV_S_MAX ))
				{
					state_mach = 2;
				}
			}
				break;

			case 2:
			{
				if ((*( pIrRowData+i) > IR_SHARP_LG1_S_MIN ) && (*( pIrRowData+i) < IR_SHARP_LG1_S_MAX ))
				{
					if ((*( pIrRowData+i-1) > IR_SHARP_1T_P_MIN ) && (*( pIrRowData+i-1) < IR_SHARP_1T_P_MAX ))
					{
						frame2 <<= 1;
						frame2 |= 1;
						data_cnt++;
					}
				}
				else if ((*( pIrRowData+i) > IR_SHARP_LG0_S_MIN ) && (*( pIrRowData+i) < IR_SHARP_LG0_S_MAX ))
				{
					if ((*( pIrRowData+i-1) > IR_SHARP_1T_P_MIN ) && (*( pIrRowData+i-1) < IR_SHARP_1T_P_MAX ))
					{
						frame2 <<= 1;
						data_cnt++;
					}
				}

				if ( data_cnt >= 30 ) // after 40 msec command invert data
				{
					if( ( frame1 & 0x03ff ) == ( (~(frame2 & 0x03ff)) & 0x03ff ) )
					{
						ir_payload.payload |= frame1;
						ir_payload.payload = ((ir_payload.payload<<16)|frame2 );
						ir_payload.info.bits.protocol_type = IR_PROTOCOL_38K_SHARP;
					}

					state_mach = -1; // done
				}	
			}
				break;

			default:
			{
			}
				break;

		}
	}

#if 0
	TRACE_DEBUG((0, "sharp = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));	
#endif 

	return ir_payload;
}


static IR_PROTOCOL_PAYLOAD IRLowLevelDecoder_SONY_Decode( const uint16 *pIrRowData, uint16 length )
{

	int8 state_mach = 0;
	uint8 i = 0;
	uint8 data_cnt = 0;
	IR_PROTOCOL_PAYLOAD ir_payload;
	
	ir_payload.payload	 = 0;
	ir_payload.info.byte = 0;
	
	if ( pIrRowData == NULL )
		return ir_payload;

	state_mach = 0;
	for( i = 0; i < length; i++ )
	{
		switch( state_mach )
		{
			case 0:
			{
				if ((*( pIrRowData+i) > IR_SONY_LEAD_P_MIN ) && (*( pIrRowData+i ) < IR_SONY_LEAD_P_MAX ))
				{
 					state_mach = 1;
				}
			}
				break;

			case 1:
			{
				if ((*( pIrRowData+i ) > IR_SONY_1T_S_MIN ) && (*( pIrRowData+i ) < IR_SONY_1T_S_MAX ))
				{
					ir_payload.info.bits.protocol_type = IR_PROTOCOL_40K_SONY;
					state_mach = 2;

				}
			
			}
				break;

			case 2: // decode data
			{
				if ((*( pIrRowData+i ) > IR_SONY_LG1_P_MIN ) && (*( pIrRowData+i ) < IR_SONY_LG1_P_MAX ))
				{
					if (*( pIrRowData+i+1 ) > IR_SONY_1T_S_MAX )
					{
						ir_payload.payload <<= 1;
						ir_payload.payload |= 1;
						data_cnt++;
						state_mach = 3; 
					}
					else if ((*( pIrRowData+i+1 ) > IR_SONY_1T_S_MIN ) && (*( pIrRowData+i+1 ) < IR_SONY_1T_S_MAX ))
					{
						ir_payload.payload <<= 1;
						ir_payload.payload |= 1;
						data_cnt++;
					}
				}
				else if ((*( pIrRowData+i ) > IR_SONY_LG0_P_MIN ) && (*( pIrRowData+i ) < IR_SONY_LG0_P_MAX ))
				{
					if (*( pIrRowData+i+1 ) > IR_SONY_1T_S_MAX )
					{
						ir_payload.payload <<=1;
						data_cnt++;
						state_mach = 3; 
						
					}
					else if ((*( pIrRowData+i+1 ) > IR_SONY_1T_S_MIN ) && (*( pIrRowData+i+1 ) < IR_SONY_1T_S_MAX ))
					{
						ir_payload.payload <<=1;
						data_cnt++;
					}
				}
			}
				break;

			case 3:
			{
#if 0
				TRACE_DEBUG((0, "sony = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));
				TRACE_DEBUG((0, "data cnt = %d", data_cnt ));
#endif

				state_mach = -1; // done
				ir_payload.info.bits.protocol_type = IR_PROTOCOL_40K_SONY;
			}
				break;

			default:
			{
				
			}
				break;

		}
	}

#if 0
	TRACE_DEBUG((0, "sony = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));
#endif 	

 	return ir_payload;
}

static IR_PROTOCOL_PAYLOAD IRLowLevelDecoder_JVC_Decode( const uint16 *pIrRowData, uint16 length )
{

	int8 state_mach = 0;
	uint8 i = 0;
	uint8 data_cnt = 0;
	IR_PROTOCOL_PAYLOAD ir_payload;
	

	ir_payload.payload	 = 0;
	ir_payload.info.byte = 0;
	
	if ( pIrRowData == NULL )
		return ir_payload;

	state_mach = 0;
	for( i = 0; i < length; i++ )
	{
		switch( state_mach )
		{
			case 0:
			{
				if ((*( pIrRowData+i) > IR_JVC_LEAD_P_MIN ) && (*( pIrRowData+i ) < IR_JVC_LEAD_P_MAX ))
				{
					ir_payload.info.bits.protocol_type = IR_PROTOCOL_38K_JVC;
					state_mach = 1;
				}
				else if ((*( pIrRowData+i) > IR_JVC_REPEAT_S_MIN ) && (*( pIrRowData+i ) < (IR_JVC_REPEAT_S_MAX*2) ))
				{
					ir_payload.info.bits.protocol_type = IR_PROTOCOL_38K_JVC;
					ir_payload.info.bits.repeat_type = IR_PROTOCOL_38K_JVC_REPEAT;
					state_mach = 2;
				}
			}
				break;

			case 1:
			{
				if ((*( pIrRowData+i ) > IR_JVC_LEAD_S_MIN ) && (*( pIrRowData+i ) < IR_JVC_LEAD_S_MAX ))
				{
					state_mach = 2;
				}
			}
				break;

			case 2: // decode data
			{
				if ((*( pIrRowData+i ) > IR_JVC_LG1_S_MIN ) && (*( pIrRowData+i ) < IR_JVC_LG1_S_MAX ))
				{
					if ((*( pIrRowData+i-1 ) > IR_JVC_1T_P_MIN ) && (*( pIrRowData+i-1 ) < IR_JVC_1T_P_MAX ))
					{
						ir_payload.payload <<= 1;
						ir_payload.payload |= 1;
						data_cnt++;
					}
				}
				else if ((*( pIrRowData+i ) > IR_JVC_LG0_S_MIN ) && (*( pIrRowData+i ) < IR_JVC_LG0_S_MAX ))
				{
					if ((*( pIrRowData+i-1 ) > IR_JVC_1T_P_MIN ) && (*( pIrRowData+i-1 ) < IR_JVC_1T_P_MAX ))
					{
						ir_payload.payload <<=1;	
						data_cnt++;
					}
				}

				if ( data_cnt >= 16 )
				{
					//TRACE_DEBUG((0, "data index = %d", i));
					state_mach = -1;
					break;
				}
			}
				break;

			default:
			{
			}
				break;

		}
	}

#if 0
	TRACE_DEBUG((0, "jvc = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));
#endif 	

 	return ir_payload;
}

static IR_PROTOCOL_PAYLOAD IRLowLevelDecoder_NEC_Decode( const uint16 *pIrRowData, uint16 length )
{

	int8 state_mach = 0;
	uint8 i = 0;
	uint8 data_cnt = 0;
	IR_PROTOCOL_PAYLOAD ir_payload;
	

	ir_payload.payload	 = 0;
	ir_payload.info.byte = 0;
	
	if ( pIrRowData == NULL )
		return ir_payload;

	state_mach = 0;
	for( i = 0; i < length; i++ )
	{
		switch( state_mach )
		{
			case 0:
			{
				if ((*( pIrRowData+i) > IR_NEC_LEAD_P_MIN ) && (*( pIrRowData+i ) < IR_NEC_LEAD_P_MAX ))
				{
					ir_payload.info.bits.protocol_type = IR_PROTOCOL_38K_NEC;
					state_mach = 1;
				}
			}
				break;

			case 1:
			{
				if ((*( pIrRowData+i ) > IR_NEC_LEAD_S_MIN ) && (*( pIrRowData+i ) < IR_NEC_LEAD_S_MAX ))
				{
					state_mach = 2;
				}
				else if ((*( pIrRowData+i ) > IR_NEC_REPEAT_S_MIN ) && (*( pIrRowData+i ) < IR_NEC_REPEAT_S_MAX ))
				{
					ir_payload.info.bits.repeat_type = IR_PROTOCOL_38K_NEC_REPEAT;
					state_mach = -1; // done
				}
			}
				break;

			case 2: // decode data
			{
				if ((*( pIrRowData+i ) > IR_NEC_LG1_S_MIN ) && (*( pIrRowData+i ) < IR_NEC_LG1_S_MAX ))
				{
					if ((*( pIrRowData+i-1 ) > IR_NEC_1T_P_MIN ) && (*( pIrRowData+i-1 ) < IR_NEC_1T_P_MAX ))
					{
						ir_payload.payload <<= 1;
						ir_payload.payload |= 1;
						data_cnt++;
					}
				}
				else if ((*( pIrRowData+i ) > IR_NEC_LG0_S_MIN ) && (*( pIrRowData+i ) < IR_NEC_LG0_S_MAX ))
				{
					if ((*( pIrRowData+i-1 ) > IR_NEC_1T_P_MIN ) && (*( pIrRowData+i-1 ) < IR_NEC_1T_P_MAX ))
					{
						ir_payload.payload <<=1;	
						data_cnt++;
					}
				}

				if ( data_cnt >= 32 )
				{
					state_mach = 3;
					break;
				}
			}
				break;

			case 3: // check protocol type
			{
				uint8 *pData;
				
				pData = (uint8*)&ir_payload.payload;
				if( ( ((*(pData+3))) == ( ~(*(pData+2))&0x0ff  ) ) && ( ((*(pData+1))) == ( ~(*(pData+0))&0x0ff  ) ))
				{
					ir_payload.info.bits.protocol_type = IR_PROTOCOL_38K_NEC;
					state_mach = -1; // done
				}
			}
				break;

			default:
			{
			}
				break;

		}
	}

#if 0
	TRACE_DEBUG((0, "nec = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));
#endif 	

 	return ir_payload;
}


static void IRLowLevelDecoder_DecoderTask( void *pvParameters )
{
	IR_PROTOCOL_PAYLOAD ir_payload;

	
    for( ;; )
    {
        ir_queue_number = pIR_LowLevel->get_queue_number();
		if ( ir_queue_number > 0 )
		{
			if( ( xTaskGetTickCount( ) - xIrDecoderTimeTick ) > 110 )
			{
	            pIR_LowLevel->get_row_data( ir_rowdata );

				ir_payload.info.byte = 0;
				ir_payload.payload = IR_PROTOCOL_NULL;

				if (  ir_payload.info.bits.protocol_type == IR_PROTOCOL_NULL )
				{
					ir_payload = IRLowLevelDecoder_NEC_Decode(ir_rowdata, ir_queue_number );
					if( ir_payload.info.bits.protocol_type == IR_PROTOCOL_38K_NEC )
					{
						if( ir_payload.info.bits.repeat_type != IR_PROTOCOL_38K_NEC_REPEAT )
						{
							mIrPayload.payload = IRLowLevelDecoder_BitsReverse(ir_payload.payload);
						}
						
						mIrPayload.info.bits.repeat_type = ir_payload.info.bits.repeat_type;
						mIrPayload.info.bits.protocol_type = ir_payload.info.bits.protocol_type;
						pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );	
					}
				}

				if (  ir_payload.info.bits.protocol_type == IR_PROTOCOL_NULL )
				{
					ir_payload = IRLowLevelDecoder_SONY_Decode(ir_rowdata, ir_queue_number );
					if( ir_payload.info.bits.protocol_type == IR_PROTOCOL_40K_SONY )
					{
						mIrPayload.info.bits.repeat_type = ir_payload.info.bits.repeat_type;
						mIrPayload.info.bits.protocol_type = ir_payload.info.bits.protocol_type;						
						mIrPayload.payload = ir_payload.payload;
						pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );	
					}
				}

				if (  ir_payload.info.bits.protocol_type == IR_PROTOCOL_NULL )
				{
					ir_payload = IRLowLevelDecoder_Sharp_Decode(ir_rowdata, ir_queue_number );
					if( ir_payload.info.bits.protocol_type == IR_PROTOCOL_38K_SHARP )
					{
						mIrPayload.info.bits.repeat_type = ir_payload.info.bits.repeat_type;
						mIrPayload.info.bits.protocol_type = ir_payload.info.bits.protocol_type;						
						mIrPayload.payload = ir_payload.payload;
						pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );	
					}
				}

				if (  ir_payload.info.bits.protocol_type == IR_PROTOCOL_NULL )
				{
					ir_payload = IRLowLevelDecoder_JVC_Decode(ir_rowdata, ir_queue_number );
					if( ir_payload.info.bits.protocol_type == IR_PROTOCOL_38K_JVC )
					{
						mIrPayload.info.bits.repeat_type = ir_payload.info.bits.repeat_type;
						mIrPayload.info.bits.protocol_type = ir_payload.info.bits.protocol_type;						
						mIrPayload.payload = ir_payload.payload;
						pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );	
					}
				}

				if (  ir_payload.info.bits.protocol_type == IR_PROTOCOL_NULL )
				{
					ir_payload = IRLowLevelDecoder_RC5_Decode(ir_rowdata, ir_queue_number );
					if( ir_payload.info.bits.protocol_type == IR_PROTOCOL_36K_RC5 )
					{
						mIrPayload.info.bits.repeat_type = ir_payload.info.bits.repeat_type;
						mIrPayload.info.bits.protocol_type = ir_payload.info.bits.protocol_type;						
						mIrPayload.payload = ir_payload.payload;
						pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );	
					}
				}
	
				memset( ir_rowdata, 0, IR_ROW_DATA_MAX ); 
				xIrDecoderTimeTick = xTaskGetTickCount( );
			}
			
		}
		else
		{
			xIrDecoderTimeTick = xTaskGetTickCount( );
		}

		

		
        vTaskDelay( 10 ); 
    }
}

void IRLowLevelDecoder_CreateTask( void )
{
	mIrPayload.info.byte = 0;
	mIrPayload.payload= 0;
	memset( ir_rowdata, 0, IR_ROW_DATA_MAX );
	
    if ( xTaskCreate( IRLowLevelDecoder_DecoderTask,
        ( portCHAR * ) "IR_Decoder",
         configMINIMAL_STACK_SIZE, NULL,
         tskHMI_EVENT_PRIORITY, NULL ) != pdPASS)
    {
    	TRACE_ERROR((0, " IRLowLevelDecoder_ServiceHandle task create error !! "));
    }
}

