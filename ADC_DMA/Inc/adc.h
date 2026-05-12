#ifndef ADC_H_
#define ADC_H_

uint32_t pa0_adc_read(void);

extern ADC_HandleTypeDef hadc1;
void adc_pa0_DMA_init(void);
#endif /* ADC_H_ */
