/*
 * pwm_driver_hal.h
 *
 *  Created on: Nov 03, 2023
 *      Author: sgaviriav
 */

#ifndef PWMDRIVER_H_
#define PWMDRIVER_H_

#include "stm32f4xx.h"

enum
{
	PWM_CHANNEL_1 =	0,
	PWM_CHANNEL_2,
	PWM_CHANNEL_3,
	PWM_CHANNEL_4
};


/* Estructura para la configuración del PWM */
typedef struct
{
	uint8_t		channel; 		// Canal PWM relacionado con el TIMER
	uint32_t	prescaler;		// A qué velocidad se incrementa el Timer -> PSC (Determina la "unidad" de tiempo para el periodo)
	uint16_t	periodo;		// Indica el número de veces que el Timer se incrementa -> ARR. Representa el periodo del PWM (PSC * ARR)
	uint16_t	dutyCycle;		// Es un valor entre 0-ARR, en el cual estará activa la señal dentro del periodo del PWM.
}PWM_Config_t;

/* Handler para cargar la configuración del PWM */
typedef struct
{
	TIM_TypeDef		*ptrTIMx;	// Timer al que esta asociado el PWM
	PWM_Config_t	config;	// Configuración inicial del PWM
}PWM_Handler_t;


/* Funciones públicas del PWM */
void pwm_Config(PWM_Handler_t *ptrPwmHandler);
void setFrequency(PWM_Handler_t *ptrPwmHandler);
void updateFrequency(PWM_Handler_t *ptrPwmHandler, uint16_t newPeriod);
void setDutyCycle(PWM_Handler_t *ptrPwmHandler);
void updateDutyCycle(PWM_Handler_t *ptrPwmHandler, uint16_t newDuty);
void enableOutput(PWM_Handler_t *ptrPwmHandler);
void startPwmSignal(PWM_Handler_t *ptrPwmHandler);
void stopPwmSignal(PWM_Handler_t *ptrPwmHandler);

#endif /* PWMDRIVER_H_ */
