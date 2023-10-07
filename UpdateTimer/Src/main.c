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

// Se define un pin de prueba
GPIO_Handler_t stateLed = {0}; // PinA5

Timer_Handler_t blinkTimer = {0};


/* Función principal del programa */
int main(void)
{

	/* Configurando el pin */
	stateLed.pGPIOx								= GPIOA;
	stateLed.pinConfig.GPIO_PinNumber			= PIN_5;
	stateLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	stateLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	stateLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	stateLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&stateLed);
	gpio_WritePin(&stateLed, SET);

	/* Configurando el Timer */
	blinkTimer.pTIMx								= TIM2;
	blinkTimer.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	blinkTimer.TIMx_Config.TIMx_Period				= 250;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (250 ms)
	blinkTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	blinkTimer.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos la configuración del Timer */
	timer_Config(&blinkTimer);

	/* Encendemos el Timer */
	timer_SetState(&blinkTimer, TIMER_ON);


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
