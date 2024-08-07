/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Código de prueba para el ADC
 ******************************************************************************
 **/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "adc_driver_hal.h"
#include "usart_driver_hal.h"

// Se define un pin para el Blinky
GPIO_Handler_t stateLed = {0}; // PinA5

Timer_Handler_t blinkTimer = {0}; // Timer 2 para el blinky
Timer_Handler_t ADCTimer = {0}; // Timer 5 para el ADC

// Comunicación Rs-232 con el PC, ya habilitada en la board Nucleo 32F411
USART_Handler_t handlerUSART2= {0}; // Pin para la transmisión serial
GPIO_Handler_t pinTx = {0};	// Pin para la transmisión serial
GPIO_Handler_t pinRx = {0};	// Pin para la recepción serial
rxDataUsart received_USARTx = {0};
uint8_t sendMsg = 0;
char bufferData[64] = {0};

// Configuración de la conversión análogo-digital
ADC_Config_t potenciometro = {0};


// Headers de las funciones
void configPeripherals(void);
uint8_t isNumber(char caracter);
uint8_t isLetter(char caracter);
void analyzeCommand(char *buffer);
void configTimerPeriod();



/* ===== Funciión principal del programa ===== */
int main(void){

	received_USARTx.rxData_USART2 = '\0';

	/* Cargamos la configuración de los periféricos */
	configPeripherals();


	/* Loop forever */
	while(1){
		if(sendMsg){
			sendMsg = 0; // Bajamos la bandera
			usart_WriteMsg(&commSerial, "Hello World! \n\r");
		}
		if(received_USARTx.rxData_USART2){ // Si no está vacía la variable que almacena los datos recibidos
			if(received_USARTx.rxData_USART2 == 'p'){
				usart_WriteMsg(&commSerial, "Testing, Testing!!\n\r");
			}

			if(received_USARTx.rxData_USART2 == 's'){
				usart_WriteMsg(&commSerial, "make simple ADC\n\r");

			}

			if(received_USARTx.rxData_USART2 == 'C'){
				usart_WriteMsg(&commSerial, "make continuous ADC\n\r");
				adc_StartContinuousConv();
			}

			if(received_USARTx.rxData_USART2 == 'S'){
				usart_WriteMsg(&commSerial, "stop continuous ADC\n\r");
				adc_StopContinuousConv();
			}
			received_USARTx.rxData_USART2 = '\0';
		}

	}

}	// Fin del main


/*
 * ===== IMPORTANTE ADC =====
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
	stateLed.pGPIOx								= GPIOA;
	stateLed.pinConfig.GPIO_PinNumber			= PIN_5;	// PinA5 -> Led2 del STM32F411
	stateLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	stateLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	stateLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	stateLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&stateLed);
	gpio_WritePin(&stateLed, SET);

	/* Configurando los pines para el puerto serial
	 *  - Usamos el PinA2 para TX
	 */
	pinTx.pGPIOx								= GPIOA;
	pinTx.pinConfig.GPIO_PinNumber				= PIN_2;
	pinTx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinTx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinTx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinTx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinTx);


	/* - Usamos el PinA3 para RX */
	pinRx.pGPIOx								= GPIOA;
	pinRx.pinConfig.GPIO_PinNumber				= PIN_3;
	pinRx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinRx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinRx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinRx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinRx);


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


	/* Configurando el Timer del Blinky*/
	ADCTimer.pTIMx								= TIM5;
	ADCTimer.TIMx_Config.TIMx_Prescaler			= 160;	// Genera incrementos de 1 ms
	ADCTimer.TIMx_Config.TIMx_Period			= 500;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (500 ms)
	ADCTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	ADCTimer.TIMx_Config.TIMx_InterruptEnable	= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos la configuración del Timer */
	timer_Config(&ADCTimer);

	/* Encendemos el Timer */
	timer_SetState(&ADCTimer, TIMER_ON);

	// 3. ===== Interrupciones EXTI =====



	// 4. ===== USARTS =====
	/* Configurando el puerto serial USART2 */
	handlerUSART2.ptrUSARTx						= USART2;
	handlerUSART2.USART_Config.baudrate			= USART_BAUDRATE_115200;
	handlerUSART2.USART_Config.datasize			= USART_DATASIZE_8BIT;
	handlerUSART2.USART_Config.parity			= USART_PARITY_NONE;
	handlerUSART2.USART_Config.stopbits			= USART_STOPBIT_1;
	handlerUSART2.USART_Config.mode				= USART_MODE_RXTX;
	handlerUSART2.USART_Config.enableIntRX		= USART_RX_INTERRUP_ENABLE;

	/* Cargamos la configuración de USART */
	usart_Config(&handlerUSART2);

	/*
	 * Escribimos el caracter nulo para asegurarnos de empezar
	 * una transmisión "limpia"
	 */
	usart_WriteChar(&handlerUSART2, '\0');

	// 5. ===== ADC =====
	/*
	 * Condiguramos el canal 0 del ADC
	 */
	potenciometro.channel				= CHANNEL_0;
	potenciometro.resolution			= RESOLUTION_12_BIT;
	potenciometro.dataAlignment			= ALIGNMENT_RIGHT;
	potenciometro.samplingPeriod		= SAMPLING_PERIOD_84_CYCLES;
	potenciometro.interrupState			= ADC_INT_ENABLE;

	/* Cargamos la configuración del ADC */
	adc_ConfigSingleChannel(&potenciometro);


}	// Fin de la configuración de los periféricos

/**
 * @brief	Revisa si el caracter recibido es un número
 * @return	1 si se cumple, 0 de otra forma
 */
uint8_t isNumber(char caracter){
	if ((rxData >= '0') && (rxData <= '9')) {// Se verifica si el caracter es un número
		return 1;
	}
	return 0;
}

/**
 * @brief	Revisa si el caracter recibido es una letra
 * @return	1 si se cumple, 0 de otra forma
 */
uint8_t isLetter(char caracter){

	// Se verifica si el caracter es una letra
	if (((rxData >= 'a') && (rxData <= 'z'))
			|| ((rxData >= 'A') && (rxData <= 'Z'))) {
		return 1;
	}
	return 0;
}


/**
 * @brief	Ejecuta una acción dependiendo del comando recibido
 * @param	*buffer	String a evaluar
 */
void analyzeCommand(char *buffer){

	// Comando recibido = h0
	if (strcmp(buffer, "h0") == 0) {
		usart_WriteMsg(&handlerUsart2, "Menu de ayuda (help): \n");
		usart_WriteMsg(&handlerUsart2,
	  		"1)  h0 -> Se imprime el menu de ayuda \n");
		usart_WriteMsg(&handlerUsart2,
				"2)  b0 -> Apagar blinky \n");
		usart_WriteMsg(&handlerUsart2,
				"3)  b1 -> Prender blinky \n");
		usart_WriteMsg(&handlerUsart2,
				"4)  b2 -> Colocar un periodo al blinky de 250 ms  \n");
		usart_WriteMsg(&handlerUsart2,
				"5)  b3 -> Colocar un periodo al blinky de 500 ms  \n");
		usart_WriteMsg(&handlerUsart2,
				"6)  b4 -> Colocar un periodo al blinky de 1000 ms   \n");
		usart_WriteMsg(&handlerUsart2,
				"7)  at0 -> Apagar timer del muestreo \n");
		usart_WriteMsg(&handlerUsart2,
				"8)  at1 -> Prender el timer del muestreo  \n");
		usart_WriteMsg(&handlerUsart2,
				"9)  at2 -> Colocar un periodo de muestreo a 5 ms  \n");
		usart_WriteMsg(&handlerUsart2,
				"10) at3 -> Colocar un periodo de muestreo a 50 ms \n");
		usart_WriteMsg(&handlerUsart2,
				"11)  ar0 -> Cambiar la resolución a 12 bits \n");
		usart_WriteMsg(&handlerUsart2,
	    		"12)  ar1 -> Cambiar la resolución a 10 bits \n");
		usart_WriteMsg(&handlerUsart2,
	    		"13)  ar2 -> Cambiar la resolución a 8 bits \n");
		usart_WriteMsg(&handlerUsart2,
	    		"14)  ar3 -> Cambiar la resolución a 6 bits \n");
		usart_WriteMsg(&handlerUsart2,
	    		"15)  af0 -> Imprimir valor actual en decimal \n");
		usart_WriteMsg(&handlerUsart2,
	    		"16)  af1 -> Imprimir valor actual en hexadecimal \n");
		usart_WriteMsg(&handlerUsart2,
	    		"17)  af2 -> Imprimir valor actual en milivoltios \n");
		usart_WriteMsg(&handlerUsart2,
	    		"18)  ad0 -> Se deshabilita el modo continuo \n");
		usart_WriteMsg(&handlerUsart2,
	    		"19)  ad1 -> Se habilita el modo continuo \n");

          // Hacer los demás casos.
	}
}




/*
 * Overwiter function
 */

void Timer2_Callback(void){
	gpio_TooglePin(&stateLed);
	sendMsg = 1; // Levantamos una bandera para enviar mensajes en el Callback del USART
}

void usart2_RxCallback(void){
	received_USARTx.rxData_USART2 = usart2_getRxData();
}

void adc_CompleteCallback(void){
	potenciometro.adcData = adc_GetValue();
}

/*
 * Función assert para detectar problemas de paŕametros incorrectos
 */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
