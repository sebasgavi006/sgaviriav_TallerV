/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          :
 ******************************************************************************
 **/

/* Importando las librerías necesarias desde el PeripheralsDrivers */
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "exti_driver_hal.h"

/* Definición de los pines a utilizar */
GPIO_Handler_t stateLed = {0}; // PinA5 (led de estado)



/* Definición de los Timers a utilizar para generar las interrupciones
 * del blinky y del 7 segmentos */
Timer_Handler_t blinkTimer2 = {0};
Timer_Handler_t sevenSegmentTimer3 = {0};


/* Definición de los EXTI para las interrupciones externas del encoder */
EXTI_Config_t exti_0 = {0};	// Definimos el EXTI del canal 0 (estructura -> "objeto")
EXTI_Config_t exti_8 = {0};	// Definimos el EXTI del canal 8 (estructura -> "objeto")


/* Función principal del programa */
int main(void)
{
	/* ===== Configurando los pines que vamos a utilizar ===== */
	/* Configurando el pin del stateLed */
	stateLed.pGPIOx								= GPIOA;
	stateLed.pinConfig.GPIO_PinNumber			= PIN_5;
	stateLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	stateLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	stateLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	stateLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;



	/* Cargamos la configuración del pin */
	gpio_Config(&stateLed);
	gpio_WritePin(&stateLed, SET);


	/* ====== Configuramos las interrupciones externas (EXTI) ===== */
	/* Condigurando EXTI0 */
	exti_0.pGPIOHandler				= ;
	exti_0.edgeType					= EXTERNAL_INTERRUPT_FALLING_EDGE;

	/* Condigurando EXTI8 */
	exti_8.pGPIOHandler				= ;
	exti_8.edgeType					= EXTERNAL_INTERRUPT_FALLING_EDGE;

	/* Cargamos la configuración de los EXTI */
	exti_Config(&exti_0);
	exti_Config(&exti_0);


	/* ===== Configurando los TIMER ===== */
	/* Configurando el Timer */
	blinkTimer2.pTIMx								= TIM2;
	blinkTimer2.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	blinkTimer2.TIMx_Config.TIMx_Period				= 250;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (250 ms)
	blinkTimer2.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	blinkTimer2.TIMx_Config.TIMx_InterruptEnable	= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos la configuración del Timer */
	timer_Config(&blinkTimer2);
	timer_Config(&sevenSegmentTimer3);

	/* Encendemos los Timer */
	timer_SetState(&blinkTimer2, TIMER_ON);
	timer_SetState(&sevenSegmentTimer3, TIMER_ON);



	/* Loop forever */
	while(1){

	}

}


/*
 * Overwiter function
 */


void Timer2_Callback(void){
	gpio_TooglePin(&stateLed);
}

/* Función assert para detectar problemas de paŕametros incorrectos */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
