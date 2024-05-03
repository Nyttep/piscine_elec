#pragma once

#include <util/twi.h>

typedef enum
{
	I2C_WRITE = 0,
	I2C_READ = 1
} i2c_WRITE_READ;

// Ex00

void i2c_init(void);
void i2c_init_slave(uint8_t address);
uint8_t i2c_start(uint8_t slave_address, i2c_WRITE_READ read_write);
void i2c_stop(void);

// Ex01

#define ACK 1
#define NACK 0

void i2c_write(uint8_t data);
void i2c_read(uint8_t ack);
