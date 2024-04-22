#include "../includes/main.h"

void	wait_for_SLA_R()
{
	uart_printstr("bfore");
	while (TW_STATUS != TW_SR_GCALL_ACK);
	uart_printstr("after");
	// TWCR = (1 << TWINT) | (1 << TWEA);
}

void	ack_command(bool is_nack)
{
	if (is_nack)
		TWCR |= (1 << TWEA);
	else
		TWCR &= ~(1 << TWEA);
}

bool	exec_command(uint8_t command, bool is_looser)
{
	switch (command) {
		case CMD_LIGHT_D3_ON:
			SET(PORTB, PB4);
			ack_command(true);
			return true;
		case CMD_LIGHT_D3_OFF:
			RESET(PORTB, PB4);
			ack_command(true);
			return true;
		case CMD_LIGHT_D2_ON:
			SET(PORTB, PB2);
			ack_command(true);
			return true;
		case CMD_LIGHT_D2_OFF:
			RESET(PORTB, PB2);
			ack_command(true);
			return true;
		case CMD_LIGHT_D1_ON:
			SET(PORTB, PB1);
			ack_command(true);
			return true;
		case CMD_LIGHT_D1_OFF:
			RESET(PORTB, PB1);
			ack_command(true);
			return true;
		case CMD_LIGHT_ALL_ON:
			uart_printstr("all light on SLAVE (first command)\r\n");
			SET(PORTB, PB4);
			SET(PORTB, PB2);
			SET(PORTB, PB1);
			SET(PORTB, PB0);
			ack_command(true);
			return true;
		case CMD_LIGHT_ALL_OFF:
			RESET(PORTB, PB4);
			RESET(PORTB, PB2);
			RESET(PORTB, PB1);
			RESET(PORTB, PB0);
			RESET(PORTD, PD5);
			RESET(PORTD, PD6);
			ack_command(true);
			return true;
		case CMD_START_BUTTON_RUSH:
			uart_printstr("start RUSH\r\n");
			start_button_rush(is_looser);
			_delay_ms(3000);
			exec_command(CMD_RESET, false);
			return false;
		case CMD_STOP_BUTTON_RUSH:
			uart_printstr("Recieved CMD_STOP_BUTTON_RUSH in wrong context (was not started)\r\n");
			ack_command(true);
			return true;
		case CMD_GET_ACK_WINNER:
			uart_printstr("Recieved CMD_GET_ACK_WINNER in wrong context (not started immediatly after stop)\r\n");
			ack_command(true);
			return true;
		case CMD_RESET:
			uart_printstr("reseting !!!\r\n");
			exec_command(CMD_LIGHT_ALL_OFF, false);
			return false;
		case 0xff:
			start_button_rush_as_loser();
			return false;
		default:
			uart_printstr("Unknown command revieved on slave\r\n");
			ack_command(true);
			return true;
	}
}

uint8_t	read_command()
{
	while (TW_STATUS != TW_SR_GCALL_DATA_ACK && TW_STATUS != TW_SR_GCALL_DATA_NACK)
	{
	}
	return TWDR;
}

uint8_t	read_command_cheat(bool *is_looser)
{
	while (TW_STATUS != TW_SR_GCALL_DATA_ACK && TW_STATUS != TW_SR_GCALL_DATA_NACK)
	{
		if ((PIND & (1 << PD2)) == 0)
			*is_looser = true;
	}
	return TWDR;
}

void	slave_routine()
{
	bool	is_looser = false;
	while (1)
	{
		uint8_t command = read_command_cheat(&is_looser);
		if (!exec_command(command, is_looser))
			break;
	}
}

void	im_a_slave()
{
	uart_printstr("OH NO IM A SLAVE :'((((((((((((\r\n");
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); // start

	SET(DDRB, DDB0);
	SET(DDRB, DDB1);
	SET(DDRB, DDB2);
	SET(DDRB, DDB4);
	SET(DDRD, DDD5);
	SET(DDRD, DDD6);
	RESET(DDRD, DDD2);

	while (1)
	{
		if ((PIND & (1 << PD2)) == 0)
		{
			uart_printstr("pressed !\r\n");
			while ((PIND & (1 << PD2)) == 0);
			TWCR = (1 << TWINT) | (1 << TWEN);
			break;
		}
	}
	// _delay_ms(25);
	TWCR |= (1 << TWEA);

	TWAR = (0x18 << 1) | 1;

	slave_routine();
	uart_printstr("end routine\r\n");

}
