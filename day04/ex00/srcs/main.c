#include <avr/io.h>
#include <util/twi.h>

void	uart_tx(char c)
{
	while (!(UCSR0A & (1 << UDRE0)));//wait for empty transmit buffer
	UDR0 = c;//send character
}

void	uart_printstr(const char* str)
{
	for (uint16_t i = 0; str[i]; i++)
	{
		while (!(UCSR0A & (1 << UDRE0)));//wait for empty transmit buffer
		UDR0 = str[i];//send character
	}
}

void	uart_init()
{
	PRR &= ~(1 << PRUSART0);//making sure the USART0 is awake (p54)

	UCSR0C &= ~(1 << UMSEL00) & ~(1 << UMSEL01);// setting aynchronous mode
	UCSR0A |= (1 << U2X0);//setting clock as double speed
	//setting UBRR0 to have a baudrate of 115200
	UBRR0L = (uint8_t)(16 & 0xFF);
	UBRR0H = (uint8_t)(16 >> 8);

	//setting parity mode to no parity
	UCSR0C &= ~(1 << UPM00) & ~(1 << UPM01);

	//setting character size as 8
	UCSR0B &= ~(1 << UCSZ02);
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);

	UCSR0C &= ~(1 << USBS0);//setting 1 ending bit

	UCSR0B |= (1 << TXEN0);//enabling transmission
}

void	send_status(char status)
{
	switch(status & 0b11111000)
	{
		case TW_START:
		{
			uart_printstr("START sent\r\n");
			break;
		}
		case TW_REP_START:
		{
			uart_printstr("repeated START sent\r\n");
			break;
		}
		case TW_MT_SLA_ACK:
		{
			uart_printstr("SLA ACK reveived\r\n");
			break;
		}
		case TW_MT_SLA_NACK:
		{
			uart_printstr("SLA NACK receiver\r\n");
			break;
		}
		case TW_MT_DATA_ACK:
		{
			uart_printstr("DATA ACK receiver\r\n");
			break;
		}
		case TW_MT_DATA_NACK:
		{
			uart_printstr("DATA NACK receiver\r\n");
			break;
		}
		case TW_MT_ARB_LOST:
		{
			uart_printstr("Arbitration lost in SLA+W or data bytes\r\n");
			break;
		}
	}
}

void	i2c_init()
{
	PRR &= ~(1 << PRTWI);//waking up the I2C/TWI
	
	//bitrate is calculated with this formula: CPU_CLOCK - (16 * desired_bitrate) / (2 * desired bitrate * prescaler)
	TWBR = 72;//setting bitrate register
	TWSR &= ~(1 << TWPS0) & ~(1 << TWPS1);//setting prescaler as 1
}

void	i2c_start()
{
	char	status;
	TWCR |= (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);//sending START

	while (!(TWCR & (1 << TWINT)));//waiting for START to be sent
	status = TWSR;
	send_status(status);
	if (!((status & 0b11111000) == TW_START))//checking if START was sent correctly
	{
		uart_printstr("ERROR: TW_START not sent\r\n");
		return ;
	}

	//load SLA+W to TWDR
	TWDR = (0x38 << 1) & ~(1 << TWD0);//AHT20 address and WRITE

	TWCR |= (1 << TWEN) | (1 << TWINT);//send SLA+W

	while (!(TWCR & (1 << TWINT)));//waiting for SLA+W to be sent
	status = TWSR;
	send_status(status);
	if (!((status & 0b11111000) == TW_MT_SLA_ACK))//checking if SLA+W was sent correctly and ACK received
	{
		uart_printstr("ERROR: TW_MT_SLA_ACK not sent or received NACK\r\n");
		return ;
	}
}

void	i2c_stop()
{
	TWCR |= (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);//sending STOP
}

int main()
{
	DDRB |= (1 << PB0) | (1 << PB1); //setting DDRB1 to 1 configuring it to output
	PORTB &= ~(1 << PB0) & ~(1 << PB1); //setting PORTB1 to 0 configuring it to output LOW
	uart_init();
	i2c_init();
	i2c_start();
	i2c_stop();

	while (1)
	{
	}
	return 0;
}
