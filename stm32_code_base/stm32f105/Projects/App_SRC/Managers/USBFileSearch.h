#ifndef __USB_FILE_SEARCH_H__
#define __USB_FILE_SEARCH_H__

#include <string.h>
#include <stdio.h>
#include "Defs.h"
#include "api_typedef.h"

#include "ff.h"       /* FATFS */

#define FS_MAX_PATH_LENGTH 64
#define FS_MAX_FOLDER_NUMBER 16
#define FS_MAX_FILE_NUMBER 32//128


typedef enum
{
    FSE_Valid_WAVE_File = 0,
    FSE_Unvalid_RIFF_ID,
    FSE_Unvalid_WAVE_Format,
    FSE_Unvalid_FormatChunk_ID,
    FSE_Unsupporetd_FormatTag,
    FSE_Unsupporetd_Number_Of_Channel,
    FSE_Unsupporetd_Sample_Rate,
    FSE_Unsupporetd_Bits_Per_Sample,
    FSE_Unvalid_DataChunk_ID,
    FSE_Unsupporetd_ExtraFormatBytes,
    FSE_Unvalid_FactChunk_ID
} FSE_FormatCheck;

typedef struct
{
  char   filePath[FS_MAX_PATH_LENGTH];
  uint32_t  RIFFchunksize;
  uint16_t  FormatTag;
  uint16_t  NumChannels;
  uint32_t  SampleRate;
  uint32_t  ByteRate;
  uint16_t  BlockAlign;
  uint16_t  BitsPerSample;
  uint32_t  DataSize;
  //uint32_t  ChunkSize;
  uint32_t  Seeker;
}FSE_WaveFormat;

typedef struct
{
    //uint8_t filePath[FS_MAX_FILE_NUMBER][FS_MAX_PATH_LENGTH];
    FSE_WaveFormat fileData[FS_MAX_FILE_NUMBER];
    uint32_t fileNumber;
    
}FSE_FileList;

typedef struct _USB_FILE_SEARCH
{
    void (*init)(void);  
    void (*fileSearchTask)(void);
    unsigned int (*getWaveFileNum)(void);
    FSE_FormatCheck (*formatCheck)(char* filePath, FSE_WaveFormat* fileFormat,uint8* headerBuf,int IsChannelCheck);
    bool (*isSearchDone)(void);
}USB_FILE_SEARCH_OBJECT;


#endif

