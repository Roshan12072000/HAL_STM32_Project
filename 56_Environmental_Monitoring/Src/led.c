#include "stm32l4xx_hal.h"
#include "led.h"

void pa5_led_init(void){
	GPIO_InitTypeDef  GPIO_initStruct = {0};
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_initStruct.Pin = LED_PIN;
	GPIO_initStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_initStruct.Pull = GPIO_NOPULL;
	GPIO_initStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init (LED_PORT, &GPIO_initStruct);
}
