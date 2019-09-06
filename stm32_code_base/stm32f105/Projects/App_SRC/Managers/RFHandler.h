#ifndef __RF_HANDLER_H__
#define __RF_HANDLER_H__


typedef enum{
    RF_HANDLE_PAIRING_PHASE1,
    RF_HANDLE_PAIRING_PHASE2,
    RF_HANDLE_CHECK,
}xRFHandleCommand;

typedef struct _RF_HANDLE
{
    void (*CreateTask)(void);  
    bool (*rf_instr_sender)(void* cmds);
}RF_HANDLE_OBJECT;

#endif

