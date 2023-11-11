/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Código de prueba para el ADC
 ******************************************************************************
 **/

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "systick_driver_hal.h"

Systick_Handler_t handlerSysTick = {0};
GPIO_Handler_t handlerLed = {0};
uint8_t flag_systick = 0;

// Definiendo algunas constantes y estructuras
#define SYSTICK_ONE_SECOND	16000000
#define SYSTICK_HALF_SECOND	8000000
#define SYSTICK_FAST		1000000

#define SYSTICK_ADDRESS 	0xE000E010UL // Dirección exacta sin OFFSET a los registros del SysTick

#define SYSTICK 			((SysTick_Type *) SYSTICK_ADDRESS)	// Definimos un puntero, que toma una estructura y la apunta a una dirección

int main(void){

	flag_systick = 0;

	// Configuramos el pin PA5 (UserLed) para probar el SysTick
	handlerLed.pGPIOx							= GPIOA;
	handlerLed.pinConfig.GPIO_PinNumber			= PIN_5;
	handlerLed.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	handlerLed.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	handlerLed.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	// Cargamos la configuración del Puerto y del Pin
	gpio_Config(&handlerLed);
	gpio_WritePin(&handlerLed, SET);


	// Configuramos el SysTick según los parámetros deseados
	handlerSysTick.pSystick 						= SYSTICK;
	handlerSysTick.Systick_Config.Systick_IntState 	= SYSTICK_INT_ENABLE;
	handlerSysTick.Systick_Config.Systick_Reload 	= SYSTICK_FAST;

	// Cargamos la configuración
	systick_Config(&handlerSysTick);
	systick_SetState(&handlerSysTick, SYSTICK_ON);


	/* Loop infinito */
	while(1){

		if(flag_systick){
			flag_systick = 0;
			gpio_TooglePin(&handlerLed);
		}
	}

}


/* Callback del SysTick */
void systick_Callback(void){
	flag_systick = 1;
}
