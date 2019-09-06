#ifndef __BSP_H__
#define __BSP_H__

//! __STM32__	@{
#if defined (__ARM_CORTEX_MX__)
#include "Defs.h"
#include "stm32f0xx.h"
#include "core_cm0.h"
#endif	//! @}

#define SPI_LITTEL_ENDIAN			TRUE
#define SPI_BIG_ENDIAN				FALSE
#define SPI_RET_ERROR_LEN			-1

typedef enum { 
    eOUT_PIN = 0,
    eIN_PIN  = 1,
    eOD_PIN = 2,
} IO_Pin;

typedef struct{
    GPIO_TypeDef* PORTx; 
    uint16_t PINx;
    IO_Pin IO;
} GPIO_PhysicalPin; 

typedef enum {     
    __O_EN_1V,
    __O_RST_DSP,
    __I_SCP1_IRQ,
    __O_SPI1_DSP_CS,
    __O_FLASH_CS,    
    __O_SPI1_SCK,
    __I_SCP1_BUSY,
    __O_SCP2_FLASH_CS,    
    __O_MCU_SLAVE_MODE,
    __O_MCU_MASTER_MODE,
    __IO_MAX,
} VirtualPin;

typedef struct _bsp_board_config
{
    void ( *board_config ) ( void );
}BSP_BOARD_OBJECT;


typedef struct _bsp_gpio_obj{
    void (*initialize)( void );
    void (*set) ( VirtualPin pin );
    void (*clr) ( VirtualPin pin );
    uint8_t ( *read )( VirtualPin pin );
}BSP_GPIO_OBJECT;


typedef struct _bsp_spi_obj{
    void ( *dsp_spi_config )( void );
    void ( *dsp_spi_deconfig ) ( void );
    bool ( *dsp_spi_IsEnable ) ( void );
    bool ( *dsp_spi_write ) ( const byte data );
    bool ( *dsp_spi_read )( byte *data );
    uint8_t ( *dsp_spi_read_dummy ) ( void ); 
    int16 ( *dsp_spi_write_n_bytes ) ( const byte *data, uint16 length, bool LittelEndian);
    int16 ( *dsp_spi_read_n_bytes ) ( byte *data, uint16 length, bool LittelEndian );
    bool ( *spi_mutex_lock) ( void );
    bool ( *spi_mutex_unlock ) ( void );
    void ( *spi_2nd_config )( void );
    void ( *spi_2nd_deconfig ) ( void );
    bool ( *spi_2nd_write ) ( byte data );
    bool ( *spi_2nd_read )( byte *data );
    int16 ( *spi_2nd_write_n_bytes ) ( byte *data, uint16 length, bool LittelEndian);
    int16 ( *spi_2nd_read_n_bytes ) ( byte *data, uint16 length, bool LittelEndian );
    bool ( *dsp_select_chip ) ( void );
    bool ( *dsp_unselect_chip ) ( void );
}BSP_SPI_OBJECT;


#endif
