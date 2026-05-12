#include "uart.h"
#include "stm32l4xx_hal.h"
#include "adc.h"



uint32_t sensor_value = 10;
int main(){

	HAL_Init();
	SystemClock_Config();
	uart_init();
	adc_init_start();
	while(1){
		sensor_value = pa0_adc_read();
		 //HAL_Delay(10);
	}

}
void SysTick_Handler(void)
{
    HAL_IncTick();
}
