/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Prueba para implementar la pantalla OLED (prueba de funcionamiento)
 ******************************************************************************
 **/

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "arm_math.h"

#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "systick_driver_hal.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "usart_driver_hal.h"
#include "i2c_driver_hal.h"

#include "oled_driver.h"


// Definicion de los Handlers necesario
GPIO_Handler_t userLed = {0}; //Led de estado PinA5
Timer_Handler_t blinkTimer = {0};
Timer_Handler_t blinkString = {0};
GPIO_Handler_t pinSCL_I2C = {0};
GPIO_Handler_t pinSDA_I2C = {0};
I2C_Handler_t i2c_handler = {0};

// Handler del SysTick
Systick_Handler_t systick = {0};

/* Elementos para la comunicacion serial */
USART_Handler_t commSerial = {0};
GPIO_Handler_t pinTx = {0};
GPIO_Handler_t pinRx = {0};
uint8_t usart2DataReceived = 0;
char	bufferMsg[64] = {0};
uint8_t flagBlinkString = 0;
uint8_t flagMenu = 0;
uint8_t flagLetterM = 0;
uint8_t opAnim = 0;
uint8_t flagAnim = 0;
uint8_t countAnim = 0;

/* Variables para definir el área de escritura de cada letra */
uint8_t E4_Col = 0;
uint8_t E4_Page = 0;
uint8_t B3_Col = 0;
uint8_t B3_Page = 0;
uint8_t G3_Col = 0;
uint8_t G3_Page = 0;
uint8_t D3_Col = 0;
uint8_t D3_Page = 0;
uint8_t A2_Col = 0;
uint8_t A2_Page = 0;
uint8_t E2_Col = 0;
uint8_t E2_Page = 0;


//Definicion de las cabeceras de las funciones  del main
void configPeripherals(void);

/*
 * Funcion principal del sistema
 */
int main(void){

	/* Limpiamos todas las banderas */


	/* Definimos los valores iniciales para las variables */
	E4_Col 	= 4;		// Cuerda 1
	E4_Page = 2;
	B3_Col 	= 58;		// Cuerda 2
	B3_Page = 3;
	G3_Col 	= 4;		// Cuerda 3
	G3_Page = 4;
	D3_Col 	= 58;		// Cuerda 4
	D3_Page = 5;
	A2_Col 	= 4;		// Cuerda 5
	A2_Page = 6;
	E2_Col 	= 58;		// Cuerda 6
	E2_Page = 7;

	/* Configuramos los periféricos */
	configPeripherals();


	/* Loop forever*/
	while (1){

		/* Prueba del USART */
		if (usart2DataReceived == 't'){

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "¿Probando? ¡Funciona! Ouh-Yeah! \n\r");
			usart2DataReceived = '\0';
		}


		/* Prueba comunicación serial con la pantalla */
		if (usart2DataReceived == 'a'){

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Intento de comunicación \r\n");
			usart2DataReceived = '\0';

			i2c_handler.slaveAddress = OLED_ADDRESS;
			i2c_Config(&i2c_handler);
			uint8_t auxRead = 0;
			i2c_StartTransaction(&i2c_handler);
			i2c_SendSlaveAddressRW(&i2c_handler, i2c_handler.slaveAddress, 1);

			auxRead = i2c_ReadDataByte(&i2c_handler);
			i2c_StopTransaction(&i2c_handler);

			usart_WriteMsg(&commSerial, "\r\n");
			sprintf(bufferMsg, "Respuesta Slave: %u\r\n", (auxRead >> 6 & 1));
			usart_WriteMsg(&commSerial, bufferMsg);

		}

		/* Encender la OLED */
		if (usart2DataReceived == '8'){

			i2c_handler.slaveAddress = OLED_ADDRESS;
			i2c_Config(&i2c_handler);
			i2c_WriteSingleRegister(&i2c_handler, CONTROL_BYTE_COMMAND, 0xAF);
			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "OLED encendida -> Se lee 0\r\n");
			usart2DataReceived = '\0';
		}

		/* Apagar la OLED */
		if (usart2DataReceived == '9'){

			i2c_handler.slaveAddress = OLED_ADDRESS;
			i2c_Config(&i2c_handler);
			i2c_WriteSingleRegister(&i2c_handler, CONTROL_BYTE_COMMAND, 0xAE);
			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "OLED apagada -> Se lee 1\r\n");
			usart2DataReceived = '\0';
		}

		/* Encender la OLED */
		if (usart2DataReceived == '1'){

			oled_onDisplay(&i2c_handler);
			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "OLED encendida \r\n");

			usart2DataReceived = '\0';
		}

		/* Apagar la OLED */
		if (usart2DataReceived == '2'){

			oled_offDisplay(&i2c_handler);
			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "OLED apagada \r\n");

			usart2DataReceived = '\0';
		}

		/* Configura la OLED */
		if (usart2DataReceived == 'C'){

			uint8_t array[30] = 	{0xAE, 0x20, 0x00, 0x21, 0, 127, 0x22, 0, 7, 0x40, 0x81, 0xCF, 0xA1,
									 0xA6, 0xA8, 0x3F, 0xC8, 0xD3, 0x00, 0xD5, 0x80,
									 0xD9, 0xF1, 0xDA, 0x12, 0xDB, 0x20, 0x8D, 0x14,
									 0xAF};

			oled_sendCommand(&i2c_handler, array, 30);
			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Comando finalizado -> Debe leer 0\r\n");

			usart2DataReceived = '\0';
		}

		/* Configura la OLED */
		if (usart2DataReceived == 'c'){

			oled_Config(&i2c_handler);

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Comando finalizado -> Debe leer 0\r\n");

			usart2DataReceived = '\0';
		}

		/* Apagar la OLED */
		if (usart2DataReceived == 'y'){

			i2c_WriteSingleRegister(&i2c_handler, CONTROL_BYTE_DATA, 0b00000000);
			i2c_WriteSingleRegister(&i2c_handler, CONTROL_BYTE_DATA, 0b00000000);
			i2c_WriteSingleRegister(&i2c_handler, CONTROL_BYTE_DATA, 0b11111111);
			i2c_WriteSingleRegister(&i2c_handler, CONTROL_BYTE_DATA, 0b11111111);

			i2c_StopTransaction(&i2c_handler);

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "2 Lineas negras y 2 blancas\r\n");

			usart2DataReceived = '\0';
		}

		/* Apagar la OLED */
		if (usart2DataReceived == 'x'){

			i2c_WriteSingleRegister(&i2c_handler, CONTROL_BYTE_DATA, 0b11111111);
			i2c_WriteSingleRegister(&i2c_handler, CONTROL_BYTE_DATA, 0b11111111);
			i2c_WriteSingleRegister(&i2c_handler, CONTROL_BYTE_DATA, 0b00000000);
			i2c_WriteSingleRegister(&i2c_handler, CONTROL_BYTE_DATA, 0b00000000);

			i2c_StopTransaction(&i2c_handler);
			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "2 Lineas blancas y 2 negras\r\n");

			usart2DataReceived = '\0';
		}


		/* Limpiar toda la pantalla */
		if (usart2DataReceived == 'n'){

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Pintando negro\r\n");

			usart2DataReceived = '\0';

			/* Apagamos el Timer */
			timer_SetState(&blinkString, TIMER_OFF);

			flagLetterM = 0;
			flagAnim = 0;
			oled_clearDisplay(&i2c_handler);
			usart_WriteMsg(&commSerial, "Finalizado\r\n");

		}

		/* Prueba de función para escribir en la OLED */
		if (usart2DataReceived == 'm'){

			flagLetterM = 1;
			flagBlinkString = 1;
			flagAnim = 0;
			opAnim = 0;

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Probando String\r\n");
			usart2DataReceived = '\0';

			/* Limpiamos la pantalla primero */
			oled_clearDisplay(&i2c_handler);

			/* Pintamos la interfaz del menú principal 1 */
			uint8_t bufferString[64] = {0};

			sprintf((char *)bufferString, "SELECCIONE LA CUERDA");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 20, 4, 0);

			sprintf((char *)bufferString, "CUERDA-1 E4");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, E4_Col, E4_Page);

			sprintf((char *)bufferString, "B3 CUERDA-2");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, B3_Col, B3_Page);

			sprintf((char *)bufferString, "CUERDA-3 G3");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, G3_Col, G3_Page);

			sprintf((char *)bufferString, "D3 CUERDA-4");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, D3_Col, D3_Page);

			sprintf((char *)bufferString, "CUERDA-5 A2");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, A2_Col, A2_Page);

			sprintf((char *)bufferString, "E2 CUERDA-6");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, E2_Col, E2_Page);

			/* Encendemos el Timer */
			timer_SetState(&blinkString, TIMER_ON);

			usart_WriteMsg(&commSerial, "Finalizado\r\n");

		}

		/* Parpadeo de la opción seleccionada */
		if(flagMenu){

			uint8_t bufferString[64] = {0};
			if(flagBlinkString == 1){
				sprintf((char *)bufferString, "CUERDA-1 E4");
				oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 11, E4_Col, E4_Page);
			}
			else if(flagBlinkString == 0){
				sprintf((char *)bufferString, "CUERDA-1 E4");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, E4_Col, E4_Page);
			}
			flagBlinkString ^= 1;
			flagMenu ^= 1;
			flagAnim = 0;
			opAnim = 0;
		}


		if(usart2DataReceived == 'M'){

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Probando letras\r\n");

			usart2DataReceived = '\0';

			flagLetterM = 0;
			uint8_t array[6] = 	{0x21, 8, 16,
					0x22, 3, 3};
			oled_sendCommand(&i2c_handler, array, 6);

			uint8_t array_data[8];
			array_data[0] = 0b00001000;
			array_data[1] = 0b00001000;
			array_data[2] = 0b00001000;
			array_data[3] = 0b01001001;
			array_data[4] = 0b00101010;
			array_data[5] = 0b00011100;
			array_data[6] = 0b00001000;
			array_data[7] = 0b00000000;

			oled_sendData(&i2c_handler, array_data, 8);

			uint8_t bufferString[64] = {0};
			sprintf((char *)bufferString, "CUERDA 1 (E4)");

			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 13, 16, 3);

			usart_WriteMsg(&commSerial, "Finalizado\r\n");
		}

		if(usart2DataReceived == 'k'){

			flagLetterM = 0;
			usart2DataReceived = '\0';
			/* Limpiamos la pantalla de la configuración anterior */
			oled_clearDisplay(&i2c_handler);

			/* Pintando interfaz de afinamiento */
			uint8_t bufferString[64] = {0};
			sprintf((char *)bufferString, "AFINANDO CUERDA-1");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 17, 13, 0);

			sprintf((char *)bufferString, "NOTA: E4");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 8, 16, 2);
		}

		/* Pintando el afinamiento */
		if(usart2DataReceived == 'u'){

			usart2DataReceived = '\0';

			uint8_t bufferString[64] = {0};
			sprintf((char *)bufferString, "APRIETE");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 7, 16, 5);
			sprintf((char *)bufferString, "LA CLAVIJA");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 10, 16, 6);
			flagAnim = 1;
			opAnim = 1;
			countAnim = 0;

			/* Encendemos el Timer */
			timer_SetState(&blinkString, TIMER_ON);

		}

		/* Pintando animación */
		if(flagAnim){

			uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 6};
			oled_sendCommand(&i2c_handler, array, 6);

			/* Matriz de 0b00000000 */
			uint8_t array_zeros[78] = {0};

			/* Se guarda la figura a animar */
			uint8_t array_data[104];
			/**/array_data[0]  = 0b00000000; /**/array_data[51] = 0b00000000;  /**/array_data[26] = 0b00000000;
			array_data[79]  = 0b11111110; /**/array_data[52] = 0b00000000; array_data[27] = 0b00000000; array_data[1] = 0b00000000;
			array_data[80]  = 0b11111110; array_data[53] = 0b00000000; array_data[28] = 0b00000000; array_data[2] = 0b00000000;
			array_data[81]  = 0b11111110; array_data[54] = 0b00000000; array_data[29] = 0b00000000; array_data[3] = 0b00000000;
			array_data[82]  = 0b00000000; array_data[55] = 0b00000000; array_data[30] = 0b00000000; array_data[4] = 0b00000000;
			array_data[83]  = 0b11111111; array_data[56] = 0b00000000; array_data[31] = 0b00000000; array_data[5] = 0b00000000;
			array_data[84]  = 0b11111111; array_data[57] = 0b11111110; array_data[32] = 0b00000000; array_data[6] = 0b00000000;
			array_data[85]  = 0b11111111; array_data[58] = 0b11111110; array_data[33] = 0b00000000; array_data[7] = 0b00000000;
			array_data[86]  = 0b00000000; array_data[59] = 0b11111110; array_data[34] = 0b00000000; array_data[8] = 0b00000000;
			array_data[87]  = 0b11111111; array_data[60] = 0b00000000; array_data[35] = 0b11111111; array_data[9] = 0b11111111;
			array_data[88] = 0b11111111; array_data[61] = 0b11111111; array_data[36] = 0b11111111; array_data[10] = 0b11111111;
			array_data[89] = 0b11111111; array_data[62] = 0b11111111; array_data[37] = 0b11111111; array_data[11] = 0b11111111;
			array_data[90] = 0b00000000; array_data[63] = 0b11111111; array_data[38] = 0b00000000; array_data[12] = 0b00000000;
			array_data[91] = 0b11111111; array_data[64] = 0b00000000; array_data[39] = 0b11111110; array_data[13] = 0b00000000;
			array_data[92] = 0b11111111; array_data[65] = 0b11111111; array_data[40] = 0b11111110; array_data[14] = 0b00000000;
			array_data[93] = 0b11111111; array_data[66] = 0b11111111; array_data[41] = 0b11111110; array_data[15] = 0b00000000;
			array_data[94] = 0b00000000; array_data[67] = 0b11111111; array_data[42] = 0b00000000; array_data[16] = 0b00000000;
			array_data[95] = 0b11111111; array_data[68] = 0b00000000; array_data[43] = 0b11000000; array_data[17] = 0b00000000;
			array_data[96] = 0b11111111; array_data[69] = 0b11111111; array_data[44] = 0b11000000; array_data[18] = 0b00000000;
			array_data[97] = 0b11111111; array_data[70] = 0b11111111; array_data[45] = 0b11000000; array_data[19] = 0b00000000;
			array_data[98] = 0b00000000; array_data[71] = 0b11111111; array_data[46] = 0b00000000; array_data[20] = 0b00000000;
			array_data[99] = 0b11111111; array_data[72] = 0b00000000; array_data[47] = 0b00000000; array_data[21] = 0b00000000;
			array_data[100] = 0b11111111; array_data[73] = 0b11100000; array_data[48] = 0b00000000; array_data[2] = 0b00000000;
			array_data[101] = 0b11111111; array_data[74] = 0b11100000; array_data[49] = 0b00000000; array_data[23] = 0b00000000;
			array_data[102] = 0b00000000; array_data[75] = 0b11100000; array_data[50] = 0b00000000; array_data[24] = 0b00000000;
			/**/array_data[103] = 0b00000000; array_data[76] = 0b00000000;							   /**/array_data[25] = 0b00000000;
			/**/array_data[78] = 0b00000000; /**/array_data[77] = 0b00000000;



			switch(countAnim){
			case 0:{
				oled_sendData(&i2c_handler, array_data, 104);
				uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 5};
				oled_sendCommand(&i2c_handler, array, 6);
				oled_sendData(&i2c_handler, array_zeros, 78);
				break;
			}
			case 1:{
				oled_sendData(&i2c_handler, array_data, 104);

				uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 4};
				oled_sendCommand(&i2c_handler, array, 6);
				oled_sendData(&i2c_handler, array_zeros, 52);
				break;
			}
			case 2:{
				oled_sendData(&i2c_handler, array_data, 104);

				uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 3};
				oled_sendCommand(&i2c_handler, array, 6);
				oled_sendData(&i2c_handler, array_zeros, 26);
				break;
			}
			case 3: {
				oled_sendData(&i2c_handler, array_data, 104);
				break;
			}
			}

			flagAnim ^= 1;

		}

	} // Fin while()

	return 0;

} // Fin main()

/* ===== IMPORTANTE ADC =====
 * A continuación se muestran los pines que corresponden a
 * los diferentes canales del módulo ADC (Conversión Análogo Digital)
 *
 * 	CANAL ADC	PIN			USART		TX		RX
 *
 *	ADC1_0		PA0			USART2		PA2		PA3
 * 	ADC1_1		PA1			USART6		PC6		PC7
 * 	ADC1_2		PA2			USART1		PA9		PA10
 * 	ADC1_3		PA3			USART6		PA11	PA12
 * 	ADC1_4		PA4			USART1		PA15
 * 	ADC1_5		PA5			USART2		PD5		PD6
 * 	ADC1_6		PA6			USART1				PB3
 * 	ADC1_7		PA7			USART1		PB6		PB7
 * 	ADC1_8		PB0
 * 	ADC1_9		PB1
 * 	ADC1_10		PC0
 * 	ADC1_11		PC1
 * 	ADC1_12		PC2
 * 	ADC1_13		PC3
 * 	ADC1_14		PC4
 * 	ADC1_15		PC5
 *
 */

/*
 * Definimos una función para inicializar las configuraciones
 * de los diferentes periféricos
 */
void configPeripherals(void){

	// 1. ===== PUERTOS Y PINES =====
	/* Configurando el pin para el Blinky */
	userLed.pGPIOx								= GPIOA;
	userLed.pinConfig.GPIO_PinNumber			= PIN_5;	// PinA5 -> Led2 del STM32F411
	userLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&userLed);
	gpio_WritePin(&userLed, SET);

	/* Configurando los pines para el puerto serial
	 * Usamos el PinA2 para TX
	 */
	pinTx.pGPIOx								= GPIOA;
	pinTx.pinConfig.GPIO_PinNumber				= PIN_2;
	pinTx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinTx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinTx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinTx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinTx);


	/* Usamos el PinA3 para RX */
	pinRx.pGPIOx								= GPIOA;
	pinRx.pinConfig.GPIO_PinNumber				= PIN_3;
	pinRx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinRx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinRx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinRx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinRx);


	/* Usamos el PinB8 para SCL */
	pinSCL_I2C.pGPIOx								= GPIOB;
	pinSCL_I2C.pinConfig.GPIO_PinNumber				= PIN_8;
	pinSCL_I2C.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinSCL_I2C.pinConfig.GPIO_PinAltFunMode			= AF4;
	pinSCL_I2C.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	pinSCL_I2C.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
	pinSCL_I2C.pinConfig.GPIO_PinOutputType			= GPIO_OTYPE_OPENDRAIN;

	/* Cargamos la configuración */
	gpio_Config(&pinSCL_I2C);

	/* Usamos el PinB9 para SCL */
	pinSDA_I2C.pGPIOx								= GPIOB;
	pinSDA_I2C.pinConfig.GPIO_PinNumber				= PIN_9;
	pinSDA_I2C.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinSDA_I2C.pinConfig.GPIO_PinAltFunMode			= AF4;
	pinSDA_I2C.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	pinSDA_I2C.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
	pinSDA_I2C.pinConfig.GPIO_PinOutputType			= GPIO_OTYPE_OPENDRAIN;

	/* Cargamos la configuración */
	gpio_Config(&pinSDA_I2C);


	// 2. ===== TIMERS =====
	/* Configurando el Timer del Blinky*/
	blinkTimer.pTIMx								= TIM2;
	blinkTimer.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	blinkTimer.TIMx_Config.TIMx_Period				= 500;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (500 ms)
	blinkTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	blinkTimer.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos la configuración del Timer */
	timer_Config(&blinkTimer);

	/* Encendemos el Timer */
	timer_SetState(&blinkTimer, TIMER_ON);

	/* Configurando el Timer para el parpadeo de la opción actual disponible para seleccionar en el menu*/
	blinkString.pTIMx								= TIM5;
	blinkString.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	blinkString.TIMx_Config.TIMx_Period				= 1000;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (500 ms)
	blinkString.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	blinkString.TIMx_Config.TIMx_InterruptEnable	= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos la configuración del Timer */
	timer_Config(&blinkString);

	/* Encendemos el Timer */
	timer_SetState(&blinkString, TIMER_ON);


	// 4. ===== USARTS =====
	/* Configurando el puerto serial USART2 */
	commSerial.ptrUSARTx					= USART2;
	commSerial.USART_Config.baudrate		= USART_BAUDRATE_115200;
	commSerial.USART_Config.datasize		= USART_DATASIZE_8BIT;
	commSerial.USART_Config.parity			= USART_PARITY_NONE;
	commSerial.USART_Config.stopbits		= USART_STOPBIT_1;
	commSerial.USART_Config.mode			= USART_MODE_RXTX;
	commSerial.USART_Config.enableIntRX		= USART_RX_INTERRUP_ENABLE;

	/* Cargamos la configuración de USART */
	usart_Config(&commSerial);

	/*
	 * Escribimos el caracter nulo para asegurarnos de empezar
	 * una transmisión "limpia"
	 */
	usart_WriteChar(&commSerial, '\0');


	// 6. ===== I2C =====
	/* Configuramos el I2C */
	i2c_handler.ptrI2Cx				= I2C1;
	i2c_handler.slaveAddress		= OLED_ADDRESS;
	i2c_handler.modeI2C				= I2C_MODE_FM;

	/* Cargamos la configuración del I2C */
	i2c_Config(&i2c_handler);


	// 7. ===== SYSTICK =====
	/* Configuramos el Systick */
	systick.pSystick						= SYSTICK;
	systick.Systick_Config.Systick_Reload	= SYSTICK_PSC_1ms;
	systick.Systick_Config.Systick_IntState = SYSTICK_INT_ENABLE;

	/* Cargamos la configuración del systick */
	systick_Config(&systick);

	/* Encendemos el Systick */
	systick_SetState(&systick, SYSTICK_ON);


}	// Fin de la configuración de los periféricos


/* ===== CALLBACKS ===== */

/* Callback de Timer 2 (Controla el userLed) */
void Timer2_Callback(void){
	gpio_TooglePin(&userLed);
}

/*
 * Callback de Timer 5 (Controla la visualización
 * de la opción actual en la OLED)
 */
void Timer5_Callback(void){
	if(flagLetterM){
		flagMenu ^= 1;
	}

	/* Contador para la animación */
	if(opAnim){
		flagAnim ^= 1;
		if(flagAnim){
			countAnim++;
			if(countAnim >= 4){
				countAnim = 0;
			}
		}
	}

}


void usart2_RxCallback(void){
	usart2DataReceived = usart2_getRxData();
}

/*
 * Función assert para detectar problemas de paŕametros incorrectos
 */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
