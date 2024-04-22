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

#define CHEAT_TIMEOUT 500U

static uint8_t	mode =	 I2C_MODE_MASTER;
static uint8_t	gameMode = GAME_MODE_WAIT_MASTER;
static volatile uint8_t	pressedEvent = FALSE;
static volatile uint8_t	isReady = FALSE;
static uint8_t	hasCheat = FALSE;

#define BTN1_IS_PRESSED ((PIND & (1 << PD2)) == 0)
#define BTN2_IS_PRESSED ((PIND & (1 << PD4)) == 0)

void	readButtons(void)
{
	static uint8_t	wasPressed = FALSE;

	if (BTN1_IS_PRESSED && wasPressed == FALSE) //if button is pressed for the first time
	{
		wasPressed = TRUE;
		pressedEvent = TRUE;
	}
	else if (BTN1_IS_PRESSED == FALSE) //If state changes
		wasPressed = FALSE;
}

void	switchMode(uint8_t newMode)
{
	PORTD &= ~((1 << PD5) | (1 << PD6) | (1 << PD3)); //turn off leds
	switch (newMode)
	{
		case GAME_MODE_COUNTDOWN:
			break;
	
		case GAME_MODE_WAIT_MASTER:
			PORTD |= (1 << PD5) | (1 << PD6); //turn on red and green led => yellow
			//may want to blink led
			break;

		case GAME_MODE_WAIT_EVERYBODY:
			if (mode == I2C_MODE_MASTER)
				PORTD |= (1 << PD3) | (1 << PD6); //turn on blue and green led
			else
				PORTD |= (1 << PD3); //turn on blue led
			break;

		case GAME_MODE_PUSH:
			PORTD |= (1 << PD3); //turn on blue led
			break;

		case GAME_MODE_WIN:
			PORTD |= (1 << PD6); //Turn on green led
			break;

		case GAME_MODE_LOST:
			PORTD |= (1 << PD5); //turn on red led
			break;
		
		default:
			break;
	}
	gameMode = newMode;
}

void	initGame(void)
{
	readButtons();
	LOGI("Starting");
	switchMode(GAME_MODE_WAIT_MASTER);
	mode = I2C_MODE_SLAVE;
	isReady = FALSE;
	pressedEvent = FALSE;
	hasCheat = FALSE;
	TWCR = (1 << TWINT);
	initSlave();
	setRole();
	LOGI("Waiting for everybody to be ready");
	waitEverybody();
	readButtons();
	if (pressedEvent == TRUE)
		pressedEvent = FALSE;
	countdown();
	if (hasCheat == TRUE)
		LOGI("You have cheated !");
	switchMode(GAME_MODE_PUSH);
	if (mode == I2C_MODE_MASTER)
		masterRoutine();
	else
		slaveRoutine();
	
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
		if (pressedEvent)
		{
			mode = I2C_MODE_MASTER;
			LOGI("Switching to master");
			isReady = TRUE;
			pressedEvent = FALSE;
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

	while (1)
	{
		readButtons();
		i2c_read(&data, isReady); //send NACK if button pressed else send ACK
		if (pressedEvent) //slave should have return NACK
		{
			LOGI("Button was pressed, disconnected from bus and waiting for start condition");
			pressedEvent = FALSE;
			isReady = TRUE;
			//Wait for GCALL ?
		}
		// printHexa(TW_STATUS);
		if (data == INSTRUCTION_START_COUNTDOWN)
		{
			if (isReady == FALSE)
				LOGE("Slave was ordered to go to countdown while button not pressed");
			LOGI("Everybody is ready !");
			switchMode(GAME_MODE_COUNTDOWN);
			break;
		}
		else if (TW_STATUS == TW_SR_GCALL_DATA_NACK) //Everybody has pressed => line is free
		{
			LOGI("PING");
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

///////////////////////////////////////////////////////////////////
// ROUTINES

void	masterLaunchGame(void)
{
	uint16_t time = 0;

	TWCR = 0;
	i2c_stop();
	if (hasCheat)
	{
		switchMode(GAME_MODE_LOST);
		lose();
		return;
	}
	i2c_start(0, I2C_MODE_MASTER_TX);
	while (TW_STATUS != TW_MT_SLA_ACK) //NEED A TIMEOUT
	{
		if (time++ >= CHEAT_TIMEOUT)
			break;
		i2c_start(0, I2C_MODE_MASTER_TX);
		// TWCR = 0;
		// initMaster(); //Restart master
		_delay_ms(1);
	}
	if (TW_STATUS == TW_MT_SLA_NACK) //If no slave has answere, then we won
	{
		switchMode(GAME_MODE_WIN);
		win();
		i2c_stop();
		return ;
	}
}

void	masterRoutine(void)
{
	uint8_t	data = 0;
	masterLaunchGame();
	if (gameMode != GAME_MODE_PUSH || hasCheat)
		return;
	LOGI("Starting game as master");
	while (1)
	{
		readButtons();
		if (pressedEvent)
		{
			pressedEvent = FALSE;
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
	// _delay_ms(1); //MAybe to make sure everybody has treated information ???
	i2c_stop();
}

void	slaveLaunchGame(void)
{
	uint16_t time = 0;

	readButtons();
	TWCR = 0;
	if (hasCheat)
	{
		switchMode(GAME_MODE_LOST);
		lose();
		return;
	}
	initSlave();
	while (TW_STATUS != TW_SR_GCALL_ACK) //Wait for the master to ping the slave
	{
		if (time++ >= CHEAT_TIMEOUT) //If master is not pinging, then he probably cheated
		{
			TWCR = 0;
			switchMode(GAME_MODE_WIN);
			LOGI("You win");
			win();
			break;
		}
		TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN); //Enable interface and set TWEA to high
		_delay_ms(1);
	}
}

void	slaveRoutine(void)
{
	uint8_t		data = 0;
	uint8_t		hasWon = FALSE;

	slaveLaunchGame();
	if (gameMode != GAME_MODE_PUSH || hasCheat)
		return;
	LOGI("Starting game as slave");
	while (1)
	{
		readButtons();
		i2c_read(&data, !pressedEvent); //if pressed return ACK
		if (TW_STATUS == TW_SR_GCALL_DATA_ACK && (pressedEvent)) //ACK has been returned
		{
			hasWon = TRUE;
			pressedEvent = FALSE;
			TWCR = (1 << TWINT);
			break;
		}
		else if (TW_STATUS == TW_SR_GCALL_DATA_ACK && !pressedEvent)
		{
			LOGI("Other slave has won");
			break;
		}
		if (data == INSTRUCTION_LOSE)
		{
			LOGI("Slave received instruction lose");
			break;
		}
		TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWINT); //reset connexion
		while (TW_STATUS != TW_SR_GCALL_ACK); //Wait for the master to restart ACK
	}
	if (hasWon)
	{
		TWCR = 0; //Disable TWI interface tonot disturbed potential over slaves
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
		readButtons();
		if (pressedEvent)
		{
			cheat = TRUE;
			pressedEvent = FALSE;
		}
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
}
