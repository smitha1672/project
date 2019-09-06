#include "Defs.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "api_typedef.h"

#include "Debug.h"
#include "IRLowLevel.h"
#include "IRCmdHandler.h"
#include "I2C1LowLevel.h"

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

#define IR_SAMSUNG_1T_P_MIN ( ( (560-280)/IR_TIME_BASE ) |0x8000 )
#define IR_SAMSUNG_1T_P_MAX ( ( (560+280)/IR_TIME_BASE ) |0x8000 )
#define IR_SAMSUNG_LEAD_P_MIN ( ( (4500-280)/IR_TIME_BASE ) |0x8000 )
#define IR_SAMSUNG_LEAD_P_MAX ( ( (4500+280)/IR_TIME_BASE ) |0x8000 )
#define IR_SAMSUNG_LEAD_S_MIN ( ( (4500-280)/IR_TIME_BASE ) |0x0000 )
#define IR_SAMSUNG_LEAD_S_MAX ( ( (4500+280)/IR_TIME_BASE ) |0x0000 )
#define IR_SAMSUNG_LG1_S_MIN ( ( (1690-280)/IR_TIME_BASE ) |0x0000 )
#define IR_SAMSUNG_LG1_S_MAX ( ( (1690+280)/IR_TIME_BASE ) |0x0000 )
#define IR_SAMSUNG_LG0_S_MIN ( ( (560-280)/IR_TIME_BASE ) |0x0000 )
#define IR_SAMSUNG_LG0_S_MAX ( ( (560+280)/IR_TIME_BASE ) |0x0000 )

#define IR_PANASONIC_1T_P_MIN ( ( (400-200)/IR_TIME_BASE ) |0x8000 )
#define IR_PANASONIC_1T_P_MAX ( ( (400+200)/IR_TIME_BASE ) |0x8000 )
#define IR_PANASONIC_LEAD_P_MIN ( ( (3440-240)/IR_TIME_BASE ) |0x8000 )
#define IR_PANASONIC_LEAD_P_MAX ( ( (3440+240)/IR_TIME_BASE ) |0x8000 )
#define IR_PANASONIC_LEAD_S_MIN ( ( (1780-240)/IR_TIME_BASE ) |0x0000 )
#define IR_PANASONIC_LEAD_S_MAX ( ( (1780+240)/IR_TIME_BASE ) |0x0000 )
#define IR_PANASONIC_LG1_S_MIN ( ( (1360-240)/IR_TIME_BASE ) |0x0000 )
#define IR_PANASONIC_LG1_S_MAX ( ( (1360+240)/IR_TIME_BASE ) |0x0000 )
#define IR_PANASONIC_LG0_S_MIN ( ( (480-240)/IR_TIME_BASE ) |0x0000 )
#define IR_PANASONIC_LG0_S_MAX ( ( (480+240)/IR_TIME_BASE ) |0x0000 )

#define IR_JVC_1T_P_MIN ( ( (527-263)/IR_TIME_BASE ) |0x8000 )
#define IR_JVC_1T_P_MAX ( ( (527+263)/IR_TIME_BASE ) |0x8000 )
#define IR_JVC_LEAD_P_MIN ( ( (8440-263)/IR_TIME_BASE ) |0x8000 )
#define IR_JVC_LEAD_P_MAX ( ( (8440+263)/IR_TIME_BASE ) |0x8000 )
#define IR_JVC_LEAD_S_MIN ( ( (4220-263)/IR_TIME_BASE ) |0x0000 )
#define IR_JVC_LEAD_S_MAX ( ( (4220+263)/IR_TIME_BASE ) |0x0000 )
#define IR_JVC_REPEAT_S_MIN ( ( (23800-280)/IR_TIME_BASE ) |0x0000 )
#define IR_JVC_REPEAT_S_MAX ( ( (23800+280)/IR_TIME_BASE ) |0x0000 )
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

#define IR_RC6_SHORT_SPACE  ( (444/IR_TIME_BASE) )
#define IR_RC6_SHORT_SPACE_MIN ( (222/IR_TIME_BASE )  )
#define IR_RC6_SHORT_SPACE_MAX ( (666/IR_TIME_BASE )  )

#define IR_RC6_LONG_SPACE ( (444*2/IR_TIME_BASE) )
#define IR_RC6_LONG_SPACE_MIN ( (666/IR_TIME_BASE) )
#define IR_RC6_LONG_SPACE_MAX ( (1100/IR_TIME_BASE) )

#define IR_RC6_TOG_LONG_SPACE ( (444*3/IR_TIME_BASE) )
#define IR_RC6_TOG_LONG_SPACE_MIN ( (1110/IR_TIME_BASE) )
#define IR_RC6_TOG_LONG_SPACE_MAX ( (1552/IR_TIME_BASE) )

#define IR_RC6_TOG_LONG_PULSE ( (444*3/IR_TIME_BASE) |0x8000 )
#define IR_RC6_TOG_LONG_PULSE_MIN ( (1110/IR_TIME_BASE) |0x8000 )
#define IR_RC6_TOG_LONG_PULSE_MAX ( (1552/IR_TIME_BASE) |0x8000 )

#define IR_RC6_LEADER_P ( (2666/IR_TIME_BASE) |0x8000 )
#define IR_RC6_LEADER_P_MIN ( (2444/IR_TIME_BASE) |0x8000 )
#define IR_RC6_LEADER_P_MAX ( (2888/IR_TIME_BASE) |0x8000 )

#define IR_RC6_LEADER_S ( (889/IR_TIME_BASE) )
#define IR_RC6_LEADER_S_MIN ( (667/IR_TIME_BASE) )
#define IR_RC6_LEADER_S_MAX ( (1111/IR_TIME_BASE) )
//____________________________________________________________________________________________________________
#define IR_CMD_REPEAT_TIME 220
#define SamsungRepeatTime 125 /*Tony150304:samsung repeat time is 108 and add 15% for tolerance*/
//____________________________________________________________________________________________________________
extern IR_LOWLEVEL_OBJECT *pIR_LowLevel;
extern IR_CMD_HANDLER_OBJECT *pIRCmdHandle_ObjCtrl;
//____________________________________________________________________________________________________________
static portTickType xIrDecoderTimeTick = 0;

uint16 ir_rowdata[IR_ROW_DATA_MAX];
uint16 ir_queue_number = 0;
IR_PROTOCOL_PAYLOAD mIrPayload;

static xSemaphoreHandle semaIrDecode = NULL;
static bool b_ir_decoding = FALSE;

void ir_decode_free(void)
{
    xSemaphoreTake(semaIrDecode, portMAX_DELAY);
    b_ir_decoding = 0;
    xSemaphoreGive(semaIrDecode);
}

void ir_decode_lock(void)
{
    xSemaphoreTake(semaIrDecode, portMAX_DELAY);
    b_ir_decoding = 1;
    xSemaphoreGive(semaIrDecode);
}

void ir_decode_wait(uint16 timeout)
{
    while(timeout)
    {
        xSemaphoreTake(semaIrDecode, portMAX_DELAY);
        if(!b_ir_decoding)
        {
            xSemaphoreGive(semaIrDecode);
            break;
        }
        xSemaphoreGive(semaIrDecode);
    
        vTaskDelay(1);
        if(timeout != 0xFFFF)
            timeout--;
    }
    if(timeout==0)
    {
        TRACE_ERROR((0, "I2C wait IR timeout !!!"));
    }
}

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
    static bool FirstDataCome = FALSE;
    static portTickType timeTick;
    portTickType timeDuration;

    ir_payload.payload	 = 0;
    ir_payload.info.byte = 0;
    if ( pIrRowData == NULL )
        return ir_payload;

    if(FirstDataCome)
    {
        FirstDataCome =  FALSE;
        timeDuration = ( xTaskGetTickCount( ) - timeTick);
        if ((timeDuration/portTICK_RATE_MS) < TASK_MSEC2TICKS(220))
        {  
#if 0
            TRACE_DEBUG((0, "OUT , TIME = %d !!",(timeDuration/portTICK_RATE_MS) ));
#endif
            ir_payload.info.bits.protocol_type = IR_PROTOCOL_UNKNOW;    //Angus added , by pass this IR command. It is too closing that will effect setting mute.                 
            return ir_payload;             
        }
    }  

    state_mach = 0;
    for( i = 0; i < length; i++ )
    {
        switch( state_mach )
        {
            case 0:
            {
                if ( ( *( pIrRowData+i ) > IR_RC5_SHORT_PULSE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_SHORT_PULSE_MAX ) )
                {
                    if( ( *( pIrRowData+i+1 ) > IR_RC5_SHORT_SPACE_MIN ) && ( *( pIrRowData+i+1 ) < IR_RC5_SHORT_SPACE_MAX ) )
                    {
                        if(( *( pIrRowData+i+2 ) > IR_RC5_LONG_PULSE_MIN ) && ( *( pIrRowData+i+2 ) < IR_RC5_LONG_PULSE_MAX ))
                        {
                            state_mach = 1;
                        }
                        else if((( *( pIrRowData+i+2 ) > IR_RC5_SHORT_PULSE_MIN ) && ( *( pIrRowData+i+2 ) < IR_RC5_SHORT_PULSE_MAX )) &&
                                    (( *( pIrRowData+i+3 ) > IR_RC5_SHORT_SPACE_MIN ) && ( *( pIrRowData+i+3 ) < IR_RC5_SHORT_SPACE_MAX )))
                        {
                            state_mach = 1;
                        }
                    }
                }

            }
            break;

            case 1:
            {
                if ( ( *( pIrRowData+i ) > IR_RC5_SHORT_SPACE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_SHORT_SPACE_MAX ) )
                {
                    // one	
                    ir_payload.payload <<= 1;   // 1 -> 1
                    ir_payload.payload |=1;
                    data_cnt ++;
                    
                    state_mach = 2;
                }
                
            }
            break;
            

            case 2:
            {
                if ( ( *( pIrRowData+i ) > IR_RC5_SHORT_PULSE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_SHORT_PULSE_MAX ) )
                {
                    state_mach = 1;
                }
                else if ( ( *( pIrRowData+i ) > IR_RC5_LONG_PULSE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_LONG_PULSE_MAX ) )
                {
                    // zero
                    ir_payload.payload <<= 1; // 1 -> 0
                    data_cnt ++;
                    state_mach = 3;
                }
                
            }
            break;


            case 3:
            {
                if ( ( *( pIrRowData+i ) > IR_RC5_SHORT_SPACE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_SHORT_SPACE_MAX ) )
                {
                    state_mach = 4;
                }
                else if ( ( *( pIrRowData+i ) > IR_RC5_LONG_SPACE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_LONG_SPACE_MAX ) )
                {
                    // one
                    ir_payload.payload <<= 1; // 0 -> 1
                    ir_payload.payload |=1;
                    data_cnt ++;
                    state_mach = 2;
                }
            }
            break;


            case 4:
            {
                if ( ( *( pIrRowData+i ) > IR_RC5_SHORT_PULSE_MIN ) && ( *( pIrRowData+i ) < IR_RC5_SHORT_PULSE_MAX ) )
                {
                    // zero
                    ir_payload.payload <<= 1; // 0 -> 0
                    data_cnt ++;
                    state_mach = 3;
                }
                
            }
            break;



        }

        if (data_cnt >=13)
        {
            timeDuration = xTaskGetTickCount( );
            timeTick = timeDuration;
            FirstDataCome =  TRUE;

            ir_payload.info.bits.protocol_type = IR_PROTOCOL_36K_RC5;
            ir_payload.payload = ir_payload.payload | 0x800; //Ignore toggle bit , Angus added 
            break;
        }
    }

#if 0
	TRACE_DEBUG((0, "rc5  = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));
	TRACE_DEBUG((0, "data_cnt = %d", data_cnt ));
#endif 	
	
	return ir_payload;
}

static IR_PROTOCOL_PAYLOAD IRLowLevelDecoder_Sharp_Decode( const uint16 *pIrRowData, uint16 length )
{

	int8 state_mach = 0;
	uint8 i = 0;
	uint8 data_cnt = 0;
	IR_PROTOCOL_PAYLOAD ir_payload;
       static bool FirstDataCome = FALSE;
       static portTickType timeTick;
       portTickType timeDuration;

	uint16 frame1 = 0;
	uint16 frame2 = 0;

	ir_payload.payload	 = 0;
	ir_payload.info.byte = 0;
	
	if ( pIrRowData == NULL )
		return ir_payload;

        if(FirstDataCome)
        {
            FirstDataCome =  FALSE;
            timeDuration = ( xTaskGetTickCount( ) - timeTick);
            if ((timeDuration/portTICK_RATE_MS) < TASK_MSEC2TICKS(220))
            {  
#if 0
                TRACE_DEBUG((0, "OUT , TIME = %d !!",(timeDuration/portTICK_RATE_MS) ));
#endif
                ir_payload.info.bits.protocol_type = IR_PROTOCOL_UNKNOW;    //Angus added , by pass this IR command. It is too closing that will effect setting mute.                 
                return ir_payload;             
            }
        } 
        

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
                    timeDuration = xTaskGetTickCount( );
                    timeTick = timeDuration;
                    FirstDataCome =  TRUE;

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
    static bool FirstDataCome = FALSE;
    static portTickType timeTick;
    portTickType timeDuration;
    
	ir_payload.payload	 = 0;
	ir_payload.info.byte = 0;
	
	if ( pIrRowData == NULL )
		return ir_payload;

    if(FirstDataCome)
    {
        FirstDataCome =  FALSE;
        timeDuration = ( xTaskGetTickCount( ) - timeTick);
        if ((timeDuration/portTICK_RATE_MS) < TASK_MSEC2TICKS(220))
        {  
#if 0
            TRACE_DEBUG((0, "OUT , TIME = %d !!",(timeDuration/portTICK_RATE_MS) ));
#endif
            ir_payload.info.bits.protocol_type = IR_PROTOCOL_UNKNOW;    //Angus added , by pass this IR command. It is too closing that will effect setting mute.                 
            return ir_payload;             
        }
    }  

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
                timeDuration = xTaskGetTickCount( );
                timeTick = timeDuration;
                FirstDataCome =  TRUE;

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
    static bool FirstDataCome = FALSE;
    static portTickType timeTick;
    portTickType timeDuration;	

	ir_payload.payload	 = 0;
	ir_payload.info.byte = 0;
	
	if ( pIrRowData == NULL )
		return ir_payload;

    if(FirstDataCome)
    {
        FirstDataCome =  FALSE;
        timeDuration = ( xTaskGetTickCount( ) - timeTick);
        if ((timeDuration/portTICK_RATE_MS) < TASK_MSEC2TICKS(220))
        {  
#if 0
            TRACE_DEBUG((0, "OUT , TIME = %d !!",(timeDuration/portTICK_RATE_MS) ));
#endif
            ir_payload.info.bits.protocol_type = IR_PROTOCOL_UNKNOW;    //Angus added , by pass this IR command. It is too closing that will effect setting mute.                 
            return ir_payload;             
        }
    } 

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
					timeDuration = xTaskGetTickCount( );
					timeTick = timeDuration;
					FirstDataCome =  TRUE;
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

static IR_PROTOCOL_PAYLOAD IRLowLevelDecoder_SAMSUNG_Decode( const uint16 *pIrRowData, uint16 length )
{
        int8 state_mach = 0;
        uint8 i = 0;
        uint8 data_cnt = 0;
        IR_PROTOCOL_PAYLOAD ir_payload;
        uint8 MAX_DATA_CNT = 32;
        bool correctDecode = FALSE;
        static bool FirstDataCome = FALSE;
        static portTickType repeatTimeTick;
        static portTickType timeTick;
        portTickType timeDuration;
    
        ir_payload.payload   = 0;
        ir_payload.info.byte = 0;
        
        if ( pIrRowData == NULL )
            return ir_payload;
#if 0  /*Tony150304:It causes remote delay*/
        if(FirstDataCome)
        {
            FirstDataCome =  FALSE;
            timeDuration = ( xTaskGetTickCount( ) - timeTick);
            if ((timeDuration/portTICK_RATE_MS) < TASK_MSEC2TICKS(220))
            {  
#if 0
                TRACE_DEBUG((0, "OUT , TIME = %d !!",(timeDuration/portTICK_RATE_MS) ));
#endif
                ir_payload.info.bits.protocol_type = IR_PROTOCOL_UNKNOW;    //Angus added , by pass this IR command. It is too closing that will effect setting mute.                 
                return ir_payload;             
            }
        }   
#endif       
        state_mach = 0;
        for( i = 0; i < length; i++ )
        {
            switch( state_mach )
            {
                case 0:
                {   
                    if ((*( pIrRowData+i) > IR_SAMSUNG_LEAD_P_MIN ) && (*( pIrRowData+i ) < IR_SAMSUNG_LEAD_P_MAX ))
                    {  
                        state_mach = 1;
                    }
                }
                    break;
    
                case 1:
                {   
                    if ((*( pIrRowData+i) > IR_SAMSUNG_LEAD_S_MIN ) && (*( pIrRowData+i ) < IR_SAMSUNG_LEAD_S_MAX ))
                    {  
                        ir_payload.info.bits.protocol_type = IR_PROTOCOL_38K_SAMSUNG;  
                        if ((xTaskGetTickCount( )- repeatTimeTick) < SamsungRepeatTime)
                        {
                            ir_payload.info.bits.repeat_type = IR_PROTOCOL_38K_SAMSUNG_REPEAT;
                            repeatTimeTick = timeDuration;
                            state_mach = -1; // done
                            correctDecode = TRUE;
                        }
                        else
                        {
                            state_mach =2;
                        }
                        repeatTimeTick = xTaskGetTickCount( );
                    }
                }
                break;
    
                case 2: // decode data
                {
                    if ((*( pIrRowData+i ) > IR_SAMSUNG_LG1_S_MIN ) && (*( pIrRowData+i ) < IR_SAMSUNG_LG1_S_MAX ))
                    {
                        if ((*( pIrRowData+i-1 ) > IR_SAMSUNG_1T_P_MIN ) && (*( pIrRowData+i-1 ) < IR_SAMSUNG_1T_P_MAX ))
                        {
                            ir_payload.payload <<= 1;
                            ir_payload.payload |= 1;
                            data_cnt++;
                        }
                    }
                    else if ((*( pIrRowData+i ) > IR_SAMSUNG_LG0_S_MIN ) && (*( pIrRowData+i ) < IR_SAMSUNG_LG0_S_MAX ))
                    {
                        if ((*( pIrRowData+i-1 ) > IR_SAMSUNG_1T_P_MIN ) && (*( pIrRowData+i-1 ) < IR_SAMSUNG_1T_P_MAX ))
                        {
                            ir_payload.payload <<=1;
                            data_cnt++;
                        }
                    }

                    #if 1   //Angus added for Raken speacial samsung IR key code.
                    else if((*( pIrRowData+i ) > IR_SAMSUNG_LEAD_S_MIN ) && (*( pIrRowData+i ) < IR_SAMSUNG_LEAD_S_MAX ))
                    {
                        if ((*( pIrRowData+i-1 ) > IR_SAMSUNG_1T_P_MIN ) && (*( pIrRowData+i-1 ) < IR_SAMSUNG_1T_P_MAX ))
                        {
                            ir_payload.payload = 0;
                            data_cnt = 0;
                            MAX_DATA_CNT = 20;
                        }
                    }
                    #endif
    
                    if ( data_cnt >= MAX_DATA_CNT )
                    {
                        state_mach = 3;
                        break;
                    }
                }
                    break;
    
                case 3:
                {
#if 0
                    TRACE_DEBUG((0, "SAMSUNG = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));
                    TRACE_DEBUG((0, "data cnt = %d", data_cnt ));
#endif
                    timeDuration = xTaskGetTickCount( );
                    timeTick = timeDuration;
                    repeatTimeTick = timeDuration;
                    FirstDataCome =  TRUE;
                    state_mach = -1; // done
                    correctDecode = TRUE;
                    ir_payload.info.bits.protocol_type = IR_PROTOCOL_38K_SAMSUNG;
                }
                    break;
    
                default:
                {
                    
                }
                    break;
    
            }
        }
        
        if(ir_payload.info.bits.protocol_type == IR_PROTOCOL_38K_SAMSUNG && correctDecode == FALSE)
        {
            ir_payload.info.bits.protocol_type = IR_PROTOCOL_DECODE_ERROR;
        }
        
        return ir_payload;
    
}

static IR_PROTOCOL_PAYLOAD IRLowLevelDecoder_PANASONIC_Decode( const uint16 *pIrRowData, uint16 length )
{
	int8 state_mach = 0;
	uint8 i = 0;
	uint8 data_cnt = 0;
	IR_PROTOCOL_PAYLOAD ir_payload;

    static bool FirstDataCome = FALSE;
    static portTickType timeTick;
    portTickType timeDuration;
	ir_payload.payload	 = 0;
	ir_payload.info.byte = 0;
	
	if ( pIrRowData == NULL )
		return ir_payload;

        if(FirstDataCome)
        {
            FirstDataCome =  FALSE;
            timeDuration = ( xTaskGetTickCount( ) - timeTick);
            if ((timeDuration/portTICK_RATE_MS) < TASK_MSEC2TICKS(220))
            {   
#if 0
                TRACE_DEBUG((0, "OUT , TIME = %d !!",(timeDuration/portTICK_RATE_MS) ));
#endif
                ir_payload.info.bits.protocol_type = IR_PROTOCOL_UNKNOW;    //Angus added , by pass this IR command. It is too closing that will effect setting mute.  
                return ir_payload;             
            }
        }
        
	state_mach = 0;
	for( i = 0; i < length; i++ )
	{
		switch( state_mach )
		{
			case 0:
			{
				if ((*( pIrRowData+i) > IR_PANASONIC_LEAD_P_MIN ) && (*( pIrRowData+i ) < IR_PANASONIC_LEAD_P_MAX ))
				{
 					state_mach = 1;
				}
			}
				break;

                    case 1:
                    {
                        if ((*( pIrRowData+i) > IR_PANASONIC_LEAD_S_MIN ) && (*( pIrRowData+i ) < IR_PANASONIC_LEAD_S_MAX ))
                        {
                            state_mach =2;
                        }
                    }
                        break;

			case 2: // decode data
			{
				if ((*( pIrRowData+i ) > IR_PANASONIC_LG1_S_MIN ) && (*( pIrRowData+i ) < IR_PANASONIC_LG1_S_MAX ))
				{
					if ((*( pIrRowData+i-1 ) > IR_PANASONIC_1T_P_MIN ) && (*( pIrRowData+i-1 ) < IR_PANASONIC_1T_P_MAX ))
					{
						ir_payload.payload <<= 1;
						ir_payload.payload |= 1;
						data_cnt++;
					}
				}
				else if ((*( pIrRowData+i ) > IR_PANASONIC_LG0_S_MIN ) && (*( pIrRowData+i ) < IR_PANASONIC_LG0_S_MAX ))
				{
                                    if ((*( pIrRowData+i-1 ) > IR_PANASONIC_1T_P_MIN ) && (*( pIrRowData+i-1 ) < IR_PANASONIC_1T_P_MAX ))
					{
						ir_payload.payload <<=1;
						data_cnt++;
					}
				}

                            if ( data_cnt >= 48 )
				{
					state_mach = 3;
					break;
				}
			}
				break;

			case 3:
			{
#if 0
                		TRACE_DEBUG((0, "PANASONIC = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));
                		TRACE_DEBUG((0, "data cnt = %d", data_cnt ));
#endif
                            timeDuration = xTaskGetTickCount( );
                            timeTick = timeDuration;
                            FirstDataCome =  TRUE;

				state_mach = -1; // done
				ir_payload.info.bits.protocol_type = IR_PROTOCOL_38K_PANASONIC;
			}
				break;

			default:
			{
				
			}
				break;

		}
	}	
#if 0
            TRACE_DEBUG((0, "NG PANASONIC = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));
            TRACE_DEBUG((0, "NG data cnt = %d , length = %d", data_cnt,length ));
#endif

 	return ir_payload;

}
    

static IR_PROTOCOL_PAYLOAD IRLowLevelDecoder_RC6_Decode( const uint16 *pIrRowData, uint16 length )
{
        int8 state_mach = 0;
        uint8 i = 0;
        uint8 data_cnt = 0;
        IR_PROTOCOL_PAYLOAD ir_payload;
        bool correctDecode = FALSE;
        
        static bool FirstDataCome = FALSE;
        static portTickType timeTick;
        portTickType timeDuration;
        
        ir_payload.payload   = 0;
        ir_payload.info.byte = 0;
        
        if ( pIrRowData == NULL )
            return ir_payload;
        
        if(FirstDataCome)
        {
            FirstDataCome =  FALSE;
            timeDuration = ( xTaskGetTickCount( ) - timeTick);
            if ((timeDuration/portTICK_RATE_MS) < TASK_MSEC2TICKS(220))
            {   
#if 0
                TRACE_DEBUG((0, "OUT , TIME = %d !!",(timeDuration/portTICK_RATE_MS) ));
#endif
                ir_payload.info.bits.protocol_type = IR_PROTOCOL_UNKNOW;    //Angus added , by pass this IR command. It is too closing that will effect setting mute.  
                return ir_payload;             
            }
        }    
        state_mach = 0;
        for( i = 0; i < length; i++ )
        {
            switch( state_mach )
            {
                case 0:
                {   
                    if ((*( pIrRowData+i) > IR_RC6_LEADER_P_MIN ) && (*( pIrRowData+i ) < IR_RC6_LEADER_P_MAX ))
                    {  
                        state_mach = 1;
                    }
                }
                    break;
    
                case 1:
                {   
                    if ((*( pIrRowData+i) > IR_RC6_LEADER_S_MIN ) && (*( pIrRowData+i ) < IR_RC6_LEADER_S_MAX ))
                    {  
                        ir_payload.info.bits.protocol_type = IR_PROTOCOL_36K_RC6; 
                        state_mach =2;
                    }
                }
                break;
    
                case 2:
                {
                    if ((*( pIrRowData+i) > IR_RC6_SHORT_PULSE_MIN ) && (*( pIrRowData+i ) < IR_RC6_SHORT_PULSE_MAX ))
                    {  
                        state_mach =3;
                        //TRACE_DEBUG((0, " data[%d] !!! ", i ));
                    }
                }
                break;

                case 3:
                {
                    if((*( pIrRowData+i) > IR_RC6_SHORT_SPACE_MIN ) && (*( pIrRowData+i ) < IR_RC6_SHORT_SPACE_MAX ))
                    {
                        state_mach = 4;
                    }
                    else if((*( pIrRowData+i) > IR_RC6_LONG_SPACE_MIN ) && (*( pIrRowData+i ) < IR_RC6_LONG_SPACE_MAX ))
                    {
                        // zero  , 1 -> 0
                       ir_payload.payload <<= 1;
                       data_cnt ++;
                       state_mach = 5;
                    } 
                    #if 1   //It is native Toggle
                    else if((*( pIrRowData+i) > IR_RC6_LONG_SPACE_MIN ) && (*( pIrRowData+i ) < IR_RC6_LONG_SPACE_MAX ))
                    {
                        //toggle low
                        state_mach = 8;
                    }
                    #endif
                }
                break;

                 case 4:
                {
                    if ((*( pIrRowData+i) > IR_RC6_SHORT_PULSE_MIN ) && (*( pIrRowData+i ) < IR_RC6_SHORT_PULSE_MAX ))
                    {  
                       // one  , 1 -> 1
                       ir_payload.payload <<= 1;
                       ir_payload.payload |=1;
                       data_cnt ++;
                       state_mach = 3;
                    }  
                    //postive Toggle
                    else if((*( pIrRowData+i) > IR_RC6_LONG_PULSE_MIN ) && (*( pIrRowData+i ) < IR_RC6_LONG_PULSE_MAX ))
                    {
                        //toggle high
                        state_mach = 7;
                    }
                    
                }
                break;

                 case 5:
                 {
                    if ((*( pIrRowData+i) > IR_RC6_SHORT_PULSE_MIN ) && (*( pIrRowData+i ) < IR_RC6_SHORT_PULSE_MAX ))
                    {  
                         state_mach =6;
                    }
                    else if((*( pIrRowData+i) > IR_RC6_LONG_PULSE_MIN ) && (*( pIrRowData+i ) < IR_RC6_LONG_PULSE_MAX ))
                    {
                        // one  , 0 -> 1
                       ir_payload.payload <<= 1;
                       ir_payload.payload |=1;
                       data_cnt ++;
                       state_mach = 3;
                    }
                    //It is postive Toggle
                    else if((*( pIrRowData+i) > IR_RC6_TOG_LONG_PULSE_MIN ) && (*( pIrRowData+i ) < IR_RC6_TOG_LONG_PULSE_MAX ))
                    {
                        //toggle high
                        state_mach = 7;
                    }

                 }
                 break;

                 case 6:
                {
                    if ((*( pIrRowData+i) > IR_RC6_SHORT_SPACE_MIN ) && (*( pIrRowData+i ) < IR_RC6_SHORT_SPACE_MAX ))
                    {  
                        // zero  , 0 -> 0
                           ir_payload.payload <<= 1;
                           data_cnt ++;
                           state_mach =5;
                    }
                     //It is native Toggle
                    else if((*( pIrRowData+i) > IR_RC6_LONG_SPACE_MIN ) && (*( pIrRowData+i ) < IR_RC6_LONG_SPACE_MAX ))
                    {
                        //toggle low
                        state_mach = 8;
                    }
                }
                break;


                case 7: //process postive toggle
                {
                    if ((*( pIrRowData+i) > IR_RC6_LONG_SPACE_MIN ) && (*( pIrRowData+i ) < IR_RC6_LONG_SPACE_MAX ))
                    {  
                           //after toggle bit is one 
                           ir_payload.payload <<= 1;
                           ir_payload.payload |= 1;
                           data_cnt ++;
                           state_mach =2;
                    }
                    else if ((*( pIrRowData+i) > IR_RC6_TOG_LONG_SPACE_MIN ) && (*( pIrRowData+i ) < IR_RC6_TOG_LONG_SPACE_MAX ))
                    {
                        //after toggle bit is zero
                        ir_payload.payload <<= 1;
                        data_cnt ++;
                        state_mach =5;
                    }
                }
                break;

                case 8: //process native toggle
                {
                    if ((*( pIrRowData+i) > IR_RC6_LONG_PULSE_MIN ) && (*( pIrRowData+i ) < IR_RC6_LONG_PULSE_MAX ))
                    { 
                        //after toggle bit is zero
                        ir_payload.payload <<= 1;
                        data_cnt ++;
                        state_mach =5;
                    }
                    else if((*( pIrRowData+i) > IR_RC6_TOG_LONG_PULSE_MIN ) && (*( pIrRowData+i ) < IR_RC6_TOG_LONG_PULSE_MAX ))
                    {
                        //after toggle bit is one 
                           ir_payload.payload <<= 1;
                           ir_payload.payload |= 1;
                           data_cnt ++;
                           state_mach =3;
                    }        
                }
                break;  
            }
            
            if(data_cnt >= 19)
            {
                correctDecode = TRUE;
#if 0
                TRACE_DEBUG((0, ">>> RC6 = 0x%X:%X", GET_HIGH_U16(ir_payload.payload), GET_LOW_U16(ir_payload.payload) ));
                TRACE_DEBUG((0, ">>> data cnt = %d , length = %d", data_cnt,length )); 
#endif
                timeDuration = xTaskGetTickCount( );
                timeTick = timeDuration;
                FirstDataCome =  TRUE;

                break;
            }
        }
        
        if((ir_payload.info.bits.protocol_type == IR_PROTOCOL_36K_RC6) && (correctDecode == FALSE))
        {
            ir_payload.info.bits.protocol_type = IR_PROTOCOL_DECODE_ERROR;
        }    
        return ir_payload;
    
}


static void IRLowLevelDecoder_DecoderTask( void *pvParameters )
{
	IR_PROTOCOL_PAYLOAD ir_payload;
#if ENABLE_IR_DECODE_TASK_NEW	
    uint16 *ptr_ir_rowdata = ir_rowdata;
    uint16 tmpCnt = 0;
#endif
	
    for( ;; )
    {
    #if ENABLE_IR_DECODE_TASK_NEW
        if(ptr_ir_rowdata == ir_rowdata)
        {
            if((tmpCnt=pIR_LowLevel->get_row_data( ptr_ir_rowdata, IR_ROW_DATA_MAX, TRUE )) == 0)
            {
                continue;
            }
            ir_decode_lock();
            xIrDecoderTimeTick = xTaskGetTickCount( );
            ptr_ir_rowdata+=tmpCnt;
        }
        else
        {
            if(ptr_ir_rowdata >= ir_rowdata+IR_ROW_DATA_MAX)
            {
                ptr_ir_rowdata = ir_rowdata;
                continue;
            }
            tmpCnt=pIR_LowLevel->get_row_data( ptr_ir_rowdata, (uint16)(ir_rowdata+IR_ROW_DATA_MAX-ptr_ir_rowdata), FALSE );
            ptr_ir_rowdata+=tmpCnt;
            //if(tmpCnt > 0) /*RCU issue */
            //    xIrDecoderTimeTick = xTaskGetTickCount( );
        }
        if(( xTaskGetTickCount( ) - xIrDecoderTimeTick ) > 110) //Angus modify for decode SHARP RC success. 
        #else
        ir_queue_number = pIR_LowLevel->get_queue_number();
        if ( ir_queue_number > 0 )
        {
        	if( ( xTaskGetTickCount( ) - xIrDecoderTimeTick ) > 110 )
        #endif
        	{
        	#if !ENABLE_IR_DECODE_TASK_NEW
                        pIR_LowLevel->get_row_data( ir_rowdata );
             #else
                  ir_queue_number = (uint16)(ptr_ir_rowdata - ir_rowdata);
             #endif
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
                            #if ENABLE_IRCMD_TASK_NEW
                                pIRCmdHandle_ObjCtrl->insert_queue( 0, 0, &mIrPayload );
                            #else
                                pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );	
                            #endif
                            }
                        }

                        if (  ir_payload.info.bits.protocol_type == IR_PROTOCOL_NULL )
                        {
                        	ir_payload = IRLowLevelDecoder_SAMSUNG_Decode(ir_rowdata, ir_queue_number );
                        	if( ir_payload.info.bits.protocol_type == IR_PROTOCOL_38K_SAMSUNG )
                        	{
                        		mIrPayload.info.bits.repeat_type = ir_payload.info.bits.repeat_type;
                        		mIrPayload.info.bits.protocol_type = ir_payload.info.bits.protocol_type;						
                                if( ir_payload.info.bits.repeat_type != IR_PROTOCOL_38K_SAMSUNG_REPEAT )
                                {
                                    mIrPayload.payload = ir_payload.payload;
                                }
                            #if ENABLE_IRCMD_TASK_NEW
                                pIRCmdHandle_ObjCtrl->insert_queue( 0, 0, &mIrPayload );
                            #else
                        		pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );	
                            #endif
                        	}
                        }


                        if (  ir_payload.info.bits.protocol_type == IR_PROTOCOL_NULL )
                        {
                        	ir_payload = IRLowLevelDecoder_PANASONIC_Decode(ir_rowdata, ir_queue_number );
                        	if( ir_payload.info.bits.protocol_type == IR_PROTOCOL_38K_PANASONIC )
                        	{
                        		mIrPayload.info.bits.repeat_type = ir_payload.info.bits.repeat_type;
                        		mIrPayload.info.bits.protocol_type = ir_payload.info.bits.protocol_type;						
                        		mIrPayload.payload = ir_payload.payload;
                            #if ENABLE_IRCMD_TASK_NEW
                                pIRCmdHandle_ObjCtrl->insert_queue( 0, 0, &mIrPayload );
                            #else
                        		pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );	
                            #endif	
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
                            #if ENABLE_IRCMD_TASK_NEW
                                pIRCmdHandle_ObjCtrl->insert_queue( 0, 0, &mIrPayload );
                            #else
                                pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );	
                            #endif	
                            }
                        }

                        if (  ir_payload.info.bits.protocol_type == IR_PROTOCOL_NULL )
                        {
                            ir_payload = IRLowLevelDecoder_RC6_Decode(ir_rowdata, ir_queue_number );
                            if( ir_payload.info.bits.protocol_type == IR_PROTOCOL_36K_RC6 )
                            {
                                mIrPayload.info.bits.repeat_type = ir_payload.info.bits.repeat_type;
                                mIrPayload.info.bits.protocol_type = ir_payload.info.bits.protocol_type;                        
                                mIrPayload.payload = ir_payload.payload;
                            #if ENABLE_IRCMD_TASK_NEW
                                pIRCmdHandle_ObjCtrl->insert_queue( 0, 0, &mIrPayload );
                            #else
                                pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );   
                            #endif
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
                            #if ENABLE_IRCMD_TASK_NEW
                                pIRCmdHandle_ObjCtrl->insert_queue( 0, 0, &mIrPayload );
                            #else
                                pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );   
                            #endif
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
                            #if ENABLE_IRCMD_TASK_NEW
                                pIRCmdHandle_ObjCtrl->insert_queue( 0, 0, &mIrPayload );
                            #else
                                pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );   
                            #endif
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
                            #if ENABLE_IRCMD_TASK_NEW
                                pIRCmdHandle_ObjCtrl->insert_queue( 0, 0, &mIrPayload );
                            #else
                                pIRCmdHandle_ObjCtrl->insert_queue( mIrPayload );	
                            #endif
                            }
                        }

                        memset( ir_rowdata, 0, sizeof(ir_rowdata)/sizeof(uint8)); 
         #if !ENABLE_IR_DECODE_TASK_NEW
            xIrDecoderTimeTick = xTaskGetTickCount( );
         #else	
            ptr_ir_rowdata = ir_rowdata;
            ir_decode_free();
         #endif
        	}
#if !ENABLE_IR_DECODE_TASK_NEW
        }
        else
        {
        	xIrDecoderTimeTick = xTaskGetTickCount( );
        }

#endif		
		

#if !ENABLE_IR_DECODE_TASK_NEW
        vTaskDelay( 10 ); 
#else
        vTaskDelay( 1 );
#endif
    }
}

void IRLowLevelDecoder_CreateTask( void )
{
	mIrPayload.info.byte = 0;
	mIrPayload.payload= 0;
	memset( ir_rowdata, 0, IR_ROW_DATA_MAX );
	
    semaIrDecode = xSemaphoreCreateMutex();
    ASSERT(semaIrDecode!=NULL)
	
    if ( xTaskCreate( IRLowLevelDecoder_DecoderTask,
        ( portCHAR * ) "IR_Decoder",
         configMINIMAL_STACK_SIZE, NULL,
         tskHMI_EVENT_PRIORITY, NULL ) != pdPASS)
    {
    	TRACE_ERROR((0, " IRLowLevelDecoder_ServiceHandle task create error !! "));
    }
}

