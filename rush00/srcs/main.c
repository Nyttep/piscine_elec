#include "../includes/main.h"

void	wait_slaves()
{
	while (TW_STATUS != TW_MT_DATA_NACK)
	{
		i2c_write(0xf5);
		return ;
	}
}

void	im_your_master()
{
	uart_printstr("YESSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\r\n");
	i2c_start(true, 0);
	wait_slaves();
	begin_game();
}

int main()
{
	DDRB |= (1 << PB0) | (1 << PB1); //setting DDRB1 to 1 configuring it to output
	PORTB &= ~(1 << PB0) & ~(1 << PB1); //setting PORTB1 to 0 configuring it to output LOW
	uart_init();
	i2c_init();

	SET(DDRB, DDB0);
	SET(DDRB, DDB1);
	SET(DDRB, DDB2);
	SET(DDRB, DDB4);
	SET(DDRD, DDD5);
	SET(DDRD, DDD6);
	RESET(DDRD, DDD2);

	uart_printstr("begin !\r\n");

	TWCR |= (1 << TWEA) | (1 << TWEN);
	TWAR = (0x18 << 1) | 1;
	while (1)
	{
		if ((PIND & (1 << PD2)) == 0)
		{
			im_your_master();
			i2c_stop();
			_delay_ms(100);
			TWCR = (1 << TWEA) | (1 << TWEN);
			TWAR = (0x18 << 1) | 1;
			continue;
		}
		// _delay_ms(250);
		if (TW_STATUS == TW_SR_GCALL_ACK)
		{
			im_a_slave();
			TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWINT);
			TWAR = (0x18 << 1) | 1;
		}
	}
	return 0;
}
