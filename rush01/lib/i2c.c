#include <avr/io.h>
#include <i2c.h>
#include <uart.h>

#define SCL_CLOCK 100000L
// #define I2C_DEBUG
// #define I2C_DEBUG_READ

// TWDR; // TWI Data Register (22.5.3)
void i2c_init(void)
{
	// TWI Bit Rate Generator Register (22.5.2)
	TWBR = ((F_CPU / SCL_CLOCK) - 16) / 2;
	TWSR = 0; // TWI Status Register (22.5.2) Contains Prescaler bits
	// Enable 2-wire Serial Interface (22.7.1)
	TWCR = (1 << TWEN); // TWI Control Register (22.5.3) (22.9.2)
}

void i2c_init_slave(uint8_t address)
{
	TWAR = address << 1;
	TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
}

#define WAIT_DATA \
	while (!(TWCR & (1 << TWINT)))

uint8_t i2c_start(uint8_t slave_address, i2c_WRITE_READ read_write)
{
	// Send START Condition (22.7.1)
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	// Wait for TWINT Flag set (22.7.1)
	WAIT_DATA;

#ifdef I2C_DEBUG
	uart_printhex(TW_STATUS);
	uart_printstr(" ");
#endif
	// if START has been transmitted.
	if (TW_STATUS != TW_START && TW_STATUS != TW_REP_START)
		return (0);

	// Set AHT20 Slave Address (AHT20 7.3)
	TWDR = (slave_address << 1) | read_write;
	TWCR = (1 << TWINT) | (1 << TWEN);
	// Wait end of transmission
	WAIT_DATA;

#ifdef I2C_DEBUG
	uart_printhex(TW_STATUS);
	uart_printstr("\n\r");
#endif
	// if SLA+W has been transmitted and ACK has been received.
	if (TW_STATUS != TW_MT_SLA_ACK)
		return (0);
	return 1;
}
void i2c_stop(void)
{
	// Send STOP Condition
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	TWCR &= ~(1 << TWEN);
}

void i2c_write(uint8_t data)
{
	// Load SLA+R/W into TWDR Register. Clear TWINT bit in TWCR to start transmission of address.
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	// Wait for TWINT Flag set. This indicates that the SLA+W has been transmitted, and ACK/NACK has been received.
	WAIT_DATA;
}

void i2c_read(uint8_t ack)
{
	// Send ACK to ask for a byte.
	TWCR = _BV(TWINT) | _BV(TWEN) | (ack ? _BV(TWEA) : 0);
	WAIT_DATA;
#ifdef I2C_DEBUG_READ
	uart_printhex(TWDR);
	uart_printstr(" ");
#endif
}
