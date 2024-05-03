#ifndef ADC_H
#define ADC_H

#include <avr/io.h>
#include <avr/interrupt.h>

#define ON_ADC ISR(ADC_vect)

typedef enum
{
	ADC_NORMAL = 0,
	ADC_INT = 1,
} adc_data_type;

void adc_init(adc_data_type data_type);

uint16_t adc_read(uint8_t channel);
uint8_t adc_read8(uint8_t channel);

#endif
