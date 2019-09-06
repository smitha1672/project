#include "USBFileSearch.h"
#include "USBMusicManager.h"
#include "USBMediaManager.h"

extern USB_MUSIC_MANAGE_OBJECT *pUSB_MMP_ObjCtrl;
extern USB_MEDIA_MANAGE_OBJECT *pUSBMedia_ObjCtrl;

/*_________________________________________________________________________________________________________*/
// MARCO
//#define WAVE_MONO
//#define WAVE_BITS_PER_SAMPLE_8

/*_______________________________________________________________________________________________________*/

typedef enum
{
    LittleEndian,
    BigEndian
}Endianness;

typedef enum
{
    FSE_STATE_RESET,
    FSE_STATE_SEARCH,
    FSE_STATE_DONE,        
}FSE_State;

typedef struct
{
    bool bFolderOpen;
    DIR dir;
    uint8_t folderIdx;
    
} FSE_Control;


typedef struct
{
    char folderPath[FS_MAX_FOLDER_NUMBER][FS_MAX_PATH_LENGTH];
    uint32_t folderNumber;
} FSE_FolderList;


//____________________________________________________________________________________________________________
//static api header
static void FSE_Init(void);
static void FSE_StateDispatcher(void);
static unsigned int FSE_Get_File_Num(void);
static FSE_FormatCheck FSE_Is_Valid_Wav_File( char* filePath, FSE_WaveFormat* fileFormat,uint8* headerBuf,int IsChannelCheck);
static bool FSE_File_Search(void);
static bool FSE_Is_EndOfSearch(void);

/*_______________________________________________________________________________________________________*/
static FSE_Control gEngineControl;
static FSE_FolderList gFolderList;
static FSE_FileList gFileList;


FSE_State mFSE_state = FSE_STATE_RESET;



const USB_FILE_SEARCH_OBJECT USBFileSearch = 
{
    FSE_Init,
    FSE_StateDispatcher,
    FSE_Get_File_Num,
    FSE_Is_Valid_Wav_File,
    FSE_Is_EndOfSearch
};

const USB_FILE_SEARCH_OBJECT *pUSBFSE_Object = &USBFileSearch;


static void FSE_Init(void)
{
    MEMSET(&gEngineControl, 0, sizeof(FSE_Control));
    MEMSET(&gFolderList, 0, sizeof(FSE_FolderList));
    MEMSET(&gFileList, 0, sizeof(FSE_FileList));
    mFSE_state = FSE_STATE_RESET;

     pUSB_MMP_ObjCtrl->assign_usb_filelist(&gFileList);
     pUSBMedia_ObjCtrl->assign_usb_filelist(&gFileList);
}

static void FSE_StateDispatcher(void)
{
    bool result;
    
	switch (mFSE_state)
    {   
        case FSE_STATE_RESET:
        {
            MEMSET(&gEngineControl, 0, sizeof(FSE_Control));
            MEMSET(&gFolderList, 0, sizeof(FSE_FolderList));
            MEMSET(&gFileList, 0, sizeof(FSE_FileList));

            mFSE_state = FSE_STATE_SEARCH;
        }
        break;

        case FSE_STATE_SEARCH:
        {
            result = FSE_File_Search();
                
            if (result == FALSE)
            {
                mFSE_state = FSE_STATE_DONE;
            }
        }
        break;
        
        case FSE_STATE_DONE:
        {
            
        }
        break;

        default:
        break;
    }  
}

static unsigned int FSE_Get_File_Num(void)
{
    return gFileList.fileNumber;
}

static bool FSE_Is_EndOfSearch(void)
{
    if(mFSE_state == FSE_STATE_DONE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static bool FSE_Is_Wav_File(char* filePath)
{
    uint32_t pathLen = 0;
    
    pathLen = strlen(filePath);

    if ((filePath[pathLen-3] == 'w' || filePath[pathLen-3] == 'W') &&
        (filePath[pathLen-2] == 'a' || filePath[pathLen-2] == 'A') &&
        (filePath[pathLen-1] == 'v' || filePath[pathLen-1] == 'V'))
        return TRUE;
    else
        return FALSE;
        
}

//Return value, 1:search done, 0: searching 
static bool FSE_File_Search(void)
{
    uint8_t i = 0;
    FRESULT res = FR_OK;
    FILINFO fno;
    bool bEndDir = FALSE;
    char path[FS_MAX_PATH_LENGTH] = {0};
    FSE_FormatCheck check; 


	char* fn;
#if _USE_LFN
	static char lfn[256];
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
#endif

	
    if (gEngineControl.bFolderOpen == FALSE)
    {
        if (gEngineControl.folderIdx == 0) 
        {
            sprintf(gFolderList.folderPath[0], "0:");
            gFolderList.folderNumber++ ;
        }
        
        res = f_opendir(&(gEngineControl.dir),  gFolderList.folderPath[gEngineControl.folderIdx]);
        if (res == FR_OK)
        {
            gEngineControl.bFolderOpen = TRUE;
        }
        else
            return FALSE;
    }


    

    for (i = 0;i < 5;i++)
    {
        //MEMSET(path, 0, FS_MAX_PATH_LENGTH);
    
        res = f_readdir(&(gEngineControl.dir), &fno);
        
        if (res != FR_OK) //read error
            return FALSE;

        if (fno.fname[0] == 0) //end of dir
        {
            bEndDir = TRUE;
            break;
        }
        else
            bEndDir = FALSE;


        if (fno.fname[0] == '.') continue;     // Ignore dot entry

#if _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno.fname;
#endif

		

        if (fno.fattrib & AM_DIR)              // It is a directory 
        {
            if (gFolderList.folderNumber >=  FS_MAX_FOLDER_NUMBER)
                continue;
            
            if (gEngineControl.folderIdx == 0)
            {
                sprintf(gFolderList.folderPath[gFolderList.folderNumber], 
                        "%s%s",
                        "0:/",
                       fn ); //fno.fname
            }
            else
            {
                sprintf(gFolderList.folderPath[gFolderList.folderNumber], 
                        "%s/%s",
                        gFolderList.folderPath[gEngineControl.folderIdx],
                       fn ); //fno.fname
            }	
            gFolderList.folderNumber++;
		
        } 
        else   // It is a file.                
        {
            if (FSE_Is_Wav_File(fno.fname) == FALSE) //Not wav file
                continue;

            if (gEngineControl.folderIdx == 0)
            {
                sprintf(path, "%s%s", "0:",fn ); //fno.fname
            }
            else
            {
                sprintf(path, "%s/%s", gFolderList.folderPath[gEngineControl.folderIdx], fn ); //fno.fname
            }

            check = FSE_Is_Valid_Wav_File(path, &(gFileList.fileData[gFileList.fileNumber]),NULL,0);
        
            if (check != FSE_Valid_WAVE_File)
               continue;
#if 0
            if (gEngineControl.folderIdx == 0)
            {
                //sprintf(gFileList.filePath[gFileList.fileNumber], 
                  sprintf(gFileList.fileData[gFileList.fileNumber].filePath,   
                        "%s%s",
                        "0:/",
                        fno.fname);
            }
            else
            {
                //sprintf(gFileList.filePath[gFileList.fileNumber], 
                sprintf(gFileList.fileData[gFileList.fileNumber].filePath, 
                        "%s/%s",
                        gFolderList.folderPath[gEngineControl.folderIdx],
                        fno.fname);
            }
#endif

            gFileList.fileNumber++;

            if (gFileList.fileNumber >= FS_MAX_FILE_NUMBER)
                return FALSE;
        }
    }
    
    if (bEndDir) //End of dir
    {
        if (gEngineControl.folderIdx + 1 >= gFolderList.folderNumber) //All folder is searched
            return FALSE;
        gEngineControl.folderIdx++;
        gEngineControl.bFolderOpen = FALSE;
    }
    else
    {
        gEngineControl.bFolderOpen = TRUE;
    }


    return TRUE;
}







static uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat)
{
  uint32_t index = 0;
  uint32_t Temp = 0;
  
  for (index = 0; index < NbrOfBytes; index++)
  {
    Temp |= buffer[idx + index] << (index * 8);
  }
  
  if (BytesFormat == BigEndian)
  {
    Temp = __REV(Temp);
  }
  return Temp;
}



static FSE_FormatCheck FSE_Is_Valid_Wav_File(  char* filePath, FSE_WaveFormat* fileFormat,uint8 headerBuf[],int IsChannelCheck)
{
	uint32_t Temp = 0x00;
	uint32_t ExtraFormatBytes = 0;
	uint8_t	Subchunk1Size;
	FRESULT res;
	uint8_t buff[48];
	static FIL filFile;
    
    //FIL filFile;
    __IO uint32_t readSize = 0;
    FSE_WaveFormat format;
    __IO uint32_t SpeechOffset = 0x00;

    /* It need check wave header from FLASH if running channel check  (Angus 2013/12/25) */
    if(IsChannelCheck == 0) 
    {
        res = f_open(&filFile, filePath, FA_OPEN_EXISTING | FA_READ);
        if( res != FR_OK)
    		return FSE_Unvalid_WAVE_Format;

    	res = f_read(&filFile, (byte *)&buff, 48, (void *)&readSize );
    	if( res != FR_OK)
    		return FSE_Unvalid_WAVE_Format;
        else
            f_close(&filFile);
    }
    else
    {   
        MEMCPY((byte *)&buff, (byte *)headerBuf, 48);
    }

	/* Read chunkID, must be 'RIFF'*/
	Temp = ReadUnit(buff, 0, 4, BigEndian);

	if (Temp != CHUNK_ID)
	{
		return(FSE_Unvalid_RIFF_ID);
	}

	/* Read the file length */
	format.RIFFchunksize = ReadUnit(buff, 4, 4, LittleEndian);

	/* Read the file format, must be 'WAVE'*/
	Temp = ReadUnit(buff, 8, 4, BigEndian);
	if (Temp != FILE_FORMAT)
	{
		return(FSE_Unvalid_WAVE_Format);
	}

	/* Read the format chunk, must be'fmt '; Subchunk1ID*/
	Temp = ReadUnit(buff, 12, 4, BigEndian);
	if (Temp != FORMAT_ID)
	{
		return(FSE_Unvalid_FormatChunk_ID);
	}
	
	/* Read the length of the 'fmt' data, must be 0x10; Subchunk1Size */
	Temp = ReadUnit(buff, 16, 4, LittleEndian);
	Subchunk1Size = Temp; /*The varialbe should be followed ReadUnit after Subchunk1Size was be got; Smith*/ 
	if (Temp != 0x10)
	{
		ExtraFormatBytes = 1;
	}
	
	/* Read the audio format, must be 0x01 (PCM) */
	format.FormatTag = ReadUnit(buff, 20, 2, LittleEndian);
	Subchunk1Size -= 2; /*The varialbe should be followed ReadUnit after Subchunk1Size was be got; Smith*/
	if (format.FormatTag != WAVE_FORMAT_PCM)
	{
		return(FSE_Unsupporetd_FormatTag);
	}

	/* Read the number of channels, must be 0x01 (Mono)*/
	format.NumChannels = ReadUnit(buff, 22, 2, LittleEndian);
	Subchunk1Size -= 2; /*The varialbe should be followed ReadUnit after Subchunk1Size was be got; Smith*/

	/*Smith: mono channel is not good working 2013/5/31*/
#if defined ( WAVE_MONO )
	if (format.NumChannels != CHANNEL_MONO && format.NumChannels != CHANNEL_STEREO)
	{
		return(FSE_Unsupporetd_Number_Of_Channel);
	}
#else 
	if ( format.NumChannels != CHANNEL_STEREO )
	{
            if(!IsChannelCheck)	/* Don't care that in Noise check (Angus) */
                return(FSE_Unsupporetd_Number_Of_Channel); 
        }

#endif 

	/* Read the Sample Rate*/
	format.SampleRate = ReadUnit(buff, 24, 4, LittleEndian);
	Subchunk1Size -= 4; /*The varialbe should be followed ReadUnit after Subchunk1Size was be got; Smith*/

	/* Update the OCA value according to the .WAV file Sample Rate */
	
	switch (format.SampleRate)
	{
		case SAMPLE_RATE_8000 :
		case SAMPLE_RATE_11025:
		case SAMPLE_RATE_16000:
		case SAMPLE_RATE_22050:
		case SAMPLE_RATE_32000:
		case SAMPLE_RATE_44100:
		case SAMPLE_RATE_48000:
			break;
		default:
			return(FSE_Unsupporetd_Sample_Rate);
	}
	
	/* Read the Byte Rate*/
	format.ByteRate = ReadUnit(buff, 28, 4, LittleEndian);
	Subchunk1Size -= 4; /*The varialbe should be followed ReadUnit after Subchunk1Size was be got; Smith*/

	/* Read the block alignment*/
	format.BlockAlign = ReadUnit(buff, 32, 2, LittleEndian);
	Subchunk1Size -= 2; /*The varialbe should be followed ReadUnit after Subchunk1Size was be got; Smith*/

	/* Read the number of bits per sample*/
	format.BitsPerSample = ReadUnit(buff, 34, 2, LittleEndian);
	Subchunk1Size -= 2; /*The varialbe should be followed ReadUnit after Subchunk1Size was be got; Smith*/

	switch( format.BitsPerSample )
	{
#if defined ( WAVE_BITS_PER_SAMPLE_8 )	
		case BITS_PER_SAMPLE_8:
#endif 			
		case BITS_PER_SAMPLE_16:
			break;

		default:
			return(FSE_Unsupporetd_Bits_Per_Sample);
	}

	SpeechOffset = 36;
    
	/* If there is Extra format bytes, these bytes will be defined in "Fact Chunk" */
	if (ExtraFormatBytes == 1)
	{
		if ( Subchunk1Size )
		{
			/* Read th Extra format bytes, must be 0x00 */
			Temp = ReadUnit(buff, 36, 2, LittleEndian);
			Subchunk1Size -= 2; /*The varialbe should be followed ReadUnit after Subchunk1Size was be got; Smith*/
			if (Temp != 0x00)
			{
				return(FSE_Unsupporetd_ExtraFormatBytes);
			}

			SpeechOffset += 2;
		}

		if ( Subchunk1Size )
		{
			/* Read the Fact chunk, must be 'fact'*/
			Temp = ReadUnit(buff, 38, 4, BigEndian);
			Subchunk1Size -= 4; /*The varialbe should be followed ReadUnit after Subchunk1Size was be got; Smith*/
			if (Temp != FACT_ID)
			{
				return(FSE_Unvalid_FactChunk_ID);
			}

			SpeechOffset += 4;

		}

		if ( Subchunk1Size )
		{
			/* Read Fact chunk data Size*/
			Temp = ReadUnit(buff, 42, 4, LittleEndian);
			Subchunk1Size -= 4; /*The varialbe should be followed ReadUnit after Subchunk1Size was be got; Smith*/
			SpeechOffset += 4 + Temp;
		}
	}
	
	/* Read the Data chunk, must be 'data'*/
	Temp = ReadUnit(buff, SpeechOffset, 4, BigEndian);
	SpeechOffset += 4;
    
	if (Temp != DATA_ID)
	{
		return(FSE_Unvalid_DataChunk_ID);
	}

	/* Read the number of sample data */
	format.DataSize = ReadUnit(buff, SpeechOffset, 4, LittleEndian);

	/*audio raw data start position: Refer as "https://ccrma.stanford.edu/courses/422/projects/WaveFormat/" */
	SpeechOffset += 4; 

	//format.ChunkSize = (format.DataSize/RAM_BUFFER_SIZE);
	format.Seeker = SpeechOffset;


    MEMCPY(fileFormat, &format, sizeof(FSE_WaveFormat));
    sprintf(fileFormat->filePath, "%s", filePath);

#if 0
	f_lseek(&filefatfs, AudioRawData_Indicator.Seeker );

	res = f_read(&filefatfs, (byte *)&RAM_Buf, 6/*RAM_BUFFER_SIZE*/, (void *)&read_size );
	if ( res == FR_OK )
	{
		AudioRawData_Indicator.Seeker = (AudioRawData_Indicator.Seeker + read_size + 1);
		res = f_read(&filefatfs, (byte *)&RAM_Buf, 6/*RAM_BUFFER_SIZE*/, (void *)&read_size );
	}
#endif 	

	return(FSE_Valid_WAVE_File);

}


