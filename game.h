/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   game.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: npirard <npirard@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/19 17:07:56 by npirard           #+#    #+#             */
/*   Updated: 2024/04/20 18:57:30 by npirard          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GAME_H
# define GAME_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <error_led.h>
#include <uart.h>
#include <log.h>
#include <i2c.h>
#include <error_led.h>

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
	#define LOG_LVL LOG_INFO
#endif

void	initGame(void);
void	detectMode(void);
void	MasterMode(void);


#endif
