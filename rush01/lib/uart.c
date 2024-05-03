#include "uart.h"

// Initialization USART 20.5
void uart_init(uart_data_type data_type)
{
	// Set Baud Rate (20.11.5)
	UBRR0 = BAUD;

	// Enable transmitter/receiver (20.11.3)
	UCSR0B |= data_type | (1 << RXCIE0);

	// Set frame format: 8 data bit, N, 1 stop bit (20.11.4)
	UCSR0C = (0b11 << UCSZ00);
}

// Data Transmission (20.6)
void uart_tx(char c)
{
	// Clear TXCn Flag (20.5)
	// TXC0 = 0;
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1 << UDRE0)))
		;
	// Put data into buffer, sends the data
	UDR0 = c;
}

void uart_printstr(const char *str)
{
	if (!str)
		return;
	for (char c = *str; c != 0; ++str, c = *str)
		uart_tx(c);
}

void uart_printhex(uint8_t n)
{
	char c = n >> 4;
	uart_tx(c < 10 ? c + '0' : c - 10 + 'A');
	c = n & 0x0F;
	uart_tx(c < 10 ? c + '0' : c - 10 + 'A');
}

void uart_printbin(uint8_t n)
{
	for (uint8_t i = 0; i < 8; ++i)
		uart_tx((n & (1 << (7 - i))) ? '1' : '0');
}

void uart_printdec(uint16_t n)
{
	if (n >= 10)
		uart_printdec(n / 10);
	uart_tx(n % 10 + '0');
}

void uart_printnl(void)
{
	uart_tx('\n');
	uart_tx('\r');
}

// Data Reception (20.7)
char uart_rx(void)
{
	// Wait for data
	while (!(UCSR0A & (1 << RXC0)))
		;
	// Grab data from buffer
	return UDR0;
}
