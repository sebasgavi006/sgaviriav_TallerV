/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Actualización del GPIO
 ******************************************************************************
 **/

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"


// Se define un pin de prueba
GPIO_Handler_t userLed = {0}; // PinA5


/* Función principal del programa */
int main(void)
{

	/* Configurando el pin */
	userLed.pGPIOx								= GPIOA;
	userLed.pinConfig.GPIO_PinNumber			= PIN_5;
	userLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración */
	gpio_Config(&userLed);
	gpio_WritePin(&userLed, SET);


	/* Loop forever */
	while(1){

		gpio_TooglePin(&userLed);
		for(uint32_t j = 0; j < 200000; j++){
			_NOP();
		}

	}

	return 0;
}


/* Función assert para detectar problemas de paŕametros incorrectos */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
