#include "time.h"
#include "stm32l4xx_hal.h"

TIM_HandleTypeDef TimHandle;

void tim_timebase_init(void)
{
    // 1. Enable TIM3 clock
    __HAL_RCC_TIM3_CLK_ENABLE();

    //2. Configure Timer (1 Hz)
    TimHandle.Instance = TIM3;

    TimHandle.Init.Prescaler = 16000 - 1;   // 16 MHz / 16000 = 1 kHz
    TimHandle.Init.Period    = 1000 - 1;    // 1 kHz / 1000 = 1 Hz

    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&TimHandle);

    // 3. NVIC configuration
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    // 4. Start timer in interrupt mode
    HAL_TIM_Base_Start_IT(&TimHandle);
}

// Correct IRQ Handler (NO generic name)
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TimHandle);
}
