#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

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

void print_hex_value(uint8_t data)
{
	char hex[5];
	hex[0] = "0123456789ABCDEF"[data >> 4];
	hex[1] = "0123456789ABCDEF"[data & 0x0F];
	hex[2] = '\r';
	hex[3] = '\n';
	hex[4] = '\0';
	uart_printstr(hex);
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
			uart_printstr("MT SLA ACK received\r\n");
			break;
		}
		case TW_MT_SLA_NACK:
		{
			uart_printstr("MT SLA NACK receiver\r\n");
			break;
		}
		case TW_MT_DATA_ACK:
		{
			uart_printstr("MT DATA ACK received\r\n");
			break;
		}
		case TW_MT_DATA_NACK:
		{
			uart_printstr("MT DATA NACK received\r\n");
			break;
		}
		case TW_MT_ARB_LOST:
		{
			uart_printstr("Arbitration lost in SLA+W or data bytes\r\n");
			break;
		}
		case TW_MR_SLA_ACK:
		{
			uart_printstr("MR SLA ACK received\r\n");
			break;
		}
		case TW_MR_SLA_NACK:
		{
			uart_printstr("MR SLA NACK received\r\n");
			break;
		}
		case TW_MR_DATA_ACK:
		{
			uart_printstr("MR DATA ACK received\r\n");
			break;
		}
		case TW_MR_DATA_NACK:
		{
			uart_printstr("MR DATA NACK received\r\n");
			break;
		}
		default:
		{
			uart_printstr("Uknownm status: ");
			print_hex_value(status & 0b11111000);
		}
	}
}

uint8_t	i2c_response(uint8_t status_check)
{
	char	status;

	while (!(TWCR & (1 << TWINT)));//waiting for byte to be sent
	status = TWSR;
	send_status(status);
	if (!((status & 0b11111000) == status_check))//checking if status code is right
	{
		uart_printstr("ERROR\r\n");
		return (1);
	}
	return (0);
}

void	i2c_init()
{
	PRR &= ~(1 << PRTWI);//waking up the I2C/TWI
	
	//bitrate is calculated with this formula: CPU_CLOCK - (16 * desired_bitrate) / (2 * desired bitrate * prescaler)
	TWBR = 72;//setting bitrate register
	TWSR &= ~(1 << TWPS0) & ~(1 << TWPS1);//setting prescaler as 1
}

void	i2c_start(uint8_t address, uint8_t operation)
{
	TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);//sending START

	if (i2c_response(TW_START))
		return	;
	
	//load SLA+W/R to TWDR
	if (operation == TW_WRITE)
	{
		TWDR = (address << 1) & ~(1 << TWD0);//AHT20 address and WRITE
	}
	else
	{
		TWDR = (address << 1) | (1 << TWD0);//AHT20 address and READ

	}

	TWCR = (1 << TWEN) | (1 << TWINT);//send SLA+W/R

	if (operation == TW_WRITE)
	{
		if (i2c_response(TW_MT_SLA_ACK))
			return	;
	}
	else
	{
		if (i2c_response(TW_MR_SLA_ACK))
			return	;
	}
}

void	i2c_rstart(uint8_t address, uint8_t operation)
{
	TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);//sending START

	if (i2c_response(TW_REP_START))
		return	;
	
	//load SLA+W/R to TWDR
	if (operation == TW_WRITE)
	{
		TWDR = (address << 1) & ~(1 << TWD0);//AHT20 address and WRITE
	}
	else
	{
		TWDR = (address << 1) | (1 << TWD0);//AHT20 address and READ
	}

	TWCR = (1 << TWEN) | (1 << TWINT);//send SLA+W/R

	if (operation == TW_WRITE)
	{
		if (i2c_response(TW_MT_SLA_ACK))
			return	;
	}
	else
	{
		if (i2c_response(TW_MR_SLA_ACK))
			return	;
	}
}

void i2c_write(unsigned char data)
{
	TWDR = data;
	TWCR = (1 << TWEN) | (1 << TWINT);

	if (i2c_response(TW_MT_DATA_ACK))
		return ;
}

uint8_t	i2c_read(uint8_t last)
{
	while (!(TWCR & (1 << TWINT)));//waiting for new data
	i2c_response(TW_MR_DATA_ACK);
	uint8_t	msg = TWDR;
	if (last)
	{
		TWCR &= ~(1 << TWEA);
		TWCR |= (1 << TWINT);
	}
	else
		TWCR |= (1 << TWINT) | (1 << TWEA);
	print_hex_value(msg);
	return (msg);
}

void	i2c_stop()
{
	TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);//sending STOP
}

void	read_measure()
{
	uint8_t	data = 0xFF;

	while (data & (1 << 7))
	{
		i2c_rstart(0x38, TW_READ);
		data = i2c_read(0);
	}
	// print_hex_value(data);
	for (uint8_t i = 0; i < 5; i++)
	{
		data =  i2c_read(0);
		// print_hex_value(data);
	}
	data =  i2c_read(1);
	// print_hex_value(data);
}

uint8_t	is_init()
{
	return(i2c_read(1) & (1 << 3));
}

int main()
{
	uart_init();
	i2c_init();
	i2c_start(0x38, TW_READ);

	_delay_ms(40);
	if (!is_init())
	{
		uart_printstr("initializing\r\n");
		i2c_rstart(0x38, TW_WRITE);
		i2c_write(0xBE);
		i2c_write(0x08);
		i2c_write(0x00);
		_delay_ms(10);
		i2c_rstart(0x38, TW_READ);
	}
	uart_printstr("starting measure\r\n");
	i2c_rstart(0x38, TW_WRITE);
	i2c_write(0xAC);
	i2c_write(0x33);
	i2c_write(0x00);
	_delay_ms(80);

	read_measure();
	i2c_stop();

	while (1)
	{
		//measurement command
		// _delay_ms(10);
		// i2c_write(0x33);
		// i2c_write(0x00);
		// _delay_ms(90);

		// read_handle();
		// i2c_stop();
	}
	return 0;
}
