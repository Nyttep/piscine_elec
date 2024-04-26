#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <util/delay.h>
#include <stdbool.h>

#define EXP_ADDRESS 0b00100111
#define SW3 0
#define D11 1
#define D10 2
#define D9 3
#define DP1 4
#define DP2 5
#define DP3 6
#define DP4 7

#define LED0 0b00111111
#define LED1 0b00000110
#define LED2 0b01011011
#define LED3 0b01001111
#define LED4 0b01100110
#define LED5 0b01101101
#define LED6 0b01111101
#define LED7 0b00000111
#define LED8 0b01111111
#define LED9 0b01101111

void	init_timer()
{
	PRR &= ~(1 << PRTIM1); //setting PRTIM1 to 0 to enable timer/counters1

	//TIMER1 CONFIG
	//sets the timer as mode 4 (compare mode OCR1A) (p131-132 & p140)
	TCCR1A &= ~(1 << WGM10) & ~(1 << WGM11);
	TCCR1B |= (1 << WGM12);
	TCCR1B &= ~(1 << WGM13);

	//sets prescaler divisor of the timer1 at 1024(p143)
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCCR1B &= ~(1 << CS11);

	//do nothing when OCR1A compare (p140)
	TCCR1A &= ~(1 << COM1A0) & ~(1 << COM1A1);

	OCR1A = 15625; //setting max value of timer1 to (16000000 / 1024) to get 1Hz(p131-132)
	// OCR1A = ICR1 / 2;

	TIMSK1 |= (1 << OCIE1A);
}

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
	char hex[4];
	hex[0] = "0123456789ABCDEF"[data >> 4];
	hex[1] = "0123456789ABCDEF"[data & 0x0F];
	hex[2] = ' ';
	// hex[2] = '\r';
	// hex[3] = '\n';
	hex[3] = '\0';
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
			uart_printstr("\r\n");
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
	TWBR = 72;//setting bitrate register
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
	TWBR = 72;//setting bitrate register
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
	if (last)
	{
		TWCR &= ~(1 << TWEA);
		TWCR |= (1 << TWINT);
	}
	else
		TWCR |= (1 << TWINT) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)));//waiting for new data
	uint8_t	msg = TWDR;
	if (last)
		i2c_response(TW_MR_DATA_NACK);
	else
		i2c_response(TW_MR_DATA_ACK);
	// print_hex_value(msg);
	return (msg);
}

void	i2c_stop()
{
	TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);//sending STOP
}

void	calculate(float* values)
{
	uint8_t	mid;
	uint32_t	formatted = 0;
	float*	ret[2];

	formatted |= (uint32_t)i2c_read(0) << 12;
	formatted |= (uint32_t)i2c_read(0) << 4;
	mid = i2c_read(0);
	formatted |= (uint32_t)mid >> 4;

	values[0] = ((float)formatted / 1048576) * 100;

	formatted = 0;
	formatted |= (uint32_t)(mid & 0x0F) << 16;
	formatted |= (uint32_t)i2c_read(0) << 8;
	formatted |= (uint32_t)i2c_read(0);

	values[1] = ((float)formatted / 1048576) * 200 - 50;
}

void	read_measure(float* values)
{
	uint8_t	data = 0xFF;
	
	i2c_start(0x38, TW_READ);
	data = i2c_read(0);
	while (data & (1 << 7))
	{
		i2c_rstart(0x38, TW_READ);
		data = i2c_read(0);
	}
	
	calculate(values);
	data =  i2c_read(1);
}


uint8_t	is_calibrated()
{
	i2c_rstart(0x38, TW_READ);
	uint8_t data = i2c_read(1);
	// uart_printstr("\b\b\b   \b\b\b");
	return(data & (1 << 3));
}

void	soft_reset()
{
	i2c_start(0x38, TW_WRITE);
	i2c_write(0x5D);
	i2c_stop();
	_delay_ms(20);
}

void	calibrate()
{
	while (!is_calibrated())
	{
		uart_printstr("initializing\r\n");
		i2c_rstart(0x38, TW_WRITE);
		i2c_write(0xBE);
		i2c_write(0x08);
		i2c_write(0x00);
		_delay_ms(10);
	}
}

void	ask_measure()
{
	// uart_printstr("starting measure\r\n");
	i2c_rstart(0x38, TW_WRITE);
	i2c_write(0xAC);
	i2c_write(0x33);
	i2c_write(0x00);
	_delay_ms(80);
	i2c_stop();
	_delay_ms(50);
}

// ISR(TIMER1_COMPA_vect)
// {
// 	// static uint8_t to_send = 255;

// 	// to_send ^= (1 << 3); 
// 	// i2c_write(to_send);//switch D9
// 	// i2c_write(0);//pass next data sent
// 	char pins[3] = {D11, D10, D9};
	
// 	i2c_rstart(EXP_ADDRESS, TW_WRITE);//SLA+W
// 	i2c_write(2);//tell expander I want to write to output0
// 	int i = 2;
// 	while (i >= 0)
// 	{
// 		if (counter & (1 << i))
// 			i2c_write(255 & ~(1<< pins[i--]));
// 		else
// 			i2c_write(255 | (1<< pins[i--]));
// 	}
// }

void	render_num(uint8_t counter)
{
	char pins[3] = {D11, D10, D9};
	uint8_t	to_send = 255;
	
	i2c_rstart(EXP_ADDRESS, TW_WRITE);//SLA+W
	i2c_write(2);//tell expander I want to write to output0
	int i = 2;
	while (i >= 0)
	{
		if (counter & (1 << i))
			to_send &= ~(1<< pins[i--]);
		else
			to_send |= (1<< pins[i--]);
	}
	i2c_write(to_send);
}

bool	read_expander0(uint8_t port)
{
	i2c_rstart(EXP_ADDRESS, TW_WRITE);//SLA+W
	i2c_write(0);//tell expander I want to read to input0
	i2c_rstart(EXP_ADDRESS, TW_READ);//SLA+R
	uint8_t ret = i2c_read(1);
	return (ret & (1 << SW3));
}

void	set_led_seg(uint8_t num, uint8_t seg)
{
	i2c_rstart(EXP_ADDRESS, TW_WRITE);//SLA+W
	i2c_write(2);//tell expander I want to write to output0
	i2c_write(255 & ~(1 << seg));//tell expander which seg i want to update
	i2c_write(num);
}

int main()
{
	uart_init();
	i2c_init();
	// init_timer();

	char	button_state1 = 0;
	uint8_t	counter = 0;

	i2c_start(EXP_ADDRESS, TW_WRITE);//SLA+W
	i2c_write(6);//tell expander I want to configure a pin
	i2c_write(0b00000001);//set D9/D10/D11/DP1/DP2/DP3/DP4 as output;
	i2c_write(0b00000000);//set all led segments as output;
	set_led_seg(LED2, DP4);
	// i2c_rstart(EXP_ADDRESS, TW_WRITE);//SLA+W
	// i2c_write(2);//tell expander I want to write to output0
	// sei();
	while (1)
	{
	}
	i2c_stop();
	return 0;
}
