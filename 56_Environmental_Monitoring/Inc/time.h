

#ifndef TIME_H_
#define TIME_H_

//#define TIMx TIM3
#define _TIMx_CLK_ENABLE 	__HAL_RCC_TIM3_CLK_ENABLE()
#define TIMx_IRQn			TIM3_IRQn
#define TIMx_IRQHANDLER 	TIM3_IRQHandler

void tim_timebase_init(void);

#endif /* TIME_H_ */
