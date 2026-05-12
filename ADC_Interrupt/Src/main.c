#include "uart.h"
#include "stm32l4xx_hal.h"
#include "adc.h"


//ADC_HandleTypeDef hadc1;
uint32_t sensor_value;
int main(){

	HAL_Init();
	SystemClock_Config();
	uart_init();
	adc_pa0_interrupt_init();
	//1. Start ADC
			HAL_ADC_Start_IT(&hadc1);
	while(1){

		//2. Poll for Conversion
//		HAL_ADC_PollForConversion(&hadc1,1);
		//3. Get conversion

		 //HAL_Delay(10);
	}

}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1){
	//get conversion
	sensor_value = pa0_adc_read();
}
void SysTick_Handler(void)
{
    HAL_IncTick();
}

