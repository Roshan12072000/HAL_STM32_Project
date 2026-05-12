
#include "uart.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include "time.h"

#define LIGHT_ADDR (0x47 << 1)
I2C_HandleTypeDef hi2cl;
void i2c_scan(void)
{
    printf("Scanning I2C bus...\r\n");

    for (uint8_t addr = 1; addr < 128; addr++)
    {
        if (HAL_I2C_IsDeviceReady(&hi2cl, addr << 1, 1, 50) == HAL_OK)
        {
            printf("Found device at 0x%X\r\n", addr);
        }
    }

    printf("Scan complete\r\n");
}


void i2c1_init(void);
void light_sensor_check(void);
void light_sensor_init(void);
void read_light_IT(void);

uint8_t reg = 0x00;
uint8_t data[2];

volatile int lux = 0;
volatile int i2c_stage = 0;
volatile int data_ready = 0;

void I2C1_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&hi2cl);
}

void I2C1_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&hi2cl);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    uart_init();
    i2c1_init();

    light_sensor_init(); // ✅ REQUIRED
    light_sensor_check();
    HAL_Delay(200);
    read_light_IT();  // start once

    while(1)
    {
        if(data_ready)
        {
            data_ready = 0;
            printf("Light: %d lux\r\n", lux);
        }
    }
}
void i2c1_init(void){
	/*PB8 ---> I2C1_SCL
	 PB9 -->I2C1_SDA
	 */
	__HAL_RCC_I2C1_FORCE_RESET();
	__HAL_RCC_I2C1_RELEASE_RESET();
	GPIO_InitTypeDef  GPIO_initStruct = {0};
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_initStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_initStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_initStruct.Pull = GPIO_PULLUP;
	GPIO_initStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_initStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init (GPIOB, &GPIO_initStruct);

	// Configure I2C
	__HAL_RCC_I2C1_CLK_ENABLE();

	hi2cl.Instance = I2C1;
	 hi2cl.Init.Timing = 0x10909CEC;
	//hi2cl.Init.ClockSpeed = 400000;
	//hi2cl.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2cl.Init.OwnAddress1 = 0;
	hi2cl.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2cl.Init.DualAddressMode =  I2C_DUALADDRESS_DISABLE;
	hi2cl.Init.OwnAddress2 = 0;
	hi2cl.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	 hi2cl.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	  hi2cl.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

		HAL_I2C_Init(&hi2cl);

	  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);

	  HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);


}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
void light_sensor_init(void)
{
    uint8_t config[3];

    config[0] = 0x01;   // config register
    config[1] = 0xC4;
    config[2] = 0x10;

    HAL_I2C_Master_Transmit(&hi2cl, LIGHT_ADDR, config, 3, 100);
}
void light_sensor_check(void)
{
    if (HAL_I2C_IsDeviceReady(&hi2cl,LIGHT_ADDR, 3, 100) == HAL_OK)
    {
        printf("Light sensor detected\r\n");
    }
    else
    {
        printf("Sensor NOT detected\r\n");
    }
}
void read_light_IT(void)
{
    if (HAL_I2C_GetState(&hi2cl) == HAL_I2C_STATE_READY)
    {
        i2c_stage = 1;

        // Step 1: send register address
        HAL_I2C_Master_Transmit_IT(&hi2cl, LIGHT_ADDR, &reg, 1);
    }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (i2c_stage == 1)
        {
            i2c_stage = 2;

            // Step 2: receive data
            HAL_I2C_Master_Receive_IT(&hi2cl, LIGHT_ADDR, data, 2);
        }
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (i2c_stage == 2)
        {
            // Step 1: Combine received bytes
            uint16_t raw = (data[0] << 8) | data[1];

            // Step 2: Extract exponent & mantissa
            uint16_t exponent = (raw >> 12) & 0x0F;
            uint16_t mantissa = raw & 0x0FFF;

            // Step 3: Calculate lux
            lux = (mantissa * (1 << exponent)) / 100;

            // Step 4: Notify main loop (NO printf here)
            data_ready = 1;

            // Step 5: Restart next read (correct order)
            i2c_stage = 1;
            HAL_I2C_Master_Transmit_IT(&hi2cl, LIGHT_ADDR, &reg, 1);
        }
    }
}
