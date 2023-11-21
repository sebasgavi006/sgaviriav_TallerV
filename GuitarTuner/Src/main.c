/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Tarea 4.
 * 					 Implementación del ADC Multicanal, PWM y USART para hallar
 * 					 la frecuencia de una señal de entrada, mediante el uso de
 * 					 la Transformada Rápida de Fourier (FFT).
 ******************************************************************************
 **/

#include <stdint.h>
#include <math.h>
#include "arm_math.h"

#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "adc_driver_hal.h"
#include "usart_driver_hal.h"
#include "pwm_driver_hal.h"

// Definicion de los Handlers necesario
GPIO_Handler_t userLed = {0}; //Led de estado PinA5

Timer_Handler_t blinkTimer = {0};

// Handlers del ADC
ADC_Config_t sensores[3] = {0}; // Array de sensores
ADC_Config_t sensor1 = {0};
ADC_Config_t sensor2 = {0};
ADC_Config_t sensor3 = {0};
uint8_t Length = 3; // Tamaño del arreglo
uint8_t posicion = 0;	// Posición de la secuencia PWM
uint16_t count_ADC_Data = 0; // Contador de la cantidad de datos que lleva el arreglo ADC_Data[512]
uint8_t flagStop = 0;	// Detiene las conversiones ADC cuando finalice las 512 conversiones
uint8_t flagADC = 0;	// Bandera para controlar la finalización de una secuencia de conversiones
char flagLetra = '\0';	// Bandera para mirar qué letra hay

float32_t ADC_Data1[512] = {0};	// Arreglo para guardar los datos para la FFT del sensor 1
float32_t ADC_Data2[512] = {0};	// Arreglo para guardar los datos para la FFT del sensor 2
float32_t ADC_Data3[512] = {0};	// Arreglo para guardar los datos para la FFT del sensor 3


// PWM para generar la frecuencia de muestreo
PWM_Handler_t pwmHandler = {0};

/* Elementos para la comunicacion serial */
USART_Handler_t commSerial = {0};
GPIO_Handler_t pinTx = {0};
GPIO_Handler_t pinRx = {0};
uint8_t sendMsg = 0;
uint8_t usart2DataReceived = 0;
char bufferMsg[64] = {0};


// Código de los videos del classroom
/* Elementos para generar una selal */
#define ADC_DataSize 512	// Tamaño del arreglo de datos
uint16_t fftSize = 512;		// Tamaño del arreglo de los valores obtenidos de la transformada
float32_t frec_muestreo = 16000000/(16*25); //frecuencia de muestreo -> 40kHz
float32_t fft_magnitud[ADC_DataSize/2];
float32_t transformedSignal[ADC_DataSize];

/* Variables para instanciar e inicializar los funciones de la FFT
 * (Necesarias para el uso de las funciones FFT del CMSIS)
 */
uint32_t ifftFlag = 0;
arm_rfft_fast_instance_f32 config_Rfft_fast_f32;
arm_status statusInitFFT = ARM_MATH_ARGUMENT_ERROR;


//Definicion de las cabeceras de las funciones  del main
void configPeripherals(void);
void procesamientoFFT(float32_t *array);


/*
 * Funcion principal del sistema
 */
int main(void){

	/* Activamos el FPU (Unidad de Punto Flotante) */
	SCB->CPACR |= (0xF << 20);

	/* Configuramos los periféricos */
	configPeripherals();

	sensores[0] = sensor1;
	sensores[1] = sensor2;
	sensores[2] = sensor3;


	/* Cargamos la configuración de los sensores en la función Multicanal del ADC */
	adc_ConfigMultiChannel(sensores, Length);

	/* Configuramos el PWM que será la fuente del Trigger para empezar la conversión multicanal */
	adc_ExternalTrigger(&pwmHandler);



	/* Loop forever*/
	while (1){

		if (usart2DataReceived == 't'){
			sprintf(bufferMsg, "¿Probando? ¡Funciona! Ouh-Yeah! \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);
			usart2DataReceived = '\0';
		}

		/* Iniciamos las conversiones ADC para el sensor1 */
		if (usart2DataReceived == 'a'){

			flagLetra = usart2DataReceived;
			startPwmSignal(&pwmHandler);

			sprintf(bufferMsg, "\r\n");
			usart_WriteMsg(&commSerial, bufferMsg);

			sprintf(bufferMsg, "Empezando conversiones del Sensor 1 \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);
			usart2DataReceived = '\0';
		}

		/* Iniciamos las conversiones ADC para el sensor2 */
		if (usart2DataReceived == 'b'){

			flagLetra = usart2DataReceived;
			startPwmSignal(&pwmHandler);

			sprintf(bufferMsg, "\r\n");
			usart_WriteMsg(&commSerial, bufferMsg);

			sprintf(bufferMsg, "Empezando conversiones del Sensor 2 \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);
			usart2DataReceived = '\0';
		}

		/* Iniciamos las conversiones ADC para el sensor3 */
		if (usart2DataReceived == 'c'){

			flagLetra = usart2DataReceived;
			startPwmSignal(&pwmHandler);

			sprintf(bufferMsg, "\r\n");
			usart_WriteMsg(&commSerial, bufferMsg);

			sprintf(bufferMsg, "Empezando conversiones del Sensor 3 \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);
			usart2DataReceived = '\0';
		}

		/* Guarda los datos y reinicia la secuencia de conversión */
		if(flagADC){

			// Bajamos la bandera
			flagADC = 0;

			// Guardamos los datos adquiridos en los arreglos
			ADC_Data1[count_ADC_Data] = sensores[0].adcData;
			ADC_Data2[count_ADC_Data] = sensores[1].adcData;
			ADC_Data3[count_ADC_Data] = sensores[2].adcData;

			// Reiniciamos la secuencia de conversiones (Ronda de conversiones)
			startPwmSignal(&pwmHandler);
		}


		if(flagStop){

			// Bajamos la bandera
			flagStop = 0;

			/* Denetemos las conversiones ADC */
			stopPwmSignal(&pwmHandler);

			sprintf(bufferMsg, "\r\n");
			usart_WriteMsg(&commSerial, bufferMsg);
			sprintf(bufferMsg, "Terminé la conversión \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);

			/* Procesa los datos de la señal seleccionada */
			switch(flagLetra){
			case 'a':{
				procesamientoFFT(ADC_Data1);
			}
			case 'b':{
				procesamientoFFT(ADC_Data2);
			}
			case 'c':{
				procesamientoFFT(ADC_Data3);
			}
			}

			flagLetra = '\0';
			count_ADC_Data = 0; // Reiniciamos el contador luego de realizar el FFT
		}


	} // Fin while()

	return 0;

} // Fin main()

/*
 * ===== IMPORTANTE ADC =====
 * A continuación se muestran los pines que corresponden a
 * los diferentes canales del módulo ADC (Consersión Análogo Digital)
 *
 * 	CANAL ADC	PIN			USART		TX		RX
 *
 *	ADC1_0		PA0			USART2		PA2		PA3
 * 	ADC1_1		PA1			USART6		PC6		PC7
 * 	ADC1_2		PA2			USART1		PA9		PA10
 * 	ADC1_3		PA3			USART6		PA11	PA12
 * 	ADC1_4		PA4			USART1		PA15
 * 	ADC1_5		PA5			USART2		PD5		PD6
 * 	ADC1_6		PA6			USART1				PB3
 * 	ADC1_7		PA7			USART1		PB6		PB7
 * 	ADC1_8		PB0
 * 	ADC1_9		PB1
 * 	ADC1_10		PC0
 * 	ADC1_11		PC1
 * 	ADC1_12		PC2
 * 	ADC1_13		PC3
 * 	ADC1_14		PC4
 * 	ADC1_15		PC5
 *
 */

/*
 * Definimos una función para inicializar las configuraciones
 * de los diferentes periféricos
 */
void configPeripherals(void){

	// 1. ===== PUERTOS Y PINES =====
	/* Configurando el pin para el Blinky */
	userLed.pGPIOx								= GPIOA;
	userLed.pinConfig.GPIO_PinNumber			= PIN_5;	// PinA5 -> Led2 del STM32F411
	userLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&userLed);
	gpio_WritePin(&userLed, SET);

	/* Configurando los pines para el puerto serial
	 * Usamos el PinA2 para TX
	 */
	pinTx.pGPIOx								= GPIOA;
	pinTx.pinConfig.GPIO_PinNumber				= PIN_2;
	pinTx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinTx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinTx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinTx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinTx);


	/* Usamos el PinA3 para RX */
	pinRx.pGPIOx								= GPIOA;
	pinRx.pinConfig.GPIO_PinNumber				= PIN_3;
	pinRx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinRx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinRx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinRx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinRx);

	// 2. ===== TIMERS =====
	/* Configurando el Timer del Blinky*/
	blinkTimer.pTIMx								= TIM2;
	blinkTimer.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	blinkTimer.TIMx_Config.TIMx_Period				= 500;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (500 ms)
	blinkTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	blinkTimer.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos la configuración del Timer */
	timer_Config(&blinkTimer);

	/* Encendemos el Timer */
	timer_SetState(&blinkTimer, TIMER_ON);


	// 4. ===== USARTS =====
	/* Configurando el puerto serial USART2 */
	commSerial.ptrUSARTx					= USART2;
	commSerial.USART_Config.baudrate		= USART_BAUDRATE_115200;
	commSerial.USART_Config.datasize		= USART_DATASIZE_8BIT;
	commSerial.USART_Config.parity			= USART_PARITY_NONE;
	commSerial.USART_Config.stopbits		= USART_STOPBIT_1;
	commSerial.USART_Config.mode			= USART_MODE_RXTX;
	commSerial.USART_Config.enableIntRX		= USART_RX_INTERRUP_ENABLE;

	/* Cargamos la configuración de USART */
	usart_Config(&commSerial);

	/*
	 * Escribimos el caracter nulo para asegurarnos de empezar
	 * una transmisión "limpia"
	 */
	usart_WriteChar(&commSerial, '\0');


	// 5. ===== PWM =====
	/* Configurando el PWM */
	pwmHandler.ptrTIMx					= TIM3; 	// Timer3 usado para el PWM
	pwmHandler.config.channel			= PWM_CHANNEL_1;
	pwmHandler.config.prescaler			= 16; 		// 1 us
	pwmHandler.config.periodo			= 25;		// 25 us -> 40kHz
	pwmHandler.config.dutyCycle			= 12;		// Activo un ~50% del periodo

	/* Cargar la configuración del PWM */
	pwm_Config(&pwmHandler);


	// 6. ===== ADC =====

	/* Configuración del sensor1 */
	sensor1.channel				= CHANNEL_6;
	sensor1.resolution			= RESOLUTION_12_BIT;
	sensor1.dataAlignment		= ALIGNMENT_RIGHT;
	sensor1.samplingPeriod		= SAMPLING_PERIOD_84_CYCLES;
	sensor1.interrupState		= ADC_INT_ENABLE;

	/* Configuración del sensor2 */
	sensor2.channel				= CHANNEL_15;
	sensor2.resolution			= RESOLUTION_12_BIT;
	sensor2.dataAlignment		= ALIGNMENT_RIGHT;
	sensor2.samplingPeriod		= SAMPLING_PERIOD_84_CYCLES;
	sensor2.interrupState		= ADC_INT_ENABLE;

	/* Configuración del sensor3 */
	sensor3.channel				= CHANNEL_7;
	sensor3.resolution			= RESOLUTION_12_BIT;
	sensor3.dataAlignment		= ALIGNMENT_RIGHT;
	sensor3.samplingPeriod		= SAMPLING_PERIOD_84_CYCLES;
	sensor3.interrupState		= ADC_INT_ENABLE;



}	// Fin de la configuración de los periféricos


/* Función para realizar el cálculo de la FFT para cada sensor */
void procesamientoFFT(float32_t *array){

	/* Inicializamos la funcion de la transformada */
	statusInitFFT = arm_rfft_fast_init_f32(&config_Rfft_fast_f32, fftSize);


	/* Controlamos que la función para inicializar la FFT haya finalizado correctamente */
	if(statusInitFFT == ARM_MATH_SUCCESS){
		arm_rfft_fast_f32(&config_Rfft_fast_f32, array, transformedSignal, ifftFlag);
	}
	else{
		usart_WriteMsg(&commSerial, "FFT no inicializada correctamente\n\r");
	}

	transformedSignal[0] = 0;
	arm_cmplx_mag_f32(transformedSignal, fft_magnitud, ADC_DataSize/2);

	/* Obtenemos los valores Máximo y Mínimo de la magnitud de los complejos, y sus índices */
	float32_t   maxValue;
	float32_t   minValue;
	uint32_t 	maxIndex;
	uint32_t 	minIndex;

	arm_max_f32(fft_magnitud, ADC_DataSize/2, &maxValue, &maxIndex);
	arm_min_f32(fft_magnitud, ADC_DataSize/2, &minValue, &minIndex);

	sprintf(bufferMsg, "\r\n");
	usart_WriteMsg(&commSerial, bufferMsg);

	sprintf(bufferMsg, "max power: %f\r\n", maxValue);
	usart_WriteMsg(&commSerial, bufferMsg);

	sprintf(bufferMsg, "min power: %f\r\n", minValue);
	usart_WriteMsg(&commSerial, bufferMsg);

	sprintf(bufferMsg, "frequency: %f\r\n", ((maxIndex/2) * (frec_muestreo/ADC_DataSize)));
	usart_WriteMsg(&commSerial, bufferMsg);

	usart2DataReceived = '\0';

} // Fin FFT



/* Callback de Timer 2 (Controla el userLed) */
void Timer2_Callback(void){
	gpio_TooglePin(&userLed);
	sendMsg ++;
}

/* Callback del ADC -> Atiende la interrupción cada que finaliza una converisón
 * ADC de uno de los canales de la secuencia
 */
void adc_CompleteCallback(void){

	// Guardamos el valor de la conversión correspondiente a cada sensor de la secuencia
	sensores[posicion].adcData = adc_GetValue();
	posicion++;

	if(posicion >= Length){
		posicion = 0;
		stopPwmSignal(&pwmHandler);
		count_ADC_Data++;
		flagADC = 1;
	}

	if(count_ADC_Data > 511){
		flagStop = 1;
		flagADC = 0;
		count_ADC_Data = 0;
	}
}

void usart2_RxCallback(void){
	usart2DataReceived = usart2_getRxData();
}
/*
 * Función assert para detectar problemas de paŕametros incorrectos
 */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
