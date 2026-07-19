
#include "uart.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include "exti.h"
#include "time.h"
extern I2C_HandleTypeDef hi2cl;
SPI_HandleTypeDef hspi1;
#define CS_PIN  GPIO_PIN_6
#define CS_PORT GPIOB

#define FLASH_BASE_ADDR   0x000000
#define MAX_LOGS          5

uint32_t flash_write_addr = FLASH_BASE_ADDR;

#define CS_LOW()  HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET)
#define CS_HIGH() HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET)

void W25Q_WriteEnable(void);
uint8_t W25Q_ReadStatus(void);
void W25Q_WaitBusy(void);
void W25Q_EraseSector(uint32_t addr);
void W25Q_Write(uint32_t addr, uint8_t *data, uint16_t len);
void W25Q_Read(uint32_t addr, uint8_t *data, uint16_t len);
void W25Q_ReadID(void);
//void Print_Recent_Logs(void);
void spi1_init(void);
void Store_Log(uint16_t lux);
//void Store_Lux(uint16_t lux);
void Print_Recent_Logs(void);

#define RTC_ASYNCH_PREDIV 127
#define RTC_SYNCH_PREDIV  249   // depends on measured LSI

#define BK_FLAG 0x8888

RTC_HandleTypeDef RtcHandle;
uint8_t time[15] = {0};
uint8_t	date[15] = {0};
void rtc_init(void);
void rtc_calendar_show(uint8_t *showtime,uint8_t *showdate);
void rtc_calendar_config(void);


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	// Do something...
	if(GPIO_Pin == GPIO_PIN_13){
		Print_Recent_Logs();
	}

}
int lux;
typedef struct
{
    uint8_t year;      // 26
    uint8_t month;     // 6
    uint8_t day;       // 11

    uint8_t hours;     // 10
    uint8_t minutes;   // 13
    uint8_t seconds;   // 20

    uint16_t lux;      // 346
} LogRecord_t;
int main(void)
{
	 HAL_Init();
	    SystemClock_Config();

	    uart_init();
	    i2c1_init();
	    rtc_init();rtc_calendar_config();
	    	__HAL_RTC_RESET_HANDLE_STATE(&RtcHandle);

	    light_sensor_init(); // ✅ REQUIRED
	    light_sensor_check();
	    gpio_pc13_interrupt_init();
		spi1_init();
		HAL_Delay(10);
		W25Q_ReadID();
		W25Q_EraseSector(FLASH_BASE_ADDR);   // erase first sector

		//Read backup register
			if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR0) != BK_FLAG){
				rtc_calendar_config();
			}

		    while(1)
		       {
		    	rtc_calendar_show(time,date);
		    				 HAL_Delay(500);

		           lux = read_light();

		           /*
		            * Print current lux
		            */
		           printf("Lux = %d\r\n", lux);

		           /*
		            * Store into flash
		            */
//		           Store_Lux(lux);
		           Store_Log(lux);

		           /*
		            * Prevent sector overflow
		            */
		           if(flash_write_addr >= 0x1000)
		           {
		               flash_write_addr = FLASH_BASE_ADDR;

		               W25Q_EraseSector(FLASH_BASE_ADDR);
		           }

		           HAL_Delay(1000);
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

//void Store_Lux(uint16_t lux)
//{
//    W25Q_Write(flash_write_addr,
//               (uint8_t*)&lux,
//               2);
//
//    flash_write_addr += 2;
//}
void Store_Log(uint16_t lux)
{
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;

    LogRecord_t rec;

    HAL_RTC_GetTime(&RtcHandle,&time,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RtcHandle,&date,RTC_FORMAT_BIN);

    rec.year    = date.Year;
    rec.month   = date.Month;
    rec.day     = date.Date;

    rec.hours   = time.Hours;
    rec.minutes = time.Minutes;
    rec.seconds = time.Seconds;

    rec.lux     = lux;

    W25Q_Write(flash_write_addr,
               (uint8_t *)&rec,
               sizeof(LogRecord_t));

    flash_write_addr += sizeof(LogRecord_t);
}
//void Print_Recent_Logs(void)
//{
//    uint16_t lux;
//
//    char msg[50];
//
//    uint32_t addr;
//
//    uint8_t count;
//
//    if(flash_write_addr <= FLASH_BASE_ADDR)
//    {
//        return;
//    }
//
//    count = flash_write_addr / 2;
//
//    if(count > MAX_LOGS)
//    {
//        count = MAX_LOGS;
//    }
//
//    addr = flash_write_addr - 2;
//
//    for(uint8_t i = 0;
//        i < count;
//        i++)
//    {
//        W25Q_Read(addr,
//                  (uint8_t*)&lux,
//                  2);
//
//        sprintf(msg,
//                "Lux = %d\r\n",
//                lux);
//
//        printf("%s", msg);
//
//        if(addr >= 2)
//        {
//            addr -= 2;
//        }
//        else
//        {
//            break;
//        }
//    }
//}
void Print_Recent_Logs(void)
{
    LogRecord_t rec;

    uint32_t addr =
        flash_write_addr - sizeof(LogRecord_t);

    while(addr >= FLASH_BASE_ADDR)
    {
        W25Q_Read(addr,
                  (uint8_t *)&rec,
                  sizeof(LogRecord_t));

        printf("%02d/%02d/20%02d "
               "%02d:%02d:%02d "
               "Lux=%d\r\n",
               rec.day,
               rec.month,
               rec.year,
               rec.hours,
               rec.minutes,
               rec.seconds,
               rec.lux);

        if(addr < sizeof(LogRecord_t))
            break;

        addr -= sizeof(LogRecord_t);
    }
}
void rtc_init(void){
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_EnableBkUpAccess();
	// select RTC clock
	RCC_OscInitTypeDef  	RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef	PeriphClkInitStruct;
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;

	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	// Set RTC Clock src to LSI
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

	__HAL_RCC_RTC_ENABLE();

	//Configure calendor
	RtcHandle.Instance = RTC;
	RtcHandle.Init.HourFormat = RTC_HOURFORMAT_12;
	RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
	RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
	RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;

	HAL_RTC_Init(&RtcHandle);

}

void rtc_calendar_config(void){
	RTC_DateTypeDef sdatestructure;
	RTC_TimeTypeDef stimestructure;

	// Set Date : Sunday JULY 14th 2019
	sdatestructure.Year = 26;
	sdatestructure.Month = RTC_MONTH_JUNE;
	sdatestructure.Date = 11;
	sdatestructure.WeekDay = RTC_WEEKDAY_THURSDAY;

	HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BIN);

	//Set Time : 03:00:00
	stimestructure.Hours = 10;
	stimestructure.Minutes = 25;
	stimestructure.Seconds = 0;
	stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
	stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_RTC_SetTime(&RtcHandle,&stimestructure,RTC_FORMAT_BIN);

	//Write to Backup Register
	HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, BK_FLAG);

}

void rtc_calendar_show(uint8_t *showtime,uint8_t *showdate){
	RTC_DateTypeDef sdatestructureget;
	RTC_TimeTypeDef stimestructureget;

	HAL_RTC_GetTime(&RtcHandle,&stimestructureget,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&RtcHandle,&sdatestructureget,RTC_FORMAT_BIN);
	sprintf((char *)showtime,"%02d:%02d:%02d",stimestructureget.Hours,stimestructureget.Minutes,stimestructureget.Seconds);
//	sprintf((char *)showdate,"%02d:%02d:%02d",stimestructureget.Month,stimestructureget.Date,2000+ stimestructureget.Year);
	sprintf((char *)showdate,"%02d:%02d:%02d",
	        sdatestructureget.Month,
	        sdatestructureget.Date,
	        2000 + sdatestructureget.Year);

	printf("%02d:%02d:%02d ",stimestructureget.Hours,stimestructureget.Minutes,stimestructureget.Seconds);
	//printf("%02d:%02d:%02d",stimestructureget.Month,stimestructureget.Date,2000+ stimestructureget.Year);
	printf("%02d:%02d:%02d ",  sdatestructureget.Date,
		        sdatestructureget.Month,
		        2000 + sdatestructureget.Year);
}
