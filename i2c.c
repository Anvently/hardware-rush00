#include <i2c.h>

static uint8_t	trMode = I2C_MODE_RX, lvlMode = I2C_MODE_SLAVE;

int8_t	i2c_init(uint32_t frequency, uint8_t address, uint8_t mode)
{
	//Set frequency
	//for 100kHZ and prescaler of 1 it is 72
	TWBR = CPU_FREQ / (2 * frequency * I2C_PRESCALER) - (16 / 2 * I2C_PRESCALER); //SEEMS MODIFIED !!!

	//General call enable bit is inverted so take the 7th bit an put to the 1st bit of TWAR
	TWAR |= (address << 1) | ((address & (1 << 7)) >> 7);

	TWCR |= (1 << TWEN); //Enable TWI interface
	TWCR |= ((mode & (1 << I2C_MODE_RX)) << TWEA); //In receiver mode, TWEA is pulled to HIGH

	// Register global modes
	trMode = mode & (1 << I2C_MODE_TR);
	lvlMode = mode & (1 << I2C_MODE_LVL);

	return (0);
}

int8_t	i2c_start(uint8_t address, uint8_t mode)
{
	TWCR = (1 << TWSTA) | (1 << TWINT) | (1 << TWEN); //send start condition

	while (!I2C_READY);

	if (!(TW_STATUS & TW_START) && !(TW_STATUS & TW_REP_START))
		return (LOGE("Start condition could not be sent"), I2C_ERROR_UNLIKELY);

	LOGD("Start condition was sent !");

	TWDR = (address << 1) | mode; //Set address of receiver and mode
	TWCR = (1 << TWINT) | (1 << TWEN); //Set the interrupt flag to send content of TWDR buffer

	while (!I2C_READY);

	switch (TW_STATUS)
	{
		//Device took the line
		case TW_MT_SLA_ACK:
			LOGD("SLA ACK received from slave device");
			break;

		//No one answered to the given address
		//It should mean that there is another master in control
		//Because address doesn't exist or someone (a master ?) is pulling the SDA LOW
		case TW_MT_SLA_NACK: 
			LOGD("SLA NACK received from slave device !!");
			break;

		//Another master took control of the line
		case TW_MR_ARB_LOST:
			LOGD("Arbitration lost !!");
			break;

		//Slave device is ready to send data
		case TW_MR_SLA_ACK:
			LOGD("SLA ACK received from slave device");
			break;

		//No one answered to the given adress
		//It should mean there is another master in control
		case TW_MR_SLA_NACK:
			LOGD("SLA NACK received from slave device !!");
			break;

		default:
			LOGE("Unknown status code");
			printHexa(TW_STATUS);
	}
	return (0);
}

int8_t	i2c_stop(void)
{
	TWCR |= (1 << TWSTO) | (1 << TWINT); //send stop condition
}

int8_t	i2c_write(uint8_t data)
{
	TWDR = data; //Set address of receiver and write mode
	TWCR = (1 << TWINT) | (1 << TWEN); //Set the interrupt flag to send content of TWDR buffer

	while (!I2C_READY);

	switch (TW_STATUS)
	{
		case TW_MT_SLA_ACK:
			LOGD("SLA ACK received while writing data, not supposed to happen !");
			break;

		case TW_MT_DATA_ACK:
			LOGD("Data ACK received from slave device");
			break;

		case TW_MT_DATA_NACK:
			LOGD("Data NACK received from slave device !!");
			break;

		case TW_MT_ARB_LOST:
			LOGD("Arbitration lost !!");
			break;
		
		default:
			LOGE("Unknown status code");
			printHexa(TW_STATUS);
			return (I2C_ERROR_UNLIKELY);
	}
	return (0);
}

int8_t	i2c_read(uint8_t* dest, uint8_t stop)
{
	if (stop)
		TWCR = (1 << TWINT) | (1 << TWEN);
	else
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

	while (!I2C_READY);
	
	*dest = TWDR;
	
	switch (TWSR & 0b11111000)
	{
		case TW_MR_DATA_ACK:
			LOGD("Data ACK returned to slave device");
			break;

		case TW_MR_DATA_NACK:
			LOGD("Data NACK returned to slave device !!");
			break;

		case TW_MR_ARB_LOST:
			LOGD("Arbitration lost !!");
			break;

		case TW_SR_GCALL_DATA_ACK:
			LOGD("Data from GCALL received and ACK returned");
			break;

		case TW_SR_GCALL_DATA_NACK:
			LOGD("Data from GCALL received and NACK returned");
			break;

		case TW_SR_DATA_ACK:
			LOGD("Data received and ACK returned");
			break;

		case TW_SR_DATA_NACK:
			LOGD("Data received and NACK returned");
			break;
		
		default:
			LOGE("Unknown status code");
			printHexa(TW_STATUS);
			return (I2C_ERROR_UNLIKELY);
	}

	return (0);
}
