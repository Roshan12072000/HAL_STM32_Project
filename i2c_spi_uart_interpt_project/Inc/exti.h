/*
 * exti.h
 *
 *  Created on: May 5, 2026
 *      Author: User
 */

#ifndef EXTI_H_
#define EXTI_H_
#define LIGHT_ADDR (0x47 << 1)


void gpio_pc13_interrupt_init(void);
void i2c1_init(void);
void light_sensor_check(void);
void light_sensor_init(void);
int read_light(void);
#endif /* EXTI_H_ */
