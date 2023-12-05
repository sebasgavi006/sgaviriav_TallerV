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


//Definicion de las cabeceras de las funciones  del main
void configPeripherals(void);

/*
 * Funcion principal del sistema
 */
int main(void){

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

			uint8_t auxRead = 0;
			i2c_StartTransaction(&i2c_handler);
			i2c_SendSlaveAddressRW(&i2c_handler, i2c_handler.slaveAddress, 1);

			auxRead = i2c_ReadDataByte(&i2c_handler);
			i2c_StopTransaction(&i2c_handler);

			usart_WriteMsg(&commSerial, "\r\n");
			sprintf(bufferMsg, "Respuesta Slave: %u\r\n", (auxRead >> 6 & 1));
			usart_WriteMsg(&commSerial, bufferMsg);

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
		if (usart2DataReceived == 'c'){

			uint8_t array[26] = 	{0xAE, 0x00, 0x10,0x40, 0xB0, 0x81, 0xCF, 0xA1,
									 0xA6, 0xA8, 0x3F, 0xC8, 0xD3, 0x00, 0xD5, 0x80,
									 0xD9, 0xF1, 0xDA, 0x12, 0xDB, 0x20, 0x8D, 0x14,
									 0xAF, 0xAF};
			oled_sendCommand(&i2c_handler, array, 26);
			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Comando finalizado -> Debe leer 0\r\n");

			usart2DataReceived = '\0';
		}

		/* Apagar la OLED */
		if (usart2DataReceived == 'y'){

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Pintando negro\r\n");

			usart2DataReceived = '\0';

			uint8_t array[128] = {0};
			array[127] = 0b11111111;

			oled_sendData(&i2c_handler, array, 128);

		}

		/* Apagar la OLED */
		if (usart2DataReceived == 'x'){

			usart2DataReceived = '\0';
		}




	} // Fin while()

	return 0;

} // Fin main()

/* ===== IMPORTANTE ADC =====
 * A continuación se muestran los pines que corresponden a
 * los diferentes canales del módulo ADC (Consersión Análogo Digital)
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
