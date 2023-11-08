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
#include "timer_driver_hal.h"
#include "adc_driver_hal.h"
#include "usart_driver_hal.h"
#include "systick_driver_hal.h"

Systick_BasicConfig_t configSystick = {0};
Systick_Handler_t handlerSystick = {0};
GPIO_PinConfig_t configLed = {0};
GPIO_Handler_t handlerLed = {0};
SysTick_Type systick = {0};

#define SYSTICK_HALF_SECOND	8000000

int main(void){

	configSystick.Systick_IntState = SYSTICK_INT_ENABLE;
	configSystick.Systick_Reload = SYSTICK_HALF_SECOND;

	handlerSystick.pSystick = systick;
	handlerSystick.Systick_Config = configSystick;






}
