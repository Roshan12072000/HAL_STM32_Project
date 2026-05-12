#include "stm32l4xx_hal.h"
#include <stdio.h>

#define BUFFER_SIZE 	32


static const uint32_t aSRC_Const_Buffer[BUFFER_SIZE] = {
    0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F10,
    0x11121314, 0x15161718, 0x191A1B1C, 0x1D1E1F20,
    0x21222324, 0x25262728, 0x292A2B2C, 0x2D2E2F30,
    0x31323334, 0x35363738, 0x393A3B3C, 0x3D3E3F40,
    0x41424344, 0x45464748, 0x494A4B4C, 0x4D4E4F50,
    0x51525354, 0x55565758, 0x595A5B5C, 0x5D5E5F60,
    0x61626364, 0x65666768, 0x696A6B6C, 0x6D6E6F70,
    0x71727374, 0x75767778, 0x797A7B7C, 0x7D7E7F80
};
static uint32_t sDST_Buffer[BUFFER_SIZE];

UART_HandleTypeDef huart2;

int __io_putchar(int ch){
	HAL_UART_Transmit (&huart2, (uint8_t *)&ch, 1, 10);
	return ch;
}


DMA_HandleTypeDef DmaHandle;

void DMA_Init(void);
void SystemClock_Config(void);
void uart_init(void);
static void TransferComplete(DMA_HandleTypeDef *DmaHandle);
static void TransferError(DMA_HandleTypeDef *DmaHandle);

volatile int tx_done = 0;


int main(void)
{
    HAL_Init();
    SystemClock_Config();
    uart_init();
    printf("uart\r\n");
    DMA_Init();

    while (1)
    {


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
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling  = UART_OVERSAMPLING_16;

	HAL_UART_Init(&huart2);
}
void DMA_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* ================= TX ================= */
    DmaHandle.Instance = DMA1_Channel5;
    DmaHandle.Init.Request = DMA_REQUEST_2;   // 🔥 important
    DmaHandle.Init.Direction = DMA_MEMORY_TO_MEMORY;
    DmaHandle.Init.PeriphInc = DMA_PINC_ENABLE;
    DmaHandle.Init.MemInc = DMA_MINC_ENABLE;
    DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    DmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    DmaHandle.Init.Mode = DMA_NORMAL;
    DmaHandle.Init.Priority = DMA_PRIORITY_HIGH;
   // DmaHandle.Init.MemBurst = DMA_MBURST_SINGLE;
    //DmaHandle.Init.PeriphBurst = DMA_PBURST_SINGLE;

    HAL_DMA_Init(&DmaHandle);


    // Register call back.
    HAL_DMA_RegisterCallback(&DmaHandle, HAL_DMA_XFER_CPLT_CB_ID, TransferComplete);
    HAL_DMA_RegisterCallback(&DmaHandle, HAL_DMA_XFER_ERROR_CB_ID, TransferError);

    /* ================= NVIC ================= */
    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

    HAL_DMA_Start_IT(&DmaHandle,(uint32_t)aSRC_Const_Buffer,(uint32_t)sDST_Buffer,BUFFER_SIZE);

}
static void TransferComplete(DMA_HandleTypeDef *DmaHandle)
{
	// Do something.
	printf("Transfer complete!! \n\r");
}
static void TransferError(DMA_HandleTypeDef *DmaHandlee)
{
	// Do something.
	printf("Transfer Error!! \n\r");
}

void DMA1_CH5_IRQHandler(void){
	HAL_DMA_IRQHandler(&DmaHandle);
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

