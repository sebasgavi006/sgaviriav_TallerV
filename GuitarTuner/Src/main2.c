/*
 ******************************************************************************
 * @file           : main.c
 * @author         :
 * @brief          : Main de prueba para la Transformada Rápida de Fourier (FFT)

 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "stm32f4xx.h"

#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "exti_driver_hal.h"
#include "usart_driver_hal.h"
#include "adc_driver_hal.h"

#include "arm_math.h"

// Definicion de los Handlers necesario
GPIO_Handler_t userLed = {0}; //Led de estado PinA5

Timer_Handler_t blinkTimer = {0};

// Handlers del ADC
ADC_Config_t sensores[3] = {0}; // Array de sensores
ADC_Config_t sensor1 = {0};
ADC_Config_t sensor2 = {0};
ADC_Config_t sensor3 = {0};
uint8_t Length = 3; // Tamaño del arreglo
uint8_t posicion = 0;	// Posicion de la secuencia PWM
uint16_t count_ADC_Data = 0; // Contador de la cantidad de datos que lleva el arreglo ADC_Data[512]
uint8_t flagStop = 0;
uint8_t flagADC = 0;

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
/* arreglos para las librerias cmsis */
float32_t srcNumber[4] = {-0.987, 32.26, -45.21, -987.321};
float32_t destNumber[4] = {0};
uint32_t dataSize = 0;

/* Para utilizar la funcion seno */
float32_t sineValue = 0.0;
float32_t sineArgValue = 0.0;

/* Elementos para generar una selal */
#define SINE_DATA_SIZE 512//Tamaño del arreglo de datos
float32_t fs = 16000000/(16*25); //frecuencia de muestreo
float32_t f0 = 800.0; // frecuencia fundamental de la señal
float32_t dt = 0.0;   //periodo del muestreo
float32_t stopTime = 1.0; //quizas no es necesario
float32_t amplitud = 5; //amplitud de la señal
float32_t sineSignal[SINE_DATA_SIZE];
float32_t fft_power[SINE_DATA_SIZE/2];
float32_t transformedSignal[SINE_DATA_SIZE];
float32_t *ptrSineSignal;

uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
arm_rfft_fast_instance_f32 config_Rfft_fast_f32;
arm_cfft_radix4_instance_f32 configRadix4_f32;
arm_status status = ARM_MATH_ARGUMENT_ERROR;
arm_status statusInitFFT = ARM_MATH_ARGUMENT_ERROR;
uint16_t fftSize = 512;

//Definicion de las cabeceras de las funciones  del main
void configPeripherals(void);
void createSignal(void);

/*
 * Funcion principal del sistema
 */
int main(void){

	/* Activamos el FPU */
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
			sprintf(bufferMsg, "Testing? ¡Works! \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);
			usart2DataReceived = '\0';
		}

		/* Iniciamos las conversiones ADC */
		if (usart2DataReceived == 'a'){

			startPwmSignal(&pwmHandler);

			sprintf(bufferMsg, "\r\n");
			usart_WriteMsg(&commSerial, bufferMsg);

			sprintf(bufferMsg, "Empezando conversiones \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);
			usart2DataReceived = '\0';
		}

		if(flagADC){

			flagADC = 0;

			ADC_Data1[count_ADC_Data] = sensores[0].adcData;
//			sprintf(bufferMsg, "Dato: %i -> %f \n\r", count_ADC_Data, ADC_Data1[count_ADC_Data]);
//			usart_WriteMsg(&commSerial, bufferMsg);

			ADC_Data2[count_ADC_Data] = sensores[1].adcData;

			ADC_Data3[count_ADC_Data] = sensores[2].adcData;

			startPwmSignal(&pwmHandler);
		}

		if(flagStop){

			flagStop = 0;

			/* Denetemos las conversiones ADC */
			stopPwmSignal(&pwmHandler);

			sprintf(bufferMsg, "\r\n");
			usart_WriteMsg(&commSerial, bufferMsg);
			sprintf(bufferMsg, "Terminé \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);
		}

			/* Sacar el valor de la transformada para hallar la frecuencia de la señal */
			if (usart2DataReceived == '1'){

				/* Inicializamos la funcion de la transformada */
				statusInitFFT = arm_rfft_fast_init_f32(&config_Rfft_fast_f32, fftSize);

				if(statusInitFFT == ARM_MATH_SUCCESS){
					sprintf(bufferMsg, "Initialization...SUCCESS! \n");
					usart_WriteMsg(&commSerial, bufferMsg);
				}

				sprintf(bufferMsg, "FFT \n");
				usart_WriteMsg(&commSerial, bufferMsg);

				if(statusInitFFT == ARM_MATH_SUCCESS){
					arm_rfft_fast_f32(&config_Rfft_fast_f32, ADC_Data1, transformedSignal, ifftFlag);


				}
				else{
					usart_WriteMsg(&commSerial, "FFT NOT INITIALIZED...\n");
				}

				transformedSignal[0] = 0;
				arm_cmplx_mag_f32(transformedSignal, fft_power, SINE_DATA_SIZE/2);

				for (int i = 1; i < SINE_DATA_SIZE/2; i++) {
					sprintf(bufferMsg, "%i\tfrq: %.1f\tenergy %.2f\r\n", i, i * fs/SINE_DATA_SIZE, fft_power[i]);
					usart_WriteMsg(&commSerial, bufferMsg);
				}

				float32_t   maxValue;
				float32_t   minValue;
				uint32_t 	maxIndex;
				uint32_t 	minIndex;


				arm_max_f32(fft_power, SINE_DATA_SIZE/2, &maxValue, &maxIndex);
				arm_min_f32(fft_power, SINE_DATA_SIZE/2, &minValue, &minIndex);

				sprintf(bufferMsg, "\r\n");
				usart_WriteMsg(&commSerial, bufferMsg);

				sprintf(bufferMsg, "max power: %f\r\n", maxValue);
				usart_WriteMsg(&commSerial, bufferMsg);

				sprintf(bufferMsg, "max index: %lu\r\n", maxIndex);
				usart_WriteMsg(&commSerial, bufferMsg);

				sprintf(bufferMsg, "min power: %f\r\n", minValue);
				usart_WriteMsg(&commSerial, bufferMsg);

				sprintf(bufferMsg, "min index: %lu\r\n", minIndex);
				usart_WriteMsg(&commSerial, bufferMsg);

				sprintf(bufferMsg, "frequency: %f\r\n", ((maxIndex/2) * (fs/SINE_DATA_SIZE)));
				usart_WriteMsg(&commSerial, bufferMsg);

				usart2DataReceived = '\0';
				count_ADC_Data = 0; // Reiniciamos el contador luego de realizar el FFT


			} // Fin FFT



//			/* Se crea una señal Seno de prueba */
//			if (usart2DataReceived == 'C'){
//				createSignal();
//				sprintf(bufferMsg, "Creando la señal... \n\r");
//				usart_WriteMsg(&commSerial, bufferMsg);
//				usart2DataReceived = '\0';
//			}
//
//			/* Para probar el seno -> Imprime los valores de la señal creada */
//			if (usart2DataReceived == 'P'){
//
//				stopTime = 0.0;
//				int i = 0;
//
//				sprintf(bufferMsg, "Signal values: time - sine\n");
//				usart_WriteMsg(&commSerial, bufferMsg);
//
//				while(stopTime < 0.01){
//					stopTime = dt*i;
//					i++;
//					sprintf(bufferMsg, "%#.5f ; %f\n", stopTime, ADC_Data1[i]);
//					usart_WriteMsg(&commSerial, bufferMsg);
//				}
//				usart2DataReceived = '\0';
//			}
//
//
//
//
//
//			/* Saca el valor absoluto a la señal, para que sea positiva (DC -> Como entrada del Micro) */
//			if (usart2DataReceived == 'A'){
//
//				stopTime = 0.0;
//				int i = 0;
//
//				sprintf(bufferMsg, "Valor Absoluto \n");
//				usart_WriteMsg(&commSerial, bufferMsg);
//
//				arm_abs_f32(sineSignal, transformedSignal, SINE_DATA_SIZE);
//
//				while(stopTime < 0.01){
//					stopTime = dt*i;
//					i++;
//					sprintf(bufferMsg, "%#.5f ; %#.6f\n", stopTime, transformedSignal[i]);
//					usart_WriteMsg(&commSerial, bufferMsg);
//				}
//				usart2DataReceived = '\0';
//			}
//
//			/* Inicializa el proceso de la FFT */
//			if (usart2DataReceived == 'I'){
//
//				statusInitFFT = arm_rfft_fast_init_f32(&config_Rfft_fast_f32, fftSize);
//
//				if(statusInitFFT == ARM_MATH_SUCCESS){
//					sprintf(bufferMsg, "Initialization...SUCCESS! \n");
//					usart_WriteMsg(&commSerial, bufferMsg);
//				}
//
//				usart2DataReceived = '\0';
//			}
//
//			/* Ingresa los datos de la señal de entrada en la función de Transformada */
//			if (usart2DataReceived == 'F'){
//
//				stopTime = 0.0;
//				int i = 0;
//				int j = 0;
//
//				sprintf(bufferMsg, "FFT \n");
//				usart_WriteMsg(&commSerial, bufferMsg);
//
//				if(statusInitFFT == ARM_MATH_SUCCESS){
//					arm_rfft_fast_f32(&config_Rfft_fast_f32, sineSignal, transformedSignal, ifftFlag);
//
//					arm_abs_f32(transformedSignal, sineSignal, fftSize);
//
//					for(i=1; i < fftSize; i++){
//						if(i%2){
//							sprintf(bufferMsg, "%u ; %#.6f n", j, 2*sineSignal[i]);
//							usart_WriteMsg(&commSerial, bufferMsg);
//							i++;
//						}
//					}
//				}
//				else{
//					usart_WriteMsg(&commSerial, "FFT NOT INITIALIZED...\n");
//				}
//				usart2DataReceived = '\0';
//			}
//
//
//			if (usart2DataReceived == 'D'){
//				arm_cmplx_mag_f32(transformedSignal, fft_power, SINE_DATA_SIZE/2);
//				for (int i = 1; i < SINE_DATA_SIZE/2; i++) {
//					sprintf(bufferMsg, "%i\tfrq: %.1f\tenergy %.6f\r\n", i, i * fs/SINE_DATA_SIZE, fft_power[i]);
//					usart_WriteMsg(&commSerial, bufferMsg);
//				}
//				usart2DataReceived = '\0';
//			}
//
//			/* Obtiene los valores de la Frecuencia de la señal obtenido a través de la Transformada */
//			if (usart2DataReceived == 'X'){
//
//				float32_t   maxValue;
//				float32_t   minValue;
//				uint32_t 	maxIndex;
//				uint32_t 	minIndex;
//
//
//				arm_max_f32(fft_power, SINE_DATA_SIZE/2, &maxValue, &maxIndex);
//				arm_min_f32(fft_power, SINE_DATA_SIZE/2, &minValue, &minIndex);
//
//				sprintf(bufferMsg, "\r\n");
//				usart_WriteMsg(&commSerial, bufferMsg);
//
//				sprintf(bufferMsg, "max power: %f\r\n", maxValue);
//				usart_WriteMsg(&commSerial, bufferMsg);
//
//				sprintf(bufferMsg, "max power: %lu\r\n", maxIndex);
//				usart_WriteMsg(&commSerial, bufferMsg);
//
//				sprintf(bufferMsg, "min power: %f\r\n", minValue);
//				usart_WriteMsg(&commSerial, bufferMsg);
//
//				sprintf(bufferMsg, "max index: %lu\r\n", maxIndex);
//				usart_WriteMsg(&commSerial, bufferMsg);
//
//				sprintf(bufferMsg, "frequency: %f\r\n", ((maxIndex/2) * (fs/SINE_DATA_SIZE)));
//				usart_WriteMsg(&commSerial, bufferMsg);
//
//				usart2DataReceived = '\0';
//				count_ADC_Data = 0; // Reiniciamos el contador luego de realizar el FFT
//
//		}

	} // Fin while()

	return 0;
} // Fin mian()

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
	 *  - Usamos el PinA2 para TX
	 */
	pinTx.pGPIOx								= GPIOA;
	pinTx.pinConfig.GPIO_PinNumber				= PIN_2;
	pinTx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinTx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinTx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinTx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinTx);


	/* - Usamos el PinA3 para RX */
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
	pwmHandler.config.periodo			= 25;		// 125 us -> 8kHz
	pwmHandler.config.dutyCycle			= 12;		// Activo un ~10% para eficiencia energética

	/* Cargar la configuración del PWM */
	pwm_Config(&pwmHandler);


	// 6. ===== ADC =====

	/* Configuración del sensor1 -> PC0 */
	sensor1.channel				= CHANNEL_6;
	sensor1.resolution			= RESOLUTION_12_BIT;
	sensor1.dataAlignment		= ALIGNMENT_RIGHT;
	sensor1.samplingPeriod		= SAMPLING_PERIOD_84_CYCLES;
	sensor1.interrupState		= ADC_INT_ENABLE;

	/* Configuración del sensor1 */
	sensor2.channel				= CHANNEL_15;
	sensor2.resolution			= RESOLUTION_12_BIT;
	sensor2.dataAlignment		= ALIGNMENT_RIGHT;
	sensor2.samplingPeriod		= SAMPLING_PERIOD_84_CYCLES;
	sensor2.interrupState		= ADC_INT_ENABLE;

	/* Configuración del sensor1 */
	sensor3.channel				= CHANNEL_7;
	sensor3.resolution			= RESOLUTION_12_BIT;
	sensor3.dataAlignment		= ALIGNMENT_RIGHT;
	sensor3.samplingPeriod		= SAMPLING_PERIOD_84_CYCLES;
	sensor3.interrupState		= ADC_INT_ENABLE;



}	// Fin de la configuración de los periféricos


void createSignal(void){
	dt = 1 / fs; // periodo
	for(int i = 0;  i < SINE_DATA_SIZE; i++){
		sineSignal[i] = amplitud * arm_sin_f32(2*M_PI*f0*(dt*i));
	}
}


void Timer2_Callback(void){
	gpio_TooglePin(&userLed);
	sendMsg ++;
}


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
