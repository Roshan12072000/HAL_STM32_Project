
#include "uart.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include "time.h"

SPI_HandleTypeDef hspi1;
#define CS_PIN  GPIO_PIN_6
#define CS_PORT GPIOB

#define CS_LOW()  HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET)
#define CS_HIGH() HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET)

void W25Q_WriteEnable(void);
uint8_t W25Q_ReadStatus(void);
void W25Q_WaitBusy(void);
void W25Q_EraseSector(uint32_t addr);
void W25Q_Write(uint32_t addr, uint8_t *data, uint16_t len);
void W25Q_Read(uint32_t addr, uint8_t *data, uint16_t len);
void W25Q_ReadID(void);

void spi1_init(void);

uint8_t tx_data[] = "Hello SPI TEST!";
uint8_t rx_data[50];

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    uart_init();
		spi1_init();
		HAL_Delay(10);
		W25Q_ReadID();
		W25Q_EraseSector(0x000000);   // erase first sector

		    W25Q_Write(0x000000, tx_data, sizeof(tx_data));

		    W25Q_Read(0x000000, rx_data, sizeof(tx_data));
		    rx_data[sizeof(tx_data) - 1] = '\0';
		    printf("%s\r\n", rx_data);
    while(1)
    {
    	;
    }
}
void spi1_init(void){
	GPIO_InitTypeDef  GPIO_initStruct = {0};
		__HAL_RCC_SPI1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		/*
		 * PA5 ---> SPI_SCK
		 * PA6 ---> SPI1_MISO
		 * PA7 ---> SPI1_MOSI
		 */
		GPIO_initStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_initStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_initStruct.Alternate = GPIO_AF5_SPI1;
		GPIO_initStruct.Pull = GPIO_NOPULL;
		GPIO_initStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init (GPIOA, &GPIO_initStruct);

		// CS Pin (manual)
		    GPIO_initStruct.Pin = CS_PIN;
		    GPIO_initStruct.Mode = GPIO_MODE_OUTPUT_PP;
		    GPIO_initStruct.Pull = GPIO_NOPULL;
		    GPIO_initStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		    HAL_GPIO_Init(CS_PORT, &GPIO_initStruct);
		    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);

		//Configure SPI MODE
		hspi1.Instance = SPI1;
		hspi1.Init.Mode = SPI_MODE_MASTER;
		hspi1.Init.Direction = SPI_DIRECTION_2LINES;
		hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
		 hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
		hspi1.Init.NSS = SPI_NSS_SOFT;
		hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
		hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
		hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
		 hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

		if (HAL_SPI_Init(&hspi1) != HAL_OK)
		    {
		        printf("SPI Failed to initiate\r\n");
		    }
}

void W25Q_WriteEnable(void)
{
    uint8_t cmd = 0x06;

    CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    CS_HIGH();
    HAL_Delay(1);
}

uint8_t W25Q_ReadStatus(void)
{
    uint8_t cmd = 0x05;
    uint8_t status;

    CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &status, 1, HAL_MAX_DELAY);
    CS_HIGH();

    return status;
}

void W25Q_WaitBusy(void)
{
	uint32_t timeout = HAL_GetTick();

	    while (W25Q_ReadStatus() & 0x01) // BUSY bit
	    {
	        if (HAL_GetTick() - timeout > 1000)
	        {
	            printf("Flash timeout\r\n");
	            break;
	        }
	    }
}

void W25Q_EraseSector(uint32_t addr)
{
    uint8_t cmd[4];

    W25Q_WriteEnable();

    cmd[0] = 0x20; // sector erase
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
    CS_HIGH();

    W25Q_WaitBusy();
}

void W25Q_Write(uint32_t addr, uint8_t *data, uint16_t len)
{
    uint8_t cmd[4];

    W25Q_WriteEnable();

    cmd[0] = 0x02; // page program
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
    CS_HIGH();

    W25Q_WaitBusy();
}

void W25Q_Read(uint32_t addr, uint8_t *data, uint16_t len)
{
    uint8_t cmd[4];

    cmd[0] = 0x03; // read
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, data, len, HAL_MAX_DELAY);
    CS_HIGH();
}

void W25Q_ReadID(void)
{
    uint8_t cmd = 0x9F;
    uint8_t id[3] = {0xAA, 0xAA, 0xAA};

    CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, id, 3, HAL_MAX_DELAY);
    CS_HIGH();

    //printf("ID: %02X %02X %02X\r\n", id[0], id[1], id[2]);
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

