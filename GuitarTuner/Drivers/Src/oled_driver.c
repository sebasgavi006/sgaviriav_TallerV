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


/*
 * Función para enviar un  comando (configuración) a la pantalla
 */
void oled_sendCommand(I2C_Handler_t *ptrHandlerI2C, uint8_t *command, uint16_t length){

	/* 1. Generamos la condición de "Start" */
	i2c_StartTransaction(ptrHandlerI2C);

	/* 2. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, OLED_WRITE);

	/* 3. Enviamos la dirección de memoria que deseamos escribir */
	i2c_SendMemoryAddress(ptrHandlerI2C, CONTROL_BYTE_COMMAND);

	/* 4. Enviamos el valor que deseamos escribir en el registro seleccionado */
	for(uint16_t i = 0; i < length; i++){
		i2c_SendDataByte(ptrHandlerI2C, command[i]);
//		if(i < length){
//			i2c_SendDataByte(ptrHandlerI2C, CONTROL_BYTE_COMMAND);
//		}
	}

	/* 5. Generamos la condición Stop, para que el slave se detenga después de 1 byte */
	i2c_StopTransaction(ptrHandlerI2C);

}


/*
 * Función para enviar un dato a la memoria de la pantalla GDDRAM (Graphic Display Data RAM)
 */
void oled_sendData(I2C_Handler_t *ptrHandlerI2C, uint8_t *data, uint16_t length){

	/* 1. Generamos la condición de "Start" */
	i2c_StartTransaction(ptrHandlerI2C);

	/* 2. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, OLED_WRITE);

	/* 3. Enviamos la dirección de memoria que deseamos escribir */
	i2c_SendMemoryAddress(ptrHandlerI2C, CONTROL_BYTE_DATA);

	/* 4. Enviamos el valor que deseamos escribir en el registro seleccionado */
	for(uint16_t i = 0; i < length; i++){
		i2c_SendDataByte(ptrHandlerI2C, data[i]);
//		if(i < length){
//			i2c_SendDataByte(ptrHandlerI2C, CONTROL_BYTE_DATA);
//		}
	}

	/* 5. Generamos la condición Stop, para que el slave se detenga después de 1 byte */
	i2c_StopTransaction(ptrHandlerI2C);

}


/*
 * Función para hacer la matriz de bytes de una letra
 */
void setLetter(uint8_t letter, uint8_t *letterArray){
	/* Inicializamos el arreglo donde se codifica la letra */

	switch(letter){
	case 'A':
	letterArray[0] = 0b01111000;
	letterArray[1] = 0b00010100;
	letterArray[2] = 0b00010010;
	letterArray[3] = 0b00010100;
	letterArray[4] = 0b01111000;
	break;

	case 'a':
	letterArray[0] = 0b00110000;
	letterArray[1] = 0b01001000;
	letterArray[2] = 0b01001000;
	letterArray[3] = 0b00101000;
	letterArray[4] = 0b01111000;
	break;

	case 'o':
	letterArray[0] = 0b00110000;
	letterArray[1] = 0b01001000;
	letterArray[2] = 0b01001000;
	letterArray[3] = 0b00110000;
	letterArray[4] = 0b00000000;
	break;

	case 'B':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b01001010;
	letterArray[2] = 0b01001010;
	letterArray[3] = 0b01001010;
	letterArray[4] = 0b00110100;
	break;

	case 'b':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b01010000;
	letterArray[2] = 0b01010000;
	letterArray[3] = 0b01010000;
	letterArray[4] = 0b00100000;
	break;

	case 'C':
	letterArray[0] = 0b00111100;
	letterArray[1] = 0b01000010;
	letterArray[2] = 0b01000010;
	letterArray[3] = 0b01000010;
	letterArray[4] = 0b00100100;
	break;

	case 'c':
	letterArray[0] = 0b00110000;
	letterArray[1] = 0b01001000;
	letterArray[2] = 0b01001000;
	letterArray[3] = 0b01001000;
	letterArray[4] = 0b00100000;
	break;

	case 'D':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b01000010;
	letterArray[2] = 0b01000010;
	letterArray[3] = 0b01000010;
	letterArray[4] = 0b00111100;
	break;

	case 'd':
	letterArray[0] = 0b00100000;
	letterArray[1] = 0b01010000;
	letterArray[2] = 0b01010000;
	letterArray[3] = 0b01010000;
	letterArray[4] = 0b01111110;
	break;

	case 'E':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b01001010;
	letterArray[2] = 0b01001010;
	letterArray[3] = 0b01001010;
	letterArray[4] = 0b01000010;
	break;

	case 'e':
	letterArray[0] = 0b00111000;
	letterArray[1] = 0b01010100;
	letterArray[2] = 0b01010100;
	letterArray[3] = 0b01010100;
	letterArray[4] = 0b01001000;
	break;

	case 'F':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b00001010;
	letterArray[2] = 0b00001010;
	letterArray[3] = 0b00001010;
	letterArray[4] = 0b00000010;
	break;

	case 'f':
	letterArray[0] = 0b01111000;
	letterArray[1] = 0b00010100;
	letterArray[2] = 0b00010100;
	letterArray[3] = 0b00000100;
	letterArray[4] = 0b00001000;
	break;

	case 'G':
	letterArray[0] = 0b00111100;
	letterArray[1] = 0b01000010;
	letterArray[2] = 0b01010010;
	letterArray[3] = 0b01010010;
	letterArray[4] = 0b00110100;
	break;

	case 'g':
	letterArray[0] = 0b00100000;
	letterArray[1] = 0b01001000;
	letterArray[2] = 0b01010100;
	letterArray[3] = 0b01010100;
	letterArray[4] = 0b00111000;
	break;

	case 'H':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b00001000;
	letterArray[2] = 0b00001000;
	letterArray[3] = 0b00001000;
	letterArray[4] = 0b01111110;
	break;

	case 'I':
	letterArray[0] = 0b01000010;
	letterArray[1] = 0b01000010;
	letterArray[2] = 0b01111110;
	letterArray[3] = 0b01000010;
	letterArray[4] = 0b01000010;
	break;

	case 'J':
	letterArray[0] = 0b00110010;
	letterArray[1] = 0b01000010;
	letterArray[2] = 0b01000010;
	letterArray[3] = 0b00111110;
	letterArray[4] = 0b00000010;
	break;

	case 'K':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b00010000;
	letterArray[2] = 0b00011000;
	letterArray[3] = 0b00100100;
	letterArray[4] = 0b01000010;
	break;

	case 'L':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b01000000;
	letterArray[2] = 0b01000000;
	letterArray[3] = 0b01000000;
	letterArray[4] = 0b01000000;
	break;

	case 'l':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b01000000;
	letterArray[2] = 0b00000000;
	letterArray[3] = 0b00000000;
	letterArray[4] = 0b00000000;
	break;

	case 'M':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b00000100;
	letterArray[2] = 0b00001000;
	letterArray[3] = 0b00000100;
	letterArray[4] = 0b01111110;
	break;

	case 'm':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b00000100;
	letterArray[2] = 0b00001000;
	letterArray[3] = 0b00000100;
	letterArray[4] = 0b01111110;
	break;

	case 'N':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b00000100;
	letterArray[2] = 0b00011000;
	letterArray[3] = 0b00100000;
	letterArray[4] = 0b01111110;
	break;

	case 'O':
	letterArray[0] = 0b00111100;
	letterArray[1] = 0b01000010;
	letterArray[2] = 0b01000010;
	letterArray[3] = 0b01000010;
	letterArray[4] = 0b00111100;
	break;

	case 'P':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b00010010;
	letterArray[2] = 0b00010010;
	letterArray[3] = 0b00010010;
	letterArray[4] = 0b00001100;
	break;

	case 'p':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b00010010;
	letterArray[2] = 0b00010010;
	letterArray[3] = 0b00010010;
	letterArray[4] = 0b00001100;
	break;

	case 'Q':
	letterArray[0] = 0b00011100;
	letterArray[1] = 0b00100010;
	letterArray[2] = 0b00110010;
	letterArray[3] = 0b01110010;
	letterArray[4] = 0b01011100;
	break;

	case 'R':
	letterArray[0] = 0b01111110;
	letterArray[1] = 0b00010010;
	letterArray[2] = 0b00010010;
	letterArray[3] = 0b00110010;
	letterArray[4] = 0b01001100;
	break;

	case 'r':
	letterArray[0] = 0b01111000;
	letterArray[1] = 0b00010000;
	letterArray[2] = 0b00010000;
	letterArray[3] = 0b00110000;
	letterArray[4] = 0b00000000;
	break;

	case 'S':
	letterArray[0] = 0b00100100;
	letterArray[1] = 0b01001010;
	letterArray[2] = 0b01001010;
	letterArray[3] = 0b01001010;
	letterArray[4] = 0b00110100;
	break;

	case 's':
	letterArray[0] = 0b00100100;
	letterArray[1] = 0b01001010;
	letterArray[2] = 0b01001010;
	letterArray[3] = 0b01001010;
	letterArray[4] = 0b00110100;
	break;

	case 'T':
	letterArray[0] = 0b00000010;
	letterArray[1] = 0b00000010;
	letterArray[2] = 0b01111110;
	letterArray[3] = 0b00000010;
	letterArray[4] = 0b00000010;
	break;

	case 't':
	letterArray[0] = 0b00000000;
	letterArray[1] = 0b00000100;
	letterArray[2] = 0b01111110;
	letterArray[3] = 0b01000100;
	letterArray[4] = 0b00000000;
	break;

	case 'U':
	letterArray[0] = 0b00111110;
	letterArray[1] = 0b01000000;
	letterArray[2] = 0b01000000;
	letterArray[3] = 0b01000000;
	letterArray[4] = 0b00111110;
	break;

	case 'u':
	letterArray[0] = 0b00111000;
	letterArray[1] = 0b01000000;
	letterArray[2] = 0b01000000;
	letterArray[3] = 0b01000000;
	letterArray[4] = 0b00111000;
	break;

	case 'V':
	letterArray[0] = 0b00011110;
	letterArray[1] = 0b00100000;
	letterArray[2] = 0b01000000;
	letterArray[3] = 0b00100000;
	letterArray[4] = 0b00011110;
	break;

	case 'W':
	letterArray[0] = 0b00111110;
	letterArray[1] = 0b01000000;
	letterArray[2] = 0b00111000;
	letterArray[3] = 0b01000000;
	letterArray[4] = 0b00111111;
	break;

	case 'X':
	letterArray[0] = 0b01100010;
	letterArray[1] = 0b00010110;
	letterArray[2] = 0b00001000;
	letterArray[3] = 0b00010110;
	letterArray[4] = 0b01100010;
	break;

	case 'Y':
	letterArray[0] = 0b00000110;
	letterArray[1] = 0b00001000;
	letterArray[2] = 0b01110000;
	letterArray[3] = 0b00001000;
	letterArray[4] = 0b00000110;
	break;

	case 'Z':
	letterArray[0] = 0b01100010;
	letterArray[1] = 0b01010010;
	letterArray[2] = 0b01001010;
	letterArray[3] = 0b01000110;
	letterArray[4] = 0b01000010;
	break;

	case '!':
	letterArray[0] = 0b01011110;
	letterArray[1] = 0b00000000;
	letterArray[2] = 0b00000000;
	letterArray[3] = 0b00000000;
	letterArray[4] = 0b00000000;
	break;

	case '0':
	letterArray[0] = 0b00111100;
	letterArray[1] = 0b01000110;
	letterArray[2] = 0b01011010;
	letterArray[3] = 0b01100010;
	letterArray[4] = 0b00111100;
	break;

	case '1':
	letterArray[0] = 0b00001000;
	letterArray[1] = 0b00000100;
	letterArray[2] = 0b01111110;
	letterArray[3] = 0b00000000;
	letterArray[4] = 0b00000000;
	break;

	case '2':
	letterArray[0] = 0b01001100;
	letterArray[1] = 0b01100010;
	letterArray[2] = 0b01010010;
	letterArray[3] = 0b01001010;
	letterArray[4] = 0b01000100;
	break;

	case '3':
	letterArray[0] = 0b00100100;
	letterArray[1] = 0b01000010;
	letterArray[2] = 0b01001010;
	letterArray[3] = 0b01001010;
	letterArray[4] = 0b00110100;
	break;

	case '4':
	letterArray[0] = 0b00010000;
	letterArray[1] = 0b00011000;
	letterArray[2] = 0b00010100;
	letterArray[3] = 0b01111110;
	letterArray[4] = 0b00010000;
	break;

	case '5':
	letterArray[0] = 0b01001110;
	letterArray[1] = 0b01001010;
	letterArray[2] = 0b01001010;
	letterArray[3] = 0b01001010;
	letterArray[4] = 0b00110010;
	break;

	case '6':
	letterArray[0] = 0b00111100;
	letterArray[1] = 0b01010010;
	letterArray[2] = 0b01010010;
	letterArray[3] = 0b01010010;
	letterArray[4] = 0b00100100;
	break;

	case '7':
	letterArray[0] = 0b01000010;
	letterArray[1] = 0b00100010;
	letterArray[2] = 0b00010010;
	letterArray[3] = 0b00001010;
	letterArray[4] = 0b00000110;
	break;

	case '8':
	letterArray[0] = 0b00110100;
	letterArray[1] = 0b01001010;
	letterArray[2] = 0b01001010;
	letterArray[3] = 0b01001010;
	letterArray[4] = 0b00110100;
	break;

	case '9':
	letterArray[0] = 0b00100100;
	letterArray[1] = 0b01001010;
	letterArray[2] = 0b01001010;
	letterArray[3] = 0b01001010;
	letterArray[4] = 0b00111100;
	break;

	case '/':
	letterArray[0] = 0b01000000;
	letterArray[1] = 0b00100000;
	letterArray[2] = 0b00011000;
	letterArray[3] = 0b00000100;
	letterArray[4] = 0b00000010;
	break;

	case ':':
	letterArray[0] = 0b00000000;
	letterArray[1] = 0b00000000;
	letterArray[2] = 0b00101000;
	letterArray[3] = 0b00000000;
	letterArray[4] = 0b00000000;
	break;

	case '-':
	letterArray[0] = 0b00000000;
	letterArray[1] = 0b00010000;
	letterArray[2] = 0b00010000;
	letterArray[3] = 0b00010000;
	letterArray[4] = 0b00000000;
	break;

	case '_':
	letterArray[0] = 0b01000000;
	letterArray[1] = 0b01000000;
	letterArray[2] = 0b01000000;
	letterArray[3] = 0b01000000;
	letterArray[4] = 0b01000000;
	break;

	case '.':
	letterArray[0] = 0b00000000;
    letterArray[1] = 0b00000000;
    letterArray[2] = 0b00000000;
    letterArray[3] = 0b01100000;
    letterArray[4] = 0b01100000;
    break;

	case 167:
	letterArray[0] = 0b00001110;
	letterArray[1] = 0b00010001;
	letterArray[2] = 0b00010001;
	letterArray[3] = 0b00010001;
	letterArray[4] = 0b00001110;
	break;

	case '=':
	letterArray[0] = 0b00010000;
	letterArray[1] = 0b00010000;
	letterArray[2] = 0b00010000;
	letterArray[3] = 0b00010000;
	letterArray[4] = 0b00010000;
	break;

	case ' ':
	letterArray[0] = 0b00000000;
	letterArray[1] = 0b00000000;
	letterArray[2] = 0b00000000;
	letterArray[3] = 0b00000000;
	letterArray[4] = 0b00000000;
	break;

	default:{
	__NOP();
	break;
	}
	}

}


/*
 * Función para escribir una palabra en un segmento específico de la pantalla
 */
void oled_setString(I2C_Handler_t *ptrHandlerI2C, uint8_t *string, uint8_t display_mode, uint8_t length, uint8_t start_column, uint8_t start_page){

	/* Ubicamos el area donde se escribirá la palabra
	 * - Una letra ocupa 5 columnas (5 páginas -> 8 bits por página)
	 */
	uint8_t array[6] = 	{0x21,start_column, (start_column+(6*length)-1),
			0x22, start_page, start_page};
	oled_sendCommand(ptrHandlerI2C, array, 6);

	/*
	 * Escribimos el string letra por letra
	 */
	uint8_t letterArray[6];

	if(display_mode == NORMAL_DISPLAY){
		letterArray[5] = 0x00;
		uint8_t i = 0;
		while( i < length){
			setLetter(string[i], letterArray);
			oled_sendData(ptrHandlerI2C, letterArray, 6);
			i++;
		}
	}
	else if(display_mode == INVERSE_DISPLAY){
		letterArray[5] = 0xFF;
		uint8_t i = 0;
		while( i < length){
			setLetter(string[i], letterArray);
			for(uint8_t x = 0; x < 6; x++){
				letterArray[x-1] = ~letterArray[x-1];
			}
			oled_sendData(ptrHandlerI2C, letterArray, 6);
			i++;
		}
	}

}

/*
 * Función para limpiar toda la pantalla OLED
 */
void oled_clearDisplay(I2C_Handler_t *ptrHandlerI2C){

	/* Establecemos los límites de las columnas y páginas de la pantalla */
	oled_setColumn(ptrHandlerI2C, 0, 127);
	oled_setPage(ptrHandlerI2C, 0, 7);

	/* Pintamos todos los píxeles de negro (All -> 0x00) */
	uint8_t array[1024] = {0};
	oled_sendData(ptrHandlerI2C, array, 1024);

	/* Ubicamos el puntero al inicio de la primera columna y la primera página */
	array[0] = 0x40;
	oled_sendCommand(ptrHandlerI2C, array, 1);
}

/*
 * Función para encender la OLED (Carga lo que haya quedado almacenado en la GDDRAM
 */
void oled_onDisplay(I2C_Handler_t *ptrHandlerI2C){
	/* Comando para encender la pantalla OLED */
	uint8_t array[1] = {0xAF};
	oled_sendCommand(ptrHandlerI2C, array, 1);
}


/*
 * Función para apagar la OLED (Sleep mode)
 */
void oled_offDisplay(I2C_Handler_t *ptrHandlerI2C){
	/* Comando para apagar la pantalla OLED */
	uint8_t array[1] = {0xAE};
	oled_sendCommand(ptrHandlerI2C, array, 1);
}


/*
 * Función para cargar la configuración inicial de la OLED
 */
void oled_Config(I2C_Handler_t *ptrHandlerI2C){

	/* 1. Cargamos la secuencia de comandos */
	uint8_t buffer[30] = 	{0xAE, 0x20, 0x00, 0x21, 0, 127, 0x22, 0, 7, 0x40, 0x81, 0xCF, 0xA1,
							 0xA6, 0xA8, 0x3F, 0xC8, 0xD3, 0x00, 0xD5, 0x80,
							 0xD9, 0xF1, 0xDA, 0x12, 0xDB, 0x20, 0x8D, 0x14,
							 0xAF};

	/* 2. Se envía la configuración por I2C */
	oled_sendCommand(ptrHandlerI2C, buffer, 30);
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


/*
 * Función que envía un comando para configurar la pantalla en modo Normal
 */
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
	uint8_t array[3] = {0x21, start_column, end_column};
	oled_sendCommand(ptrHandlerI2C, array, 3);
}


/*
 * Función para enviar el valor inicial y final de las páginas que se usaran, para los modos
 * horizontal y vertical en el Direccionamiento de memoria
 */
void oled_setPage(I2C_Handler_t *ptrHandlerI2C, uint8_t start_page, uint8_t end_page){
	uint8_t array[3] = {0x22, start_page, end_page};
	oled_sendCommand(ptrHandlerI2C, array, 3);
}
