#include "stm32l4xx_hal.h"

ADC_HandleTypeDef hadcl;

uint32_t pa0_adc_read(void){
	return HAL_ADC_GetValue(&hadcl);
}
static void adc_pa0_continous_conv_init(void);
void adc_init_start(void){
	adc_pa0_continous_conv_init();
	HAL_ADC_Start(&hadcl);
}
void adc_pa0_continous_conv_init(void)
{
    // Configure pa0 as analog pin

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configure adc module for continuous conversion
    ADC_ChannelConfTypeDef sConfig = {0};
    __HAL_RCC_ADC_CLK_ENABLE();

    hadcl.Instance = ADC1;
    hadcl.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadcl.Init.Resolution = ADC_RESOLUTION_12B;
    hadcl.Init.ContinuousConvMode = ENABLE;
    hadcl.Init.DiscontinuousConvMode = DISABLE;
    hadcl.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadcl.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadcl.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadcl.Init.NbrOfConversion = 1;
    hadcl.Init.DMAContinuousRequests = DISABLE;


    HAL_ADC_Init(&hadcl);

    sConfig.Channel = ADC_CHANNEL_5;
    sConfig.Rank = 1;
    sConfig.SamplingTime =  ADC_SAMPLETIME_640CYCLES_5;


    HAL_ADC_ConfigChannel(&hadcl,&sConfig);
}
