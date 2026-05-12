#include "stm32l4xx_hal.h"
#include <stdio.h>
UART_HandleTypeDef huart2;

int __io_putchar(int ch){
	HAL_UART_Transmit (&huart2, (uint8_t *)&ch, 1, 10);
	return ch;
}


DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

uint8_t rx_counter = 0, tx_counter = 0;
//uint8_t tx_buffer[] = "Hello from DMA TX\r\n";
uint8_t tx_buffer[] = {10,20,30,40,50,60,70,80,90,100};
uint8_t rx_buffer[10];
void DMA_Init(void);
void SystemClock_Config(void);
void uart_init(void);
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}

void DMA2_CH7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart2_tx);
}

void DMA2_CH6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart2_rx);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    tx_counter++;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	    {
	        rx_counter++;
	    }
}




volatile int tx_done = 0;
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    uart_init();
    DMA_Init();
   //HAL_UART_Transmit(&huart2, (uint8_t*)"UART OK\r\n", 9, 100);
    /* Start DMA */
    HAL_UART_Transmit_DMA(&huart2, tx_buffer, 10);
  HAL_UART_Receive_DMA(&huart2, rx_buffer,10);

    while (1)
    {
    	;
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
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);


	//configure UART MODULE
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling  = UART_OVERSAMPLING_16;

	HAL_UART_Init(&huart2);

	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
}
void DMA_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* ================= TX ================= */
    hdma_usart2_tx.Instance = DMA1_Channel7;
    hdma_usart2_tx.Init.Request = DMA_REQUEST_2;   // 🔥 important
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&hdma_usart2_tx);

    /* ================= RX ================= */
    hdma_usart2_rx.Instance = DMA1_Channel6;
    hdma_usart2_rx.Init.Request = DMA_REQUEST_2;   // 🔥 important
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&hdma_usart2_rx);

    /* ================= LINK ================= */
    __HAL_LINKDMA(&huart2, hdmatx, hdma_usart2_tx);
    __HAL_LINKDMA(&huart2, hdmarx, hdma_usart2_rx);

    /* ================= NVIC ================= */
    HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

    HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
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

