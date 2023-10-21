/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Basic project, base for all new projects
 ******************************************************************************
 **/

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "usart_driver_hal.h"
#include "adc_driver_hal.h"

// Se define un pin de prueba
GPIO_Handler_t stateLed = {0}; // PinA5
Timer_Handler_t blinkTimer = {0}; // Timer 2
USART_Handler_t pinTx = {0}; // Pin para la transmisión serial
USART_Handler_t pinRx = {0}; // Pin para la recepción serial
uint8_t receiveChar = '\0';
uint8_t sendMsg = 0;
char bufferData[64] = {0};


// Configuración de la conversión análogo-digital
ADC_Config_t potenciometro = {0};



// Headers de las funciones
void configPeripherals(void);


/* ===== Funciión principal del programa ===== */
int main(void){

	/* Cargamos la configuración de los periféricos */
	configPeripherals();




	/* Loop forever */
	while(1){

	}

}	// Fin del main


/*
 * ===== IMPORTANTE ADC =====
 * A continuación se muestran los pines que corresponden a
 * los diferentes canales del módulo ADC (Consersión Análogo Digital)
 *
 * 	CANAL ADC	PIN
 *
 *	ADC1_0		PA0
 * 	ADC1_1		PA1
 * 	ADC1_2		PA2
 * 	ADC1_3		PA3
 * 	ADC1_4		PA4
 * 	ADC1_5		PA5
 * 	ADC1_6		PA6
 * 	ADC1_7		PA7
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
	/* Configurando el pin para el Blinky*/
	stateLed.pGPIOx								= GPIOA;
	stateLed.pinConfig.GPIO_PinNumber			= PIN_5;	// PinA5 -> Led2 del STM32F411
	stateLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	stateLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	stateLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	stateLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&stateLed);
	gpio_WritePin(&stateLed, SET);




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


	// 3. ===== Interrupciones EXTI =====



	// 4. ===== USARTS =====



	// 5. ===== ADC =====


}	// Fin de la configuración de los periféricos



/*
 * Overwiter function
 */

void Timer4_Callback(void){
	gpio_TooglePin(&stateLed);
}


/*
 * Función assert para detectar problemas de paŕametros incorrectos
 */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
