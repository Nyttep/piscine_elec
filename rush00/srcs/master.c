#include "../includes/main.h"

void	init_leds()
{
	SET(DDRB, DDB0);
	SET(DDRB, DDB1);
	SET(DDRB, DDB2);
	SET(DDRB, DDB4);

	SET(DDRD, DDD5);
	SET(DDRD, DDD6);
	RESET(DDRD, DDD2);
}

bool	check_button()
{
	if ((PIND & (1 << PD2)) == 0)
	{
		SET(PORTD, PD6);
		return true;
	}
	return false;
}

bool	get_winner(unsigned char data)
{
	TWDR = data;
	TWCR = (1 << TWEN) | (1 << TWINT);

	if (i2c_response(TW_MT_DATA_NACK)) // found a winner
		return true;
	return false; // no winner
}

void	begin_game()
{

	init_leds();

	// admitting we are in write mode

	SET(DDRD, DDD3);
	SET(PORTD, PD3);

	_delay_ms(500);

	uart_printstr("all light on master (first command)\r\n");
	i2c_write(CMD_LIGHT_D3_ON);
	i2c_write(CMD_LIGHT_D2_ON);
	i2c_write(CMD_LIGHT_D1_ON);
	SET(PORTB, PB4);
	SET(PORTB, PB2);
	SET(PORTB, PB1);
	_delay_ms(500);

	uart_printstr("all light on master (first command)\r\n");
	i2c_write(CMD_LIGHT_D3_OFF);
	RESET(PORTB, PB4);
	_delay_ms(500);

	i2c_write(CMD_LIGHT_D2_OFF);
	RESET(PORTB, PB2);
	_delay_ms(500);

	i2c_write(CMD_LIGHT_D1_OFF);
	RESET(PORTB, PB1);
	_delay_ms(1000);

	i2c_write(CMD_LIGHT_ALL_ON);
	SET(PORTB, PB4);
	SET(PORTB, PB2);
	SET(PORTB, PB1);
	SET(PORTB, PB0);
	i2c_write(CMD_START_BUTTON_RUSH);
	uart_printstr("start RUSH\r\n");
	bool is_pressed = false;
	while (1) // someone pressed button
	{
		i2c_write(0x19);
		if ((is_pressed = check_button()) == true || TW_STATUS == TW_MT_DATA_NACK) // if pressed button
		{
			uart_printstr("fini par le break\r\n");
			i2c_write(CMD_STOP_BUTTON_RUSH);
			break;
		}
	}
	uart_printstr("FINIIIIIIIII\r\n");
	RESET(PORTD, PD3);
	if (is_pressed)
	{
		SET(PORTD, PD6);
	}
	else
	{
		SET(PORTD, PD5);
	}
	_delay_ms(3000);
	// i2c_write(CMD_RESET);
	exec_command(CMD_RESET, false);
}

// M : command
// S : ACK
// M : command
// S : NACK (button)
// M : restart
// S : 