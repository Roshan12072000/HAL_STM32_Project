
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
int read_light(void);
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    uart_init();
    i2c1_init();

    light_sensor_init(); // ✅ REQUIRED
    light_sensor_check();
    HAL_Delay(200);
    while(1)
    {
        int lux = read_light();
      printf("Light: %d lux\r\n", lux);
        HAL_Delay(500);
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
}int read_light(void)
{
    uint8_t reg = 0x00;
    uint8_t data[2];

    HAL_I2C_Master_Transmit(&hi2cl, LIGHT_ADDR, &reg, 1, 100);
    HAL_I2C_Master_Receive(&hi2cl, LIGHT_ADDR, data, 2, 100);

    uint16_t raw = (data[0] << 8) | data[1];

    uint16_t exponent = (raw >> 12) & 0x0F;
    uint16_t mantissa = raw & 0x0FFF;

    int lux = (mantissa * (1 << exponent)) / 100;

    return lux;
}
