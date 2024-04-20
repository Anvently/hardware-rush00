#ifndef LOG_LVL
 #define LOG_LVL LOG_INFO
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <log.h>
#include <uart.h>
#include <i2c.h>
#include <stdlib.h>
#include <game.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#ifndef TRUE
 #define TRUE 1
#endif

#ifndef FALSE
 #define FALSE 0
#endif

#ifndef HEXCODE
 #define HEXCODE "0123456789ABCDEF"
#endif

volatile uint8_t	isPressed = FALSE;

volatile uint32_t	MILLI_COUNTER = 0;
volatile uint16_t	LED_TIMER = 0;

ISR (TIMER1_COMPA_vect) //triggered every ms
{
	
	if (LED_TIMER++ >= 2000)
		PORTB &= ~(1 << PB0); //Reset error led every 2s
	MILLI_COUNTER++;
}

ISR (INT0_vect) //Button 1
{
	if (EICRA == 0b01) //If the button was pressed while the toggle detection mode is set
						// => that means event is actually caused by button release
	{
		EICRA = 0b10; //Set the detection mode back to falling edge 
		MILLI_COUNTER = 0; //Reset the timer to 0
		return;
	}
	if (MILLI_COUNTER > 20) //If the boutton had enough time to debounce
	{
		isPressed = TRUE;
		MILLI_COUNTER = 0; //Reset the counter to 0
		EICRA = 0b01; //Set the detection mode to toggle to detect button release event
	}
}

int	main(void)
{
	/* ------------------------ BUTTON 1 INTERRUPT CONFIG ----------------------- */

	EIMSK |= (1 << INT0);

	EICRA |= (1 << ISC01); //falling edge on INT0 pin will trigger INT0

	//  Timer 1 is set to 1000Hz, every OCR1A match compare will generate an interrupt incrementing MILLI_COUNTER

	OCR1A = 8000; //Set TOP value
	TCCR1B |= (1 << CS10); //prescale to 1

	//Set operation mode to 11
	TCCR1A |= (1 << WGM11) | (1 << WGM10);
	TCCR1B |= (1 << WGM13);
	
	TIMSK1 |= (1 << OCIE1A);
	sei();

	LOGI("Starting");
	initGame();
	// detectMode();

	// while (1);m
}
