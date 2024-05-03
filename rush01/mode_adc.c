#include <avr/io.h>
#include <adc.h>

extern volatile uint16_t displed_value;

// mode rv1
void mode_0(void)
{
	displed_value = adc_read(0);
}

// mode ldr
void mode_1(void)
{
	displed_value = adc_read(1);
}

// mode ntc
void mode_2(void)
{
	displed_value = adc_read(2);
}

// mode internal temperature
void mode_3(void)
{
	displed_value = adc_read(8);
}
