/*
 * adc_driver_hal.c
 *
 *  Created on: Oct 13, 2023
 *      Author: sgaviriav
 */

/* Importando las librerías */

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "adc_driver_hal.h"

/* ===== Headers for private functions ===== */
static void adc_enable_clock_peripheral(void);
static void adc_set_resolution(ADC_Config_t *adcConfig);
static void adc_set_alignment(ADC_Config_t *adcConfig);
static void adc_set_sampling_and_hold(ADC_Config_t *adcConfig);
static void adc_set_one_channel_sequence(ADC_Config_t *adcConfig);
static void adc_set_sequence(ADC_Config_t *adcConfig[], uint8_t Length);
static void adc_config_interrupt(ADC_Config_t *adcConfig);

/* Variables y elementos que necesita internamente el driver para
 * funcionar adecuadamente
 * */
GPIO_Handler_t handlerADCPin = {0};
uint16_t adcRawData = 0;


/*
 * Función para configurar un solo canal del ADC
 */
void adc_ConfigSingleChannel(ADC_Config_t *adcConfig) {

	/* 1. Configuramos el PinX para que cumpla la función del canal análogo deseado */
	adc_ConfigAnalogPin(adcConfig->channel);

	/* 2. Activamos la señal de reloj para el ADC */
	adc_enable_clock_peripheral();

	// Limpiamos los registros antes de comenzar a configurar
	ADC1->CR1 = 0;
	ADC1->CR2 = 0;

	/* Comenzamos la configuración de ADC1 */

	/* 3. Resolución de ADC1 */
	adc_set_resolution(adcConfig);

	/* 4. Configuramos el modo Scan como desactivado */
	adc_ScanMode(SCAN_OFF);

	/* 5. Configuramos la alineación de los datos (derecha o izquierda) */
	adc_set_alignment(adcConfig);

	/* 6. Desactivamos el "continuous mode" */
	adc_StopContinuousConv();

	/* Se configura el sampling (muestreo) */
	adc_set_sampling_and_hold(adcConfig);

	/* 8. Configuramos la secuencia y cuantos elementos hay en la secuencia */
	adc_set_one_channel_sequence(adcConfig);

	/* 9. Configuramos el prescaler del ADC en 2:1 (el más rápido que se puede tener)
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
 * Función para configurar una secuencia de múltiples canales del ADC
 */
void adc_ConfigMultiChannel(ADC_Config_t *adcConfig[], uint8_t Length) {

	/* 1. Configuramos el PinX para que cumpla la función del canal análogo deseado.
	 * Esto se hace para cada pin de cada canal */
	for(uint8_t i = 0; i < Length; i++){
		adc_ConfigAnalogPin(adcConfig[i]->channel);
	}

	/* 2. Activamos la señal de reloj para el ADC */
	adc_enable_clock_peripheral();

	// Limpiamos los registros antes de comenzar a configurar
	ADC1->CR1 = 0;
	ADC1->CR2 = 0;

	/* 3. Resolución de ADC1
	 * Basta con configurar la resolución del primer sensor, pues es la
	 * misma para todo el módulo ADC.
	 */
	adc_set_resolution(adcConfig[0]);

	/* 4. Configuramos el Modo Scan como activado */
	adc_ScanMode(SCAN_ON);

	/* 5. Configuramos la alineación de los datos (derecha o izquierda) */
	adc_set_alignment(adcConfig[0]);

	/* 6. Desactivamos el "continuous mode" */
	adc_StopContinuousConv();


	/* Comenzamos la configuración de ADC1 */

	/* 7. Se configura el sampling (muestreo) para cada canal */
	for(uint8_t i=0; i < Length; i++){
		adc_set_sampling_and_hold(adcConfig[0]);
	}

	/* 8. Configuramos la secuencia y cuantos elementos hay en la secuencia */
	adc_set_sequence(adcConfig, Length);

	/* 9. Configuramos el prescaler del ADC en 2:1 (el más rápido que se puede tener)
	 * (Corresponde a la división más pequeña)
	 */
	ADC->CCR &= ~ADC_CCR_ADCPRE;

	/* 10. Desactivamos las interrupciones globales */
	__disable_irq();

	/* 11. Configuramos la interrupción (si se encuentra activa), además de
	 * inscribir/remover la interrupción en el NVIC
	 */
	adc_config_interrupt(adcConfig[0]);

	/* 12. Seleccionamos la opción para lanzar la interrupción cada que uno de los
	 * canales de la secuencia termine la conversión
	 */
	ADC1->CR2 |= ADC_CR2_EOCS;

	/* 13. Activamos el modulo ADC */
	adc_peripheralOnOFF(ADC_ON);

	/* 14. Activamos las interrupciones globales */
	__enable_irq();

} // Fin de la función adc_ConfigMultiChannel()


/*
 * Enable Clock for ADC peripheral
 * */
static void adc_enable_clock_peripheral(void) {
	/* Encendemos el periférico ADC en el RCC del APB2 */
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
}

/*
 * Función para configurar la resolución de conversión
 * */
static void adc_set_resolution(ADC_Config_t *adcConfig) {

	/* Limpiamos el registro CR1->RES */
	ADC1->CR1 &= ~ADC_CR1_RES;

	/* Vemos la configuración y seleccionamos la resolución
	 * en el registro CR1->RES, según la configuración
	 */
	switch (adcConfig->resolution) {
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

/*
 * Configuración para el alineamiento (left or right)
 */
static void adc_set_alignment(ADC_Config_t *adcConfig) {

	/* Leemos la configuración cargada para el alineamiento */
	switch (adcConfig->dataAlignment) {
	/* Se configura el alineamiento a la izquierda */
	case ALIGNMENT_LEFT: {
		ADC1->CR2 |= ADC_CR2_ALIGN;
		break;
	}
		/* Se configura el alineamiento a la derecha */
	case ALIGNMENT_RIGHT: {
		ADC1->CR2 &= ~ADC_CR2_ALIGN;
		break;
	}
	default: {
		ADC1->CR2 &= ~ADC_CR2_ALIGN;// Por predeterminado, queda alineado a la derecha (right)
		break;
	}
	}

}	// Fin de la configuración del alineamiento

/*
 * Relacionando con el valor del tiempo de carga del capacitor HOLD
 */
static void adc_set_sampling_and_hold(ADC_Config_t *adcConfig) {

	/* Limpiamos los registros del Sampling Time Register ADC1->SMPR1 Y ADC1->SMPR2 */

	/* Obtenemos el canal para el que queremos configurar el valor del Sampling */
	switch (adcConfig->channel) {
	case CHANNEL_0: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 0 */
		ADC1->SMPR2 &= ~ADC_SMPR2_SMP0;

		// Configuramos para el Canal 0
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP0;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP0_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP0_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP0_1 | ADC_SMPR2_SMP0_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP0_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP0_2 | ADC_SMPR2_SMP0_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP0_2 | ADC_SMPR2_SMP0_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP0;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_1: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 1 */
		ADC1->SMPR2 &= ~ADC_SMPR2_SMP1;

		// Configuramos para el Canal 1
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP1;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP1_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP1_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP1_1 | ADC_SMPR2_SMP1_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP1_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP1_2 | ADC_SMPR2_SMP1_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP1_2 | ADC_SMPR2_SMP1_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP1;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_2: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 2 */
		ADC1->SMPR2 &= ~ADC_SMPR2_SMP2;

		// Configuramos para el Canal 2
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP2;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP2_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP2_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP2_1 | ADC_SMPR2_SMP2_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP2_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP2_2 | ADC_SMPR2_SMP2_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP2_2 | ADC_SMPR2_SMP2_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP2;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_3: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 3 */
		ADC1->SMPR2 &= ~ADC_SMPR2_SMP3;

		// Configuramos para el Canal 3
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP3;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP3_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP3_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP3_1 | ADC_SMPR2_SMP3_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP3_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP3_2 | ADC_SMPR2_SMP3_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP3_2 | ADC_SMPR2_SMP3_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP3;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_4: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 4 */
		ADC1->SMPR2 &= ~ADC_SMPR2_SMP4;

		// Configuramos para el Canal 4
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP4;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP4_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP4_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP4_1 | ADC_SMPR2_SMP4_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP4_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP4_2 | ADC_SMPR2_SMP4_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP4_2 | ADC_SMPR2_SMP4_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP4;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_5: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 5 */
		ADC1->SMPR2 &= ~ADC_SMPR2_SMP5;

		// Configuramos para el Canal 5
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP5;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP5_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP5_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP5_1 | ADC_SMPR2_SMP5_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP5_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP5_2 | ADC_SMPR2_SMP5_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP5_2 | ADC_SMPR2_SMP5_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP5;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_6: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 6 */
		ADC1->SMPR2 &= ~ADC_SMPR2_SMP6;

		// Configuramos para el Canal 6
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP6;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP6_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP6_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP6_1 | ADC_SMPR2_SMP6_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP6_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP6_2 | ADC_SMPR2_SMP6_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP6_2 | ADC_SMPR2_SMP6_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP6;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_7: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 7 */
		ADC1->SMPR2 &= ~ADC_SMPR2_SMP7;

		// Configuramos para el Canal 7
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP7;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP7_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP7_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP7_1 | ADC_SMPR2_SMP7_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP7_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP7_2 | ADC_SMPR2_SMP7_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP7_2 | ADC_SMPR2_SMP7_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP7;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_8: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 8 */
		ADC1->SMPR2 &= ~ADC_SMPR2_SMP8;

		// Configuramos para el Canal 8
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP8;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP8_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP8_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP8_1 | ADC_SMPR2_SMP8_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP8_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP8_2 | ADC_SMPR2_SMP8_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP8_2 | ADC_SMPR2_SMP8_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP8;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_9: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 9 */
		ADC1->SMPR2 &= ~ADC_SMPR2_SMP9;

		// Configuramos para el Canal 9
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR2 &= ~ADC_SMPR2_SMP9;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP9_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP9_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP9_1 | ADC_SMPR2_SMP9_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP9_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP9_2 | ADC_SMPR2_SMP9_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR2 |= (ADC_SMPR2_SMP9_2 | ADC_SMPR2_SMP9_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR2 |= ADC_SMPR2_SMP9;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_10: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 10 */
		ADC1->SMPR1 &= ~ADC_SMPR1_SMP10;

		// Configuramos para el Canal 10
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP10;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP10_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP10_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP10_1 | ADC_SMPR1_SMP10_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP10_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP10_2 | ADC_SMPR1_SMP10_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP10_2 | ADC_SMPR1_SMP10_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP10;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_11: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 11 */
		ADC1->SMPR1 &= ~ADC_SMPR1_SMP11;

		// Configuramos para el Canal 11
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP11;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP11_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP11_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP11_1 | ADC_SMPR1_SMP11_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP11_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP11_2 | ADC_SMPR1_SMP11_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP11_2 | ADC_SMPR1_SMP11_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP11;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_12: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 12 */
		ADC1->SMPR1 &= ~ADC_SMPR1_SMP12;

		// Configuramos para el Canal 12
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP12;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP12_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP12_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP12_1 | ADC_SMPR1_SMP12_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP12_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP12_2 | ADC_SMPR1_SMP12_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP12_2 | ADC_SMPR1_SMP12_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP12;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_13: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 13 */
		ADC1->SMPR1 &= ~ADC_SMPR1_SMP13;

		// Configuramos para el Canal 13
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP13;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP13_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP13_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP13_1 | ADC_SMPR1_SMP13_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP13_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP13_2 | ADC_SMPR1_SMP13_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP13_2 | ADC_SMPR1_SMP13_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP13;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_14: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 14 */
		ADC1->SMPR1 &= ~ADC_SMPR1_SMP14;

		// Configuramos para el Canal 14
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP14;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP14_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP14_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP14_1 | ADC_SMPR1_SMP14_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP14_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP14_2 | ADC_SMPR1_SMP14_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP14_2 | ADC_SMPR1_SMP14_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP14;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	case CHANNEL_15: {
		/* Limpiamos el registro correspondiente al Sampling del Canal 15 */
		ADC1->SMPR1 &= ~ADC_SMPR1_SMP15;

		// Configuramos para el Canal 15
		switch (adcConfig->samplingPeriod) {
		case SAMPLING_PERIOD_3_CYCLES: {
			ADC1->SMPR1 &= ~ADC_SMPR1_SMP15;		// 0b000
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP15_0;	// 0b001
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP15_1;	// 0b010
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP15_1 | ADC_SMPR1_SMP15_0);	// 0b011
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP15_2;	// 0b100
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP15_2 | ADC_SMPR1_SMP15_0);	// 0b101
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES: {
			ADC1->SMPR1 |= (ADC_SMPR1_SMP15_2 | ADC_SMPR1_SMP15_1);	// 0b110
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES: {
			ADC1->SMPR1 |= ADC_SMPR1_SMP15;	// 0b111
			break;
		}
		default: {
			__NOP();
			break;
		}
		}
		break;
	}
	}

}	// Fin de la función adc_set_sampling_and_hold()

/*
 * Configura el número de elementos en la secuencia (un solo elemento en este caso)
 * Configura también cuál es el canal que adquiere la señal ADC
 */
static void adc_set_one_channel_sequence(ADC_Config_t *adcConfig) {

	/* De momento como sólo usamos un canal, lo configuramos para que sea el
	 * primer canal de al secuencia
	 */

	/* Las secuencias se configuran en los registros ADC1->SQR1, ADC1->SQR2 y ADC1->SQR3
	 * Configuramos la cantidad de canales que se usarań en la secuencia
	 */
	ADC1->SQR1 &= ~ADC_SQR1_L; // Aquí ponemos cuántos canales vamos a usar. Por defecto 0b0000 para 1 conversión

	/* Para configurar el CanalX en la posición 1 de la secuencia. Escribimos el número del canal
	 * en el registro correspondiente a la posición de la secuencia que queremos que ocupe
	 */
	ADC1->SQR3 &= ~(0b11111 << ADC_SQR3_SQ1_Pos); // Limpiamos los bits de la secuencia 1
	ADC1->SQR3 |= (adcConfig->channel << ADC_SQR3_SQ1_Pos); // Se haría similar para las demás posiciones de la secuencia

}


/*
 * Configuramos la secuencia de la conversión, de acuerdo al orden del arreglo que se
 * debe definir en el main, cargando las configuraciones individuales de cada "sensor"
 * que va a perternecer al arreglo
 */
static void adc_set_sequence(ADC_Config_t *adcConfig[], uint8_t Length){

	// 1. Primero, limpiamos todos los registros de la secuencia
	ADC1->SQR1 = 0;
	ADC1->SQR2 = 0;
	ADC1->SQR3 = 0;

	// Ponemos la cantidad de canales que conforman la secuencia (Length - 1)
	ADC1->SQR1 |= ((Length - 1) << ADC_SQR1_L_Pos);

	/*
	 * Cargamos el valor de cada Canal de acuerdo a la posición de los diferentes
	 * sensores en el arreglo
	 */
	for(uint8_t i = 0; i < Length; i++){

		uint8_t posicion = i + 1; // Esta variable me permite identificar la posicion de la secuencia

		/*
		 * Miramos en qué parte de la secuencia vamos, para asignar los valores en el
		 * registro ADC1->SQR3, ADC1->SQR2 ó ADC1->SQR1
		 */

		// Posiciones de la secuencia 1-6, registro ADC1->SQR3
		if(posicion <= ULTIMA_POSICION_SQR3){
			// Las posiciones en el registro se mueven cada 5 bits
			ADC1->SQR3 |= ((adcConfig[i]->channel) << (ADC_SQR3_SQ2_Pos * i)); // Se haría similar para las demás posiciones de la secuencia
		}
		// Posiciones de la secuencia 7-12, registro ADC1->SQR2
		else if(posicion >= PRIMERA_POSICION_SQR2  && posicion <= ULTIMA_POSICION_SQR2){
			ADC1->SQR2 |= ((adcConfig[i]->channel) << (ADC_SQR2_SQ8_Pos * (i - ULTIMA_POSICION_SQR3)));
		}
		// Posiciones de la secuencia 13-16
		else if(posicion >= PRIMERA_POSICION_SQR1){
			ADC1->SQR1 |= ((adcConfig[i]->channel) << (ADC_SQR1_SQ14_Pos * (i - ULTIMA_POSICION_SQR2)));
		}
	}

} // Fin adc_set_sequence()

/*
 * Configura el enable de las interrupciones y la activación del NVIC
 */
static void adc_config_interrupt(ADC_Config_t *adcConfig) {

	/*
	 * Activamos/Desactivamos las interrupciones debido a la finalización de una conversión
	 * simple o de una secuencia
	 */
	switch (adcConfig->interrupState) {
	case ADC_INT_ENABLE: {
		// Activamos las interrupciones del ADC debidas a conversiones
		ADC1->CR1 |= ADC_CR1_EOCIE;

		// Activamos el canal del NVIC para leer las interrupciones (Matriculamos la interrupción)
		__NVIC_EnableIRQ(ADC_IRQn);

		break;
	}
	case ADC_INT_DISABLE: {
		// Desactivamos las interrupciones del ADC
		ADC1->CR1 &= ~ADC_CR1_EOCIE;

		// Desmatriculamos la interrupción del canal del NVIC
		__NVIC_DisableIRQ(ADC_IRQn);

		break;
	}
	default: {
		__NOP();
		break;
	}
	}

}	// Fin de la función adc_config_interrupt


/*
 * Controla la activación y desactivación del módulo ADC desde el
 * registro CR2 del ADC
 */
void adc_peripheralOnOFF(uint8_t state) {

	if (state == ADC_ON) {
		// Activamos el módulo ADC
		ADC1->CR2 |= ADC_CR2_ADON;
	} else {
		// Desactivamos el módulo ADC
		ADC1->CR2 &= ~ADC_CR2_ADON;
	}

}


/*
 * Aquí activamos o desactivamos el Scan Mode.
 * Funciona de la mano con la secuencia de varios canales.
 * No es necesario para el caso de un canal simple
 */
void adc_ScanMode(uint8_t state) {

	if (state == SCAN_ON) {
		// Activamos el Scan mode en el CR1
		ADC1->CR1 |= ADC_CR1_SCAN;
	} else {
		// Desactivamos el Scan mode
		ADC1->CR1 &= ~ADC_CR1_SCAN;
	}

}


/*
 * Función que inicia la conversión ADC simple
 */
void adc_StartSingleConv(void) {

	ADC1->CR2 &= ~ADC_CR2_CONT;
	// Encendemos la conversión simple solo si está encendido el ADC
	ADC1->CR2 |= ADC_CR2_SWSTART; // Realiza una única conversión y se detiene automáticamente

}

/*
 * Función de inicia la conversión ADC continua
 */
void adc_StartContinuousConv(void) {

	// Permitir que el proceso de conversión continua solo si está encendido el ADC
	ADC1->CR2 |= ADC_CR2_CONT;
	// Encendemos la conversión
	ADC1->CR2 |= ADC_CR2_SWSTART;

}

/*
 * Función que detiene la conversión ADC continua
 */
void adc_StopContinuousConv(void) {

	// Detenemos el proceso de conversión continua, cambiando a modo de conversión simple
	ADC1->CR2 &= ~ADC_CR2_CONT;
	// Apagamos la conversión simple también
	ADC1->CR2 &= ~ADC_CR2_SWSTART;

}

/*
 * Función que retorna el último dato adquirido por el ADC
 */
uint16_t adc_GetValue(void) {
	return adcRawData;
}

/*
 * Esta es la ISR de la interrupción por conversión ADC
 */
void ADC_IRQHandler(void) {

	if (ADC1->SR & ADC_SR_EOC) {
//		// Bajamos a bandera, indicando que ya se está atendiendo la interrupción
//		ADC1->SR &= ~ADC_SR_EOC;

		// Guardamos el valor del DR (Data Register) en una variable
		adcRawData = ADC1->DR; /* 	Aunque la variable es de 16 bits y el
		 Data Register es de 32, los 16 bits menos
		 significativos del final no importan */

		// Se llama la función CallBack que atiende la interrupción
		adc_CompleteCallback();
	}
}

__attribute__((weak)) void adc_CompleteCallback(void) {
	__NOP();
}

/*
 * Esta función configura los pines que deseamos que funcionen con ADC
 */
void adc_ConfigAnalogPin(uint8_t adcChannel) {

	//Configuramos cada Pin independientemente, según el canal seleccionado
	switch (adcChannel) {
	case CHANNEL_0: {
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_1: {
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_2: {
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_2;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_3: {
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_3;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_4: {
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_4;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_5: {
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_5;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_6: {
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_6;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_7: {
		handlerADCPin.pGPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_7;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_8: {
		handlerADCPin.pGPIOx = GPIOB;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_9: {
		handlerADCPin.pGPIOx = GPIOB;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_10: {
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_11: {
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_12: {
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_2;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_13: {
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_3;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_14: {
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_4;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	case CHANNEL_15: {
		handlerADCPin.pGPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_5;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		break;
	}
	default: {
		__NOP();
		break;
	}

	}
	gpio_Config(&handlerADCPin);
}

/*
 * Configuramos para hacer conversiones en múltiples canales
 * y con un orden específico (secuencia)
 */

/*
 * Configuramos para Trigger externo
 */

