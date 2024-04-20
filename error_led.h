#ifndef ERROR_LED_H
# define ERROR_LED_H

#include <avr/interrupt.h>
#include <avr/io.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#ifndef TRUE
 #define TRUE 1
#endif

#ifndef FALSE
 #define FALSE 0
#endif

void	init(void);
uint8_t	error(void);

#endif