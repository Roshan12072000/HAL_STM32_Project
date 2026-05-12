


#ifndef TIME_H_
#define TIME_H_
#include "stm32l4xx_hal.h"
#include <stdint.h>
extern TIM_HandleTypeDef htim2;

void tim2_us_init(void);
void delay_us(uint16_t us);

#endif /* TIME_H_ */
