/*
 * systick_driver_hal.c
 *
 *  Created on: Oct 20, 2023
 *      Author: sgaviriav
 */
#include <stdint.h>
#include "stm32f4xx.h"
#include "systick_driver_hal.h"


static uint32_t countTicks = 0;

/*
 * Cabeceras de las funciones privadas
 */
static void systick_config_interrupt(Systick_Handler_t *pSystickHandler);

/*
 * Función para configurar el Systick
 */
void systick_Config(Systick_Handler_t *pSystickHandler){

	// La variable que cuenta los Ticks del sistema empieza en 0, para asegurarnos mejor
	countTicks = 0;

	/*
	 * 1. Asignar el valor del Reload
	 */

	// Asignamos el valor del Reload cargado en la estructura de configuración, en el registro correspondiente
	pSystickHandler->pSystick->LOAD = pSystickHandler->Systick_Config.Systick_Reload;

	/*
	 * 2. Limpiamos el valor actual del contador del Systick
	 */
	pSystickHandler->pSystick->VAL = 0;


	/* Configurar el registro CTRL
	 * 	3a. Configuramos la fuente de la señal (en este caso, la signa clock del procesador)
	 */
	// Limpiamos el registro
	pSystickHandler->pSystick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;

	// Asignamos la señal de reloj principal (Fosc -> Frecuencia de oscilanción) al Systick
	pSystickHandler->pSystick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

	/*
	 * 	3b. Configuramos las interrupciones del Systick
	 */
	// Desactivamos las interrupciones globales
	__disable_irq();

	// Configuramos las interrupciones del Systick
	systick_config_interrupt(pSystickHandler);

	// Activamos las interrupciones globales
	__enable_irq();

	/*
	 * 4. Arrancamos con el Timer Systick apagado
	 */
	systick_SetState(pSystickHandler, SYSTICK_OFF);
}


/*
 * Con esta función encendemos o apagamos el Timer
 */
void systick_SetState(Systick_Handler_t *pSystickHandler, uint8_t newState){

	/* Mirammos cuál estado queremos configurar */
	switch(newState){
	case SYSTICK_ON: {
		pSystickHandler->pSystick->CTRL |= SysTick_CTRL_ENABLE_Msk;
		break;
	}
	case SYSTICK_OFF: {
		pSystickHandler->pSystick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
		break;
	}
	default: {
		__NOP();
		break;
	}
	}

}


/*
 * Devuelve la cantidad de Ticks (cuenta los ticks, o del tiempo según el Reload d)
 */
uint32_t systick_GetTicks(void){
	return countTicks;
}


/*
 * Activamos o desactivamos las interrupciones. También las matriculamos o desmatriculamos del NVIC
 */
static void systick_config_interrupt(Systick_Handler_t *pSystickHandler){

	if(pSystickHandler->Systick_Config.Systick_IntState == SYSTICK_INT_ENABLE){
		// Limpiamos la posición
		pSystickHandler->pSystick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

		// Activamos la interrupción
		pSystickHandler->pSystick->CTRL |= SysTick_CTRL_TICKINT_Msk;

		// Matriculamos la interrupción en el NVIC
		NVIC_EnableIRQ(SysTick_IRQn);
	}
	else{
		// Desactivamos la interrupción
		pSystickHandler->pSystick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

		// Desmatriculamos la interrupción en el NVIC
		NVIC_DisableIRQ(SysTick_IRQn);
	}
}


__attribute__((weak)) void systick_Callback(void){
	__NOP();
}

/*
 * El Handler de IRQ del Systick (El nombre de esta función es propio de las librerías del Cortex M4 -> )
 */
void SysTick_Handler(void){

	// Verificamos si la interrupción se dio por el Systick
	if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){
		/* Limpiamos la bandera que indica que la interrupción se ha generado */
		SysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG_Msk;

		/* Cada que se de una interrupción, aumentamos en 1 el contador de ticks */
		countTicks++;

		/* Llamamos a la función que se debe encargar de hacer algo con esta interrupción */
		systick_Callback();
	}

}
