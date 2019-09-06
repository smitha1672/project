#include "freertos_conf.h"
#include "freertos_task.h"
#include "Debug.h"

#include "SOUND_BAR_V1_PinDefinition.h"
#include "config.h"
#include "I2C1LowLevel.h"

//______________________________________________________________________________

#define configI2C1_DMA 1
#define I2C1_DEBUG 0
#define DRV_mutex 1
#define APP_mutex 1

#if 0
#define LSM303DLHC_LONG_TIMEOUT 0x1000 //ST example
#endif
//______________________________________________________________________________

typedef FlagStatus ( *i2c_get_flag_status )( I2C_TypeDef* I2Cx, uint32_t I2C_FLAG );

typedef ErrorStatus ( *i2c_check_event ) ( I2C_TypeDef* I2Cx, uint32_t I2C_EVENT );

//! < Private variable    @{
static DMA_InitTypeDef   sEEDMA_InitStructure; 
static __IO uint32_t  sEETimeout = sEE_LONG_TIMEOUT;   
static __IO uint16_t* sEEDataReadPointer;   
static __IO uint16_t*  sEEDataWritePointer;  

static bool bIsI2C_enable = FALSE;

#if DRV_mutex
static xSemaphoreHandle _DRV_I2C1_SEMA = NULL;
static portTickType _DRV_xI2C1_BlockTime = portMAX_DELAY;
#endif

#if APP_mutex
//for i2c1 mutex
#if 0 // David, fix warning.
static xSemaphoreHandle _I2C1_SEMA = NULL;
static portTickType _xI2C1_BlockTime = portMAX_DELAY;
#endif
#endif

//!    @}

static void sEE_Init(void);

static void sEE_LowLevel_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction);

static void sEE_LowLevel_DeInit(void);

extern void ir_decode_wait(uint16 timeout);

/*______________________________________________________________________________________________*/
#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
static int err_cnt = 0;
uint32_t sEE_TIMEOUT_UserCallback(uint8_t    DevAddr)
{
    err_cnt++;

    
#if 0
    I2C_SoftwareResetCmd(sEE_I2C,ENABLE); 
    I2C_SoftwareResetCmd(sEE_I2C,DISABLE);
    sEE_LowLevel_DeInit( );
    sEE_Init();
    
    //ST Added
    sEE_I2C->CR1 &= ((uint16_t)0xFEFF);
    
    (void) sEE_I2C->SR1;
    (void) sEE_I2C->SR2;
    //ST End
#endif

#if 1
 if(err_cnt % 50) 
 {
    // When error count over 50, we need to reset 9533 for relase SDA pin
    // TODO
#if 0
   GPIOMiddleLevel_Clr(__O_RST_HDMI_SIL953x);
   for(uint32_t i=0; i<0x5000; i++) __NOP();
   GPIOMiddleLevel_Set(__O_RST_HDMI_SIL953x);
#endif
 }
 
 (void) I2C1->DR;
 I2C1->CR1   &= ~I2C_CR1_ACK;
 
 TRACE_DEBUG((0,"1 : CR1 0x%X, CR2 0x%X,SR1 0x%X,SR2 0x%X,",sEE_I2C->CR1,sEE_I2C->CR2,sEE_I2C->SR1,sEE_I2C->SR2));

 uint32_t TRISE = I2C1->TRISE;
 uint32_t   CCR = I2C1->CCR;
 uint32_t  OAR1 = I2C1->OAR1;
 uint32_t   CR1 = I2C1->CR1;
 uint32_t   CR2 = I2C1->CR2;
  
 RCC->APB1RSTR |=  RCC_APB1RSTR_I2C1RST;
 for(uint32_t i=0; i<0x500; i++) __NOP();
 RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
 
 I2C1->CR1 |=  I2C_CR1_SWRST;
 for(uint32_t i=0; i<0x500; i++) __NOP();
 I2C1->CR1 &= ~I2C_CR1_SWRST;
  
 I2C1->TRISE = TRISE;
 I2C1->CCR   = CCR;
 I2C1->OAR1  = OAR1;
 I2C1->CR2   = CR2;
 I2C1->CR1   = 0x401; //CR1|I2C_CR1_ACK;
 
 (void) I2C1->SR1;
 (void) I2C1->SR2;
 //GPIOMiddleLevel_Clr(__O_SP_TEST_PE1);
 
 TRACE_DEBUG((0,"2 : CR1 0x%X, CR2 0x%X,SR1 0x%X,SR2 0x%X,",sEE_I2C->CR1,sEE_I2C->CR2,sEE_I2C->SR1,sEE_I2C->SR2));
#endif

    __enable_irq();//123
    
    return sEE_FAIL;
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */

#ifdef USE_DEFAULT_CRITICAL_CALLBACK
/**
  * @brief  Start critical section: these callbacks should be typically used
  *         to disable interrupts when entering a critical section of I2C communication
  *         You may use default callbacks provided into this driver by uncommenting the 
  *         define USE_DEFAULT_CRITICAL_CALLBACK.
  *         Or you can comment that line and implement these callbacks into your 
  *         application.
  * @param  None.
  * @retval None.
  */
void sEE_EnterCriticalSection_UserCallback(void)
{
    __disable_irq();  
}

/**
  * @brief  Start and End of critical section: these callbacks should be typically used
  *         to re-enable interrupts when exiting a critical section of I2C communication
  *         You may use default callbacks provided into this driver by uncommenting the 
  *         define USE_DEFAULT_CRITICAL_CALLBACK.
  *         Or you can comment that line and implement these callbacks into your 
  *         application.
  * @param  None.
  * @retval None.
  */
void sEE_ExitCriticalSection_UserCallback(void)
{
    __enable_irq();
}
#endif /* USE_DEFAULT_CRITICAL_CALLBACK */

#if DRV_mutex
static bool _DRV_I2C1_sema_mutex_create(void)
{
    if (_DRV_I2C1_SEMA == NULL)
    {
        _DRV_I2C1_SEMA  = xSemaphoreCreateMutex();

        if (_DRV_I2C1_SEMA)
        {
            _DRV_xI2C1_BlockTime = portMAX_DELAY;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return TRUE;
    }
}

bool DRV_I2C1_sema_mutex_take(void)
{
#if DRV_mutex
    if (_DRV_I2C1_sema_mutex_create())
    {
        //take mutex for blocking
        if ( xSemaphoreTake( _DRV_I2C1_SEMA, _DRV_xI2C1_BlockTime ) != pdTRUE )
        {
            TRACE_DEBUG((0, "Drv_I2C1_sema_mutex_take busy 3333 !! "));
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        //false to take mutex
        return FALSE;
    }
#else
        return TRUE;
#endif
}

bool DRV_I2C1_sema_mutex_give(void)
{
#if DRV_mutex
    if (_DRV_I2C1_sema_mutex_create())
    {
        //give mutex for unblocking
        if ( xSemaphoreGive( _DRV_I2C1_SEMA) != pdTRUE )
        {
            //TRACE_DEBUG((0, "Drv_I2C1_sema_mutex_give busy 3333 !! "));
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        //false to give mutex
        return FALSE;
    }
#else
    return TRUE;
#endif
}

#endif

bool i2c1_flag_timeout_check( i2c_get_flag_status i2c_flag_status, I2C_TypeDef* I2Cx, uint32_t I2C_FLAG, uint32 time )
{
    uint32_t timeout = time;
    bool ret = FALSE;

    while( i2c_flag_status( I2Cx, I2C_FLAG ) )
    {
        if ( (timeout--) == 0 )
        {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

bool i2c1_event_timeout_check( i2c_check_event i2c_event_status, I2C_TypeDef* I2Cx, uint32_t I2C_EVENT, uint32 time )
{
    uint32_t timeout = time;
    bool ret = FALSE;

    while( !i2c_event_status( I2Cx, I2C_EVENT ) )
    {
        if ( (timeout--) == 0 )
        {
            ret = TRUE;
            break;
        }
    }
    return ret;
}


static void sEE_LowLevel_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction)
{ 
    /* Initialize the DMA with the new parameters */
    if (Direction == sEE_DIRECTION_TX)
    {
        /* Configure the DMA Tx Channel with the buffer address and the buffer size */
        sEEDMA_InitStructure.DMA_MemoryBaseAddr = pBuffer;
        sEEDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    
        sEEDMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;
        
        /*Smith JUNE 14 2013: Fix wave sound cannot be continually outputted when Treble, volume..was controlled by USER.@{*/
        sEEDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        //!@}
        
        DMA_Init(sEE_I2C_DMA_CHANNEL_TX, &sEEDMA_InitStructure);
        /* Enable the DMA Tx Channel */
        DMA_Cmd(sEE_I2C_DMA_CHANNEL_TX, ENABLE);
    }
    else
    { 
        /* Configure the DMA Rx Channel with the buffer address and the buffer size */
        sEEDMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuffer;
        sEEDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        sEEDMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;  

        /*Smith JUNE 14 2013: Fix wave sound cannot be continually outputted when Treble, volume..was controlled by USER.@{*/
        sEEDMA_InitStructure.DMA_Priority = DMA_Priority_High;
        //!@}
        
        //DMA_Init(sEE_I2C_DMA_CHANNEL_RX, &sEEDMA_InitStructure);
        
        /* Inform the DMA that the next End Of Transfer Signal will be the last one */
        //I2C_DMALastTransferCmd(sEE_I2C, ENABLE); 

        /* Enable the DMA Rx Channel */
        //DMA_Cmd(sEE_I2C_DMA_CHANNEL_RX, DISABLE);  
    }
}

static uint32_t __STM32_I2C_readSlave(    uint8_t    DevAddr,
                                                uint16_t ReadAddr, 
                                                uint8_t *pBuffer, 
                                                uint16_t *NumByteToRead, 
                                                bool longAddr)
{

    uint16_t NumByteToReadN = *NumByteToRead;
    bool IsTimeout = FALSE;
    uint8 state_mach = 0;
    uint32_t ret = sEE_OK;
#if 0 //ST example
    __IO uint32_t LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
    __IO uint32_t temp;
#endif
#if 1
    for( state_mach = 0; state_mach <= 5; state_mach++ )
    {
        IsTimeout = FALSE;    

        switch( state_mach )
        {
            case 0:
            {
                IsTimeout = i2c1_flag_timeout_check( I2C_GetFlagStatus, sEE_I2C, I2C_FLAG_BUSY, sEE_FLAG_TIMEOUT );
            }
                break;

            case 1:
            {
                /* Send STRAT condition */ 
                //I2C_GenerateSTART(sEE_I2C, ENABLE); 
                sEE_I2C->CR1 |= ((uint16_t)0x0100);
                //IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
                IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, 0x00000001, sEE_FLAG_TIMEOUT );
            }
                break;

            case 2:
            {
                /* Send device address for read */ 
                I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Transmitter); 
                //IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
                IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, 0x00000002, sEE_FLAG_TIMEOUT );
                if ( IsTimeout == FALSE )
                {
                    /* Clear EV6 by setting again the PE bit */
                    I2C_Cmd(sEE_I2C, ENABLE);
                }
            }
                break;

            case 3:
            {
                if ( longAddr == TRUE )
                {
                    /* Send the device's internal address to read to */
                    I2C_SendData(sEE_I2C, (uint8_t)((ReadAddr & 0xFF00) >> 8));
                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
                    if ( IsTimeout == TRUE )
                    {
                        break;    
                    }

                    /* Send the device's internal address to read to */
                    I2C_SendData(sEE_I2C, (uint8_t)(ReadAddr & 0x00FF));    
                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
                    if ( IsTimeout == TRUE )
                    {
                        break;    
                    }

                    /* Send STRAT condition a second time */  
                    //I2C_GenerateSTART(sEE_I2C, ENABLE);
                    sEE_I2C->CR1 |= ((uint16_t)0x0100);
                    
                    //IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, 0x00000001, sEE_FLAG_TIMEOUT );
                    if ( IsTimeout == TRUE )
                    {
                        break;    
                    }

                    /* Send device address for read */
                    I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Receiver);
                    //IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, 0x00000002, sEE_FLAG_TIMEOUT );
                    if ( IsTimeout == TRUE )
                    {
                        break;    
                    }
                }
                else 
                {
                    /* Send the device's internal address to read to */
                    I2C_SendData(sEE_I2C, (uint8_t)(ReadAddr & 0x00FF));
                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
                    if ( IsTimeout == TRUE )
                    {
                        break;    
                    }
                    
                    /* Send STRAT condition a second time */  
                    //I2C_GenerateSTART(sEE_I2C, ENABLE);
                    sEE_I2C->CR1 |= ((uint16_t)0x0100);
                    
                    //IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, 0x00000001, sEE_FLAG_TIMEOUT );
                    if ( IsTimeout == TRUE )
                    {
                        break;    
                    }

                    /* Send device address for read */
                    I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Receiver);
                    //IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, 0x00000002, sEE_FLAG_TIMEOUT );
                    if ( IsTimeout == TRUE )
                    {
                        break;    
                    }
                }
            }
                break;

            case 4:
            {
                /* While there is data to be read */
                while( NumByteToReadN )  
                {
                    if( NumByteToReadN == 1 )
                    {
                        // Disable Acknowledgement 
                        I2C_AcknowledgeConfig(sEE_I2C, DISABLE);

                        // Send STOP Condition 
                        I2C_GenerateSTOP(sEE_I2C, ENABLE);
                    }

                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED, sEE_FLAG_TIMEOUT );
                    if ( IsTimeout == FALSE )
                    {
                        /*Fix USB and speaker test output is not continute issue, smith*/
                        __disable_irq();
                        
                        // Read a byte from the HMC5843 
                        *pBuffer = I2C_ReceiveData(sEE_I2C);

                        /*Fix USB and speaker test output is not continute issue, smith*/
                        __enable_irq();

                        
                        // Point to the next location where the byte read will be saved 
                        pBuffer++; 

                        // Decrement the read bytes counter 
                        NumByteToReadN--;         
                    }
                    else 
                    {
                        break;
                    }
                }
            }
                break;

            case 5:
            {
                /* Enable Acknowledgement to be ready for another reception */
                I2C_AcknowledgeConfig(sEE_I2C, ENABLE);
            }
                break;


        }

        if ( IsTimeout == TRUE )
        {
            //TRACE_ERROR((0, "__STM32_I2C_readSlave error state = %d; Device Addr = 0x%X; Register = 0x%X", state_mach,  DevAddr, ReadAddr ));
            
	    TRACE_ERROR((0, "rerr1 %d", state_mach ));
            break;
        }
    }
    
     *NumByteToRead = NumByteToReadN; 


    if ( IsTimeout == TRUE )
    {
        ret = sEE_TIMEOUT_UserCallback(DevAddr);        
    }

    
    return ret;
        
#else //ST example
  restart:

  LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
/* Send START condition */
  I2C_GenerateSTART(sEE_I2C, ENABLE);
  /* Test on EV5 and clear it */
  while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  {
    if (LSM303DLHC_Timeout-- == 0)
      return sEE_FAIL;
  }

    LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
    /* Send slave address for read */
    I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Transmitter);

    while (!I2C_CheckEvent(sEE_I2C,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
      if (LSM303DLHC_Timeout-- == 0)
      {
        I2C_ClearFlag(sEE_I2C,I2C_FLAG_BUSY|I2C_FLAG_AF);
        goto restart;
      } 
    }

    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd(sEE_I2C, ENABLE);

    I2C_SendData(sEE_I2C, ReadAddr);

    /* Test on EV8 and clear it */
    LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
    while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      if (LSM303DLHC_Timeout-- == 0)
       return sEE_FAIL;
    }

    if (NumByteToReadN == 0x01)
    {
      restart3:  
      /* Send START condition */
      I2C_GenerateSTART(sEE_I2C, ENABLE);
      while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT));
      /* Send Slave address for read */
      I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Receiver);
      /* Wait until ADDR is set */
      LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
      while (!I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_ADDR))   
      {
        if (LSM303DLHC_Timeout-- == 0) 
        {
          I2C_ClearFlag(sEE_I2C,I2C_FLAG_BUSY|I2C_FLAG_AF);
          goto restart3;
        }
      }
      /* Clear ACK */
      I2C_AcknowledgeConfig(sEE_I2C, DISABLE);
      __disable_irq();
      /* Clear ADDR flag */
      temp = sEE_I2C->SR2;
      /* Program the STOP */
      //111 I2C_GenerateSTOP(sEE_I2C, ENABLE);
      __enable_irq();
      while ((I2C_GetLastEvent(sEE_I2C) & 0x0040) != 0x000040); /* Poll on RxNE */
      /* Read the data */
      *pBuffer = I2C_ReceiveData(sEE_I2C);
      /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
      while ((sEE_I2C->CR1&0x200) == 0x200);
      /* Enable Acknowledgement to be ready for another reception */
      I2C_AcknowledgeConfig(sEE_I2C, ENABLE);

        //*NumByteToRead = 1; 
      return sEE_OK;
    }
    else
    if(NumByteToReadN == 0x02)
    {
        restart4:     
        /* Send START condition */
        I2C_GenerateSTART(sEE_I2C, ENABLE);
        while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT));
        /* Send EEPROM address for read */
        I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Receiver);
        sEE_I2C->CR1 = 0xC01; /* ACK=1; POS =1 */
        LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
        while (!I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_ADDR))
        {
          if (LSM303DLHC_Timeout-- == 0) 
          {
            I2C_ClearFlag(sEE_I2C,I2C_FLAG_BUSY|I2C_FLAG_AF);
            goto restart4;     
          }         
        }
        __disable_irq();
        /* Clear ADDR */
        temp = sEE_I2C->SR2;
        /* Disable ACK */
        I2C_AcknowledgeConfig(sEE_I2C, DISABLE);
        __enable_irq();
        while ((I2C_GetLastEvent(sEE_I2C) & 0x0004) != 0x00004); /* Poll on BTF */
         __disable_irq();
        /* Program the STOP */
        //111 I2C_GenerateSTOP(sEE_I2C, ENABLE);
        /* Read first data */
        *pBuffer = sEE_I2C->DR;
        pBuffer++;
        /* Read second data */
        *pBuffer = sEE_I2C->DR;
        __enable_irq();
        sEE_I2C->CR1 = 0x0401; /* POS = 0, ACK = 1, PE = 1 */

        //*NumByteToRead = 2; 
        return sEE_OK;
    }
      else
      {
    restart2:
        LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
        /* Send START condition */
        I2C_GenerateSTART(sEE_I2C, ENABLE);
        /* Test on EV5 and clear it */
        while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT))
        {
          if (LSM303DLHC_Timeout-- == 0) return sEE_FAIL;
        }
        LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
        /* Send slave address for read */
        I2C_Send7bitAddress(sEE_I2C,  DevAddr, I2C_Direction_Receiver);
        while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        {
          
          if (LSM303DLHC_Timeout-- == 0) 
          {
            I2C_ClearFlag(sEE_I2C,I2C_FLAG_BUSY|I2C_FLAG_AF);
            goto restart2;
          }
        }
      
        /* While there is data to be read; here the safe procedure is implemented */
        while (NumByteToReadN)
        {
      
          if (NumByteToReadN != 3) /* Receive bytes from first byte until byte N-3 */
          {
            while ((I2C_GetLastEvent(sEE_I2C) & 0x00004) != 0x000004); /* Poll on BTF */
            /* Read data */
            *pBuffer = I2C_ReceiveData(sEE_I2C);
            pBuffer++;
            /* Decrement the read bytes counter */
            NumByteToReadN--;
          }
      
          if (NumByteToReadN == 3)  /* it remains to read three data: data N-2, data N-1, Data N */
          {
      
            /* Data N-2 in DR and data N -1 in shift register */
            while ((I2C_GetLastEvent(sEE_I2C) & 0x000004) != 0x0000004); /* Poll on BTF */
            /* Clear ACK */
            I2C_AcknowledgeConfig(sEE_I2C, DISABLE);
            __disable_irq();
            /* Read Data N-2 */
            *pBuffer = I2C_ReceiveData(sEE_I2C);
            pBuffer++;
            /* Program the STOP */
            //111 I2C_GenerateSTOP(sEE_I2C, ENABLE);
            /* Read DataN-1 */
            *pBuffer = I2C_ReceiveData(sEE_I2C);
            __enable_irq();
            pBuffer++;
            while ((I2C_GetLastEvent(sEE_I2C) & 0x00000040) != 0x0000040); /* Poll on RxNE */
            /* Read DataN */
            *pBuffer = sEE_I2C->DR;
            /* Reset the number of bytes to be read by master */
            NumByteToReadN = 0;
          }
        }
          /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
          while ((sEE_I2C->CR1&0x200) == 0x200);
          /* Enable Acknowledgement to be ready for another reception */
          I2C_AcknowledgeConfig(sEE_I2C, ENABLE);

          //*NumByteToRead = NumByteToReadN; 
          return sEE_OK;
        }

#endif

        
}


static uint32_t __STM32_I2C_writeSlave(    uint8_t DevAddr, 
                                                uint16_t WriteAddr, 
                                                uint8_t *pBuffer, 
                                                uint16_t *NumByteToWrite, 
                                                bool longAddr,
                                                bool b_DMA_used)
{ 
    /* Set the pointer to the Number of data to be written. This pointer will be used 
    by the DMA Transfer Completer interrupt Handler in order to reset the 
    variable to 0. User should check on this variable in order to know if the 
    DMA transfer has been complete or not. */
    sEEDataWritePointer = NumByteToWrite;
    uint16_t NumByteForWrite = *NumByteToWrite;
    uint16_t index = 0; /*Smith Fixes Bug: if length > 255, system crash.*/
    bool IsTimeout = FALSE;
    uint8 state_mach = 0;
    uint32_t ret = sEE_OK;
#if 0 //ST example
    __IO uint32_t LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
    __IO uint32_t temp;
#endif

#if 1
    for( state_mach = 0; state_mach <= 4; state_mach++ )
    {
        IsTimeout = FALSE;    
        
        switch ( state_mach )
        {
            case 0:
            {
                IsTimeout = i2c1_flag_timeout_check( I2C_GetFlagStatus, sEE_I2C, I2C_FLAG_BUSY, sEE_FLAG_TIMEOUT );
            }
                break;

            case 1:
                {
                /*!< Send START condition */
                //I2C_GenerateSTART(sEE_I2C, ENABLE); 
                sEE_I2C->CR1 |= ((uint16_t)0x0100);
                
                //IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
                IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, 0x00000001, sEE_FLAG_TIMEOUT );
            }
                break;

            case 2:
            {
                /*!< Send EEPROM address for write */
                I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Transmitter);
                //IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
                IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, 0x00000002, sEE_FLAG_TIMEOUT );
            }
                break;

            case 3:
            {

                if ( longAddr == TRUE)
                {
                    /*!< Send the EEPROM's internal address to write to : MSB of the address first */
                    I2C_SendData(sEE_I2C, (uint8_t)((WriteAddr & 0xFF00) >> 8));
                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT ); 
                    if ( IsTimeout == TRUE )
                        break;    

                    /*!< Send the EEPROM's internal address to write to : LSB of the address */
                    I2C_SendData(sEE_I2C, (uint8_t)(WriteAddr & 0x00FF));
                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
                    if ( IsTimeout == TRUE )
                        break;

                }
                else
                {
                    /*!< Send the EEPROM's internal address to write to : only one byte Address */
                    I2C_SendData(sEE_I2C, WriteAddr);
                    IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
                    if ( IsTimeout == TRUE )
                        break;
                }
            }
                break;

            case 4:
            {
                if( b_DMA_used )
                {
                    /* Configure the DMA Tx Channel with the buffer address and the buffer size */
                    I2C_DMACmd(sEE_I2C, ENABLE);
                    sEE_LowLevel_DMAConfig((uint32_t)pBuffer, (uint16_t)(*NumByteToWrite), sEE_DIRECTION_TX);
                }
                else
                {
                    /* Disable the interrupts mechanism to prevent the I2C communication from corruption */
                    //__disable_irq();
                
                    for (index = 0; index < NumByteForWrite; index ++ )
                    {  
                        /* Prepare the register value to be sent */
                        I2C_SendData(sEE_I2C, *(pBuffer+index));
                
                        /*!< Wait till all data have been physically transferred on the bus */
                        sEETimeout = sEE_FLAG_TIMEOUT;
                        while(!I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_BTF))
                        {
                            if((sEETimeout--) == 0)
                            {   
                                IsTimeout = TRUE;
                                break;
                            }   
                        }
                
                        if ( IsTimeout == TRUE )
                            break;  
                
                        /*!< Decrement the read bytes counter */
                        (uint16_t)(*NumByteToWrite)--;
                    }

                    /* Re-enable the interrupt mechanism */
                    //__enable_irq();

                    /*!< Send STOP condition */ /*Smith 23 Sept.: if STOP condition is never to send that will make I2C flag busy */
                    I2C_GenerateSTOP(sEE_I2C, ENABLE);
                
                    /* Perform a read on SR1 and SR2 register to clear eventualaly pending flags */
                    //(void)sEE_I2C->SR1;
                    //(void)sEE_I2C->SR2;
                    
                    /* Re-enable the interrupt mechanism */
                    //__enable_irq();
                }
            }
                break;
        }    

        if ( IsTimeout == TRUE )
        {
            TRACE_ERROR((0, "werr1 %d", state_mach ));
#if( I2C1_DEBUG == 1 )        
            //TRACE_ERROR((0, "__STM32_I2C_writeSlave error state = %d; Device Addr = 0x%X; Register = 0x%X", state_mach,  DevAddr, WriteAddr ));
            TRACE_ERROR((0, "s = %d; D = 0x%X; R = 0x%X", state_mach,  DevAddr, WriteAddr ));
#endif            
            break;
        }
    }

    if ( IsTimeout == TRUE )
    {
        ret = sEE_TIMEOUT_UserCallback(DevAddr);
    }

    /* If all operations OK, return sEE_OK (0) */
    return ret;

#else  //ST example
restart1:
  LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
  /* Send START condition */
  I2C_GenerateSTART(sEE_I2C, ENABLE);
  /* Test on EV5 and clear it */
  while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  {
    if (LSM303DLHC_Timeout-- == 0) return sEE_FAIL;
  }

/* Send slave address for write */
I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Transmitter);

LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
/* Test on EV6 and clear it */
while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
{
  
  if (LSM303DLHC_Timeout-- == 0) 
  {
    I2C_ClearFlag(sEE_I2C,I2C_FLAG_BUSY|I2C_FLAG_AF);
    goto restart1;
  }
}

LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;

/* Transmit the first address for r/w operations */
I2C_SendData(sEE_I2C, WriteAddr);

/* Test on EV8 and clear it */
while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
{
  if (LSM303DLHC_Timeout-- == 0)
    return sEE_FAIL;
}

if (NumByteForWrite == 0x01)
{
  LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
  /* Prepare the register value to be sent */
  I2C_SendData(sEE_I2C, *pBuffer);

  /* Test on EV8 and clear it */
  while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
  {
    if (LSM303DLHC_Timeout-- == 0)
      return sEE_FAIL;
  }

  /* End the configuration sequence */
  //111 I2C_GenerateSTOP(sEE_I2C, ENABLE);
  return sEE_OK;
}

I2C_SendData(sEE_I2C, *pBuffer);
pBuffer++;
NumByteForWrite--;
/* While there is data to be written */
while (NumByteForWrite--)
{
  while ((I2C_GetLastEvent(sEE_I2C) & 0x04) != 0x04);  /* Poll on BTF */
  /* Send the current byte */
  I2C_SendData(sEE_I2C, *pBuffer);
  /* Point to the next byte to be written */
  pBuffer++;

}

LSM303DLHC_Timeout = LSM303DLHC_LONG_TIMEOUT;
/* Test on EV8_2 and clear it, BTF = TxE = 1, DR and shift registers are
 empty */
while (!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
{
  if (LSM303DLHC_Timeout-- == 0) return sEE_FAIL;
}
/* Send STOP condition */
//111 I2C_GenerateSTOP(sEE_I2C, ENABLE);
return sEE_OK;

#endif
}

void sEE_I2C_DMA_TX_IRQHandler(void)
{
    /* Check if the DMA transfer is complete */ 
    if(DMA_GetFlagStatus(sEE_I2C_DMA_FLAG_TX_TC) != RESET)
    {  
        /* Disable the DMA Tx Channel and Clear all its Flags */  
        DMA_Cmd(sEE_I2C_DMA_CHANNEL_TX, DISABLE);
        DMA_ClearFlag(sEE_I2C_DMA_FLAG_TX_GL);

        /*!< Wait till all data have been physically transferred on the bus */
        sEETimeout = sEE_LONG_TIMEOUT;
        while(!I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_BTF))
        {
            if((sEETimeout--) == 0) 
            {
                //TRACE_ERROR((0, "TX timeout "));
                I2C_DMACmd(sEE_I2C, DISABLE);
                DRV_I2C1_sema_mutex_give();
                return;
            }
        }

        /*!< Send STOP condition */
        I2C_GenerateSTOP(sEE_I2C, ENABLE);

        /* Perform a read on SR1 and SR2 register to clear eventualaly pending flags */
        (void)sEE_I2C->SR1;
        (void)sEE_I2C->SR2;

        /* Reset the variable holding the number of data to be written */
        *sEEDataWritePointer = 0;  

        //release mutex for i2c1
        //TRACE_ERROR((0, "DMA G"));
        I2C_DMACmd(sEE_I2C, DISABLE);
        DRV_I2C1_sema_mutex_give();
    }
}

void sEE_I2C_DMA_RX_IRQHandler(void)
{
    /* Check if the DMA transfer is complete */
    if(DMA_GetFlagStatus(sEE_I2C_DMA_FLAG_RX_TC) != RESET)
    {      
        /*!< Send STOP Condition */
        I2C_GenerateSTOP(sEE_I2C, ENABLE);    

        /* Disable the DMA Rx Channel and Clear all its Flags */  
        DMA_Cmd(sEE_I2C_DMA_CHANNEL_RX, DISABLE);
        DMA_ClearFlag(sEE_I2C_DMA_FLAG_RX_GL);

        /* Reset the variable holding the number of data to be read */
        *sEEDataReadPointer = 0;
    }
}

static void sEE_LowLevel_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;  

    /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
    RCC_APB2PeriphClockCmd(sEE_I2C_SCL_GPIO_CLK | sEE_I2C_SDA_GPIO_CLK, ENABLE);

    /*!< sEE_I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(sEE_I2C_CLK, ENABLE);

    /*!< GPIO configuration */  
    /*!< Configure sEE_I2C pins: SCL */
    GPIO_InitStructure.GPIO_Pin = sEE_I2C_SCL_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//GPIO_Mode_Out_PP;//GPIO_Mode_AF_PP;//GPIO_Mode_AF_OD;
    GPIO_Init(sEE_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = sEE_I2C_SDA_PIN;
    GPIO_Init(sEE_I2C_SDA_GPIO_PORT, &GPIO_InitStructure); 

    /* Configure and enable I2C DMA TX Channel interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_TX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_I2C_DMA_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure and enable I2C DMA RX Channel interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_RX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_I2C_DMA_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);  

    /*!< I2C DMA TX and RX channels configuration */
    /* Enable the DMA clock */
    RCC_AHBPeriphClockCmd(sEE_I2C_DMA_CLK, ENABLE);

    sEEDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)sEE_I2C_DR_Address;
    sEEDMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)0;   /* This parameter will be configured durig communication */
    sEEDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    /* This parameter will be configured durig communication */
    sEEDMA_InitStructure.DMA_BufferSize = 0xFFFF;            /* This parameter will be configured durig communication */
    sEEDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    sEEDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    sEEDMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
    sEEDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    sEEDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

    /*Smith JUNE 14 2013: Fix wave sound cannot be continually outputted when Treble, volume..was controlled by USER.@{*/
    sEEDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    //!@}
    
    sEEDMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    /* I2C TX DMA Channel configuration */
    DMA_DeInit(sEE_I2C_DMA_CHANNEL_TX);
    DMA_Init(sEE_I2C_DMA_CHANNEL_TX, &sEEDMA_InitStructure);  


    /* Configure the DMA Rx Channel with the buffer address and the buffer size */
    sEEDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)sEE_I2C_DR_Address;
    sEEDMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)0;   /* This parameter will be configured durig communication */
    sEEDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;    /* This parameter will be configured durig communication */
    sEEDMA_InitStructure.DMA_BufferSize = 0xFFFF;            /* This parameter will be configured durig communication */
    sEEDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    sEEDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    sEEDMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
    sEEDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    sEEDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

    /*Smith JUNE 14 2013: Fix wave sound cannot be continually outputted when Treble, volume..was controlled by USER.@{*/
    sEEDMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    //!@}
    
    sEEDMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    /* I2C RX DMA Channel configuration */
    DMA_DeInit(sEE_I2C_DMA_CHANNEL_RX);
    DMA_Init(sEE_I2C_DMA_CHANNEL_RX, &sEEDMA_InitStructure);  

    /* Enable the DMA Channels Interrupts */
#if ( configI2C1_DMA == 1 )    
    DMA_ITConfig(sEE_I2C_DMA_CHANNEL_TX, DMA_IT_TC, ENABLE);
#else
    DMA_ITConfig(sEE_I2C_DMA_CHANNEL_RX, DMA_IT_TC, DISABLE);  
#endif

    DMA_ITConfig(sEE_I2C_DMA_CHANNEL_RX, DMA_IT_TC, DISABLE);    
}

static void sEE_LowLevel_DeInit(void)
{
#if 1    
    GPIO_InitTypeDef  GPIO_InitStructure; 
    NVIC_InitTypeDef NVIC_InitStructure;    

    /* sEE_I2C Peripheral Disable */
    I2C_Cmd(sEE_I2C, DISABLE);

    /* sEE_I2C DeInit */
    I2C_DeInit(sEE_I2C);

    /*!< sEE_I2C Periph clock disable */
    RCC_APB1PeriphClockCmd(sEE_I2C_CLK, DISABLE);

    /*!< GPIO configuration */  
    /*!< Configure sEE_I2C pins: SCL */
    GPIO_InitStructure.GPIO_Pin = sEE_I2C_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(sEE_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = sEE_I2C_SDA_PIN;
    GPIO_Init(sEE_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);

    /* Configure and enable I2C DMA TX Channel interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_TX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_I2C_DMA_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure and enable I2C DMA RX Channel interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_RX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_I2C_DMA_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);   

    /* Disable and Deinitialize the DMA channels */
    DMA_Cmd(sEE_I2C_DMA_CHANNEL_TX, DISABLE);
    DMA_Cmd(sEE_I2C_DMA_CHANNEL_RX, DISABLE);
    DMA_DeInit(sEE_I2C_DMA_CHANNEL_TX);
    DMA_DeInit(sEE_I2C_DMA_CHANNEL_RX);
#endif    
}


static void sEE_Init(void)
{ 
    I2C_InitTypeDef  I2C_InitStructure;

    sEE_LowLevel_Init();

    /*!< I2C configuration */
    /* sEE_I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;

    /* sEE_I2C Peripheral Enable */
    I2C_Cmd(sEE_I2C, ENABLE);

    /* Apply sEE_I2C configuration after enabling it */
    I2C_Init(sEE_I2C, &I2C_InitStructure);

#if ( configI2C1_DMA == 1 )    
    /* Enable the sEE_I2C peripheral DMA requests */
    //1231 I2C_DMACmd(sEE_I2C, ENABLE);
    I2C_DMACmd(sEE_I2C, DISABLE); // 111
#else
    I2C_DMACmd(sEE_I2C, DISABLE);
#endif
}


/*__________________________________________________________________________________________________________*/
void TWI_initialize( void )
{
    sEE_Init();  
    bIsI2C_enable = TRUE;
}

void TWI_Deinitialize( void )
{
    sEE_LowLevel_DeInit( );
    bIsI2C_enable = FALSE;
}

bool I2C_writeSlave(byte control, uint16 address, byte *data, uint16 length, bool longAddr, bool b_DMA_used) 
{
    int status = I2C_SUCCESS;
    uint16 len = length;
    uint8 *pbuff;

    if ( bIsI2C_enable != TRUE )
    {
        TRACE_ERROR((0, "I2C interface has not been enable !!"));
        return FALSE;
    }

    if ( data == NULL )
    {
         TRACE_ERROR((0, "I2C write buffer is null !!"));
        return FALSE;
    }

#if DRV_mutex
    if (DRV_I2C1_sema_mutex_take() != TRUE)
    {
        return FALSE;
    }
#endif
    
    pbuff = data;

    // never use DMA
    status = __STM32_I2C_writeSlave(control, address, pbuff, &len, longAddr, FALSE/*b_DMA_used*/);

#if DRV_mutex
    //if (b_DMA_used == FALSE)
    {
        //TRACE_ERROR((0, "I2C demutex"));
        DRV_I2C1_sema_mutex_give();
        //When dma be used, please give the mutex at DMA TX IRQ
    }
#endif

    if ( status != sEE_OK)
    {
        return FALSE;
    }
    
    return TRUE;
}

bool I2C_readSlave(    byte control, 
                        uint16 address, 
                        byte *data,
                        uint16 length, 
                        bool longAddr) 
{
    int status = I2C_SUCCESS;
    uint16 NumByteToRead = length;
    uint8 *pBuffer;

    if ( bIsI2C_enable != TRUE )
    {
        TRACE_ERROR((0, "I2C interface has not been enable !!"));
        return FALSE;
    }

    if ( data == NULL )
    {
         TRACE_ERROR((0, "I2C reads buffer is null !!"));
        return FALSE;
    }

    ir_decode_wait(100);

#if DRV_mutex
    if (DRV_I2C1_sema_mutex_take() != TRUE)
    {
            TRACE_DEBUG((0, "I2C_readSlave"));
        return FALSE;
    }
#endif
    //vTaskDelay(1);
    pBuffer = data;

    status = __STM32_I2C_readSlave(control, address, pBuffer, &NumByteToRead, longAddr);

#if DRV_mutex
    DRV_I2C1_sema_mutex_give();
#endif

    if ( status != sEE_OK)
    {
        return FALSE;
    }
    
    return TRUE;
}

bool I2C_writeSlave_NoSystemWait(byte control, uint16 address, byte *data, uint16 length, bool longAddr) 
{
    int status = I2C_SUCCESS;
    uint16 len = length;
    uint8 *pbuff;

    if ( bIsI2C_enable != TRUE )
    {
        TRACE_ERROR((0, "I2C interface has not been enable !!"));
        return FALSE;
    }
    
#if DRV_mutex
        if (DRV_I2C1_sema_mutex_take() != TRUE)
        {
            return FALSE;
        }
#endif
    
    pbuff = data;

    status = __STM32_I2C_writeSlave(control, address, pbuff, &len, longAddr, FALSE);

#if DRV_mutex
        DRV_I2C1_sema_mutex_give();
#endif

    if ( status != sEE_OK)
    {
        return FALSE;
    }
    
    return TRUE;
}

