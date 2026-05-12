#include "stm32l4xx_hal.h"
#include <stdio.h>

# define BTN_PIN GPIO_PIN_13
# define BTN_PORT GPIOC


void gpio_pc13_interrupt_init(void){
	GPIO_InitTypeDef  GPIO_initStruct = {0};
	__HAL_RCC_GPIOC_CLK_ENABLE();
	// Configure pc13
	GPIO_initStruct.Pin = BTN_PIN;
	GPIO_initStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_initStruct.Pull = GPIO_NOPULL;
	GPIO_initStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init (BTN_PORT, &GPIO_initStruct);

	// configure EXTI
	HAL_NVIC_SetPriority(EXTI15_10_IRQn,0,0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	// Do something...
	if(GPIO_Pin == GPIO_PIN_13){
//	printf("Button 13 Pressed!! \n\r");
	}

}
void EXTI15_10_IRQHandler(void){

	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}
