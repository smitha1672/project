#include "ButtonLowLevel.h"
#include "SOUND_BAR_V1_PinDefinition.h"
#include "Debug.h"
#include "config.h"


/* Private variables ________________________________________________________*/
static uint16 adc_value = 0;

/*_______________________________________________________________________*/



/**
 * Buttons Table - Edit to Add or Remove buttons
 */
const buttonsType buttonsTable[] =
{
    //INDEX              Min    Max   ADC   key                  Convert(voltage)
    {BTN_POWER,             0,     2}, //POWER Key
    {BTN_INPUT_SRC,       297,   315}, //306    //Input Key     (2.396~2.468V~2.541)        
    {BTN_BT,              209,   239}, //222    //BT Paring Key (1.686~1.791V~1.928)
    {BTN_VOL_UP,          139,   171}, //148    //Vol+ Key      (1.121~1.194V~1.379)
    {BTN_VOL_DOWN,         66,    85}, //76     //Vol- Key      (0.532~0.613V~0.685)
    {BTN_MULTI_RET,       113,   138}, //129    //Vol- + BT     (0.911~1.041V~1.113)
    {BTN_MULTI_DEMO1,     273,   296}, //288    //input + BT    (2.202~2.323V~2.388)
    {BTN_MULTI_DEMO2,     240,   272}, //257    //input + vol+  (1.936~2.073V~2.194)
    {BTN_MULTI_DEMO3,     172,   208}, //196    //input + vol-  (1.387~1.581V~1.678)
    {BTN_MULTI_VER,        86,   112}, //96     //vol+ + vol-   (0.693~0.774V~0.903)
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
            adc_value = (ADC_GetConversionValue(ADC2)/10);
        }
            break;

        case 0:
        {
            adc_value = (ADC_GetConversionValue(ADC1)/10);
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
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable ADC1, ADC2, and GPIOC clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | ADC_PORT_CLK, ENABLE);

    /*Configuration ADC channel*/
    GPIO_InitStructure.GPIO_Pin = ADC1_IN0 | ADC2_IN1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(ADC_PORT, &GPIO_InitStructure);

    //! ADC1 configuration  @{
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_28Cycles5);    /* ADC1 regular channels configuration */ 
    //! @}

    //! ADC2 configuration  @{
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC2, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_28Cycles5);    /* ADC2 regular channels configuration */ 
    //! @}

    //!    Enable ADC1@{
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);                    /* Enable ADC1 reset calibration register */   
    while(ADC_GetResetCalibrationStatus(ADC1));    /* Check the end of ADC1 reset calibration register */
    
    ADC_StartCalibration(ADC1);                    /* Start ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));        /* Check the end of ADC1 calibration */
    //!@}

    //!    Enable ADC2@{
    ADC_Cmd(ADC2, ENABLE);
    ADC_ResetCalibration(ADC2);                    /* Enable ADC2 reset calibration register */   
    while(ADC_GetResetCalibrationStatus(ADC2));    /* Check the end of ADC2 reset calibration register */

    ADC_StartCalibration(ADC2);                    /* Start ADC2 calibration */
    while(ADC_GetCalibrationStatus(ADC2));        /* Check the end of ADC2 calibration */
    //!@}

    /* Start ADC1 Software Conversion */ 
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* Start ADC2 Software Conversion */ 
    ADC_SoftwareStartConvCmd(ADC2, ENABLE);
    
}

uint8 ButtonLowLevel_get_btn_ADC_Value( void )
{
    uint8 KeyValue =0;
    KeyValue = adc_value /10;
    return KeyValue;
}

