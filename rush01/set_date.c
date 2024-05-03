#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <uart.h>
#include <rtc.h>

bool is_int(uint8_t c) { return c >= '0' && c <= '9'; }

uint8_t get_int(uint8_t c) { return c - '0'; }

uint16_t parse_int(const uint8_t *str)
{
	uint16_t n = 0;
	for (uint8_t c = *str; c != 0; ++str, c = *str)
	{
		if (!is_int(c))
			return n;
		n = n * 10 + get_int(c);
	}
	return n;
}

#define ERROR_MSG "Invalid date format. Use: DD/MM/YYYY HH:MM:SS\n\r"
#define LINE_SIZE 32

bool parse_rtc(uint8_t line[LINE_SIZE])
{
	uint8_t *cpy = line;
	rtc_data data = {0};

	data.day = parse_int(cpy);
	if (data.day > 31 || data.day == 0)
		return true;
	while (is_int(*cpy))
		cpy++;
	if (*cpy != '/')
		return true;
	cpy++;
	data.month = parse_int(cpy);
	if (data.month > 12 || data.month == 0)
		return true;
	while (is_int(*cpy))
		cpy++;
	if (*cpy != '/')
		return true;
	cpy++;
	uint16_t year = parse_int(cpy);
	if (year < 1970 || year > 2099)
		return true;
	if (year < 2000)
		data.century = 0;
	data.year = year - (data.century ? 2000 : 1900);
	while (is_int(*cpy))
		cpy++;
	if (*cpy != ' ')
		return true;
	cpy++;
	data.hour = parse_int(cpy);
	if (data.hour > 23 || data.hour == 0)
		return true;
	while (is_int(*cpy))
		cpy++;
	if (*cpy != ':')
		return true;
	cpy++;
	data.min = parse_int(cpy);
	if (data.min > 59 || data.min == 0)
		return true;
	while (is_int(*cpy))
		cpy++;
	if (*cpy != ':')
		return true;
	cpy++;
	data.sec = parse_int(cpy);
	if (data.sec > 59 || data.sec == 0)
		return true;
	rtc_set_data(data);
	if (data.month == 2 && data.day > 28)
		return true;
	return false;
}

// 28/02/2023 18:03:17
ISR(USART_RX_vect)
{
	static uint8_t i = 0;
	static uint8_t line[LINE_SIZE] = {0};
	uint8_t c = UDR0;
	if (i >= LINE_SIZE - 1)
		return;
	if (c == 0x7f)
	{
		if (i > 0)
		{
			uart_tx('\b');
			uart_tx(' ');
			uart_tx('\b');
			line[i] = 0;
			i--;
		}
		return;
	}
	if (c == '\r')
	{
		if (i == 0)
			return;
		line[i] = 0;
		uart_printnl();
		if (parse_rtc(line))
			uart_printstr(ERROR_MSG);
		i = 0;
	}
	else
	{
		uart_tx(c);
		line[i++] = c;
	}
}
