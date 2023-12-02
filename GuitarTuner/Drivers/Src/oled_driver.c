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




///* Funciones públicas para el manejo de la comunicación I2C con la OLED */
//
///*
// * Función para iniciar la comunicación con la OLED.
// * Por defecto, siempre se envía el ADDRESS y el bit de WRITTE
// */
//void oled_startComunication(I2C_Handler_t *ptrHandlerI2C, uint8_t ){
//
//	/* 1. Generamos la condición de "Start" */
//	i2c_StartTransaction(ptrHandlerI2C);
//
//	/* 2. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
//	/* 2A. Definimos una variable auxiliar para leer los
//	 * registros para la secuencia de la bandera del ADDR
//	 */
//	uint8_t auxByte = 0;
//	(void) auxByte;
//
//	/* 2B. Enviamos la dirección del Slave. En el bit menos significativo ponemos
//	 * el valor Escritura = LOW (0)
//	 */
//	ptrHandlerI2C->ptrI2Cx->DR = ptrHandlerI2C->slaveAddress;
//
//	/* Esperamos hasta que la bandera del evento ADDR se levante
//	 * (esto nos indica que la dirección fue enviada satisfactoriamente,
//	 * junto con el bit de Lectura o Escritura)
//	 */
//	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_ADDR)){
//		__NOP();
//	}
//
//	/* 3B. Debemos limpiar la bandera de la recepción de ACK del ADDR, para
//	 * lo cual debemos leer en secuencia el I2C_SR1 y luego el I2C_SR2
//	 */
//	auxByte = ptrHandlerI2C->ptrI2Cx->SR1;
//	auxByte = ptrHandlerI2C->ptrI2Cx->SR2;
//
//}


/*
 * Función para enviar un  comando (configuración) a la pantalla
 */
void oled_sendCommand(I2C_Handler_t *ptrHandlerI2C, uint8_t *command, uint8_t length){

	/* 1. Generamos la condición de "Start" */
	i2c_StartTransaction(ptrHandlerI2C);

	/* 2. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, OLED_WRITE);

	/* 3. Enviamos la dirección de memoria que deseamos escribir */
	i2c_SendMemoryAddress(ptrHandlerI2C, CONTROL_BYTE_COMMAND);

	/* 4. Enviamos el valor que deseamos escribir en el registro seleccionado */
	for(uint8_t i = 0; i < length; i++){
		i2c_SendDataByte(ptrHandlerI2C, command[i]);
		if(i < length){
			i2c_SendDataByte(ptrHandlerI2C, CONTROL_BYTE_COMMAND);
		}
	}

	/* 5. Generamos la condición Stop, para que el slave se detenga después de 1 byte */
	i2c_StopTransaction(ptrHandlerI2C);

}


/*
 * Función para enviar un dato a la memoria de la pantalla GDDRAM (Graphic Display Data RAM)
 */
void oled_sendData(I2C_Handler_t *ptrHandlerI2C, uint8_t *data, uint8_t length){

	/* 1. Generamos la condición de "Start" */
	i2c_StartTransaction(ptrHandlerI2C);

	/* 2. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, OLED_WRITE);

	/* 3. Enviamos la dirección de memoria que deseamos escribir */
	i2c_SendMemoryAddress(ptrHandlerI2C, CONTROL_BYTE_DATA);

	/* 4. Enviamos el valor que deseamos escribir en el registro seleccionado */
	for(uint8_t i = 0; i < length; i++){
		i2c_SendDataByte(ptrHandlerI2C, data[i]);
		if(i < length){
			i2c_SendDataByte(ptrHandlerI2C, CONTROL_BYTE_DATA);
		}
	}

	/* 5. Generamos la condición Stop, para que el slave se detenga después de 1 byte */
	i2c_StopTransaction(ptrHandlerI2C);

}


/*
 * Función para encender la OLED
 */
void oled_onDisplay(I2C_Handler_t *ptrHandlerI2C){
	/* Comando para encender la pantalla OLED */
	uint8_t array[1] = {0xAF};
	oled_sendCommand(ptrHandlerI2C, array, 1);
}


/*
 * Función para apagar la OLED
 */
void oled_offDisplay(I2C_Handler_t *ptrHandlerI2C){
	/* Comando para apagar la pantalla OLED */
	uint8_t array[1] = {0xAE};
	oled_sendCommand(ptrHandlerI2C, array, 1);
}

/*
 * Función para definir el modo de direccionamiento de memoria de la pantalla.
 * Cada modo, define la manera en que se hará la escritura de los píxeles
 * en la pantalla y, por tanto, el orden como debemos enviar la información (sendData)
 */
void oled_setAddressingMode(I2C_Handler_t *ptrHandlerI2C, uint8_t mode){
	uint8_t array[2] = {0};
	array[0] = 0x20;
	array[1] = mode;
	oled_sendCommand(ptrHandlerI2C, array, 2);
}



/*
 * Función para establecer el constraste de la pantalla
 */
void oled_setContrastControl(I2C_Handler_t *ptrHandlerI2C, uint8_t contrast){
	uint8_t array[2] = {0};
	array[0] = 0x81;
	array[1] = contrast;
	oled_sendCommand(ptrHandlerI2C, array, 2);

}


/*
 * Función para indicar si el SEG0 está ubicado en la columna 0 o en la columna 127
 */
void oled_setSegmentRemap(I2C_Handler_t *ptrHandlerI2C, uint8_t column_address){
	oled_sendCommand(ptrHandlerI2C, &column_address, 1);
}


void oled_setNormalDisplay(I2C_Handler_t *ptrHandlerI2C){
	oled_sendCommand(ptrHandlerI2C, (uint8_t *)0xA6, 1);
}



/* ===== Las siguientes funciones establecen la manera en cómo funcionará el =====
 * ===== puntero de filas y columnas para los modos de direccionamiento de   =====
 * ===== memoria Horizontal y Vertical										 =====
 */
/*
 * Envía el valor de la columna inicial y final para los modos Horizontal y Vertical del
 * Direccionamiento de memoria
 */
void oled_setColumn(I2C_Handler_t *ptrHandlerI2C, uint8_t start_column, uint8_t end_column){
	uint8_t array[3] = {0};
	array[0] = 0x21;
	array[1] = start_column;
	array[2] = end_column;
	oled_sendCommand(ptrHandlerI2C, array, 1);
}


/*
 * Función para enviar el valor inicial y final de las páginas que se usaran, para los modos
 * horizontal y vertical en el Direccionamiento de memoria
 */
void oled_setPage(I2C_Handler_t *ptrHandlerI2C, uint8_t start_page, uint8_t end_page){
	uint8_t array[3] = {0};
	array[0] = 0x22;
	array[1] = start_page;
	array[2] = end_page;
	oled_sendCommand(ptrHandlerI2C, array, 1);
}
