#ifndef ADC_H_
#define ADC_H_

uint32_t pa0_adc_read(void);
void adc_init_start(void);
extern ADC_HandleTypeDef hadc1;
void adc_pa0_single_conv_init(void);
#endif /* ADC_H_ */
