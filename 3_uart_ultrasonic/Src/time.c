#include "time.h"
#include "stm32l4xx_hal.h"

TIM_HandleTypeDef htim2;

void tim2_us_init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;

    // Your clock = 4 MHz (MSI range 6)
    // 4 MHz / 4 = 1 MHz → 1 tick = 1 µs
    htim2.Init.Prescaler = 4 - 1;
    htim2.Init.Period = 0xFFFFFFFF;

    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    HAL_TIM_Base_Init(&htim2);

    HAL_TIM_Base_Start(&htim2);   // NO interrupt
}

void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}
