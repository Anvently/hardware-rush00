#ifndef LOG_H
# define LOG_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <error_led.h>
#include <uart.h>

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

#ifndef LOG_LVL
	#define LOG_LVL LOG_DEBUG
#endif

#ifndef HEXCODE
 #define HEXCODE "0123456789ABCDEF"
#endif


void	LOGI(const char* str);
void	LOGD(const char* str);
void	LOGE(const char* str);

void	print(const char* str, uint8_t newline);

void	printHexa(uint8_t);
void	printHexa16(uint16_t);
void	printHexa32(uint32_t);
void	printHexa64(uint64_t);

#endif
