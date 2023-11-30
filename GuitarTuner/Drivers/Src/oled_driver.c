/*
 * oled_driver.c
 *
 *  Created on: Nov 29, 2023
 *      Author: sgaviriav
 */

// Importando librerías necesarias
#include <stdint.h>
#include "oled_driver.h"
#include "i2c_driver_hal.h"




/* Funciones públicas para el manejo de la comunicación I2C con la OLED */

/*
 * Función para iniciar la comunicación con la OLED.
 * Por defecto, siempre se envía el ADDRESS y el bit de WRITTE
 */
void oled_startComunication(I2C_Handler_t *ptrHandlerI2C){

	/* 1. Generamos la condición de "Start" */
	i2c_StartTransaction(ptrHandlerI2C);

	/* 2. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
	/* 2A. Definimos una variable auxiliar para leer los
	 * registros para la secuencia de la bandera del ADDR
	 */
	uint8_t auxByte = 0;
	(void) auxByte;

	/* 2B. Enviamos la dirección del Slave. En el bit menos significativo ponemos
	 * el valor Escritura = LOW (0)
	 */
	ptrHandlerI2C->ptrI2Cx->DR = OLED_ADDRESS_W;

	/* Esperamos hasta que la bandera del evento ADDR se levante
	 * (esto nos indica que la dirección fue enviada satisfactoriamente,
	 * junto con el bit de Lectura o Escritura)
	 */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_ADDR)){
		__NOP();
	}

	/* 3B. Debemos limpiar la bandera de la recepción de ACK del ADDR, para
	 * lo cual debemos leer en secuencia el I2C_SR1 y luego el I2C_SR2
	 */
	auxByte = ptrHandlerI2C->ptrI2Cx->SR1;
	auxByte = ptrHandlerI2C->ptrI2Cx->SR2;

}


/*
 * Función para enviar un comando (configuración) a la pantalla
 */
void oled_sendCommand(I2C_Handler_t *ptrHandlerI2C, uint8_t command){

	/* 1. Cargamos el Control Byte primero, para indicar que es un comando */
	ptrHandlerI2C->ptrI2Cx->DR = CONTROL_BYTE_COMMAND;

	/* Esperamos hasta que el byte sea transmitido */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_TXE)){
		__NOP();
	}

	/* 2. Enviamos el comando que deseamos para la pantalla */
	ptrHandlerI2C->ptrI2Cx->DR = command;

	/* Esperamos hasta que el byte sea transmitido */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_BTF)){
		__NOP();
	}

}


/*
 * Función para enviar un dato (gestionado por GDDRAM -> Grapich Display Data RAM)
 * para la gestión de los píxeles en la pantalla (Display)
 */
void oled_sendData(I2C_Handler_t *ptrHandlerI2C, uint8_t data){

	/* 1. Cargamos el Control Byte primero, para indicar que es un dato */
	ptrHandlerI2C->ptrI2Cx->DR = CONTROL_BYTE_DATA;

	/* Esperamos hasta que el byte sea transmitido */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_TXE)){
		__NOP();
	}

	/* 2. Enviamos el dato que deseamos almacenar en la GDDRAM */
	ptrHandlerI2C->ptrI2Cx->DR = data;

	/* Esperamos hasta que el byte sea transmitido */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_BTF)){
		__NOP();
	}
}
