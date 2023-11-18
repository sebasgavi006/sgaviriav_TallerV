/*
 * adc_driver_hal.h
 *
 *  Created on: Oct 13, 2023
 *      Author: sgaviriav
 */

#ifndef ADC_DRIVER_HAL_H_
#define ADC_DRIVER_HAL_H_

#include <stdint.h>
#include "stm32f4xx.h"
#include "pwm_driver_hal.h"

enum{
	CHANNEL_0 = 0,
	CHANNEL_1,
	CHANNEL_2,
	CHANNEL_3,
	CHANNEL_4,
	CHANNEL_5,
	CHANNEL_6,
	CHANNEL_7,
	CHANNEL_8,
	CHANNEL_9,
	CHANNEL_10,
	CHANNEL_11,
	CHANNEL_12,
	CHANNEL_13,
	CHANNEL_14,
	CHANNEL_15
};

enum{
	RESOLUTION_12_BIT = 0,
	RESOLUTION_10_BIT,
	RESOLUTION_8_BIT,
	RESOLUTION_6_BIT
};

enum{
	ALIGNMENT_RIGHT = 0,
	ALIGNMENT_LEFT
};

enum{
	SCAN_OFF = 0,
	SCAN_ON
};

enum{
	ADC_OFF = 0,
	ADC_ON
};

enum{
	ADC_INT_DISABLE = 0,
	ADC_INT_ENABLE
};

enum{
	TRIGGER_AUTO = 0,
	TRIGGER_MANUAL,
	TRIGGER_EXT
};

enum{
	SAMPLING_PERIOD_3_CYCLES = 0b000,
	SAMPLING_PERIOD_15_CYCLES = 0b001,
	SAMPLING_PERIOD_28_CYCLES = 0b010,
	SAMPLING_PERIOD_56_CYCLES = 0b011,
	SAMPLING_PERIOD_84_CYCLES = 0b100,
	SAMPLING_PERIOD_112_CYCLES = 0b101,
	SAMPLING_PERIOD_144_CYCLES = 0b110,
	SAMPLING_PERIOD_480_CYCLES = 0b111
};

// Posiciones para la secuencia de la opción multicanal del ADC
enum{
	PRIMERA_POSICION_SQR3 	= 1,
	ULTIMA_POSICION_SQR3 	= 6,
	PRIMERA_POSICION_SQR2 	= 7,
	ULTIMA_POSICION_SQR2 	= 12,
	PRIMERA_POSICION_SQR1 	= 13,
	ULTIMA_POSICION_SQR1 	= 16
};

// Timer y Canal de PWM
enum{
	EXT_TRIGGER_TIM1_CC1	= 0b0000,
	EXT_TRIGGER_TIM1_CC2	= 0b0001,
	EXT_TRIGGER_TIM1_CC3	= 0b0010,
	EXT_TRIGGER_TIM2_CC2	= 0b0011,
	EXT_TRIGGER_TIM2_CC3	= 0b0100,
	EXT_TRIGGER_TIM2_CC4	= 0b0101,
	EXT_TRIGGER_TIM3_CC1	= 0b0111,
	EXT_TRIGGER_TIM4_CC4	= 0b1001,
	EXT_TRIGGER_TIM5_CC1	= 0b1010,
	EXT_TRIGGER_TIM5_CC2	= 0b1011,
	EXT_TRIGGER_TIM5_CC3	= 0b1100,
	EXT_TRIGGER_EXTI11		= 0b1111,
};


/*
 * ADC Config definition
 * This structure is used to configure a single ADC channel.
 * - Channels			-> configures inside the driver the correct GPIO pin as ADC channel
 * - Resolution			-> 6bit, 8bit, 10bit, 12bit are the possible options.
 * - Sampling Period	-> Related to the SARs procedure.
 * - Data aligment		-> left or right, depends on the app. Default is right
 * - ADC data			-> holds the data (in the register).
 */

typedef struct{
	uint8_t		channel;			// Canal ADC que será utilizado para la conversión ADC
	uint8_t		resolution;			// Precisión con la que el ADC hace la adquisición del dato
	uint16_t	samplingPeriod;		// Tiempo deseado para hacer la adquisición del dato
	uint8_t		dataAlignment;		// Alineación a la izquierda
	uint16_t	adcData;			// Dato de la conversión (valor binario)
	uint8_t		interrupState;		// Para configurar si se desea o no trabajar con la interrupción
}ADC_Config_t;


/* Header definitions for the public functions of adc_driver_hal */
void adc_ConfigSingleChannel(ADC_Config_t *adcConfig);
void adc_ConfigAnalogPin(uint8_t adcChannel);
void adc_CompleteCallback(void);
void adc_StartSingleConv(void);
void adc_ScanMode(uint8_t state);
void adc_StartContinuousConv(void);
void adc_StopContinuousConv(void);
void adc_peripheralOnOFF(uint8_t state);
uint16_t adc_GetValue(void);

void adc_ConfigMultiChannel(ADC_Config_t *adcConfig, uint8_t Length);
void adc_ExternalTrigger(PWM_Handler_t *handlerPWM);


/* Configuraciones avanzadas del ADC */
//void adc_ConfigMultichannel(ADC_Config_t *adcConfig, uint8_t numeroDeCanales);
//void adc_ConfigTrigger(uint8_t sourceType, PWM_Handler_t *triggerSignal);

void adc_CompleteCallback(void);

#endif /* ADC_DRIVER_HAL_H_ */
