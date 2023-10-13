/*
 * adc_driver_hal.c
 *
 *  Created on: Oct 13, 2023
 *      Author: sgaviriav
 */

/* Importando las librerías */
#include "stm32f446xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "adc_driver_hal.h"

/* ===== Headers for private functions ===== */
static void adc_enable_clock_peripheral(void);
static void adc_set_resolution(ADC_Config_t *adcConfig);
static void adc_set_alignment(ADC_Config_t *adcConfig);
static void adc_set_sampling_and_hold(ADC_Config_t *adcConfig);
static void adc_set_one_channel_sequence(ADC_Config_t *adcConfig);
static void adc_config_interrupt(ADC_Config_t *adcConfig);


/* Variables y elementos que necesita internamente el driver para
 * funcionar adecuadamente */
GPIO_Handler_t	handlerADCPin = {0};
uint16_t		adcRawData = 0;


/*
 * Función para configurar un solo canal del ADC
 */
void adc_ConfigSingleChannel(ADC_Config_t *adcConfig){

	/* 1. Configuramos el PinX para que cumpla la función del canal análogo deseado */
	adc_ConfigAnalogPin(adcConfig->channel);

	/* 2. Activamos la señal de reloj para el ADC */
	adc_enable_clock_peripheral();

	// Limpiamos los registros antes de comenzar a configurar
	ADC1->CR1 = 0;
	ADC2->CR2 = 0;

	/* Comenzamos la configuración de ADC1 */

	/* 3. Resolución de ADC1 */
	adc_set_resolution(adcConfig);

	/* 4. Configuramos el modo Scan como desactivado */
	adc_ScanMode(SCAN_OFF);

	/* 5. Configuramos la alineación de los datos (derecha o izquierda) */
	adc_set_alignment(adcConfig);

	/* 6. Desactivamos el "continuous mode" */
	adc_StopContinuousConv();

	/* Se coonfigura el sampling (muestreo) */
	adc_set_sampling_and_hold(adcConfig);

	/* 8. Configuramos la secuencia y cuantos elementos hay en la secuencia */
	adc_set_one_channel_sequence(adcConfig);

	/* 9. Configuramos el prescaler del ADC en 2:1 (el más rápiod que se puede tener)
	 * (Corresponde a la división más pequeña)
	 */
	ADC->CCR &= ~ADC_CCR_ADCPRE;

	/* 10. Desactivamos las interrupciones globales */
	__disable_irq();

	/* 11. Configuramos la interrupción (si se encuentra activa), además de
	 * inscribir/remover la interrupción en el NVIC
	 * */
	adc_config_interrupt(adcConfig);

	/* 12. Activamos el modulo ADC */
	adc_peripheralOnOFF(ADC_ON);

	/* 13. Activamos las interrupciones globales */
	__enable_irq();

}


/*
 * Enable Clock for ADC peripheral
 * */
static void adc_enable_clock_peripheral(void){
	/* Encendemos el periférico ADC en el RCC del APB2 */
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
}


/*
 * Función para configurar la resolución de conversión
 * */
static void adc_set_resolution(ADC_Config_t *adcConfig){

	/* Limpiamos el registro CR1->RES */
	ADC1->CR1 &= ~ADC_CR1_RES;

	/* Vemos la configuración y seleccionamos la resolución
	 * en el registro CR1->RES, según la configuración
	 */
	switch(adcConfig->resolution){
	case RESOLUTION_12_BIT: {
		ADC1->CR1 &= ~ADC_CR1_RES;
		break;
	}
	case RESOLUTION_10_BIT: {
		ADC1->CR1 |= ADC_CR1_RES_0;
		break;
	}
	case RESOLUTION_8_BIT: {
		ADC1->CR1 |= ADC_CR1_RES_1;
		break;
	}
	case RESOLUTION_6_BIT: {
		ADC1->CR1 |= ADC_CR1_RES;
		break;
	}
	default: {
		/* Ponemos una resolution de 12bit por defecto */
		ADC1->CR1 &= ~ADC_CR1_RES;
		break;
	}
	} // Fin del switch-case

}	// Fin del configuración de resolución






