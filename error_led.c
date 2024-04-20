/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_led.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: npirard <npirard@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/19 17:16:06 by npirard           #+#    #+#             */
/*   Updated: 2024/04/20 18:48:52 by npirard          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOG_H
# define LOG_H

#include <error_led.h>

static uint8_t	isInit = FALSE;

/// @brief Turn on LED1 for 2s
/// @param  
/// @return 
/// @note timer1 is configured at 0.5Hz and interrupt is enable to catch TIMER1_COMPA
uint8_t	error(void)
{
	if (isInit = FALSE)
		init();
	PORTB |= (1 << PB0); //set error led to ON
	return (1);
}

void	init(void)
{
	DDRB |= (1 << DDD0); //set led 0 to output

	/* ------------------------------ TIMER CONFIG ------------------------------ */
	// We want 0.5Hz
	//		=> with prescaler =  64, OCR1A = 62500

	// //Mode to 11
	// TCCR1A |= (1 << WGM11) | (1 << WGM10);
	// TCCR1B |= (1 << WGM13);

	// TCCR1B |= (1 << CS12); //prescaler to 256

	// TIMSK1 |= (1 << OCIE1A); //Enable OCR1A match compare interrupt

	// OCR1A = 62500;

	// sei();
}

#endif