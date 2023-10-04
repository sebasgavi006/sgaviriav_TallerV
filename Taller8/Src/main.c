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
#include "exti_driver_hal.h"

// Se definen los pines
GPIO_Handler_t stateLed = {0}; // PinA5
GPIO_Handler_t userButton = {0}; // PinC13

// Se define la interrupción EXTI13
EXTI_Config_t exti_13 = {0}; // EXTI13



/* Función principal del programa */
int main(void)
{

	/* Configurando los pines */
	stateLed.pGPIOx								= GPIOA;
	stateLed.pinConfig.GPIO_PinNumber			= PIN_5;
	stateLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	stateLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	stateLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	stateLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	userButton.pGPIOx							= GPIOC;
	userButton.pinConfig.GPIO_PinNumber			= PIN_13;
	userButton.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;


	/* Cargamos la configuración del pin */
	gpio_Config(&stateLed);
	gpio_WritePin(&stateLed, SET);
	gpio_Config(&userButton);


	/* Configuramos la interrupción externa */
	exti_13.pGPIOHandler			= &userButton;
	exti_13.edgeType				= EXTERNAL_INTERRUPT_FALLING_EDGE;

	/* Cargamos la configuración del EXTI */
	exti_Config(&exti_13);

	/* Loop forever */
	while(1){

	}

}


/*
 * Overwiter function
 */
void callback_ExtInt13(void){
	gpio_TooglePin(&stateLed);
}

/* Función assert para detectar problemas de paŕametros incorrectos */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
