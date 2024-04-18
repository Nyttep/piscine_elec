#include <avr/io.h>
#include <avr/interrupt.h>

void	uart_printstr(const char* str)
{
	for (uint16_t i = 0; str[i]; i++)
	{
		while (!(UCSR0A & (1 << UDRE0)));//wait for empty transmit buffer
		UDR0 = str[i];//send character
	}
}

ISR(TIMER1_COMPA_vect) //interrupt function when OCR1A compare match in timer1
{
	uart_printstr("Hello World! ");
}

void	timer1_init()
{
	//TIMER1 CONFIG
	//sets the timer as mode 15 (compare mode FAST PWN OCR1A) (p131-132 & p140)
	TCCR1A |= (1 << WGM11) | (1 << WGM10);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);

	//sets prescaler divisor of the timer1 at 1024(p143)
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCCR1B &= ~(1 << CS11);

	//overflow on OCR1A (p140)
	TCCR1A &= ~(1 << COM1A0) & ~(1 << COM1A1);

	TIMSK1 |= (1 << OCIE1A);//enabling interrupt 

	OCR1A = (16000000 / 1024) * 2; //setting max value of timer1 to (16000000 / 1024) * 2 to get 0.5Hz(p131-132)
	
}

void	uart_init()
{
	PRR &= ~(1 << PRUSART0);//making sure the USART0 is awake (p54)

	UCSR0C &= ~(1 << UMSEL00) & ~(1 << UMSEL01);// setting asynchronous mode
	UCSR0A |= (1 << U2X0);//setting clock as double speed
	// //setting UBRR0 to have a baudrate of 115200
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

int main()
{
	uart_init();
	timer1_init();
	sei();

	while (1)
	{
	}
	return 0;
}
