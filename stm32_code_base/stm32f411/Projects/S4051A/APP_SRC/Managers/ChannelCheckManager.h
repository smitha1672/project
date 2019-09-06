#ifndef __CHANNEL_CHECK_MANAGER_H__
#define __CHANNEL_CHECK_MANAGER_H__

typedef struct _CCK_MANAGE
{
    //void (*init)(void);  
    void (*CreateTask)(void);  
    void (*GetNoiseVersionNum)(uint8 * vNum);
    //bool (*instr_sender)( void* cmds);
    uint32 (*Get_NoiseFile_Start_Addr)(void);
    void (*start)(const void * params);
    void (*stop)(const void * params);
}CCK_MANAGE_OBJECT;

#define BYTE_OF_VERSION     6


#endif

