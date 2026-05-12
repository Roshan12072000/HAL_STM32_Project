#include "stm32l4xx_hal.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

//  Read function (NO start, NO polling in continuous mode)
uint32_t pa0_adc_read(void)
{
    return HAL_ADC_GetValue(&hadc1);
}



void adc_pa0_DMA_init(void)
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
    hadc1.Init.DMAContinuousRequests = ENABLE;

   // hadc1.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;

    HAL_ADC_Init(&hadc1);

    // Channel configuration (PA0 → ADC1_IN5 ✅)
    sConfig.Channel      = ADC_CHANNEL_5;
    sConfig.Rank         =  1;

    //  Correct sampling time for L4
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;

    HAL_ADC_ConfigChannel(&hadc1, &sConfig);


    // DMA Configuration :
    __HAL_RCC_DMA2_CLK_ENABLE();

    hdma_adc1.Instance = DMA2_Channel3;

    hdma_adc1.Init.Request = DMA_REQUEST_0;

    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;

    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;

    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;

    hdma_adc1.Init.Mode = DMA_CIRCULAR;

    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&hdma_adc1);

    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

    HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);
}


void DMA2_CH3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_adc1);
}
