#include "stm32f10x_conf.h"
#include "BackupAccessLowLevel.h"

#define BKP_DR_NUMBER 42

const uint16_t BKPDataReg[BKP_DR_NUMBER] =
{
    BKP_DR1, BKP_DR2, BKP_DR3, BKP_DR4, BKP_DR5, BKP_DR6, BKP_DR7, BKP_DR8,
    BKP_DR9, BKP_DR10, BKP_DR11, BKP_DR12, BKP_DR13, BKP_DR14, BKP_DR15, BKP_DR16,
    BKP_DR17, BKP_DR18, BKP_DR19, BKP_DR20, BKP_DR21, BKP_DR22, BKP_DR23, BKP_DR24,
    BKP_DR25, BKP_DR26, BKP_DR27, BKP_DR28, BKP_DR29, BKP_DR30, BKP_DR31, BKP_DR32,
    BKP_DR33, BKP_DR34, BKP_DR35, BKP_DR36, BKP_DR37, BKP_DR38, BKP_DR39, BKP_DR40,
    BKP_DR41, BKP_DR42
};

uint16 BackupAccessLowLevel_ReadBackupRegister(uint8 index)
{
    uint16 ret = 0;
#if ( configSTM32F411_PORTING == 1 )
    ret = BKP_ReadBackupRegister(BKPDataReg[index]);
#endif
    return ret;
}

void BackupAccessLowLevel_WriteBackupRegister(uint8 index, uint16 value)
{
    /* Enable write access to Backup domain */
    PWR_BackupAccessCmd(ENABLE);
#if ( configSTM32F411_PORTING == 1 )
    BKP_WriteBackupRegister(BKPDataReg[index], value);
#endif    
}

void BackupAccessLowLevel_Configuration(void)
{
#if ( configSTM32F411_PORTING == 1 )
    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); /*Smith mark; RCC_APB1Periph_BKP is for backup domin.*/

    /* Enable write access to Backup domain */
    PWR_BackupAccessCmd(ENABLE);
#endif
}

void BackupAccessLowLevel_resetBackupRegister(void)
{
#if ( configSTM32F411_PORTING == 1 )
    BKP_DeInit();
#endif
}

