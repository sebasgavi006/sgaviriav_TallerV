/*
 * oled_driver.h
 *
 *  Created on: Nov 29, 2023
 *      Author: sgaviriav
 */

#ifndef OLED_DRIVER_H_
#define OLED_DRIVER_H_

#include <stdint.h>
#include "stm32f4xx.h"
#include "i2c_driver_hal.h"

/* Definimos valores de bits que corresponden a constantes */
#define OLED_ADDRESS			0b0111100	// Dirección de la pantalla OLED y bit de escritura
#define OLED_WRITE				0b0			// Bit para indicar escritura en la OLED
#define OLED_READ				0b1			// Bit para indicar lectura en la OLED
#define	CONTROL_BYTE_COMMAND	0b00000000	// Byte de control para indicar que se enviará un comando
#define	CONTROL_BYTE_DATA		0b01000000	// Byte de control para indicar que se enviará un dato de escritura


enum
{
	OLED_DISPLAY_OFF = 0xAE,
	OLED_DISPLAY_ON
};


enum
{
	OLED_DISPLAY_ON_RESUME = 0xA4,
	OLED_DISPLAY_ON_RESTART
};


/* Modo de direccionamiento de memoria */
enum
{
	HORIZONTAL_ADDRESSING = 0b00,
	VERTICAL_ADDRESSING,
	PAGE_ADDRESSING
};

enum{
	MAX_CONTRAST = 0xFF,
	MIN_CONTRAST = 0x01,
	MIDDLE_CONTRAST = 0x81
};

enum{
	SEG0_IN_COLUMN_0 = 0,
	SEG0_IN_COLUMN_127
};

enum{
	NORMAL_DISPLAY = 0xA6,
	INVERSE_DISPLAY
};


/* Funciones públicas para el manejo de la comunicación I2C con la OLED */
void oled_startComunication(I2C_Handler_t *ptrHandlerI2C);
void oled_stopComunication(I2C_Handler_t *ptrHandlerI2C);
void oled_sendCommand(I2C_Handler_t *ptrHandlerI2C, uint8_t *command, uint16_t length);
void oled_sendData(I2C_Handler_t *ptrHandlerI2C, uint8_t *data, uint16_t length);
void oled_setString(I2C_Handler_t *ptrHandlerI2C, uint8_t *string, uint8_t display_mode, uint8_t length, uint8_t start_column, uint8_t start_page);
void setLetter(uint8_t letter, uint8_t *letterArray);
void oled_onDisplay(I2C_Handler_t *ptrHandlerI2C);
void oled_offDisplay(I2C_Handler_t *ptrHandlerI2C);
void oled_Config(I2C_Handler_t *ptrHandlerI2C);
void oled_setAddressingMode(I2C_Handler_t *ptrHandlerI2C, uint8_t mode);
void oled_setDisplayStartLine(I2C_Handler_t *ptrHandlerI2C, uint8_t row);
void oled_setColumn(I2C_Handler_t *ptrHandlerI2C, uint8_t start_column, uint8_t end_column);
void oled_setPage(I2C_Handler_t *ptrHandlerI2C, uint8_t start_page, uint8_t end_page);
void oled_setContrastControl(I2C_Handler_t *ptrHandlerI2C, uint8_t contrast);
void oled_setSegmentRemap(I2C_Handler_t *ptrHandlerI2C, uint8_t column_address);
void oled_setNormalDisplay(I2C_Handler_t *ptrHandlerI2C);


#endif /* OLED_DRIVER_H_ */
