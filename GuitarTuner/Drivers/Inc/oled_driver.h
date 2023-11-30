/*
 * oled_driver.h
 *
 *  Created on: Nov 29, 2023
 *      Author: sgaviriav
 */

#ifndef OLED_DRIVER_H_
#define OLED_DRIVER_H_

#include <stdint.h>
#include "i2c_driver_hal.h"

/* Definimos valores de bits que corresponden a constantes */
#define OLED_ADDRESS_W			0b01111000	// Dirección de la pantalla OLED y bit de escritura
#define OLED_ADDRESS_R			0b01111001	// Dirección de la pantalla OLED y bit de lectura
#define	CONTROL_BYTE_COMMAND	0b00000000	// Byte de control para indicar que se enviará un comando
#define	CONTROL_BYTE_DATA		0b01000000	// Byte de control para indicar que se enviará un dato de escritura





/* Funciones públicas para el manejo de la comunicación I2C con la OLED */
void oled_startComunication(I2C_Handler_t *ptrHandlerI2C);
void oled_sendCommand(uint8_t command);
void oled_sendData(uint8_t data);


#endif /* OLED_DRIVER_H_ */
