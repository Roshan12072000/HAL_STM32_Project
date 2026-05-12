#include "uart.h"
#include "stm32l4xx_hal.h"
#include "adc.h"


//ADC_HandleTypeDef hadc1;
uint32_t sensor_value = 0;
int main(){

	HAL_Init();
	SystemClock_Config();
	uart_init();
	adc_pa0_single_conv_init();
	while(1){
		//1. Start ADC
		HAL_ADC_Start(&hadc1);
		//2. Poll for Conversion
		HAL_ADC_PollForConversion(&hadc1,1);
		//3. Get conversion
		sensor_value = pa0_adc_read();
		 //HAL_Delay(10);
	}

}
void SysTick_Handler(void)
{
    HAL_IncTick();
}

