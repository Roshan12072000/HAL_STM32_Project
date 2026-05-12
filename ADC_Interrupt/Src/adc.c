#include "stm32l4xx_hal.h"

ADC_HandleTypeDef hadc1;

//  Read function (NO start, NO polling in continuous mode)
uint32_t pa0_adc_read(void)
{
    return HAL_ADC_GetValue(&hadc1);
}



void adc_pa0_interrupt_init(void)
{
    // GPIO configuration (PA0 → Analog)
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin  = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // ADC configuration
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_ADC_CLK_ENABLE();
//    __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_SYSCLK);

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
//    hadc1.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait      = DISABLE;

    //  Continuous mode enabled
    hadc1.Init.ContinuousConvMode    = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;

    hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;

    hadc1.Init.NbrOfConversion       = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;

   // hadc1.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;

    HAL_ADC_Init(&hadc1);

    // Channel configuration (PA0 → ADC1_IN5 ✅)
    sConfig.Channel      = ADC_CHANNEL_5;
    sConfig.Rank         =  1;

    //  Correct sampling time for L4
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;


    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
}

void ADC1_2_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&hadc1);
}
