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

int	main(void)
{
	DDRD |= (1 << PD5) | (1 << PD6) | (1 << PD3); //for RDB leds
	LOGI("Reset");
	while (1)
		initGame();

	while (1);
}

