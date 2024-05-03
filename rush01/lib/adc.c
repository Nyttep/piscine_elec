#include <avr/io.h>
#include <avr/interrupt.h>
#include <adc.h>

void adc_init(adc_data_type data_type)
{
	// Set voltage reference to AVCC
	ADMUX = (0b01 << REFS0);
	// Set prescaler to 128
	ADCSRA = (0b111 << ADPS0);
	// Enable ADC
	ADCSRA |= (1 << ADEN);
	if (data_type == ADC_INT)
	{
		// Enable ADC interrupt
		ADCSRA |= (1 << ADIE);
		sei();
	}
}

static void adc_start(uint8_t channel)
{
	// Clear previous channel selection
	ADMUX &= ~(0b1111 << MUX0);
	// Select ADC channel
	ADMUX |= ((channel & 0xF) << MUX0);
	// Start conversion
	ADCSRA |= (1 << ADSC);
}

static void adc_wait(void)
{
	// Wait for conversion to complete
	while (ADCSRA & (1 << ADSC))
		;
}

uint16_t adc_read(uint8_t channel)
{
	// Read in 10 bits
	ADMUX &= ~(1 << ADLAR);
	adc_start(channel);
	adc_wait();

	// Read 10 bits of ADC value
	return ADC & 0x3FF;
}

uint8_t adc_read8(uint8_t channel)
{
	// Read in 8 bits
	ADMUX |= (1 << ADLAR);
	adc_start(channel);
	adc_wait();
	return ADCH;
}
