/*
 * systick_driver_hal.h
 *
 *  Created on: Oct 20, 2023
 *      Author: sgaviriav
 */

#ifndef SYSTICK_DRIVER_HAL_H_
#define SYSTICK_DRIVER_HAL_H_

#include <stdint.h>
#include "stm32f4xx.h"

#define SYSTICK_ADDRESS 	0xE000E010UL // Dirección exacta sin OFFSET a los registros del SysTick

#define SYSTICK 			((SysTick_Type *) SYSTICK_ADDRESS)	// Definimos un puntero, que toma una estructura y la apunta a una dirección

#define SYSTICK_PSC_1ms		16000	// El SysTick funciona a 1ms

#define SYSTICK_500ms		500
#define SYSTICK_1s			1000
#define SYSTICK_2s			2000
#define SYSTICK_3s			3000
#define SYSTICK_5s			5000
#define SYSTICK_10s			10000

enum{
	SYSTICK_OFF = 0,
	SYSTICK_ON
};

enum{
	SYSTICK_INT_DISABLE = 0,
	SYSTICK_INT_ENABLE
};


// Definiendo la estructura de la configuración del Timer Systick
typedef struct
{
	uint32_t	Systick_Reload;		/* Valor de las repeticiones del signal clock del procesador para hacer una cuenta
									 *  - Recordar que 16.000.000 repeticiones son 1 ms */
	uint8_t		Systick_IntState;	// Activa o desactiva las interrupciones
} Systick_BasicConfig_t;

// Definiendo la estructura para el Handler
typedef struct
{
	SysTick_Type				*pSystick;		// Esta variable apunta hacia la dirección de la estructura definida en CMSIS
	Systick_BasicConfig_t		Systick_Config;	// Esta variable asigna el valor de dicho registro
} Systick_Handler_t;


/* Funciones públicas del driver */
void systick_Config(Systick_Handler_t *pSystickHandler);
void systick_SetState(Systick_Handler_t *pSystickHandler, uint8_t newState);
uint64_t systick_GetTicks();
void systick_Delay_ms(uint32_t wait_time_ms);

/* Esta función debe ser sobre-escrita en el main para que el sistema funcione */
void systick_Callback(void);

#endif /* SYSTICK_DRIVER_HAL_H_ */
