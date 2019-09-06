#ifndef __AMT_CHANNEL_CHECK_DISPATCHER_H__
#define __AMT_CHANNEL_CHECK_DISPATCHER_H__

#include "Defs.h"

/************************ Channel Check ************************/
#define TotalCheckFile  6
#define WaveHeaderSize 50

#define Index_L_StartAddr     0
#define Index_R_StartAddr     1
#define Index_C_StartAddr     2       
#define Index_Ls_StartAddr    3    
#define Index_Rs_StartAddr    4
#define Index_W_StartAddr     5


#define BYTE_OF_ChunkID         4
#define BYTE_OF_ChunkSize      4

/* The sequence is followed with Vizio UI Spec.(Angus 2014/1/7) */
typedef enum{
    NOISE_OUTPUT_NULL = 0,
    NOISE_OUTPUT_LEFT_CHANNEL,
    NOISE_OUTPUT_CENTER_CHANNEL,
    NOISE_OUTPUT_RIGHT_CHANNEL,
    NOISE_OUTPUT_RIGHT_REAR_CHANNEL,
    NOISE_OUTPUT_LEFT_REAR_CHANNEL,
    NOISE_OUTPUT_SUBWOOFER_CHANNEL
}NoiseOutputChannel;


typedef enum{
    ASSIGN_EVENT_IDLE = 0,
    ASSIGN_INIT_NOISE,
    ASSIGN_START_NOISE,
    ASSIGN_PLAY_NOISE,
    ASSIGN_STOP_NOISE,
    ASSIGN_END_NOISE
}NOISE_GENERATOR_CHECK_EVENT_ASSIGN;

void NoiseGeneratorDispatcher_EventAssign(NOISE_GENERATOR_CHECK_EVENT_ASSIGN event);
void NoiseGeneratorDispatcher_poll(void);

void getNoiseGeneratorMemAddr(void);
uint32 getNoiseGeneratorStartAddr(void);

void NoiseGenerator_SetMute(bool state);

#endif

