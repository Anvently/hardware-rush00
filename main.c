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

#define BTN1_IS_PRESSED ((PIND & (1 << PD2)) == 0)
#define BTN2_IS_PRESSED ((PIND & (1 << PD4)) == 0)

volatile uint8_t	isPressed = FALSE;

void	readButtons(void)
{
	if (BTN1_IS_PRESSED && isPressed == FALSE) //if button is pressed for the first time
		isPressed = TRUE;
	else if (BTN1_IS_PRESSED == FALSE) //If state changes
		isPressed = FALSE;
	// if (BTN2_IS_PRESSED && isPressed2 == FALSE) //if button is pressed for the first time
	// {
	// 	decrement();
	// 	isPressed2 = TRUE;
	// }
	// else if (BTN2_IS_PRESSED == FALSE) //If state changes
	// 	isPressed2 = FALSE;
}

int	main(void)
{
	LOGI("Starting");
	initGame();
	// detectMode();

	while (1);
}

