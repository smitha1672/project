#ifndef __BACKUP_ACCRESS_LOW_LEVEL_H__
#define __BACKUP_ACCRESS_LOW_LEVEL_H__

#include "Defs.h"


uint16 BackupAccessLowLevel_ReadBackupRegister(uint8 index);

void BackupAccessLowLevel_WriteBackupRegister(uint8 index, uint16 value);

void BackupAccessLowLevel_Configuration(void);

void BackupAccessLowLevel_resetBackupRegister(void);

#endif /*__BACKUP_ACCRESS_LOW_LEVEL_H__*/

