#include "../includes/main.h"

void	start_button_rush(bool is_looser)
{
	if (is_looser)
	{
		start_button_rush_as_loser();
		return ;
	}
	bool	is_pressed = false;
	while (1)
	{
		uart_printstr("dans le rush\r\n");
		if ((PIND & (1 << PD2)) == 0)
		{
			is_pressed = true;
			TWCR &= ~(1 << TWEA);
		}
		else
			TWCR |= (1 << TWEA);
		TWCR |= (1 << TWINT);
		if (is_pressed)
		{
			uart_printstr("break de is pressed\r\n");
			break ;
		}
		if (read_command() == CMD_STOP_BUTTON_RUSH)
		{
			_delay_ms(10);
			break;
		}
	}
	TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN);
	
	if (is_pressed)
	{
		SET(PORTD, PD6);
	}
	else
	{
		SET(PORTD, PD5);
	}
	uart_printstr("coucou slave fini\r\n");
}

void	start_button_rush_as_loser()
{
	while (1)
	{
		TWCR |= (1 << TWINT);
		if (read_command() == CMD_STOP_BUTTON_RUSH)
		{
			_delay_ms(10);
			break;
		}
	}
	TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN);
	SET(PORTD, PD5);
	uart_printstr("coucou slave fini loser\r\n");
}
