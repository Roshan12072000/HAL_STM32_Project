
#include "uart.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include "time.h"

volatile uint8_t flag = 0;

int main(void)
{
    HAL_Init();

    SystemClock_Config();   // ✅ FIXED ORDER

    uart_init();
    tim_timebase_init();

    while(1)
    {
        if(flag)
        {
            flag = 0;
            printf("A second just elapsed!\r\n");
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    flag = 1;   // no printf here
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
