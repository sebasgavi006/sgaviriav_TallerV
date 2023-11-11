/**
 ***************************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Este proyecto constituye el Parcial de Taller V - 2023-02
 *
 ***************************************************************************************
 **/

/* Importando las librerías necesarias desde el PeripheralsDrivers */
#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "exti_driver_hal.h"
#include "usart_driver_hal.h"
#include "adc_driver_hal.h"
#include "systick_driver_hal.h"
#include "pwm_driver_hal.h"

/* ===== Definición de variables ===== */
/* Definición de variables contadoras para cada modo
 * del Encoder (Resolución y Sensor) */
uint8_t contadorSensor = 1;
uint16_t contadorPWM_Encoder = 0;
uint16_t contadorPWM_ADC = 0;
uint8_t contadorModo = 0;

// Estas constantes guardan el valor del PWM
uint16_t valorPWM_colorR = 0;
uint16_t valorPWM_colorG = 0;
uint16_t valorPWM_colorB = 0;

// Guarda el valor ADC de los 3 sensores
uint16_t valorADC_Sensor1 = 0;
uint16_t valorADC_Sensor2 = 0;
uint16_t valorADC_Sensor3 = 0;



/* Definicón de la variable para leer la entrada del Data del Encoder */

uint8_t readData = 0;

/* Definición de las banderas */
uint8_t flagMode = 0;		// Bandera del EXTI del Switch. Se asigna 0 para indicar que se inicializa en Modo Resolución
uint8_t flagData = 0;		// Bandera del EXTI del Encoder.
uint8_t flagConv = 0;		// Bandera para atender interrupciones por conversión
uint8_t flagDisplay = 0;
uint8_t sendMsg = 0;		// Bandera para mostrar la lectura del ADC cada 2 segundos

// Constantes para identificar los 3 sensores
enum
{
	SENSOR1	= 1,	// Pin PC5 -> ADC_15
	SENSOR2 = 2,	// Pin PA7 -> ADC_7
	SENSOR3 = 3		// Pin PA6 -> ADC_6
};

// Constantes para establecer los canales de cada sensor
enum
{
	SENSOR1_CH = CHANNEL_15,
	SENSOR2_CH = CHANNEL_7,
	SENSOR3_CH = CHANNEL_6
};

// Valores decimales para 12bits y 0bits
enum
{
	NUMBER_12BITS	= 0b111111111111,
	NUMBER_0BITS	= 0b0
};

// Constantes para los canales del PWM para cada color del RGB
enum
{
	CHANNEL_R = PWM_CHANNEL_4,	// PB11
	CHANNEL_G = PWM_CHANNEL_2,	// PB3
	CHANNEL_B = PWM_CHANNEL_1	// PA15
};


// Constantes para ON y OFF
enum
{
	OFF = 0,
	ON
};

/* ===== Definición de los pines a utilizar ===== */

// Pin para el led de estado
GPIO_Handler_t stateLed = {0}; // Pin PA5 (led de estado)

// Pines para el encoder
GPIO_Handler_t encoderClk = {0}; // Pin PC8 (Canal 8 del EXTI)
GPIO_Handler_t data = {0};	// Pin PB2
GPIO_Handler_t sw = {0};	// Pin PA0 (Canal 0 del EXTI)

// Pines para el 7-segmentos
GPIO_Handler_t segmentoA = {0}; // Pin PB5
GPIO_Handler_t segmentoB = {0}; // Pin PC4
GPIO_Handler_t segmentoC = {0}; // Pin PC3
GPIO_Handler_t segmentoD = {0}; // Pin PB0
GPIO_Handler_t segmentoE = {0}; // Pin PC2
GPIO_Handler_t segmentoF = {0}; // Pin PB10
GPIO_Handler_t segmentoG = {0}; // Pin PC0
GPIO_Handler_t segmentoPunto = {0}; // Pin


// Pines para controlar la activación-desactivación de los transistores para alternar
// entre los cristales del 7-segmentos
GPIO_Handler_t display2 = {0}; // Pin PA10


/* Definición de los Timers a utilizar para generar las interrupciones
 * del blinky y del 7 segmentos */
Timer_Handler_t blinkTimer3 = {0};		// Timer 3 para el stateLed
Timer_Handler_t messageTimer4 = {0};	// Timer 4 para enviar mensajes seriales cada 2 segundos
Timer_Handler_t displayTimer5 = {0};	// Timer 5 para prender y apagar el display


/* Definición de los EXTI para las interrupciones externas del encoder */
EXTI_Config_t exti0 = {0};	// Definimos el EXTI del Switch (estructura -> "objeto")
EXTI_Config_t exti8 = {0};	// Definimos el EXTI del Encoder (estructura -> "objeto")

/* Pines de los canales para la conversión ADC */
ADC_Config_t sensor = {0};		// Canal X -> Pin P

/* Configuración de la comunicación serial */
USART_Handler_t usart2 = {0}; // Configuración de la transmisión serial por USART2
GPIO_Handler_t pinTx = {0};	// P -> Pin para la transmisión serial
GPIO_Handler_t pinRx = {0};	// P -> Pin para la recepción serial
rxDataUsart received_USARTx = {0};	// Estructura para guardar los datos según el USART utilizado
char bufferData[64] = {0};

/* Configuramos el Handler del PWM */
PWM_Handler_t pwmHandler = {0};
GPIO_Handler_t pwmPin = {0};
GPIO_Handler_t pinR = {0};
GPIO_Handler_t pinG = {0};
GPIO_Handler_t pinB = {0};


/* Declaramos el Handler del SysTick */
Systick_Handler_t systickHandler = {0};


/* ===== Headers de las funciones a utilizar en el main ===== */

// Función para cargar las configuraciones de los periféricos a utilizar
void systemConfig(void);

// Función para representar un numero_x en el 7-segmentos
void displayNumeros(uint8_t numero_x);

// Función para controlar los segmentos en el modo Sensores
void modeSensor(void);

// Función para configurar el sensor y la resolución cuando modifique desde el encoder o por transmisión serial
void configSensor(ADC_Config_t sensorHandler, uint8_t canal);

// Función para evaluar si se aumenta o disminuyen los contadores del sensor y la resolución
void evaluateEncoder(void);



/*
 * ======== FUNCIÓN PRINCIPAL DEL PROGRAMA ========
 */
int main(void)
{
	// Iniciamos con los contadores en valores preestablecidos

	contadorModo = 0;
	contadorSensor = SENSOR1;
	contadorPWM_Encoder = 0;
	contadorPWM_ADC = 0;
	valorPWM_colorR = 0;
	valorPWM_colorG = 0;
	valorPWM_colorB = 0;
	received_USARTx.rxData_USART2 = '\0';

	// Iniciamos con todas las banderas abajo
	flagData = 0;
	flagConv = 0;
	flagDisplay = 0;
	sendMsg = 0;


	// Cargamos la configuración de los periféricos
	systemConfig();

	// Dejamos el punto encendido
	gpio_WritePin(&segmentoPunto, RESET);

	// Empezamos a hacer conversiones
	adc_StartContinuousConv();

	// Iniciamos el PWM
	startPwmSignal(&pwmHandler);


	/* Loop forever */
	while(1){

		switch(contadorModo){
		case 0: {
			// PWM para R
			pwmHandler.config.channel = CHANNEL_R;
			pwm_Config(&pwmHandler);
			updateDutyCycle(&pwmHandler, valorADC_Sensor1);

			// PWM para G
			pwmHandler.config.channel = CHANNEL_G;
			pwm_Config(&pwmHandler);
			updateDutyCycle(&pwmHandler, valorADC_Sensor2);

			// PWM para B
			pwmHandler.config.channel = CHANNEL_B;
			pwm_Config(&pwmHandler);
			updateDutyCycle(&pwmHandler, valorADC_Sensor3);
			break;
		}
		case 1: {
			// PWM para R
			pwmHandler.config.channel = CHANNEL_R;
			pwm_Config(&pwmHandler);
			updateDutyCycle(&pwmHandler, valorADC_Sensor1);
			break;
		}
		case 2: {
			// PWM para G
			pwmHandler.config.channel = CHANNEL_G;
			pwm_Config(&pwmHandler);
			updateDutyCycle(&pwmHandler, valorADC_Sensor2);
			break;
		}
		case 3: {
			// PWM para B
			pwmHandler.config.channel = CHANNEL_B;
			pwm_Config(&pwmHandler);
			updateDutyCycle(&pwmHandler, valorADC_Sensor3);
			break;
		}
		case 4: {
			break;
		}
		case 5: {
			break;
		}
		case 6: {
			break;
		}
		}

		// Función para mostrar en el display
		if(flagDisplay){
			displayNumeros(contadorModo);
			flagDisplay = 0;
			gpio_TooglePin(&display2);
		}

		// Función para atender la conversión
		if(flagConv){

			flagConv = 0; // Bajamos la bandera

			switch(sensor.channel){
			case SENSOR1_CH: {
				valorADC_Sensor1 = sensor.adcData;
				configSensor(sensor, SENSOR2_CH);
				break;
			}
			case SENSOR2_CH: {
				valorADC_Sensor2 = sensor.adcData;
				configSensor(sensor, SENSOR3_CH);
				break;
			}
			case SENSOR3_CH: {
				valorADC_Sensor3 = sensor.adcData;
				configSensor(sensor, SENSOR1_CH);
				break;
			}
			}
		} // Fin func. conversion




		// Se atiende la interrupción del Encoder o de la comunicación serial y se modifican el sensor o la resolución
		if(flagData){
			flagData = 0;	// Bajamos la bandera
			evaluateEncoder();		// Modificamos el contador según corresponda (contadorSensor o contadorRes)
		}


		// Se atiende la interrupción del Timer4
		// - Enviamos un mensaje cada 2 segundos del estado actual del dispositivo
		if(sendMsg){
			sendMsg = 0; // Bajamos la bandera de envío del mensaje serial periódico
			if(sensor.adcData){
				sprintf(bufferData, "Sensor1, Conversion: %u\n\r", valorADC_Sensor1);
				usart_WriteMsg(&usart2, bufferData);
				sprintf(bufferData, "Sensor2, Conversion: %u\n\r", valorADC_Sensor2);
				usart_WriteMsg(&usart2, bufferData);
				sprintf(bufferData, "Sensor3, Conversion: %u\n\r", valorADC_Sensor3);
				usart_WriteMsg(&usart2, bufferData);
				sprintf(bufferData, "Valor actual del Encoder %u\n\r", contadorPWM_Encoder);
				usart_WriteMsg(&usart2, bufferData);
				sensor.adcData = 0;
			}

		} // Func. para Timer 4


	} // Fin del Loop Forever

} // ======== FIN DE LA FUNCIÓN PRINCIPAL ========



/* ===== Funciones auxiliares ===== */

/*
 * Función para cargar las configuraciones de los periféricos
 */
void systemConfig(void){

	/* ===== Configurando los pines que vamos a utilizar ===== */

	/* Configuraciones de los pines del 7-segmentos */
	segmentoA.pGPIOx							= GPIOB;
	segmentoA.pinConfig.GPIO_PinNumber			= PIN_5;
	segmentoA.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoA.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoA.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoA.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoB.pGPIOx							= GPIOC;
	segmentoB.pinConfig.GPIO_PinNumber			= PIN_4;
	segmentoB.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoB.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoB.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoB.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoC.pGPIOx							= GPIOC;
	segmentoC.pinConfig.GPIO_PinNumber			= PIN_2;
	segmentoC.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoC.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoC.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoC.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoD.pGPIOx							= GPIOB;
	segmentoD.pinConfig.GPIO_PinNumber			= PIN_0;
	segmentoD.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoD.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoD.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoD.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoE.pGPIOx							= GPIOC;
	segmentoE.pinConfig.GPIO_PinNumber			= PIN_1;
	segmentoE.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoE.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoE.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoE.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoF.pGPIOx							= GPIOB;
	segmentoF.pinConfig.GPIO_PinNumber			= PIN_10;
	segmentoF.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoF.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoF.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoF.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoG.pGPIOx							= GPIOC;
	segmentoG.pinConfig.GPIO_PinNumber			= PIN_0;
	segmentoG.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoG.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoG.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoG.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	segmentoPunto.pGPIOx							= GPIOC;
	segmentoPunto.pinConfig.GPIO_PinNumber			= PIN_3;
	segmentoPunto.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	segmentoPunto.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	segmentoPunto.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	segmentoPunto.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración de los pines */
	gpio_Config(&segmentoA);
	gpio_Config(&segmentoB);
	gpio_Config(&segmentoC);
	gpio_Config(&segmentoD);
	gpio_Config(&segmentoE);
	gpio_Config(&segmentoF);
	gpio_Config(&segmentoG);
	gpio_Config(&segmentoPunto);

	/* Empezamos con los leds apagados */
	gpio_WritePin(&segmentoA, RESET);
	gpio_WritePin(&segmentoB, RESET);
	gpio_WritePin(&segmentoC, RESET);
	gpio_WritePin(&segmentoD, RESET);
	gpio_WritePin(&segmentoE, RESET);
	gpio_WritePin(&segmentoF, RESET);
	gpio_WritePin(&segmentoG, RESET);
	gpio_WritePin(&segmentoPunto, RESET);


	/* ===== Configuramos los pines del Encoder ===== */

	/* Pin del Encoder Clock */
	encoderClk.pGPIOx							= GPIOC;
	encoderClk.pinConfig.GPIO_PinNumber			= PIN_8;
	encoderClk.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;

	/* Pin del Switch */
	sw.pGPIOx							= GPIOA;
	sw.pinConfig.GPIO_PinNumber			= PIN_0;
	sw.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;

	/* Pin de la salida de datos del encoder */
	data.pGPIOx							= GPIOB;
	data.pinConfig.GPIO_PinNumber		= PIN_2;
	data.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;

	/* Se cargan las configuraciones de los respectivos pines */
	gpio_Config(&encoderClk);
	gpio_Config(&sw);
	gpio_Config(&data);


	/* ===== Configuramos los leds de estado, modo y los pines para conmutar los cristales del 7-segmentos ====== */

	/* Configurando el pin del stateLed */
	stateLed.pGPIOx								= GPIOA;
	stateLed.pinConfig.GPIO_PinNumber			= PIN_5;
	stateLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	stateLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	stateLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	stateLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&stateLed);
	gpio_WritePin(&stateLed, SET); // El led de estado empieza encendido

	/* Configurando el pin del cristal2 (7-segmentos) */
	display2.pGPIOx								= GPIOA;
	display2.pinConfig.GPIO_PinNumber			= PIN_10;
	display2.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	display2.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	display2.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	display2.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&display2);
	gpio_WritePin(&display2, RESET); // Apagamos el Pin


	/* ===== Configuramos los pines RX y TX para la transmisión serial ===== */

	/* Configurando los pines para el puerto serial mediante USART2
	 *  - Usamos el PinA2 para TX (Se conecta al blanco)
	 */
	pinTx.pGPIOx								= GPIOA;
	pinTx.pinConfig.GPIO_PinNumber				= PIN_2;
	pinTx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinTx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinTx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinTx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinTx);


	/* - Usamos el PinA3 para RX (Se conecta al verde) */
	pinRx.pGPIOx								= GPIOA;
	pinRx.pinConfig.GPIO_PinNumber				= PIN_3;
	pinRx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinRx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinRx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinRx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinRx);


	/* ====== Configuramos las interrupciones externas (EXTI) ===== */

	/* Condigurando EXTI0 */
	exti0.pGPIOHandler				= &sw;
	exti0.edgeType					= EXTERNAL_INTERRUPT_RISING_EDGE;	/* Configurando el pin del cristal1 (7-segmentos) */

	/* Cargamos la configuración del EXTI */
	exti_Config(&exti0);

	/* Condigurando EXTI8 */
	exti8.pGPIOHandler				= &encoderClk;
	exti8.edgeType					= EXTERNAL_INTERRUPT_RISING_EDGE;

	/* Cargamos la configuración del EXTI */
	exti_Config(&exti8);


	/* ===== Configurando los TIMER ===== */

	/* Configurando el TIMER3 para el Blinky (Timer de 16 bits)*/
	blinkTimer3.pTIMx									= TIM3;
	blinkTimer3.TIMx_Config.TIMx_Prescaler				= 16000;	// Genera incrementos de 1 ms
	blinkTimer3.TIMx_Config.TIMx_Period					= 500;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (500 ms)
	blinkTimer3.TIMx_Config.TIMx_mode					= TIMER_UP_COUNTER;	// El Timer cuenta ascendente
	blinkTimer3.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Configurando el TIMER4 para mostrar un mensaje cada 2 segundos */
	messageTimer4.pTIMx									= TIM4;
	messageTimer4.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	messageTimer4.TIMx_Config.TIMx_Period				= 2000;		// Para un intervalo de 2s
	messageTimer4.TIMx_Config.TIMx_mode					= TIMER_UP_COUNTER;	// El Timer cuenta ascendente
	messageTimer4.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Configuración para el TIMER2 que muestra el display */
	/* Configurando el TIMER4 para mostrar un mensaje cada 2 segundos */
	displayTimer5.pTIMx									= TIM5;
	displayTimer5.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	displayTimer5.TIMx_Config.TIMx_Period				= 8;		// Para un intervalo de 2s
	displayTimer5.TIMx_Config.TIMx_mode					= TIMER_UP_COUNTER;	// El Timer cuenta ascendente
	displayTimer5.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos y encendemos los Timer */
	timer_Config(&blinkTimer3);
	timer_SetState(&blinkTimer3, TIMER_ON);

	timer_Config(&messageTimer4);
	timer_SetState(&messageTimer4, TIMER_ON);

	timer_Config(&displayTimer5);
	timer_SetState(&displayTimer5, TIMER_ON);


	/* ===== Configurando el puerto serial USART2 ===== */

	usart2.ptrUSARTx					= USART2;
	usart2.USART_Config.baudrate		= USART_BAUDRATE_115200;
	usart2.USART_Config.datasize		= USART_DATASIZE_8BIT;
	usart2.USART_Config.parity			= USART_PARITY_NONE;
	usart2.USART_Config.stopbits		= USART_STOPBIT_1;
	usart2.USART_Config.mode			= USART_MODE_RXTX;
	usart2.USART_Config.enableIntRX		= USART_RX_INTERRUP_ENABLE;

	/* Cargamos la configuración del USART */
	usart_Config(&usart2);

	/*
	 * Escribimos el caracter nulo para asegurarnos de empezar
	 * una transmisión "limpia"
	 */
	usart_WriteChar(&usart2, '\0');


	/* ===== Configuramos los canales del ADC ===== */

	// Configuramos el objeto Sensor que servirá para configurar cualquiera de los 3 potenciómetros
	sensor.channel				= SENSOR1_CH;
	sensor.resolution			= RESOLUTION_12_BIT;
	sensor.dataAlignment		= ALIGNMENT_RIGHT;
	sensor.samplingPeriod		= SAMPLING_PERIOD_84_CYCLES;
	sensor.interrupState		= ADC_INT_ENABLE;

	/* Cargamos la configuración del ADC */
	adc_ConfigSingleChannel(&sensor);


	/* ===== Configuramos el PWM ===== */
	/* - Usamos el  para el Canal del Timer del PWM */
	pinR.pGPIOx								= GPIOB;
	pinR.pinConfig.GPIO_PinNumber			= PIN_11;
	pinR.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinR.pinConfig.GPIO_PinAltFunMode		= AF1;
	pinR.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	pinR.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinR);

	/* - Usamos el  para el Canal del Timer del PWM */
	pinG.pGPIOx								= GPIOB;
	pinG.pinConfig.GPIO_PinNumber			= PIN_3;
	pinG.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinG.pinConfig.GPIO_PinAltFunMode		= AF1;
	pinG.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	pinG.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinG);

	/* - Usamos el  para el Canal del Timer del PWM */
	pinB.pGPIOx								= GPIOA;
	pinB.pinConfig.GPIO_PinNumber			= PIN_15;
	pinB.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinB.pinConfig.GPIO_PinAltFunMode		= AF1;
	pinB.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	pinB.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinB);


	/* Configurando el PWM */
	pwmHandler.ptrTIMx					= TIM2; 			// Timer3 usado para el PWM
	pwmHandler.config.channel			= CHANNEL_R;
	pwmHandler.config.prescaler			= 16000; 			// 1 ms
	pwmHandler.config.periodo			= NUMBER_12BITS;	// El periodo de la señal será el valor máximo de la reoslución del ADC
	pwmHandler.config.dutyCycle			= NUMBER_0BITS;		// Empezamos con la señal en bajo

	/* Cargar la configuración del PWM */
	pwm_Config(&pwmHandler);


	/* ===== Configuramos el SysTick ===== */
	systickHandler.pSystick							= SYSTICK;
	systickHandler.Systick_Config.Systick_IntState	= SYSTICK_INT_ENABLE;

	/* Cargamos la configuración del SysTick */
	systick_Config(&systickHandler);
	systick_SetState(&systickHandler, SYSTICK_ON);


} // Fin systemConfig()


/*
 * Función para controlar los segmentos en el modo Sensores
 */
void modeSensor(void){

	// Evaluamos cuál sensor está activo (seleccionado)
	switch(contadorSensor){
	case SENSOR1:{
		// Mostramos el número 1
		displayNumeros(SENSOR1);
		break;
	}
	case SENSOR2:{
		// Mostramos el número 2
		displayNumeros(SENSOR2);
		break;
	}
	case SENSOR3:{
		// Mostramos el número 3
		displayNumeros(SENSOR3);
		break;
	}
	default:{
		__NOP();
		break;
	}
	}

} // Fin función modeSensor()


/*
 * Función para configurar la resolución o el sensor, de acuerdo a la selección desde el Encoder o la transmisión serial
 */
void configSensor(ADC_Config_t sensorHandler, uint8_t canal){

	// Cambiamos la resolución del sensor introducido y cargamos la configuración
	sensorHandler.channel = canal;
	adc_ConfigSingleChannel(&sensorHandler);

} // Fin función configSensor()


// Función para encender los leds del 7-segmentos, para representar el numero_x
void displayNumeros(uint8_t numero_x){

	// Hacemos un switch para cada numero del 0-9
	switch(numero_x){
	case 0:{
		// El led se enciende cuando el voltaje dado por el pin sea bajo
		gpio_WritePin(&segmentoA, RESET);
		gpio_WritePin(&segmentoB, RESET);
		gpio_WritePin(&segmentoC, RESET);
		gpio_WritePin(&segmentoD, RESET);
		gpio_WritePin(&segmentoE, RESET);
		gpio_WritePin(&segmentoF, RESET);
		gpio_WritePin(&segmentoG, SET);
		break;
	}

	case 1:{
		gpio_WritePin(&segmentoA, SET);
		gpio_WritePin(&segmentoB, RESET);
		gpio_WritePin(&segmentoC, RESET);
		gpio_WritePin(&segmentoD, SET);
		gpio_WritePin(&segmentoE, SET);
		gpio_WritePin(&segmentoF, SET);
		gpio_WritePin(&segmentoG, SET);
		break;
	}

	case 2:{
		gpio_WritePin(&segmentoA, RESET);
		gpio_WritePin(&segmentoB, RESET);
		gpio_WritePin(&segmentoC, SET);
		gpio_WritePin(&segmentoD, RESET);
		gpio_WritePin(&segmentoE, RESET);
		gpio_WritePin(&segmentoF, SET);
		gpio_WritePin(&segmentoG, RESET);
		break;
	}

	case 3:{
		gpio_WritePin(&segmentoA, RESET);
		gpio_WritePin(&segmentoB, RESET);
		gpio_WritePin(&segmentoC, RESET);
		gpio_WritePin(&segmentoD, RESET);
		gpio_WritePin(&segmentoE, SET);
		gpio_WritePin(&segmentoF, SET);
		gpio_WritePin(&segmentoG, RESET);
		break;
	}

	case 4:{
		gpio_WritePin(&segmentoA, SET);
		gpio_WritePin(&segmentoB, RESET);
		gpio_WritePin(&segmentoC, RESET);
		gpio_WritePin(&segmentoD, SET);
		gpio_WritePin(&segmentoE, SET);
		gpio_WritePin(&segmentoF, RESET);
		gpio_WritePin(&segmentoG, RESET);
		break;
	}

	case 5:{
		gpio_WritePin(&segmentoA, RESET);
		gpio_WritePin(&segmentoB, SET);
		gpio_WritePin(&segmentoC, RESET);
		gpio_WritePin(&segmentoD, RESET);
		gpio_WritePin(&segmentoE, SET);
		gpio_WritePin(&segmentoF, RESET);
		gpio_WritePin(&segmentoG, RESET);
		break;
	}

	case 6:{
		gpio_WritePin(&segmentoA, RESET);
		gpio_WritePin(&segmentoB, SET);
		gpio_WritePin(&segmentoC, RESET);
		gpio_WritePin(&segmentoD, RESET);
		gpio_WritePin(&segmentoE, RESET);
		gpio_WritePin(&segmentoF, RESET);
		gpio_WritePin(&segmentoG, RESET);
		break;
	}

	case 7:{
		gpio_WritePin(&segmentoA, RESET);
		gpio_WritePin(&segmentoB, RESET);
		gpio_WritePin(&segmentoC, RESET);
		gpio_WritePin(&segmentoD, SET);
		gpio_WritePin(&segmentoE, SET);
		gpio_WritePin(&segmentoF, SET);
		gpio_WritePin(&segmentoG, SET);
		break;
	}

	case 8:{
		gpio_WritePin(&segmentoA, RESET);
		gpio_WritePin(&segmentoB, RESET);
		gpio_WritePin(&segmentoC, RESET);
		gpio_WritePin(&segmentoD, RESET);
		gpio_WritePin(&segmentoE, RESET);
		gpio_WritePin(&segmentoF, RESET);
		gpio_WritePin(&segmentoG, RESET);
		break;
	}

	case 9:{
		gpio_WritePin(&segmentoA, RESET);
		gpio_WritePin(&segmentoB, RESET);
		gpio_WritePin(&segmentoC, RESET);
		gpio_WritePin(&segmentoD, SET);
		gpio_WritePin(&segmentoE, SET);
		gpio_WritePin(&segmentoF, RESET);
		gpio_WritePin(&segmentoG, RESET);
		break;
	}

	default:{
		__NOP();
		break;
	}
	}

}


// Función para evaluar si se aumenta o disminuye el contador del Duty Cycle del PWM, para el caso del Encoder
void evaluateEncoder(void){
	if(readData == 1){
		// Giro en sentido horario -> Contador aumenta
		if(contadorPWM_Encoder == NUMBER_12BITS){
			contadorPWM_Encoder -= 10;
		}
		contadorPWM_Encoder += 10;
	}
	else{
		// Giro en sentido anti-horario -> Contador disminuye
		if(contadorPWM_Encoder == NUMBER_0BITS){
			contadorPWM_Encoder -= 10;
		}
		contadorPWM_Encoder += 10;
	}

} // Fin Función evaluateEncoder()


// Función para evaluar el aumento cuando presionamos el SW
void evaluateSW(void){

	if(flagMode == 0){
		if(contadorModo == 9){
			contadorModo = 0;
		}
		else{
			contadorModo++;
		}
	}
}


/* ===== Overwiter function ===== */

/* Función callback para la interrupción del EXTI0, es decir, la interrupción
 * externa debida al switch
 * */
void callback_ExtInt0(void){
	flagMode = 1;	// Aplicamos un XOR a la bandera para que cambie de valor binario

}


/* Función callback que atiende la interrupción del EXTI8, en este caso, corresponde
 * a la interrupcón externa del encoder (CW o CCW)
 * */
void callback_ExtInt8(void){
	flagData = 1;
	readData = gpio_ReadPin(&data);

}


/* Función que atiende la interrupción debida al TIMER3, es decir, que controla
 * al blinky (Led de estado)
 * */
void Timer3_Callback(void){
	gpio_TooglePin(&stateLed);
}


/* Esta función atiende la interrupción del TIMER4, la cual envía por transmisión
 * serial el último dato guardado en el dataBuffer
 */
void Timer4_Callback(void){
	sendMsg = 1;
}

void Timer5_Callback(void){
	flagDisplay = 1;
}


/* Función que atiende la interrupción debibo a la finalización de una
 * conversión ADC
 */
void adc_CompleteCallback(void){
	flagConv = 1;
	sensor.adcData = adc_GetValue();
}

/* Función assert para detectar problemas de paŕametros incorrectos */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
