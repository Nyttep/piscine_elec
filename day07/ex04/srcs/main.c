#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void	init_rgb()
{
	DDRD |= (1 << PD3) | (1 << PD5) | (1 << PD6); //setting DDRB1 to 1 configuring it to output
	PORTD &= ~(1 << PD3) & ~(1 << PD5) & ~(1 << PD6); //setting PORTB1 to 0 configuring it to output LOW

	PRR &= ~(1 << PRTIM2) & ~(1 << PRTIM0); //setting PRTIM2/PRTIM0 to 0 to enable timer/counters2 and 0

	//TIMER0 CONFIG
	//sets the timer as mode 1 (PWN Phase correct) (p131-132 & p140)
	TCCR0A |= (1 << WGM00);
	TCCR0A &= ~(1 << WGM01) & ~(1 << WGM02);

	//sets prescaler divisor of the timer0 at 256(p143)
	TCCR0B &= ~(1 << CS00) & ~(1 << CS01);
	TCCR0B |= (1 << CS00);

	TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
	TCCR0A &= ~(1 << COM0A0) & ~(COM0B0); 

	OCR0A = 0; //setting max value of timer1 to (16000000 / 255) / 244 to get 200Hz(p131-132)
	OCR0B = 0;

	//TIMER2 CONFIG

	TCCR2A |= (1 << WGM20);
	TCCR2A &= ~(1 << WGM21) & ~(1 << WGM22);

	//sets prescaler divisor of the timer2 at 256(p143)
	TCCR2B &= ~(1 << CS20);
	TCCR2B |= (1 << CS21) | (1 << CS22);

	TCCR2A |= (1 << COM2B1);
	TCCR2A &= ~(1 << COM2B0);

	OCR2B = 0; //setting max value of timer1 to (16000000 / 255) / 244 to get 200Hz(p131-132)

}

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

void	read_adc(uint8_t pin)
{
	PRR &= ~(1 << PRADC);//disabling power reduction

	ADMUX = pin;//select which pin to read, here the ADC0

	//setting voltage reference as internal 1.1V
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);

	ADMUX |= (1 << ADLAR);//left adjust the result (push it to the left)	

	ADCSRA |= (1 << ADPS0) | (1 << ADPS1)| (1 << ADPS2);//sets the prescaler at 128

	ADCSRA |= (1 << ADEN);//turning ON the ADC

	ADCSRA |= (1 << ADSC);//start conversion
}

void	handle_leds(uint8_t value)
{
	if (value >= (255 / 4))
		PORTB |= (1 << PB0);
	else
		PORTB &= ~(1 << PB0);
	if (value >= (255 / 4) * 2)
		PORTB |= (1 << PB1);
	else
		PORTB &= ~(1 << PB1);
	if (value >= (255 / 4) * 3)
		PORTB |= (1 << PB2);
	else
		PORTB &= ~(1 << PB2);
	if (value >= 255)
		PORTB |= (1 << PB4);
	else
		PORTB &= ~(1 << PB4);
}

int main()
{
	//INIT LEDS
	DDRB |= 1 | (1 << PB1) | (1 << PB2) | (1 << PB4); //setting DDRB0 DDRB1 DDRB2 DDRB4 to 1 configuring it to output
	PORTB &= ~1 & ~(1 << PB1) & ~(1 << PB2) & ~(1 << PB4); //setting PORTB0 PORTB1 PORTB2 PORTB4 to 0 configuring it to output LOW

	init_rgb();
	uart_init();
	uint8_t	res = 0;
	while (1)
	{
		read_adc(0b00000000);
		while (ADCSRA & (1 << ADSC));
		res = ADCH;
		wheel(res);
		handle_leds(res);
		print_dec_value(res);
		uart_printstr("\r\n");
	}
	return 0;
}
