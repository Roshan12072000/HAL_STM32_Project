#include "uart.h"
#include "stm32l4xx_hal.h"
#include "adc.h"
#include<stdio.h>

//ADC_HandleTypeDef hadc1;
uint32_t sensor_value;
int main(){

	HAL_Init();
	SystemClock_Config();
	uart_init();
	adc_pa0_DMA_init();
	//1. Start ADC
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&sensor_value, 1);
	while(1){
		 printf("%lu\r\n", sensor_value);

		    HAL_Delay(500);
	}

}
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1){
//	//get conversion
//	sensor_value = pa0_adc_read();
//}
void SysTick_Handler(void)
{
    HAL_IncTick();
}

