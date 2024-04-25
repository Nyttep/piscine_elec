#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// void	init_rgb()
// {
// 	DDRD |= (1 << PD3) | (1 << PD5) | (1 << PD6); //setting DDRB1 to 1 configuring it to output
// 	PORTD &= ~(1 << PD3) & ~(1 << PD5) & ~(1 << PD6); //setting PORTB1 to 0 configuring it to output LOW

// 	PRR &= ~(1 << PRTIM2) & ~(1 << PRTIM0); //setting PRTIM2/PRTIM0 to 0 to enable timer/counters2 and 0

// 	//TIMER0 CONFIG
// 	//sets the timer as mode 1 (PWN Phase correct) (p131-132 & p140)
// 	TCCR0A |= (1 << WGM00);
// 	TCCR0A &= ~(1 << WGM01) & ~(1 << WGM02);

// 	//sets prescaler divisor of the timer0 at 256(p143)
// 	TCCR0B &= ~(1 << CS00) & ~(1 << CS01);
// 	TCCR0B |= (1 << CS00);

// 	TCCR0A |= (1 << COM0A0) | (1 << COM0A1) | (1 << COM0B0) | (1 << COM0B1);

// 	OCR0A = 0; //setting max value of timer1 to (16000000 / 255) / 244 to get 200Hz(p131-132)
// 	OCR0B = 0;

// 	//TIMER2 CONFIG

// 	TCCR2A |= (1 << WGM20);
// 	TCCR2A &= ~(1 << WGM21) & ~(1 << WGM22);

// 	//sets prescaler divisor of the timer2 at 256(p143)
// 	TCCR2B &= ~(1 << CS20);
// 	TCCR2B |= (1 << CS21) | (1 << CS22);

// 	TCCR2A |= (1 << COM2B0) | (1 << COM2B1);

// 	OCR2B = 0; //setting max value of timer1 to (16000000 / 255) / 244 to get 200Hz(p131-132)

// }

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

// void	read_adc(uint8_t pin)
// {
// 	PRR &= ~(1 << PRADC);//disabling power reduction

// 	ADMUX = pin;//select which pin to read, here the ADC0

// 	//setting voltage reference as internal 1.1V
// 	ADMUX |= (1 << REFS0);
// 	ADMUX &= ~(1 << REFS1);

// 	ADMUX |= (1 << ADLAR);//left adjust the result (push it to the left)	

// 	ADCSRA |= (1 << ADPS0) | (1 << ADPS1)| (1 << ADPS2);//sets the prescaler at 128

// 	ADCSRA |= (1 << ADEN);//turning ON the ADC

// 	ADCSRA |= (1 << ADSC);//start conversion
// }

// void	handle_leds(uint8_t value)
// {
// 	if (value >= (255 / 4))
// 		PORTB |= (1 << PB0);
// 	else
// 		PORTB &= ~(1 << PB0);
// 	if (value >= (255 / 4) * 2)
// 		PORTB |= (1 << PB1);
// 	else
// 		PORTB &= ~(1 << PB1);
// 	if (value >= (255 / 4) * 3)
// 		PORTB |= (1 << PB2);
// 	else
// 		PORTB &= ~(1 << PB2);
// 	if (value >= 255)
// 		PORTB |= (1 << PB4);
// 	else
// 		PORTB &= ~(1 << PB4);
// }

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
	spi_send(0b11101000);
	spi_send(b);
	spi_send(g);
	spi_send(r);
}

void	end_frame()
{
	for (uint8_t i = 0; i < 4; i++)
		spi_send(255);
}


int main()
{
	uart_init();
	spi_init();

	start_frame();
	led_frame(255, 0, 0);
	led_frame(0, 0, 0);
	led_frame(0, 0, 0);
	end_frame();
	while (1)
	{
	}
	return 0;
}
