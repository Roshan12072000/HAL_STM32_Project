#include "stm32l4xx_hal.h"

void uart_init(void);
void SystemClock_Config(void);
UART_HandleTypeDef huart2;
int main(){


	HAL_Init();
	SystemClock_Config();
	uart_init();
	char msg[20] = "Hello From Stm32\r\n";
	while(1){
		 HAL_UART_Transmit (&huart2, (uint8_t *) msg, 20, 100);
		 HAL_Delay(10);
	}
}

void uart_init(void){


	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//ENABLE UART PIN CLOCK Access
	__HAL_RCC_GPIOA_CLK_ENABLE();
	// ENABLE UART MODULE CLOCK ACCESS
	__HAL_RCC_USART2_CLK_ENABLE();

	// Configure pins to act as alternate function pin (uart)

	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);


	//configure UART MODULE
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling  = UART_OVERSAMPLING_16;

	HAL_UART_Init(&huart2);
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
void SystemClock_Config(void) {
	RCC_OscInitTypeDef osc = { 0 };
	RCC_ClkInitTypeDef clk = { 0 };

	osc.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	osc.MSIState = RCC_MSI_ON;
	osc.MSIClockRange = RCC_MSIRANGE_6; // 4 MHz
	osc.PLL.PLLState = RCC_PLL_NONE;

	HAL_RCC_OscConfig(&osc);

	clk.ClockType = RCC_CLOCKTYPE_SYSCLK |
	RCC_CLOCKTYPE_HCLK |
	RCC_CLOCKTYPE_PCLK1 |
	RCC_CLOCKTYPE_PCLK2;

	clk.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
	clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
	clk.APB1CLKDivider = RCC_HCLK_DIV1;
	clk.APB2CLKDivider = RCC_HCLK_DIV1;

	HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0);
}
