#include <game.h>

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
static uint8_t	gameMode = GAME_MODE_WAIT_MASTER;
static volatile uint8_t	isPressed = FALSE;

#define BTN1_IS_PRESSED ((PIND & (1 << PD2)) == 0)
#define BTN2_IS_PRESSED ((PIND & (1 << PD4)) == 0)

uint8_t	readButtons(void)
{
	if (BTN1_IS_PRESSED)
		isPressed = TRUE;
	// if (BTN1_IS_PRESSED && isPressed == FALSE) //if button is pressed for the first time
	// 	isPressed = TRUE;
	// else if (BTN1_IS_PRESSED == FALSE) //If state changes
	// 	isPressed = FALSE;
	return (isPressed);
}

void	switchMode(uint8_t newMode)
{
	switch (newMode)
	{
		case GAME_MODE_COUNTDOWN:
			break;

		PORTD &= ~((1 << PD5) | (1 << PD6) | (1 << PD3)); //turn off blue leds

		case GAME_MODE_WAIT_MASTER:
			PORTD |= (1 << PD6) | (1 << PD3); //turn on red and green led => yellow
			//may want to blink led
			break;

		case GAME_MODE_WAIT_EVERYBODY:
			if (mode == I2C_MODE_MASTER)
				PORTD |= (1 << PD5); //turn on blue led
			else
				PORTD |= (1 << PD6) | (1 << PD3); //turn on red and green led => yellow
			break;

		case GAME_MODE_PUSH:
		
			break;

		case GAME_MODE_WIN:
			PORTD |= (1 << PD6);
			break;

		case GAME_MODE_LOST:
			PORTD |= (1 << PD3);
			break;
		
		default:
			break;
	}
	gameMode = mode;
}

#define MAX_VALUE 10000

// void	waitEverybodyMaster(void)
// {
// 	static uint8_t	receivedACK = FALSE;
// 	while (1)
// 	{	
// 		i2c_write(INSTRUCTION_CHECK_PRESS);
// 		printHexa(TW_STATUS);
// 		if (TW_STATUS == TW_MT_DATA_ACK)
// 		{
// 			i2c_write(INSTRUCTION_CHECK_NOT_PRESS);
// 			if (TW_STATUS == TW_MT_DATA_ACK) //It means a slave is telling he didn't press
// 			{
// 				while (TW_STATUS == TW_MT_DATA_ACK) //keep asking if a child didn't pressed
// 													//the button
// 					i2c_write(INSTRUCTION_CHECK_NOT_PRESS);
// 			}
// 			else if (TW_STATUS == TW_MT_DATA_NACK)//that mean all the slave pressed the button
// 			{
// 				TWCR = 0;
// 				initMaster(); //resend GCALL SLA
// 				while (TW_STATUS != TW_MT_SLA_ACK); //wait for sla
// 				i2c_write(INSTRUCTION_START_COUNTDOWN);
// 				LOGI("All slaves are ready");
// 				switchMode(GAME_MODE_COUNTDOWN);
// 				break;
// 			}
// 			else
// 				LOGE("Unexpected status code");
// 		}
// 		else if (TW_STATUS == TW_MT_DATA_NACK) //A slave is not ready
// 		{
// 			TWCR = 0;
// 			initMaster(); //resend GCALL SLA
// 			while (TW_STATUS != TW_MT_SLA_ACK); //wait for sla
// 		}
// 	}
// }

void	waitEverybodyMaster(void)
{
	static uint8_t	receivedACK = FALSE;
	while (1)
	{	
		i2c_write(INSTRUCTION_CHECK_PRESS);
		// printHexa(TW_STATUS);
		if (TW_STATUS == TW_MT_DATA_ACK)
		{
			LOGI("All slaves are ready");
			switchMode(GAME_MODE_COUNTDOWN);
			i2c_write(2);	
			break;
		}
		else if (TW_STATUS == TW_MT_DATA_NACK) //A slave is not ready
		{
			TWCR = 0;
			initMaster(); //resend GCALL SLA
			while (TW_STATUS != TW_MT_SLA_ACK); //wait for sla
		}
	}
}

// void	waitEverybodySlave(void)
// {
// 	uint8_t	data = 0;
// 	uint8_t	otherPressed = FALSE;
// 	uint8_t	returnNACK = FALSE;
// 	while (1)
// 	{
// 		// if (readButtons() == FALSE)
// 		// 	LOGI("sending NACK");
// 		// else
// 		// 	LOGI("sending ACK");
// 		if ((otherPressed == FALSE && readButtons() == FALSE)
// 		)

// 		i2c_read(&data, returnNACK); //send ACK if button pressed else send NACK
// 		if (data == INSTRUCTION_START_COUNTDOWN)
// 		{
// 			if (isPressed == FALSE)
// 				LOGE("Slave was ordered to go to countdown while button not pressed");
// 			LOGI("Everybody is ready !");
// 			switchMode(GAME_MODE_COUNTDOWN);
// 			break;
// 		}
// 		else if (TW_STATUS == TW_SR_GCALL_DATA_NACK) //If returned not pressed
// 		{
// 			//Make the slave return to SLA recognition mode 
// 			TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWINT);
// 			while (!I2C_READY);
// 			if (TW_STATUS == TW_SR_GCALL_ACK) // if answered
// 			{
// 				data = 0;
// 				continue;
// 			}
// 			else
// 			{
// 				LOGE("Slave received unexpected status");
// 				printHexa(TW_STATUS);
// 			}
// 		}
// 	}
// }

void	waitEverybodySlave(void)
{
	uint8_t	data = 0;
	while (1)
	{
		if (readButtons() == FALSE)
			LOGI("sending NACK");
		else
			LOGI("sending ACK");
		i2c_read(&data, !readButtons()); //send ACK if button pressed else send NACK
		if (data == INSTRUCTION_START_COUNTDOWN)
		{
			if (isPressed == FALSE)
				LOGE("Slave was ordered to go to countdown while button not pressed");
			LOGI("Everybody is ready !");
			switchMode(GAME_MODE_COUNTDOWN);
			break;
		}
		else if (TW_STATUS == TW_SR_GCALL_DATA_NACK) //If returned not pressed
		{
			//Make the slave return to SLA recognition mode 
			TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWINT);
			while (!I2C_READY);
			if (TW_STATUS == TW_SR_GCALL_ACK) // if answered
			{
				data = 0;
				continue;
			}
			else
			{
				LOGE("Slave received unexpected status");
				printHexa(TW_STATUS);
			}
		}
	}
}

void	waitEverybody(void)
{
	switch (mode)
	{
		case I2C_MODE_MASTER:
			waitEverybodyMaster();
			break;
		
		case I2C_MODE_SLAVE:
			waitEverybodySlave();
			break;
	}
}

void	initGame(void)
{
	switchMode(GAME_MODE_WAIT_MASTER);
	initSlave();
	setRole(); //Will block until a master take the lead
	LOGI("Waiting for everybody to be ready");
	waitEverybody();
	isPressed = FALSE;
	countdown();
	switchMode(GAME_MODE_PUSH);
	if (mode == I2C_MODE_MASTER)
	{
		LOGI("Starting game as master");
		masterRoutine();
	}
	else
	{
		LOGI("Starting game as slave");
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
			LOGI("Switching to master");
			initMaster();
			switchMode(GAME_MODE_WAIT_EVERYBODY);
			break;
		}
		else if (TW_STATUS == TW_SR_GCALL_ACK)
		{
			mode = I2C_MODE_SLAVE;
			LOGI("Switching to slave");
			switchMode(GAME_MODE_WAIT_EVERYBODY);
			break;
		}
	}
}

void	initMaster(void)
{
										
	i2c_init(100000, 0, I2C_MODE_MASTER_TX); //Init TWI interface enabling general call recognition
					//    and master mode (not pulling TWEA at beginning)

	TWCR = (1 << TWSTA) | (1 << TWINT) | (1 << TWEN); //send start condition

	while (!I2C_READY);

	if (!(TW_STATUS & TW_START) && !(TW_STATUS & TW_REP_START))
		LOGE("Start condition could not be sent");

	LOGD("Start condition was sent !");

	TWDR = 0; //Set address of receiver and mode
	TWCR = (1 << TWINT) | (1 << TWEN); //Set the interrupt flag to send content of TWDR buffer
}

void	initSlave(void)
{
	TWAR = 0b00000001; //enable response to general call
	TWBR = 72;
	TWCR = (1 << TWEA) | (1 << TWEN); //Enable interface and set TWEA to high
}

///////////////////////////////////////////////////////////////////
// ROUTINES

void	masterRoutine(void)
{
	uint8_t	data = 0;
	while (1)
	{
		readButtons();
		i2c_write(INSTRUCTION_CHECK_PRESS);
		if (TW_STATUS == TW_MT_DATA_NACK)
		{
			switchMode(GAME_MODE_LOST);
			lose();
			i2c_write(INSTRUCTION_LOSE);
			break ; 
		}
		else if (isPressed)
		{
			i2c_write(INSTRUCTION_LOSE);
			switchMode(GAME_MODE_WIN);
			win();
			break ; 
		}
	}
	_delay_ms(1); //MAybe to make sure everybody has treated information ???
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
			switchMode(GAME_MODE_WIN);
			win();
			break;
		}
		if (data == INSTRUCTION_LOSE)
		{
			switchMode(GAME_MODE_LOST);
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

	_delay_ms(6000);

	PORTD &= ~((1 << PD5) | (1 << PD6) | (1 << PD3));
	
}
