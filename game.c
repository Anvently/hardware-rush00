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
static uint8_t	hasCheat = FALSE;

#define BTN1_IS_PRESSED ((PIND & (1 << PD2)) == 0)
#define BTN2_IS_PRESSED ((PIND & (1 << PD4)) == 0)

uint8_t	readButtons(void)
{
	// if (BTN1_IS_PRESSED)
	// 	isPressed = TRUE;
	if (BTN1_IS_PRESSED && isPressed == FALSE) //if button is pressed for the first time
		isPressed = TRUE;
	else if (isPressed)
		isPressed = FALSE;
	else if (BTN1_IS_PRESSED == FALSE) //If state changes
		isPressed = FALSE;
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

void	waitEverybodyMaster(void)
{
	while (1)
	{	
		i2c_write(INSTRUCTION_CHECK_PRESS);
		// printHexa(TW_STATUS);
		if (TW_STATUS == TW_MT_DATA_ACK) //If a slave is not ready
			continue ;
		else if (TW_STATUS == TW_MT_DATA_NACK) //All slave are ready
		{
			LOGI("All slaves are ready");
			TWCR = 0;
			initMaster(); //resend GCALL SLA
			LOGI("Master is going to countdown");
			// while (TW_STATUS != TW_MT_SLA_ACK); //wait for slave to receive SLA GCE
			// i2c_write(INSTRUCTION_START_COUNTDOWN);
			switchMode(GAME_MODE_COUNTDOWN);
			break;
		}
	}
}

void	waitEverybodySlave(void)
{
	uint8_t	data = 0;
	uint8_t	wasPressed = FALSE;
	while (1)
	{
		i2c_read(&data, readButtons()); //send NACK if button pressed else send ACK
		if (readButtons() && wasPressed == FALSE)
		{
			LOGI("Button was pressed, disconnected from bus and waiting for start condition");
			wasPressed = TRUE;
		}
		if (data == INSTRUCTION_START_COUNTDOWN)
		{
			if (readButtons() == FALSE)
				LOGE("Slave was ordered to go to countdown while button not pressed");
			LOGI("Everybody is ready !");
			switchMode(GAME_MODE_COUNTDOWN);
			break;
		}
		else if (TW_STATUS == TW_SR_GCALL_DATA_NACK) //Everybody has pressed => line is free
		{
			//Make the slave return to SLA recognition mode 
			TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWINT);
			while (!I2C_READY && TW_STATUS != TW_SR_GCALL_ACK);
			LOGI("Everybody is ready !");
			switchMode(GAME_MODE_COUNTDOWN);
			break;
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
	// isPressed = FALSE;
	readButtons();
	countdown();
	if (hasCheat == TRUE)
	{
		switchMode(GAME_MODE_LOST);
		lose();
	}
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
		if (readButtons())
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
	if (TW_STATUS != TW_MT_SLA_ACK)
	{
		switchMode(GAME_MODE_WIN);
		win();
		i2c_stop();
		return ;
	}
	while (1)
	{
		if (readButtons())
		{
			i2c_write(INSTRUCTION_LOSE);
			LOGI("loose send");
			switchMode(GAME_MODE_WIN);
			win();
			break ; 
		}
		i2c_write(INSTRUCTION_CHECK_PRESS);
		if (TW_STATUS == TW_MT_DATA_ACK) //If a slave has won
		{
			LOGI("A slave has won");
			// TWCR = 0;
			// initMaster(); //resend GCALL SLA
			// i2c_write(INSTRUCTION_LOSE);
			// LOGI("Master has sent instruction lose");
			// while (TW_STATUS != TW_MT_SLA_ACK)
			// {
			// 	printHexa(TW_STATUS);
			// } //wait for slave to receive SLA GCE
			// LOGI("sla ackreceived");
			
			switchMode(GAME_MODE_LOST);
			lose();
			break ; 
		}
		else if (TW_STATUS != TW_MT_DATA_NACK)
			LOGE("unexpected");
		TWCR = 0;
		initMaster(); //resend GCALL SLA
		while (TW_STATUS != TW_MT_SLA_ACK); //wait for slave to receive SLA GCE
	}
	_delay_ms(1); //MAybe to make sure everybody has treated information ???
	i2c_stop();
}

void	slaveRoutine(void)
{
	uint8_t	hasWon = FALSE;
	while (1)
	{
		readButtons();
		uint8_t	data = 0;
		i2c_read(&data, !readButtons()); //if pressed return ACK
											//if already won return NACK
		if (TW_STATUS == TW_SR_GCALL_DATA_ACK) //ACK has been returned
		{
			hasWon = TRUE;
			break;
		}
		// else if (TW_SR_GCALL_DATA_NACK && hasWon == TRUE) //If has won and returned NACK
		// 												  //Need to end the game
		// {
		// 	LOGI("Last communication ?");
		// }
		if (data == INSTRUCTION_LOSE)
		{
			LOGI("Slave received instruction lose");
			break;
		}
		// initSlave(); //reset
		// TWCR = 0;
		TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWINT); //reset connexion
		// while (!I2C_READY && TW_STATUS != TW_SR_GCALL_ACK);
		while (!I2C_READY);
	}
	if (hasWon)
	{
		switchMode(GAME_MODE_WIN);
		win();
	}
	else
	{
		switchMode(GAME_MODE_LOST);
		lose();
	}
}

///////////////////////////////////////////////////////////////////
// LED

uint8_t	delayCheck(double d)
{
	uint8_t	cheat = FALSE;

	for (double i = 0; i < d; i++)
	{
		if (readButtons() == TRUE)
			cheat = TRUE;
		_delay_ms(1);
	} 
	return (cheat);
}

void	countdown(void)
{
	DDRB |= (1 <<PB0) | (1 <<PB1) | (1 <<PB2) | (1 <<PB4);
	PORTB |= (1 <<PB0) | (1 <<PB1) | (1 <<PB2) | (1 <<PB4);
	
	if (delayCheck(1000))
		hasCheat = TRUE;
	PORTB &= ~(1 <<PB4);
	if (delayCheck(1000))
		hasCheat = TRUE;
	PORTB &= ~(1 <<PB2);
	if (delayCheck(1000))
		hasCheat = TRUE;
	PORTB &= ~(1 <<PB1);
	if (delayCheck(1000))
		hasCheat = TRUE;
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
