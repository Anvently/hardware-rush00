#ifndef GAME_H
# define GAME_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <error_led.h>
#include <uart.h>
#include <log.h>
#include <i2c.h>
#include <error_led.h>
#include <util/delay.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#ifndef TRUE
 #define TRUE 1
#endif

#ifndef FALSE
 #define FALSE 0
#endif

#define LOG_DISABLE 0
#define LOG_ERROR 1
#define LOG_INFO 2
#define LOG_DEBUG 3

#define GAME_MODE_WAIT_MASTER 0
#define GAME_MODE_WAIT_EVERYBODY 1
#define GAME_MODE_COUNTDOWN 2
#define GAME_MODE_PUSH 3
#define GAME_MODE_WIN 4
#define GAME_MODE_LOST 5

#define INSTRUCTION_CHECK_PRESS 1
#define INSTRUCTION_CHECK_NOT_PRESS 2
#define INSTRUCTION_START_COUNTDOWN 3
#define INSTRUCTION_LOSE 4

#ifndef LOG_LVL
	#define LOG_LVL LOG_INFO
#endif

void	countdown(void);
void	detectMode(void); ////UNUSED
void	initGame(void);
void	initMaster(void);
void	initSlave(void);
void	lose(void);
void	masterRoutine(void);
void	readButtons(void);
void	setRole(void);
void	slaveRoutine(void);
void	win(void);

void	waitEverybody(void);

#endif
