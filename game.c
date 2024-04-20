/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   game.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: npirard <npirard@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/20 15:48:45 by npirard           #+#    #+#             */
/*   Updated: 2024/04/20 16:22:05 by npirard          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <i2c.h>
#include <log.h>
#include <error_led.h>

/*

Different modes 
	- device is a MASTER who won control of SDA in general call
		- continuously send DATA in general call mode
			- if a slave didn't had a button pushed, he send ACK
			- if a slave had a button pushed, he send NACK
				- he knows for sure that he won
		- if NACK, it means that a slave pushed the button
			- master knows he has lost
			- he stops communication to inform the slaves they have lost
			
		

*/

static uint8_t	mode =	 I2C_MODE_MASTER;

void	init(void)
{
	i2c_init(100000, I2C_ENABLE_GC, mode); //Init TWI interface enabling general call recognition
										   // and master mode (not pulling TWEA at beginning)
	i2c_start_MT(0x00);
}
