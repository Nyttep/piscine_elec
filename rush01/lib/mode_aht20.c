#include "modes.h"
#include "aht20.h"
#include <avr/interrupt.h>
#include <uart.h>

extern volatile uint16_t displed_value;
extern volatile aht20_data aht20;

void	mode_6()
{
	float temp = calc_temp(aht20.temp);
	temp = temp * 10;
	if ((uint16_t)temp % 10 >= 5)
	{
		temp += 10;
	}
	temp = temp / 10;
	displed_value = (uint16_t)temp;
}

void	mode_7()
{
	float temp = calc_temp(aht20.temp);
	temp = (temp * (9.f / 5.f)) + 32;
	temp = temp * 10;
	if ((uint16_t)temp % 10 >= 5)
	{
		temp += 10;
	}
	temp = temp / 10;
	displed_value = temp;
}

void	mode_8()
{
	displed_value = calc_humi(aht20.humidity);
}