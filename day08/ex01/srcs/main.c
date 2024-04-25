#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void	set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	OCR0A = g;
	OCR0B = r;
	OCR2B = b;
}

void	wheel(uint8_t pos) {
	pos = 255 - pos;
	if (pos < 85) {
		set_rgb(255 - pos * 3, 0, pos * 3);
	} else if (pos < 170) {
		pos = pos - 85;
		set_rgb(0, pos * 3, 255 - pos * 3);
	} else {
	pos = pos - 170;
	set_rgb(pos * 3, 255 - pos * 3, 0);
	}
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

char	uart_rx()
{
	while (!(UCSR0A & (1 << RXC0)));
	return (UDR0);
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
	UCSR0B |= (1 << RXEN0);//enabling reception
}

void	print_dec_value(uint16_t	nbr)
{
	if (nbr >= 0 && nbr <= 9 )
		uart_tx(nbr + '0');
	else
	{
		print_dec_value(nbr / 10);
		print_dec_value(nbr % 10);
	}
}

void print_hex_value(uint8_t data)
{
	char hex[3];
	hex[0] = "0123456789ABCDEF"[data >> 4];
	hex[1] = "0123456789ABCDEF"[data & 0x0F];
	// hex[2] = ' ';
	// hex[2] = '\r';
	// hex[3] = '\n';
	hex[2] = '\0';
	uart_printstr(hex);
}

void	spi_init()//initializing SPI as master
{
	PRR &= ~(1 << PRSPI);//disabling power reduction

	//configuring SPI pins
	DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB5);//setting SS/MOSI/SCK to output
	DDRB &= ~(1 << PB4);//setting MISO to input

	SPCR |= (1 << SPE) | (1 << MSTR);//enabling SPI as master

	SPCR &= ~(1 << DORD);//sets data order as MSB first

	SPCR &= ~(1 << CPOL) & ~(1 << CPHA);//sets clock as mode 0

	//sets clock frequency as /16
	SPCR |= (1 << SPR0);
	SPCR &= ~(1 << SPR1);
}	

void	spi_send(uint8_t data)
{
	SPDR = data;//load data
	while (!(SPSR & (1 << SPIF)));//wait for data to be sent
}


void	start_frame()
{
	for (uint8_t i = 0; i < 4; i++)
		spi_send(0);
}

void	led_frame(uint8_t r, uint8_t g, uint8_t b)
{
	spi_send(0b11100100);
	spi_send(b);
	spi_send(g);
	spi_send(r);
}

void	end_frame()
{
	for (uint8_t i = 0; i < 4; i++)
		spi_send(255);
}

void	set_leds(uint8_t RGB[3][3])
{
	start_frame();
	
	for (uint8_t i = 0; i < 3; i++)
		led_frame(RGB[i][0], RGB[i][1], RGB[i][2]);
		
	end_frame();
}

int main()
{
	uart_init();
	spi_init();

	while (1)
	{
		set_leds((uint8_t [3][3]){{255, 0, 0}, {0, 0, 0}, {0, 0, 0}});
		_delay_ms(1000);
		set_leds((uint8_t [3][3]){{0, 255, 0}, {0, 0, 0}, {0, 0, 0}});
		_delay_ms(1000);
		set_leds((uint8_t [3][3]){{0, 0, 255}, {0, 0, 0}, {0, 0, 0}});
		_delay_ms(1000);
		set_leds((uint8_t [3][3]){{255, 255, 0}, {0, 0, 0}, {0, 0, 0}});
		_delay_ms(1000);
		set_leds((uint8_t [3][3]){{0, 255, 255}, {0, 0, 0}, {0, 0, 0}});
		_delay_ms(1000);
		set_leds((uint8_t [3][3]){{255, 0, 255}, {0, 0, 0}, {0, 0, 0}});
		_delay_ms(1000);
		set_leds((uint8_t [3][3]){{255, 255, 255}, {0, 0, 0}, {0, 0, 0}});
		_delay_ms(1000);
	}
	return 0;
}
