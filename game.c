#include <game.h>

extern volatile uint8_t	isPressed;

/*

Different modes 
	- device is a MASTER who won control of SDA in general call
		- continuously send DATA in general call mode
			- if a slave didn't had a button pushed, he send ACK
			- if a slave had a button pushed, he send NACK
				- he knows for sure that he won
		- if NACK, it means that a slave pushed the button
			- master knows he has lost
			- he send a END byte communication to inform the slaves they have lost
				- (PROBLEM  (or not): how to difference STOP condition from unplugging the cable ?)

	- device is a SLAVE who lost control of SDA in general call
		- receive and ACK the data received as long as the button is not pressed
		- if the button is pressed, return NACK to master
			- turn on success condition (blinking led etc...)
		- if detect a END byte
			- he lost
		- if a MASTER interrupt its connexion (game) how to detect it ??
*/

static uint8_t	mode =	 I2C_MODE_MASTER;

void	initGame(void)
{
	initSlave();
	setRole(); 
	// while (!I2C_READY);
	// detectMode(); //check status of i2c_start

	if (mode == I2C_MODE_MASTER)
	{
		LOGI("Master mode");
		initMaster();
		countdown();
		masterRoutine();
	}
	else
	{
		// TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
		LOGI("Slave mode");
		initSlave();
		countdown();
		slaveRoutine();
	}
}

///////////////////////////////////////////////////////////////////
// ALL SLAVES

// set board as master or slave
void	setRole(void)
{
	// Wait for general call if slave
	// Wait for button Pressed if master
	while (1)
	{
		readButtons();
		if (isPressed)
		{
			mode = I2C_MODE_MASTER;
			break;
		}
		else if (//general call)
		{
			mode = I2C_MODE_SLAVE;
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////
// Wait for everybody ready

void	initMaster(void)
{
										
	i2c_init(100000, 0, I2C_MODE_MASTER_TX); //Init TWI interface enabling general call recognition
					//    and master mode (not pulling TWEA at beginning)
	TWCR = (1 << TWSTA) | (1 << TWINT) | (1 << TWEN); //send start condition

	while (!I2C_READY);

	if (!(TW_STATUS & TW_START) && !(TW_STATUS & TW_REP_START))
		LOGI("Start condition could not be sent");

	LOGD("Start condition was sent !");

	TWDR = 0; //Set address of receiver and mode
	TWCR = (1 << TWINT) | (1 << TWEN); //Set the interrupt flag to send content of TWDR buffer


	uint8_t	data = 0;
	while (1)
	{
		readButtons();
		i2c_write(1);
		if (TW_STATUS == TW_MT_DATA_ACK)
		{
			LOGI("All slaves are ready");
			i2c_write(2);
			break ; 
		}
	}
	i2c_stop();
}

void	initSlave(void)
{
	TWAR = 0b00000001; //enable response to general call
	TWBR = 72;
	TWCR = (1 << TWEA) | (1 << TWEN); //Enable interface and set TWEA to high

	while (1)
	{
		readButtons();
		uint8_t	data = 0;
		i2c_read(&data, isPressed);
		if (isPressed)
		{
			i2c_write(ACK);
			break;
		}
		if (data == 2)
			break;
		i2c_write(NACK);
	}
	i2c_stop(); // parce que j'ecris ? 
}

///////////////////////////////////////////////////////////////////
// ROUTINES

void	masterRoutine(void)
{
	uint8_t	data = 0;
	while (1)
	{
		readButtons();
		i2c_write(1);
		if (TW_STATUS == TW_MT_DATA_NACK)
		{
			lose();
			i2c_write(0);
			break ; 
		}
		else if (isPressed)
		{
			win();
			i2c_write(0);
			break ; 
		}
	}
	i2c_stop();
}

void	slaveRoutine(void)
{
	while (1)
	{
		readButtons();
		uint8_t	data = 0;
		i2c_read(&data, isPressed);
		if (isPressed)
		{
			win();
			break;
		}
		if (data == 0)
		{
			lose();
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////
// LED

void	countdown(void)
{
	DDRB |= (1 <<PB0) | (1 <<PB1) | (1 <<PB2) | (1 <<PB4);
	PORTB |= (1 <<PB0) | (1 <<PB1) | (1 <<PB2) | (1 <<PB4);

	_delay_ms(1000);
	PORTB &= ~(1 <<PB4);
	_delay_ms(1000);
	PORTB &= ~(1 <<PB2);
	_delay_ms(1000);
	PORTB &= ~(1 <<PB1);
	_delay_ms(1000);
	PORTB &= ~(1 <<PB0);
}

void	win(void)
{
	print("Victory !! :-D", 1);

	DDRD |= (1<<PD3) | (1<<PD5) | (1<<PD6);

	for (int i = 0; i < 15; i++)
	{
		PORTD |= (1<<PD3);
		_delay_ms(80);
		PORTD |= (1<<PD5);
		_delay_ms(80);
		PORTD &= ~(1<<PD3);
		_delay_ms(80);
		PORTD |= (1<<PD6);
		_delay_ms(80);
		PORTD &= ~(1<<PD5);
		_delay_ms(80);
		PORTD &= ~(1<<PD6);
	}
}

void	lose(void)
{
	print("LOOSER :-(", 1);

	DDRD |= (1<<PD5);
	PORTD |= (1<<PD5);
	_delay_ms(6000);
	
}



// void	detectMode(void)
// {
// 	switch (TW_STATUS)
// 	{
// 		//Device took the line
// 		case TW_MT_SLA_ACK:
// 			LOGI("Another master detected. Device is now entering master mode");
// 			mode = I2C_MODE_MASTER;
// 			break;

// 		//No one answered to the given address
// 		//Means that you are master
// 		case TW_MT_SLA_NACK: 
// 			LOGI("Device entering slave mode ?");
// 			mode = I2C_MODE_SLAVE;
// 			break;

// 		//Another master took control of the line. SHould not happen (because general call should be answered)
// 		case TW_MR_ARB_LOST:
// 			LOGI("Arbitration lost. Device entering slave mode.");
// 			mode = I2C_MODE_SLAVE; 
// 			break;

// 		//Lost arbitration and addressed by general call => slave mode 
// 		case TW_SR_ARB_LOST_GCALL_ACK:
// 			LOGI("Arbitration lost and general call answered. Device entering slave mode.");
// 			mode = I2C_MODE_SLAVE; 
// 			break;

// 		//Slave answered acknowledge to general call (not supposed to happen)
// 		case TW_SR_GCALL_ACK:
// 			LOGI("Slave answered general call");
// 			mode = I2C_MODE_SLAVE;
// 			break;

// 		//Slave received data and returned acknowledge => game is still running
// 		case TW_SR_GCALL_DATA_ACK:
// 			LOGI("Slave ACK data received");
// 			break;

// 		//Slave received data when addressed in general call, but returned NACK
// 		//Means that the slave cleared TWEA flag in the previous read beacause he won
// 		//So not supposed to happen
// 		case TW_SR_GCALL_DATA_NACK:
// 			LOGI("Slave NACK data received");
// 			break;


// 		// //Slave device is ready to send data
// 		// case TW_MR_SLA_ACK:
// 		// 	LOGI("SLA ACK received from slave device");
// 		// 	break;

// 		// //No one answered to the given adress
// 		// //It should mean there is another master in control
// 		// case TW_MR_SLA_NACK:
// 		// 	LOGI("SLA NACK received from slave device !!");
// 		// 	break;

// 		default:
// 			print("Unknown status code: ", 0);
// 			printHexa(TW_STATUS);
// 	}
// }