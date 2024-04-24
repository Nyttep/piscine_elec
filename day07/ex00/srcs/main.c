#include <avr/io.h>
#include <avr/interrupt.h>
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

void	read_adc(uint8_t pin)
{
	PRR &= ~(1 << PRADC);//disabling power reduction

	ADMUX = pin;//select which pin to read, here the ADC0

	//setting voltage reference as AVcc
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);

	ADMUX |= (1 << ADLAR);//left adjust the result (push it to the left)	

	ADCSRA |= (1 << ADPS0) | (1 << ADPS1)| (1 << ADPS2);//sets the prescaler at 128

	ADCSRA |= (1 << ADEN);//turning ON the ADC

	ADCSRA |= (1 << ADSC);//start conversion
}

int main()
{
	uart_init();
	
	while (1)
	{
		read_adc(0b00000000);
		while (ADCSRA & (1 << ADSC));
		print_hex_value(ADCH);
		uart_printstr("\r\n");
		_delay_ms(20);
	}
	return 0;
}
