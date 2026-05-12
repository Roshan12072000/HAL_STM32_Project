#include "stm32l4xx_hal.h"
#include <stdio.h>
#include "uart.h"
#include "time.h"

//int __io_putchar(int ch){
//	HAL_UART_Transmit (&huart2, (uint8_t *)&ch, 1, 10);
//	return ch;
//}
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
// ---------- GPIO for Ultrasonic ----------
void ultrasonic_gpio_init(void);

// ---------- printf redirection ----------
extern UART_HandleTypeDef huart2;


int read_distance(void)
{
    uint32_t time = 0;
    uint32_t timeout;

    // Trigger pulse (VERY IMPORTANT)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    delay_us(2);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    delay_us(10);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

    //  Wait for ECHO HIGH
    timeout = 100000;
    while (!(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1)) && timeout--);

    if(timeout == 0)
        return -1;

    //  Start timer
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    //  Wait for ECHO LOW
    timeout = 100000;
    while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) && timeout--);

    if(timeout == 0)
        return -1;

    //  Read time
    time = __HAL_TIM_GET_COUNTER(&htim2);

    //  Convert to distance
    return time / 58;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    uart_init();
    tim2_us_init();              // microsecond timer
    ultrasonic_gpio_init();

    while (1)
    {
        int distance = read_distance();

       printf("Distance: %d cm\r\n", distance);
    	HAL_Delay(500);
    }
}

// ---------- GPIO INIT ----------
void ultrasonic_gpio_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    // TRIG → PA0 (Output)
    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    // ECHO → PA1 (Input)
    gpio.Pin = GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio);
}

// ---------- SysTick ----------
void SysTick_Handler(void)
{
    HAL_IncTick();
}
