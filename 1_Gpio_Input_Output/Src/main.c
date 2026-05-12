#include "stm32l4xx_hal.h"

# define BTN_PIN GPIO_PIN_13
# define BTN_PORT GPIOC
# define LED_PIN GPIO_PIN_5
# define LED_PORT GPIOA

void pa5_led_init(void);
void pc13_btn_init(void);

uint8_t button_Status;

int main(){

	HAL_Init();
	pa5_led_init();
	pc13_btn_init();
	while(1){
		button_Status = HAL_GPIO_ReadPin(BTN_PORT,BTN_PIN);
		HAL_GPIO_WritePin (LED_PORT, LED_PIN, !button_Status);
	}
}
void pa5_led_init(void){
	GPIO_InitTypeDef  GPIO_initStruct = {0};
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_initStruct.Pin = LED_PIN;
	GPIO_initStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_initStruct.Pull = GPIO_NOPULL;
	GPIO_initStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init (LED_PORT, &GPIO_initStruct);
}

void pc13_btn_init(void){
	GPIO_InitTypeDef  GPIO_initStruct = {0};
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_initStruct.Pin = BTN_PIN;
	GPIO_initStruct.Mode = GPIO_MODE_INPUT;
	GPIO_initStruct.Pull = GPIO_NOPULL;
	GPIO_initStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init (BTN_PORT, &GPIO_initStruct);

}
void SysTick_Handler(void)
{
    HAL_IncTick();
}
