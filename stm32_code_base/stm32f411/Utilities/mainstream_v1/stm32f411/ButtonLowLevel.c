#include "ButtonLowLevel.h"
#include "Debug.h"
#include "config.h"
#include "GPIOMiddleLevel.h"
#define DMA_adc

/* Private variables ________________________________________________________*/
static uint16 adc_value = 0;
static uint16 ADCConvertedValues[2];
/*_______________________________________________________________________*/



/**
 * Buttons Table - Edit to Add or Remove buttons
 */
const buttonsType buttonsTable[] =
{
    //INDEX              Min    Max   ADC   key
    {BTN_POWER,             0,     5}, //POWER Key
    {BTN_INPUT_SRC,       297,   315}, //306    //Input Key
    {BTN_BT,              209,   239}, //222    //BT Paring Key
    {BTN_VOL_UP,          139,   171}, //148    //Vol+ Key
    {BTN_VOL_DOWN,         66,    85}, //76     //Vol- Key
    {BTN_MULTI_RET,       113,   138}, //129    //Vol- + BT
    {BTN_MULTI_DEMO1,     273,   296}, //288    //input + BT
    {BTN_MULTI_DEMO2,     240,   272}, //257    //input + vol+
    {BTN_MULTI_DEMO3,     172,   208}, //196    //input + vol-
    {BTN_MULTI_VER,        86,   112}, //96     //vol+ + vol-
//    {BTN_VIZIO_TV_REMOTE,       255,   255},  //255  //POWER + vol+
//    {BTN_ENERGY_STAR,       255,   255}  //255  //POWER + vol-
};
static uint16 ButtonLowLevel_get_ADC_Value( uint8 buttonID )
{
    switch( buttonID )
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8: 
        case 9: 
        case 10:
        case 11:
        {
            adc_value = (ADCConvertedValues[1]/10);
        }
            break;

        case 0:
        {
            adc_value = (ADCConvertedValues[0]/10);
        }
            break;
    }
    return adc_value;
}

uint8 ButtonLowLevel_numberOfButtons()
{
    return ((sizeof(buttonsTable) / sizeof(buttonsType)));
}


bool ButtonLowLevel_isPressed(uint8 buttonID)
{
    bool retVal = FALSE;
    buttonsType btn;
    uint32 value = 0xFFFFFFFF;

    btn = buttonsTable[buttonID];

    value = ButtonLowLevel_get_ADC_Value( buttonID );

    if ( (value >= btn.adc_valueMin)&& (value <= btn.adc_valueMax) )
    {
        retVal = TRUE;
    }

    return retVal;
}

void ButtonLowLevel_initialIze( void )
{
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;    
    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef DMA_adc
    DMA_InitTypeDef       DMA_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
#endif
    /* Enable ADC1, ADC2, and GPIOC clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

#ifdef DMA_adc
    /* DMA2_Stream4 channel0 configuration RM383 P167****************************/
    DMA_DeInit(DMA2_Stream4); 
    DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADCConvertedValues[0];
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = 2;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;         
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream4, &DMA_InitStructure);
    /* DMA2_Stream4 enable */
    DMA_Cmd(DMA2_Stream4, ENABLE);
#endif

    /*Configuration ADC channel*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1| GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;    
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* ADC Common Init ********************************************************/
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 2;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_3Cycles);    /* ADC1 regular channels configuration */ 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_3Cycles);    /* ADC1 regular channels configuration */ 
#ifdef DMA_adc
    /* Enable DMA request after last transfer (Single-ADC mode) */
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);
#endif
    ADC_Cmd(ADC1, ENABLE);

    /* Start ADC1 Software Conversion */ 
    ADC_SoftwareStartConv(ADC1);

    //! @} 
}

uint8 ButtonLowLevel_get_btn_ADC_Value( void )
{
    uint8 KeyValue =0;
    KeyValue = adc_value /10;
    return KeyValue;
}

