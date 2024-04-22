#include "../includes/main.h"

void	send_status()
{
	switch(TW_STATUS)
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
			uart_printstr("SLA ACK reveived\r\n");
			break;
		}
		case TW_MT_SLA_NACK:
		{
			uart_printstr("SLA NACK receiver\r\n");
			break;
		}
		case TW_MT_DATA_ACK:
		{
			uart_printstr("DATA ACK receiver\r\n");
			break;
		}
		case TW_MT_DATA_NACK:
		{
			uart_printstr("DATA NACK receiver\r\n");
			break;
		}
		case TW_MT_ARB_LOST:
		{
			uart_printstr("Arbitration lost in SLA+W or data bytes\r\n");
			break;
		}
		default:
		{
			uart_printstr("unknown status code: ");
			print_hex_value(TW_STATUS);
			uart_printstr("\r\n");
		}
	}
}

bool	lost_arbitration(uint8_t status)
{
	switch (status)
	{
		case TW_MR_ARB_LOST:
		{
			uart_printstr("MR: arbitration lost during SLA+W/R or data\r\n");
			break;
		}
		case TW_SR_ARB_LOST_GCALL_ACK:
		{
			uart_printstr("arbitration lost during SLA+W/R, received general call\r\n");
			break;
		}
		case TW_SR_ARB_LOST_SLA_ACK:
		{
			uart_printstr("arbitration lost during SLA+W/R, received own SLA+W\r\n");
			break;
		}
		case TW_ST_ARB_LOST_SLA_ACK:
		{
			uart_printstr("MR: arbitration lost during SLA+W/R, received own SLA+R\r\n");
			break;
		}
		// case TW_BUS_ERROR:
		// {
		// 	uart_printstr("BUS ERROR illegal START/STOP\r\n");
		// 	break;
		// }
		default:
		{
			return (false);
		}
	}
	return true;
}


bool	i2c_response(uint8_t status_check)
{
	while (!(TWCR & (1 << TWINT)));//waiting for byte to be sent
	send_status();
	if (TW_STATUS != status_check)//checking if status code is right
	{
		// if (lost_arbitration(TW_STATUS))
		// 	im_a_slave();
		return (false);
	}
	return (true);
}

void	i2c_init()
{
	PRR &= ~(1 << PRTWI);//waking up the I2C/TWI
	
	//bitrate is calculated with this formula: CPU_CLOCK - (16 * desired_bitrate) / (2 * desired bitrate * prescaler)
	TWBR = 72;//setting bitrate register
	TWSR &= ~(1 << TWPS0) & ~(1 << TWPS1);//setting prescaler as 1

	TWAR = (0x18 << 1) | 1;
}

void	i2c_start(bool isWrite, uint8_t address)
{
	RESET(TWCR, TWEA);
	TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);//sending START

	if (!i2c_response(TW_START) && !i2c_response(TW_REP_START))
	{
		uart_printstr("error start\r\n");
		return	;
	}

	//load SLA+W to TWDR
	if (isWrite)
		TWDR = (address << 1) & ~(1 << TWD0);//address and WRITE
	else
		TWDR = (address << 1) | (1 << TWD0);//address and READ

	TWCR = (1 << TWEN) | (1 << TWINT);//send SLA+W

	if (!i2c_response(isWrite ? TW_MT_SLA_ACK : TW_MR_SLA_ACK))
	{
		//error
		uart_printstr("error ack\r\n");
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
	print_hex_value(msg);
	return (msg);
}

void	i2c_stop()
{
	uart_printstr("stop\r\n");
	TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);//sending STOP
}
