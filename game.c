/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   game.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: npirard <npirard@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/20 15:48:45 by npirard           #+#    #+#             */
/*   Updated: 2024/04/20 18:22:23 by npirard          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

static uint8_t	mode =	 I2C_MODE_MASTER;

void	initGame(void)
{
	i2c_init(100000, I2C_ENABLE_GC, mode); //Init TWI interface enabling general call recognition
										   // and master mode (not pulling TWEA at beginning)
	// i2c_start(0x00, I2C_MODE_TX); //Try general call as master TX

	TWCR = (1 << TWSTA) | (1 << TWINT) | (1 << TWEN); //send start condition

	while (!I2C_READY);

	if (!(TW_STATUS & TW_START) && !(TW_STATUS & TW_REP_START))
		LOGI("Start condition could not be sent");

	LOGD("Start condition was sent !");

	TWDR = 0x00 | mode; //Set address of receiver and mode
	TWCR = (1 << TWINT) | (1 << TWEN); //Set the interrupt flag to send content of TWDR buffer

	while (!I2C_READY);
	detectMode(); //check status of i2c_start
	if (mode == I2C_MODE_MASTER)
		LOGI("Master mode");
	else
	{
		// TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
		LOGI("Slave mode");
	}
	detectMode();
}

void	detectMode(void)
{
	switch (TW_STATUS)
	{
		//Device took the line
		case TW_MT_SLA_ACK:
			LOGI("Another master detected. Device is now entering master mode");
			mode = I2C_MODE_MASTER;
			break;

		//No one answered to the given address
		//Means that you are master
		case TW_MT_SLA_NACK: 
			LOGI("Device entering slave mode ?");
			mode = I2C_MODE_MASTER;
			break;

		//Another master took control of the line. SHould not happen (because general call should be answered)
		case TW_MR_ARB_LOST:
			LOGI("Arbitration lost. Device entering slave mode.");
			mode = I2C_MODE_SLAVE; 
			break;

		//Lost arbitration and addressed by general call => slave mode 
		case TW_SR_ARB_LOST_GCALL_ACK:
			LOGI("Arbitration lost and general call answered. Device entering slave mode.");
			mode = I2C_MODE_SLAVE; 
			break;

		//Slave answered acknowledge to general call (not supposed to happen)
		case TW_SR_GCALL_ACK:
			LOGI("Slave answered general call");
			break;

		//Slave received data and returned acknowledge => game is still running
		case TW_SR_GCALL_DATA_ACK:
			LOGI("Slave ACK data received");
			break;

		//Slave received data when addressed in general call, but returned NACK
		//Means that the slave cleared TWEA flag in the previous read beacause he won
		//So not supposed to happen
		case TW_SR_GCALL_DATA_NACK:
			LOGI("Slave NACK data received");
			break;


		// //Slave device is ready to send data
		// case TW_MR_SLA_ACK:
		// 	LOGI("SLA ACK received from slave device");
		// 	break;

		// //No one answered to the given adress
		// //It should mean there is another master in control
		// case TW_MR_SLA_NACK:
		// 	LOGI("SLA NACK received from slave device !!");
		// 	break;

		default:
			print("Unknown status code: ", 0);
			printHexa(TW_STATUS);
	}
}
