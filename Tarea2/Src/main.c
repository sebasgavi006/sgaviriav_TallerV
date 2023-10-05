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

// Pin para el led de estado
GPIO_Handler_t stateLed = {0}; // PinA5 (led de estado)

// Pin para el led que indica el modo seleccionado por el switch
GPIO_Handler_t modeLed = {0}; // Pin  (led de modo [Directo o Inverso])


// Pines para el encoder
GPIO_Handler_t encoderClk = {0}; // Pin8 (Canal 8 del EXTI)
GPIO_Handler_t data = {0};	// Pin
GPIO_Handler_t sw = {0};	// Pin0 (Canal 0 del EXTI)

// Pines para el 7-segmentos
GPIO_Handler_t segmentoA = {0}; // Pin
GPIO_Handler_t segmentoB = {0}; // Pin
GPIO_Handler_t segmentoC = {0}; // Pin
GPIO_Handler_t segmentoD = {0}; // Pin
GPIO_Handler_t segmentoE = {0}; // Pin
GPIO_Handler_t segmentoF = {0}; // Pin
GPIO_Handler_t segmentoG = {0}; // Pin



/* Definición de los Timers a utilizar para generar las interrupciones
 * del blinky y del 7 segmentos */
Timer_Handler_t blinkTimer2 = {0};	// Timer 2 para el stateLed
Timer_Handler_t sevenSegmentTimer3 = {0};	// Timer 3 para el 7-segmentos


/* Definición de los EXTI para las interrupciones externas del encoder */
EXTI_Config_t exti_0 = {0};	// Definimos el EXTI del Switch (estructura -> "objeto")
EXTI_Config_t exti_8 = {0};	// Definimos el EXTI del Encoder (estructura -> "objeto")


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
	gpio_WritePin(&stateLed, SET); // Encendemos el Pin


	/* Configuración del pin para el modeLed */
	modeLed.pGPIOx								= ;
	modeLed.pinConfig.GPIO_PinNumber			= ;
	modeLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	modeLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	modeLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	modeLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la confiugración del pin */
	gpio_Config(&modeLed);
	gpio_WritePin(&modeLed, SET); // Encendemos el Pin para indicar inicialmente el Modo Directo


	/* Configuraciones de los pines del 7-segmentos */
	segmentoA.pGPIOx							= ;
	segmentoA.pinConfig.GPIO_PinNumber			= ;
	segmentoA.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoA.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoA.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoA.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoB.pGPIOx							= ;
	segmentoB.pinConfig.GPIO_PinNumber			= ;
	segmentoB.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoB.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoB.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoB.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoC.pGPIOx							= ;
	segmentoC.pinConfig.GPIO_PinNumber			= ;
	segmentoC.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoC.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoC.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoC.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoD.pGPIOx							= ;
	segmentoD.pinConfig.GPIO_PinNumber			= ;
	segmentoD.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoD.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoD.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoD.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoE.pGPIOx							= ;
	segmentoE.pinConfig.GPIO_PinNumber			= ;
	segmentoE.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoE.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoE.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoE.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoF.pGPIOx							= ;
	segmentoF.pinConfig.GPIO_PinNumber			= ;
	segmentoF.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoF.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoF.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoF.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoG.pGPIOx							= ;
	segmentoG.pinConfig.GPIO_PinNumber			= ;
	segmentoG.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoG.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoG.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoG.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración de los pines */
	gpio_Config(&segmentoA);
	gpio_Config(&segmentoB);
	gpio_Config(&segmentoC);
	gpio_Config(&segmentoD);
	gpio_Config(&segmentoE);
	gpio_Config(&segmentoF);
	gpio_Config(&segmentoG);


	/* Configuramos los pines del encoder */

	/* Pin del Encoder Clock */
	encoderClk.pGPIOx							= ;
	encoderClk.pinConfig.GPIO_PinNumber			= ;
	encoderClk.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;

	/* Pin del Switch */
	sw.pGPIOx							= ;
	sw.pinConfig.GPIO_PinNumber			= ;
	sw.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;

	/* Pin del de la salida de datos del encoder */
	data.pGPIOx							= ;
	data.pinConfig.GPIO_PinNumber		= ;
	data.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;

	/* Se cargan las configuraciones de los respectivos pines */
	gpio_Config(&encoderClk);
	gpio_Config(&sw);
	gpio_Config(&data);


	/* ====== Configuramos las interrupciones externas (EXTI) ===== */
	/* Condigurando EXTI0 */
	exti_0.pGPIOHandler				= &sw;
	exti_0.edgeType					= EXTERNAL_INTERRUPT_FALLING_EDGE;

	/* Condigurando EXTI8 */
	exti_8.pGPIOHandler				= &encoderClk;
	exti_8.edgeType					= EXTERNAL_INTERRUPT_FALLING_EDGE;

	/* Cargamos la configuración de los EXTI */
	exti_Config(&exti_0);
	exti_Config(&exti_0);


	/* ===== Configurando los TIMER ===== */
	/* Configurando el TIMER2 para el Blinky*/
	blinkTimer2.pTIMx								= TIM2;
	blinkTimer2.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	blinkTimer2.TIMx_Config.TIMx_Period				= 250;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (250 ms)
	blinkTimer2.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuenta ascendente
	blinkTimer2.TIMx_Config.TIMx_InterruptEnable	= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Configurando el TIMER3 para el 7-segmentos */
	sevenSegmentTimer3.pTIMx								= TIM3;
	sevenSegmentTimer3.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	sevenSegmentTimer3.TIMx_Config.TIMx_Period				= 250;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (250 ms)
	sevenSegmentTimer3.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuenta ascendente
	sevenSegmentTimer3.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción

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


/* Función callback para la interrupción del EXTI0, es decir, la interrupción
 * externa debida al switch
 * */
void callback_ExtInt0(void){
	gpio_TooglePin(&modeLed);
}


/* Función callback que atiende la interrupción del EXTI8, en este caso, corresponde
 * a la interrupcón externa del encoder
 * */
void callback_ExtInt8(void){

}


/* Función que atiende la interrupción debida TIMER2, es decir, que controla
 * al blinky (Led de estado)
 * */
void Timer2_Callback(void){
	gpio_TooglePin(&stateLed);
}


/* Esta función atiende la interrupción del TIMER3, la cual controla el activación
 * de los cristales del 7-segmentos a una frecuencia que simule que ambos están
 * enendidos continuamente
 * */
void Timer3_Callback(void){

}

/* Función assert para detectar problemas de paŕametros incorrectos */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
