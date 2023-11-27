/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Prueba para implementar la función final de lectura del ADC
 * 					 del micrófono, para obtener la respuesta de la frecuencia
 * 					 dentro de un rango determinado de frecuencias para la
 * 					 afinación de la nota musical en cuestión (su frecuencia)
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

/* ===== CONSTANTES ===== */
#define	MCU_CLOCK_16_MHz	16000000
#define LENGTH	1

GPIO_Handler_t userLed = {0}; //Led de estado PinA5

Timer_Handler_t blinkTimer = {0};

// Handlers del ADC
ADC_Config_t sensores[LENGTH] = {0}; // Array de sensores
ADC_Config_t sensor1 = {0};
uint16_t	count_ADC_Data = 0; // Contador de la cantidad de datos que lleva el arreglo ADC_Data[512]
uint8_t		flagStop = 0;	// Detiene las conversiones ADC cuando finalice las 512 conversiones
uint8_t 	flagADC = 0;	// Bandera para controlar la finalización de una secuencia de conversiones


// PWM para generar la frecuencia de muestreo
PWM_Handler_t pwmHandler = {0};

/* Elementos para la comunicacion serial */
USART_Handler_t commSerial = {0};
GPIO_Handler_t pinTx = {0};
GPIO_Handler_t pinRx = {0};
uint8_t usart2DataReceived = 0;
char	bufferMsg[64] = {0};


// Código de los videos del classroom
/* Elementos para generar una selal */
#define 	ADC_DataSize 1024	// Tamaño del arreglo de datos
uint16_t 	fftSize = ADC_DataSize;		// Tamaño del arreglo de los valores obtenidos de la transformada
float32_t 	frec_muestreo; //frecuencia de muestreo -> 40kHz
float32_t	factor_correccion;
float32_t	frec_corregida;
float32_t	frec_real;
float32_t	frec_real_magnitud;
float32_t 	frec_prom;


float32_t	fft_reales[ADC_DataSize/2];
float32_t 	fft_magnitud[ADC_DataSize/2];
float32_t 	transformedSignal[ADC_DataSize];


/* Arreglos para guardar el los datos obtenidos del ADC */
float32_t ADC_Data1[ADC_DataSize] = {0};	// Arreglo para guardar los datos para la FFT del sensor 1


/* Obtenemos los valores Máximo y Mínimo de la magnitud de los complejos, y sus índices */
float32_t   maxValue = 0;
uint32_t 	maxIndex = 0;
float32_t   maxValue_r = 0;
uint32_t 	maxIndex_r = 0;


/* Variables para instanciar e inicializar los funciones de la FFT
 * (Necesarias para el uso de las funciones FFT del CMSIS)
 */
uint32_t ifftFlag = 0;
arm_rfft_fast_instance_f32 config_Rfft_fast_f32;
arm_status statusInitFFT = ARM_MATH_ARGUMENT_ERROR;


//Definicion de las cabeceras de las funciones  del main
void configPeripherals(void);
void procesamientoFFT(float32_t *array);
void seleccionRango(float32_t frecuencia);


/*
 * Funcion principal del sistema
 */
int main(void){

	/* Activamos el FPU (Unidad de Punto Flotante) */
	SCB->CPACR |= (0xF << 20);

	/* Configuramos los periféricos */
	configPeripherals();

	sensores[0] = sensor1;

	/* Definimos la frecuencia de muestreo de acuerdo al PWM definido */
	frec_muestreo = MCU_CLOCK_16_MHz / (pwmHandler.config.periodo * pwmHandler.config.prescaler);

	factor_correccion = (0.00101 * (frec_muestreo/1000)) + 0.0612; // Porque se obtuvo a partir de kHz

	frec_corregida = frec_muestreo * (1 - factor_correccion);

	/* Cargamos la configuración de los sensores en la función Multicanal del ADC */
	adc_ConfigMultiChannel(sensores, LENGTH);

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

			startPwmSignal(&pwmHandler);

			sprintf(bufferMsg, "\r\n");
			usart_WriteMsg(&commSerial, bufferMsg);

			sprintf(bufferMsg, "Empezando conversiones del Sensor 1 \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);
			usart2DataReceived = '\0';
		}

		if (usart2DataReceived == 's'){

			stopPwmSignal(&pwmHandler);

			sprintf(bufferMsg, "\r\n");
			usart_WriteMsg(&commSerial, bufferMsg);

			sprintf(bufferMsg, "Pausando conversiones del Sensor 1 \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);
			usart2DataReceived = '\0';
		}


		/* Se atiende la bandera que indica la finalización de las conversiones ADC */
		if(flagStop){

			// Bajamos la bandera
			flagStop = 0;

			sprintf(bufferMsg, "Terminé la conversión \n\r");
			usart_WriteMsg(&commSerial, bufferMsg);

			procesamientoFFT(ADC_Data1);

			frec_prom = frec_prom;

			usart2DataReceived = '\0';
		}

		/* Se imprime de acuerdon al rango en el que se encuentra la frecuencia */


	} // Fin while()

	return 0;

} // Fin main()

/* ===== IMPORTANTE ADC =====
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
	pwmHandler.config.periodo			= 286;		// 286 us -> 3500 Hz
	pwmHandler.config.dutyCycle			= 16;		// Tiempo activo de la señal (En alto)

	/* Cargar la configuración del PWM */
	pwm_Config(&pwmHandler);


	// 6. ===== ADC =====

	/* Configuración del sensor1 -> PC0 -> AZUL */
	sensor1.channel				= CHANNEL_10;
	sensor1.resolution			= RESOLUTION_12_BIT;
	sensor1.dataAlignment		= ALIGNMENT_RIGHT;
	sensor1.samplingPeriod		= SAMPLING_PERIOD_84_CYCLES;
	sensor1.interrupState		= ADC_INT_ENABLE;


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

	/* Calculamos la magnitud de los resultados obtenidos gracias a la transformada,
	 * dado su caracter complejo
	 */
	// Limpiamos las primeras dos posiciones, pues no nos dan información de la frecuencia

	transformedSignal[0] = 0;
	transformedSignal[1] = 0;

	for(uint16_t i = 0; i < ADC_DataSize/2; i++){
		fft_reales[i] = transformedSignal[i*2];
	}

	maxValue_r = 0;
	maxIndex_r = 0;
	arm_max_f32(fft_reales, ADC_DataSize/2, &maxValue_r, &maxIndex_r);

	frec_real = ((maxIndex_r) * (frec_corregida/ADC_DataSize));

	sprintf(bufferMsg, "\r\n");
	usart_WriteMsg(&commSerial, bufferMsg);
	sprintf(bufferMsg, "Frecuencia (Con max): %.4f Hz\r\n", frec_real);
	usart_WriteMsg(&commSerial, bufferMsg);


	/* Obtenemos el valor máximo de la magnitud de los complejos, para hallar la frecuencia dominante */
	arm_cmplx_mag_f32(transformedSignal, fft_magnitud, ADC_DataSize/2);

	maxValue = 0;
	maxIndex = 0;
	arm_max_f32(fft_magnitud, ADC_DataSize/2, &maxValue, &maxIndex);

	frec_real_magnitud = ((maxIndex) * (frec_corregida/ADC_DataSize));

	sprintf(bufferMsg, "\r\n");
	usart_WriteMsg(&commSerial, bufferMsg);
	sprintf(bufferMsg, "Frecuencia (Con magnitud): %.4f Hz\r\n", frec_real_magnitud);
	usart_WriteMsg(&commSerial, bufferMsg);

	/* Obtenemos el promedio de las dos transformadas obtenidas */
	float32_t result_fft[2] = {frec_real, frec_real_magnitud};

	arm_mean_f32(result_fft, 2, &frec_prom);

	sprintf(bufferMsg, "\r\n");
	usart_WriteMsg(&commSerial, bufferMsg);
	sprintf(bufferMsg, "Frecuencia (Promedio): %.4f Hz\r\n", frec_prom);
	usart_WriteMsg(&commSerial, bufferMsg);

	/* Limpiamos el arreglo original */
	for(uint16_t i = 0; i < (ADC_DataSize-1); i++){
		array[i] = 0;
	}

	usart2DataReceived = '\0';

} // Fin FFT


/*
 * Hola
 */
void seleccionRango(float32_t frecuencia){


} // Fin seleccionRango()



/* Callback de Timer 2 (Controla el userLed) */
void Timer2_Callback(void){
	gpio_TooglePin(&userLed);
}

/* Callback del ADC -> Atiende la interrupción cada que finaliza una converisón
 * ADC de uno de los canales de la secuencia
 */
void adc_CompleteCallback(void){

	// Guardamos el valor de la conversión correspondiente a cada sensor de la secuencia
	ADC_Data1[count_ADC_Data] = (float32_t)adc_GetValue();
	count_ADC_Data++;

	if(count_ADC_Data > (ADC_DataSize-1)){
		stopPwmSignal(&pwmHandler);
		flagStop = 1;
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
