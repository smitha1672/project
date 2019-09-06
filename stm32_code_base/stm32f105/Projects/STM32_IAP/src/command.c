/**
  ******************************************************************************
  * @file    command.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22/07/2011
  * @brief   This file provides all the IAP command functions.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "command.h"
#include "flash_layer.h"
#include "usbh_core.h"
#include "ff.h"       /* FATFS */

//! Smith implemets @{
#include "HT68F30.h"
#include "ext_flash_driver.h"
#include "SYSTEM_DEFAULT_PARMS.h"
//@}

typedef enum {
	IAP_STATE_PROJECT_ID_IDENTIFY = 0, 
	IAP_STATE_FLASH_PROGRAM,
	IAP_STATE_EXT_FLASH_PROGRAM,
	IAP_STATE_PROGRAM_FAIL, 
	IAP_STATE_PROGRAM_COMPLETE,
}IAP_State;  

// AX: A1: main board includes OTI3300; A0: main board does not include OTI3300
#define ST32F105_100_UPGRAGE_FILE "0:fwupdate.bin"

#if defined ( S3851A )
const uint8_t PID[PES_PID_LEN] = "S3851A";
#elif defined ( S4051A )
const uint8_t PID[PES_PID_LEN] = "S4051A";
#elif defined ( S4051B )
const uint8_t PID[PES_PID_LEN] = "S4051B";
#elif defined ( S3851C )
const uint8_t PID[PES_PID_LEN] = "S3851C";
#elif defined ( S3851D )
const uint8_t PID[PES_PID_LEN] = "S3851D";
#elif defined ( S3851E )
const uint8_t PID[PES_PID_LEN] = "S3851E";
#elif defined ( S3851F )
const uint8_t PID[PES_PID_LEN] = "S3851F";
#elif defined ( S4551A )
const uint8_t PID[PES_PID_LEN] = "S4551A";

#endif
//______________________________________________________________________
#define BOOT_LOADER_VERSION "1.0.0" 
//______________________________________________________________________
extern FATFS fatfs;
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;   //Jerry add
extern uint8_t CMD_index;
extern uint8_t joystick_use;
static uint8_t RAM_Buf[PAGE_SIZE] = {0xFF};

FIL filefatfs;
PES *pPES;

uint8_t mPES[20] = {0};
uint32_t mSeekPointer = 0;
uint32 mPayloadSize = 0;
uint8_t mCheckSum = 0;
uint8_t calCheckSum=0;
uint32_t mDebugTrace = 0;
uint8_t mGroupID = 0xFF;
uint8_t mModelIndex = 0xFF;

//static uint32_t tmp_program_counter = 0x00, tmp_read_size = 0x00 , ram_address = 0x00;
static uint32_t Last_PGAddress = APPLICATIONADDRESS;
uint8_t  bUpdateExtFlash = FALSE;
static uint32_t mReadBinSize = 0;


/* Private functions ---------------------------------------------------------*/
FRESULT COMMAND_isCorrectPES(void);
FRESULT COMMAND_checkProjectID(void);
FRESULT COMMAND_ProgramFlash(void);

//________________________________________________________________________________________
/**
  * @brief IAP Read all flash memory
  * @param  None
  * @retval None
  */
void COMMAND_UPLOAD(void)
{
	
	//! The feature is unnecessary	Smith mark@{
#if 0
  __IO uint32_t address = FLASH_STARTADDRESS;
  __IO uint32_t counter_read = 0x00;
  uint32_t tmp_counter = 0x00, index_offset = 0x00;
  FlagStatus readoutstatus = SET;

  /* Get the read out protection status */
  readoutstatus = FLASH_LAYER_ReadOutProtectionStatus();

  if (readoutstatus == RESET)
  {
    /* Remove UPLOAD file if exist on flash disk */
    rmfile(&efs.myFs, "UPLOAD.BIN");

    /* Init written byte counter */
    index_offset = 0x00;

    /* Open binary file to write on it */
    if ((HCD_IsDeviceConnected(&USB_OTG_FS_dev) == 1) && (file_fopen(&file, &efs.myFs, "UPLOAD.BIN", 'w') == 0))
    {
      /* Clear LCD msg */
      USBH_USR_LCDClearPart(90, 90);

      /* Display LCD message */
      LCD_DisplayStringLine(80, (uint8_t *)MSG_UPLOAD_GOING);

      /* Read flash memory */
      while ((index_offset != FLASH_SIZE) && (HCD_IsDeviceConnected(&USB_OTG_FS_dev) == 1))
      {
        for (counter_read = 0; counter_read < BUFFER_SIZE; counter_read++)
        {
          /* Check the read bytes versus the end of flash */
          if (index_offset + counter_read != FLASH_SIZE)
          {
            tmp_counter = counter_read;
            RAM_Buf[tmp_counter] = (*(uint8_t*)(address++));
          }
          /* In this case all flash was read */
          else
          {
            break;
          }
        }

        /* Write buffer to file */
        file_write(&file, counter_read, RAM_Buf);

        /* Number of byte written  */
        index_offset = index_offset + counter_read;
      }

      /* Display LCD message */
      LCD_DisplayStringLine(80, (uint8_t *)MSG_UPLOAD_DONE);

      /* Close file and filesystem */
      file_fclose(&file);
      fs_umount(&efs.myFs);
    }
  }
  else
  {
    /* Display LCD message ROP active */
    LCD_DisplayStringLine(80, (uint8_t *)MSG_ROP1);
    LCD_DisplayStringLine(90, (uint8_t *)MSG_ROP2);
  }
#endif
	//!	@}
  
}

/**
  * @brief  IAP write memory
  * @param  None
  * @retval None
  */
void COMMAND_DOWNLOAD(void)
{
#if 0/*Smith mark for firmware upgrading identify 2013/3/22  
	__IO uint8_t index_struct = 0x00, status = NEXT_MENU;
	__IO uint32_t erase_address = 0x00;

	uint8_t BlinkingCnt = 0;
	bool bInvert = FALSE;
    
    FRESULT res;
    FILINFO fno;
    DIR dir;
	
	res = f_opendir(&dir, "0:/");
	if (res == FR_OK) 
	{
	    while(HCD_IsDeviceConnected(&USB_OTG_Core)) 
	    {
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0) 
			{
				//Can not open directory
				while ( BlinkingCnt <= 20)
				{
					bInvert = ! bInvert;

					HT68F30_LightBar_USB_StateError( bInvert );

					/* 0.1 seconds delay */
					USB_OTG_BSP_mDelay(100);

					BlinkingCnt ++;
				}
				HT68F30_LightBar_USB_StateError( TRUE );
				BlinkingCnt = 0;
				break;
			}
	    }
	}

      /* Register work area for logical drives */
    f_mount(0, &fatfs); /*The api has been call on last layer*/
    if(f_open(&filefatfs, "0:upgrade.bin",FA_OPEN_EXISTING | FA_READ) == FR_OK)
    { 
		HT68F30_LightBar_USB_StateError( FALSE );	/*Upgrade.bin has been */

		//if (fileR.FileSize > FLASH_SIZE - IAP_SIZE)
		if (filefatfs.fsize > FLASH_SIZE - IAP_SIZE)
		{
			/*Flash size error !!*/

		}
		else
        {
			//! MCU is upgrading @{
			HT68F30_LightBar_USB_isUpgrading( );
			//! @}

            /* Erase necessary page to download image */
            for ((erase_address = APPLICATIONADDRESS); ((erase_address < (APPLICATIONADDRESS + filefatfs.fsize)) && (erase_address < FLASH_ENDADDRESS));(erase_address = erase_address + PAGE_SIZE))
            {
                if (FLASH_LAYER_ErasePage (erase_address) != FLASH_COMPLETE)
                {
                    while ( BlinkingCnt <= 20)
                    {
                        bInvert = ! bInvert;

                        HT68F30_LightBar_USB_StateError( bInvert );

                        /* 0.1 seconds delay */
                        USB_OTG_BSP_mDelay(10);

                        BlinkingCnt ++;
                    }

                    HT68F30_LightBar_USB_StateError( FALSE );
                    BlinkingCnt = 0;

                    while (1);
                }
            }

            /* Program flash memory */
            COMMAND_ProgramFlashMemory();

            //! MCU upgrading has finished @{
            HT68F30_LightBar_USB_UpgradingDone( );
            USB_OTG_BSP_mDelay(100);
            //! @}

            /* Close file and filesystem */
            //file_fclose(&fileR);
            f_close(&filefatfs);
            f_mount(0, NULL); 

            /*Application upgrading has been done; Sof treset*/
            COMMAND_JUMP();

      }

    }
    else
    {
			/* if upgrade.bin cannot be found, here is running; Smith*/
			while ( BlinkingCnt <= 20)
			{
				bInvert = ! bInvert;
			
				HT68F30_LightBar_USB_StateError( bInvert );
			
				/* 0.1 seconds delay */
				USB_OTG_BSP_mDelay(100);

				BlinkingCnt ++;
			}

			HT68F30_LightBar_USB_StateError( FALSE );
			BlinkingCnt = 0;
			if (Check_PowerKey_status() == 1)
			{
				//JUMP_APP();	//This is Old FW upgrade behavior nothing to do.
			}
			else
			{
				COMMAND_JUMP();
			}
			
    }
#endif 

#if 0
	/* Get the read out protection status */
	if (ls_openDir(&list, &(efs.myFs), "/") != 0)
	{
		//Can not open directory
		while ( BlinkingCnt <= 20)
		{
			bInvert = ! bInvert;
		
			HT68F30_LightBar_USB_StateError( bInvert );
		
			/* 0.1 seconds delay */
			USB_OTG_BSP_mDelay(100);

			BlinkingCnt ++;
		}
		HT68F30_LightBar_USB_StateError( TRUE );
		BlinkingCnt = 0;
	}
	else
	{
		//!	Program flash	by Smith@{
		/* Open the binary file to be downloaded */
        if (file_fopen(&fileR, &efs.myFs, (char*)&FileName[0], 'r') == 0)
        {
			HT68F30_LightBar_USB_StateError( FALSE );	/*Upgrade.bin has been */

		
			if (fileR.FileSize > FLASH_SIZE - IAP_SIZE)
			{
				/*Flash size error !!*/
			
			}
			else
			{
				//! MCU is upgrading @{
				HT68F30_LightBar_USB_isUpgrading( );
				//! @}
			
	            /* Erase necessary page to download image */
	            for (erase_address = APPLICATIONADDRESS; (erase_address < (APPLICATIONADDRESS + fileR.FileSize)) && (erase_address < FLASH_ENDADDRESS); \
	                 erase_address = erase_address + PAGE_SIZE)
	            {
					if (FLASH_LAYER_ErasePage (erase_address) != FLASH_COMPLETE)
					{
						while ( BlinkingCnt <= 20)
						{
							bInvert = ! bInvert;

							HT68F30_LightBar_USB_StateError( bInvert );

							/* 0.1 seconds delay */
							USB_OTG_BSP_mDelay(100);

							BlinkingCnt ++;
						}

						HT68F30_LightBar_USB_StateError( FALSE );
						BlinkingCnt = 0;
						
						while (1);
					}
	            }

	            /* Program flash memory */
	            COMMAND_ProgramFlashMemory();

				//! MCU upgrading has finished @{
				HT68F30_LightBar_USB_UpgradingDone( );
				USB_OTG_BSP_mDelay(1000);
				//! @}

	            /* Close file and filesystem */
	            file_fclose(&fileR);

				/*Application upgrading has been done; Sof treset*/
				COMMAND_JUMP();
          }
			
		  
        }
		else
		{
			/* if upgrade.bin cannot be found, here is running; Smith*/
			while ( BlinkingCnt <= 20)
			{
				bInvert = ! bInvert;
			
				HT68F30_LightBar_USB_StateError( bInvert );
			
				/* 0.1 seconds delay */
				USB_OTG_BSP_mDelay(100);

				BlinkingCnt ++;
			}

			HT68F30_LightBar_USB_StateError( FALSE );
			BlinkingCnt = 0;
		}
		//!	@}
	}
    #endif
}

FRESULT COMMAND_ProgramExtFlashMemory( void )
{
    uint32_t address = 0;
    uint32_t read_size = 0x00;
    uint32_t page_num = 0;
   	uint32_t insufficient_page = 0;
	uint32_t i = 0;
    FRESULT res;
    uint32_t extflashPaySize = 0;
    
    
    if ( ( mPayloadSize - (FLASH_SIZE-IAP_SIZE) ) > EXT_FLASH_ENDADDRESS )
    {
        return FR_INVALID_OBJECT; 
    }

    res = f_open(&filefatfs, ST32F105_100_UPGRAGE_FILE, FA_OPEN_EXISTING | FA_READ);
	if ( res != FR_OK )
		return FR_DISK_ERR;
    

    extflashPaySize = mPayloadSize - (FLASH_SIZE-IAP_SIZE);

     /*chip earse*/
    sFLASH_EraseChip( );
    HT68F30_LightBar_EraseExtFlash( );

    mSeekPointer = (mSeekPointer+(FLASH_SIZE-IAP_SIZE));
    mDebugTrace = mSeekPointer;

    res = f_lseek(&filefatfs, mSeekPointer);
    if ( res != FR_OK )
    {
        return FR_DISK_ERR; 
    }


	page_num = (extflashPaySize/PAGE_SIZE);
	mDebugTrace = page_num;
	if ( page_num != 0 )
	{
		for ( i = 0; i < page_num; i++ )
		{
			res = f_read(&filefatfs, RAM_Buf, PAGE_SIZE, (void *)&read_size);
			if((read_size != 0) && (res == FR_OK)) 
			{
                sFLASH_WriteBuffer(RAM_Buf, &address, read_size );
			}
           
		}

	}

	insufficient_page = (extflashPaySize%PAGE_SIZE);
	mDebugTrace = insufficient_page;
	if ( insufficient_page != 0 )
	{
		res = f_read(&filefatfs, RAM_Buf, insufficient_page, (void *)&read_size);
		mDebugTrace = read_size;
		if((read_size != 0) && (res == FR_OK)) 
		{
            sFLASH_WriteBuffer(RAM_Buf, &address, read_size );
		}
	}

    HT68F30_LightBar_ProgramExtFlash( );

	return FR_OK; 

}



/**
  * @brief  IAP jump to user program
  * @param  None
  * @retval None
  */
void COMMAND_JUMP(void)
{
	/* Software reset */
	NVIC_SystemReset();
}

/**
  * @brief  COMMAND_ProgramFlashMemory
  * @param  None
  * @retval None
  */
FRESULT COMMAND_ProgramFlashMemory(void)
{

#if 0 /*debug*/
	uint32_t read_size = 0x00;
	uint32_t page_num = 0;
	uint32_t insufficient_page = 0;
	uint32_t i = 0;

	FRESULT res;
	FLASH_Status flash_status = FLASH_COMPLETE;

	res = f_lseek(&filefatfs, mSeekPointer);

	if ( res != FR_OK )
		return FR_DISK_ERR; 

  
	/* Erase address init */
	Last_PGAddress = APPLICATIONADDRESS;


	page_num = mPayloadSize/PAGE_SIZE;
	mDebugTrace = page_num;
	if ( page_num != 0 )
	{
		for ( i = 0; i < page_num; i++ )
		{
			res = f_read(&filefatfs, RAM_Buf, PAGE_SIZE, (void *)&read_size);
			if((read_size != 0) && (res == FR_OK)) 
			{
				flash_status = FLASH_LAYER_write_n_word(&Last_PGAddress, RAM_Buf, read_size);
				if ( flash_status != FLASH_COMPLETE )
				{
					while (1);
					return FR_DISK_ERR;
				}
			}
		}

	}

	insufficient_page = mPayloadSize%PAGE_SIZE;

	mDebugTrace = insufficient_page;
	if ( insufficient_page != 0 )
	{
		res = f_read(&filefatfs, RAM_Buf, insufficient_page, (void *)&read_size);

		mDebugTrace = read_size;
		if((read_size != 0) && (res == FR_OK)) 
		{
			flash_status = FLASH_LAYER_write_n_word(&Last_PGAddress, RAM_Buf, read_size);
		
			if ( flash_status != FLASH_COMPLETE )
			{
				while(1);
				return FR_DISK_ERR;
			}
		}
	}
#endif 
	return FR_OK; 
	
}

FRESULT COMMAND_isCorrectPES(void)
{
	FRESULT res = FR_OK;
	bool isMatchedPID = FALSE;
	FRESULT flseekTag = FR_OK;
	uint8 i = 0;

	mSeekPointer = 0; /*Initiate seek pointer*/
	mReadBinSize = 0;

	while( flseekTag == FR_OK )
	{
		flseekTag = f_lseek(&filefatfs, mSeekPointer);
	
		if (f_read(&filefatfs, (byte *)&mPES, 20, (void *)&mReadBinSize ) == FR_OK )
		{
			pPES = (PES*)&mPES;
			mPayloadSize = ((pPES->payload_size[0]<<16)|(pPES->payload_size[1]<<8)|(pPES->payload_size[2]));
		}
		else
		{
			return FR_INVALID_NAME;
		}

		if( ( pPES->start_code[0] != 0x24 ) || ( pPES->start_code[1] != 0x89 ) )
		{
			mSeekPointer = (mSeekPointer + PES_HEADER_LEN + mPayloadSize); /*search next start code*/

			if ( mSeekPointer > filefatfs.fsize ) /*If seeker is greater than file size, return*/
			{
				return FR_NO_FILE;
			}
		}

		/*check PID*/
		for( i = 0; i < PES_PID_LEN; i++ )
		{
			 if (pPES->pid[i] != PID[i] )
			 {
				isMatchedPID = FALSE;
				break;
			 }

			 isMatchedPID = TRUE;
		}

		if ( isMatchedPID == FALSE )
		{
			mSeekPointer = (mSeekPointer + PES_HEADER_LEN + mPayloadSize) ; /*search next PID*/
		}
		else if ( isMatchedPID == TRUE )
		{
			mSeekPointer = (mSeekPointer + PES_HEADER_LEN);

			/*check payload size*/
			if ( mPayloadSize > filefatfs.fsize )
				return FR_NO_FILE;

			if ( mPayloadSize == 0 )
				return FR_NO_FILE;

            if ( mPayloadSize > ( FLASH_SIZE - IAP_SIZE) )
            {
                bUpdateExtFlash = TRUE;
            }
			
			
            /*TBD Verify Checksum*/
            mCheckSum =  pPES->payload_checksum;
            if( COMMAND_isCorrectCheckSum(mCheckSum) != TRUE)
            {
                return FR_NO_FILE;  //The bin file has been detected.
            }
            else
            {
                return res;
            }
		}
	}

	return res;
	
}

bool COMMAND_isCorrectCheckSum(uint8_t CheckSum)
{
       uint32_t read_size = 0x00,page_num = 0,insufficient_page = 0;
       uint32_t i ,j;
       FRESULT res;


   /********************************************************
                	Calcuation CheckSum
    *********************************************************/
	page_num = mPayloadSize/PAGE_SIZE;
	if ( page_num != 0 )
	{
		for ( i = 0; i < page_num; i++ )
		{
			res = f_read(&filefatfs, RAM_Buf, PAGE_SIZE, (void *)&read_size);
                     if(res != FR_OK)
                            return FALSE;
                        
                    for(j=0 ; j<PAGE_SIZE ;j++){
                        calCheckSum = calCheckSum ^ RAM_Buf[j];
                    }

                }

	}

	insufficient_page = mPayloadSize%PAGE_SIZE;
	if ( insufficient_page != 0 )
	{
        res = f_read(&filefatfs, RAM_Buf, insufficient_page, (void *)&read_size);
        if(res != FR_OK)
            return FALSE;

        for(j=0 ; j<insufficient_page ;j++){
            calCheckSum = calCheckSum ^ RAM_Buf[j];
        }
    }
    
    /********************************************************
                     	Compare CheckSum
     *********************************************************/
    if(CheckSum == calCheckSum)
        return TRUE;

    return FALSE;
}


FRESULT COMMAND_checkProjectID(void)
{
    FRESULT res = FR_OK;
    FILINFO fno;
    DIR dir;

	uint8_t BlinkingCnt = 0;
	bool bInvert = FALSE;
	
	res = f_opendir(&dir, "0:/");
	if (res == FR_OK) 
	{
	    while(HCD_IsDeviceConnected(&USB_OTG_Core)) 
	    {
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0) 
			{
				//Can not open directory
				while ( BlinkingCnt <= 20)
				{
					bInvert = ! bInvert;

					HT68F30_LightBar_USB_StateError( bInvert );

					/* 0.1 seconds delay */
					USB_OTG_BSP_mDelay(100);

					BlinkingCnt ++;
				}
				HT68F30_LightBar_USB_StateError( TRUE );
				BlinkingCnt = 0;
			
				break;
			}
	    }
	}
	else
	{
		return res;
	}

	res = f_open(&filefatfs, ST32F105_100_UPGRAGE_FILE, FA_OPEN_EXISTING | FA_READ);
    if( res == FR_OK)
    { 
		if( COMMAND_isCorrectPES() != FR_OK )
		{
			f_close(&filefatfs);
			f_mount(0, NULL); 

			return FR_INVALID_NAME; 
		}
    }

	return res;
}

FRESULT COMMAND_EraseInternalFlash(void)
{
    __IO uint32_t erase_address = 0x00;
    uint8_t BlinkingCnt = 0;
	bool bInvert = FALSE;
	FRESULT res;

    /* Erase necessary page to download image */
    for ((erase_address = APPLICATIONADDRESS); erase_address < FLASH_ENDADDRESS;(erase_address += PAGE_SIZE))
    {
        if (FLASH_LAYER_ErasePage (erase_address) != FLASH_COMPLETE)
        {
            while ( BlinkingCnt <= 20)
            {
                bInvert = ! bInvert;

                HT68F30_LightBar_USB_StateError( bInvert );

                /* 0.1 seconds delay */
                USB_OTG_BSP_mDelay(10);

                BlinkingCnt ++;
            }

            HT68F30_LightBar_USB_StateError( FALSE );
            BlinkingCnt = 0;
		
        	return FR_DISK_ERR;    
        }
    }

    HT68F30_LightBar_EraseInternalFlash();

    return FR_OK;

}

FRESULT COMMAND_ProgramFlash(void)
{
    uint32_t read_size = 0x00;
    uint32_t page_num = 0;
    uint32_t insufficient_page = 0;
    uint32_t i = 0;

    FRESULT res;
    FLASH_Status flash_status = FLASH_COMPLETE;
    uint8_t BlinkingCnt = 0;
	bool bInvert = FALSE;


	res = f_open(&filefatfs, ST32F105_100_UPGRAGE_FILE, FA_OPEN_EXISTING | FA_READ);
	if ( res != FR_OK )
		return FR_DISK_ERR;


#if 0
	if ( (mPayloadSize + APPLICATIONADDRESS) > FLASH_ENDADDRESS )
		return FR_INVALID_OBJECT;
#endif 

    COMMAND_EraseInternalFlash();

    res = f_lseek(&filefatfs, mSeekPointer);
    if ( res != FR_OK )
        return FR_DISK_ERR; 
    
    /* Erase address init */
    Last_PGAddress = APPLICATIONADDRESS;

    /*
        1. ((FLASH_SIZE-IAP_SIZE)/PAGE_SIZE)
        2. (mPayloadSize)/PAGE_SIZE;

    */
    
    page_num = (FLASH_SIZE-IAP_SIZE)/PAGE_SIZE;
    mDebugTrace = page_num;
    if ( page_num != 0 )
    {
        for ( i = 0; i < page_num; i++ )
        {
            res = f_read(&filefatfs, RAM_Buf, PAGE_SIZE, (void *)&read_size);
            if((read_size != 0) && (res == FR_OK)) 
            {
                flash_status = FLASH_LAYER_write_n_word(&Last_PGAddress, RAM_Buf, read_size);
                if ( flash_status != FLASH_COMPLETE )
                {
                    f_close(&filefatfs);
                    f_mount(0, NULL); 
                    
                    while (1)
                    {
                        while ( BlinkingCnt <= 20)
                        {
                            bInvert = ! bInvert;

                            HT68F30_LightBar_USB_StateError( bInvert );

                            /* 0.1 seconds delay */
                            USB_OTG_BSP_mDelay(100);

                            BlinkingCnt ++;
                        }
                        BlinkingCnt = 0;
                    }

                }
            }
        }

    }

    /*
        1. ((FLASH_SIZE-IAP_SIZE)%PAGE_SIZE)
        2. mPayloadSize%PAGE_SIZE;

    */
    
    insufficient_page =  ((FLASH_SIZE-IAP_SIZE)%PAGE_SIZE);

    mDebugTrace = insufficient_page;
    if ( insufficient_page != 0 )
    {
        res = f_read(&filefatfs, RAM_Buf, insufficient_page, (void *)&read_size);

        mDebugTrace = read_size;
        if((read_size != 0) && (res == FR_OK)) 
        {
            flash_status = FLASH_LAYER_write_n_word(&Last_PGAddress, RAM_Buf, read_size);
        
            if ( flash_status != FLASH_COMPLETE )
            {
                f_close(&filefatfs);
                f_mount(0, NULL); 
                
                while ( BlinkingCnt <= 20)
                {
                    bInvert = ! bInvert;

                    HT68F30_LightBar_USB_StateError( bInvert );

                    /* 0.1 seconds delay */
                    USB_OTG_BSP_mDelay(100);

                    BlinkingCnt ++;
                }
                BlinkingCnt = 0;
            }
        }
    }

     HT68F30_LightBar_USB_isUpgrading();

	return res;
    
}

/**
  * @brief  COMMAND_IAPExecuteApplication
  * @param  None
  * @retval None
  */

int COMMAND_IAPExecuteApplication(void)
{
	/*Smith implemented for firmware upgrading identify 2013/3/22  */
	static IAP_State iap_state = IAP_STATE_PROJECT_ID_IDENTIFY;

	switch( iap_state )
	{
		case IAP_STATE_PROJECT_ID_IDENTIFY:
		{
			if (COMMAND_checkProjectID( ) == FR_OK )
			{
				iap_state = IAP_STATE_FLASH_PROGRAM;
			}
			else
			{
				iap_state = IAP_STATE_PROGRAM_FAIL;
			}
			
		}
			break;
	
		case IAP_STATE_FLASH_PROGRAM:
		{
			if (COMMAND_ProgramFlash( ) != FR_OK )
			{
				iap_state = IAP_STATE_PROGRAM_FAIL;

                return -1;
			}
			else
			{
               
                
#if defined ( EXT_FLASH )             
                if ( bUpdateExtFlash == TRUE )
                {
                    iap_state = IAP_STATE_EXT_FLASH_PROGRAM;
                }
                else
                {
                     iap_state = IAP_STATE_PROGRAM_COMPLETE;
                }
#else
                iap_state = IAP_STATE_PROGRAM_COMPLETE;
#endif 
			}
           
		}
			break;

        case IAP_STATE_EXT_FLASH_PROGRAM:
        {
#if defined ( EXT_FLASH )     
            uint32_t addr = 0;

            bUpdateExtFlash = FALSE;

            COMMAND_ProgramExtFlashMemory();

#if 0
            memSet(RAM_Buf, PAGE_SIZE, 0xff );
            sFLASH_ReadBuffer(RAM_Buf, addr, PAGE_SIZE);
#endif            

            iap_state = IAP_STATE_PROGRAM_COMPLETE;

#endif 
            

        }
            break;

		case IAP_STATE_PROGRAM_FAIL:
		{
            f_close(&filefatfs);
			f_mount(0, NULL); 
            
			USB_OTG_BSP_mDelay(500);
			iap_state = IAP_STATE_PROJECT_ID_IDENTIFY;
			COMMAND_JUMP();
		}
			break;

		case IAP_STATE_PROGRAM_COMPLETE:
		{
            f_close(&filefatfs);
			f_mount(0, NULL); 
            
            HT68F30_LightBar_USB_UpgradingDone( );
            USB_OTG_BSP_mDelay(500);

            iap_state = IAP_STATE_PROJECT_ID_IDENTIFY;
            COMMAND_JUMP();
            
		}
			break;

	}

	return 0;
//!@}

//!	Smith mark; Only COMMAND_DOWNLOAD will be used	@{
#if 0 
	/* Execute the command switch the command index */
	switch (CMD_index)
	{
		/* Reads all flash memory */
		case CMD_UPLOAD:
			COMMAND_UPLOAD();
			break;

		/* Writes Flash memory */
		case CMD_DOWNLOAD:
			COMMAND_DOWNLOAD();
			break;

		/* Jumps to user application code located in the internal Flash memory */
		case CMD_JUMP:
			COMMAND_JUMP();
			break;
	}
#endif
//!	@}

}


/**
  * @}
  */

/**
  * @}
  */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
