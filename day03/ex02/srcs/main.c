#include <avr/io.h>
#include <avr/interrupt.h>

void	uart_tx(char c)
{
	while (!(UCSR0A & (1 << UDRE0)));//wait for empty transmit buffer
	UDR0 = c;//send character
}

char	uart_rx()
{
	while (!(UCSR0A & (1 << RXC0)));
	return (UDR0);
}

ISR(USART_RX_vect)//interrupt function when OCR1A compare match in timer1
{
	uart_tx(uart_rx());
}

void	uart_init()
{
	PRR &= ~(1 << PRUSART0);//making sure the USART0 is awake (p54)

	UCSR0C &= ~(1 << UMSEL00) & ~(1 << UMSEL01);// setting asynchronous mode
	UCSR0A |= (1 << U2X0);//setting clock as normal
	//setting UBRR0 to have a baudrate of 115200
	UBRR0L = (uint8_t)(16 & 0xFF);
	UBRR0H = (uint8_t)(16 >> 8);

	//setting parity mode to no parity
	UCSR0C &= ~(1 << UPM00) & ~(1 << UPM01);

	//setting character size as 8
	UCSR0B &= ~(1 << UCSZ02);
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);

	UCSR0C &= ~(1 << USBS0);//setting 1 ending bit

	UCSR0B |= (1 << RXCIE0);//enabling receive interrupt

	UCSR0B |= (1 << TXEN0);//enabling transmission
	UCSR0B |= (1 << RXEN0);//enabling reception
}

int main()
{
	DDRB |= (1 << PB1); //setting DDRB1 to 1 configuring it to output
	PORTB &= ~(1 << PB1); //setting PORTB1 to 0 configuring it to output LOW
	uart_init();
	sei();//enabling global interrupts

	while (1)
	{
	}
	return 0;
}
