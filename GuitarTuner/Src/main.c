/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Guitar Tuner.
 * 					 Proyecto Final: El afinador de guitarra es un dispositivo
 * 					 que permite, por medio de la frecuencia del sonido emitido
 * 					 por una guitarra acústica convencional, conocer si una
 * 					 determinada cuerda se encuentra afinada, y no ser así,
 * 					 guía al usuario a través de una interfaz gráfica hasta
 * 					 lograr el afinamiento de dicha cuerda. Así mismo, cuenta
 * 					 con un perifércio (Encoder) que le permite al usuario
 * 					 interactuar con la interfac (Pantalla OLED) durante todo
 * 					 el proceso.
 ******************************************************************************
 **/

// IMPORTACIÓN DE LIBRERÍAS NECESARIAS
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "arm_math.h"

#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "systick_driver_hal.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "exti_driver_hal.h"
#include "adc_driver_hal.h"
#include "usart_driver_hal.h"
#include "pwm_driver_hal.h"
#include "i2c_driver_hal.h"

#include "microphone_driver.h"
#include "oled_driver.h"

/* ===== CONSTANTES ===== */
#define	MCU_CLOCK_16_MHz	16000000
#define LENGTH				1

/* ===== LED DE ESTADO ===== */
GPIO_Handler_t userLed = {0}; //Led de estado PinA5
Timer_Handler_t blinkTimer = {0};


/* ===== SYSTICK ===== */
// Handler del SysTick
Systick_Handler_t systick = {0};


/* ===== COMUNICACIÓN SERIAL USART ===== */
USART_Handler_t commSerial = {0};
GPIO_Handler_t pinTx = {0};
GPIO_Handler_t pinRx = {0};
uint8_t usart2DataReceived = 0;
char	bufferMsg[64] = {0};


/* ===== MICRÓFONO ===== */

// Handlers del ADC
ADC_Config_t sensores[LENGTH] = {0}; // Array de sensores
ADC_Config_t sensor1 = {0};
uint16_t	count_ADC_Data = 0; // Contador de la cantidad de datos que lleva el arreglo ADC_Data[512]
uint8_t		flagStop = 0;	// Detiene las conversiones ADC cuando finalice las 512 conversiones

// PWM para generar la frecuencia de muestreo
PWM_Handler_t pwmHandler = {0};

// Código de los videos del classroom
/* Elementos para el procesamiento de una señal */
#define 	ADC_DataSize 1024	// Tamaño del arreglo de datos
uint16_t 	fftSize = ADC_DataSize;		// Tamaño del arreglo de los valores obtenidos de la transformada
float32_t 	frec_muestreo; //frecuencia de muestreo -> 3kHz
float32_t	factor_correccion;
float32_t	frec_corregida;
float32_t	frec_real;
float32_t	frec_real_magnitud;
float32_t 	frec_prom;
float32_t 	resolucion_FFT;

float32_t	fft_reales[ADC_DataSize/2];
float32_t 	fft_magnitud[ADC_DataSize/2];
float32_t 	transformedSignal[ADC_DataSize];

enum{
	MODO_MENU_INICIAL = 0,
	MODO_MENU_0,
	MODO_MENU_1,
	MODO_MENU_AUTOMATICO,
	MODO_MENU_AFINANDO
};

enum{
	MODO_AUTOMATICO = 1,
	MODO_MANUAL
};

enum{
	RESPUESTA_AUTO_SI = 1,
	RESPUESTA_AUTO_NO
};

/* Constantes para el manejo del rango de frecuencias
 * y la selección de la frecuencia adecuada
 */
uint8_t nota_cuerda;	// Bandera para indicar la cuerda que se está afinando
float32_t dif_frecuencias;
uint8_t flagAfinado = 0;
uint8_t flagNotaCuerda = 0;
uint8_t flagApretarClav = 0;
uint8_t flagAflojarClav = 0;

uint8_t flagModoActual;
uint8_t selecManual = 0;

/* Arreglo para guardar los datos obtenidos del ADC */
float32_t ADC_Data1[ADC_DataSize] = {0};	// Arreglo para guardar los datos para la FFT del sensor 1

/* Variables para instanciar e inicializar los funciones de la FFT
 * (Necesarias para el uso de las funciones FFT del CMSIS)
 */
uint32_t ifftFlag = 0;
arm_rfft_fast_instance_f32 config_Rfft_fast_f32;
arm_status statusInitFFT = ARM_MATH_ARGUMENT_ERROR;


/* ===== COMUNICACIÓN I2C PARA OLED ===== */
// Handlers necesarios para I2C
Timer_Handler_t blinkString = {0};
GPIO_Handler_t pinSCL_I2C = {0};
GPIO_Handler_t pinSDA_I2C = {0};
I2C_Handler_t i2c_handler = {0};

/* Banderas para la comunicación I2C */
uint8_t flagBlinkString = 0;
uint8_t flagMenuInicial = 1;
uint8_t flagMenu0 = 0;
uint8_t flagMenu1 = 0;
uint8_t flagMenu2 = 0;
uint8_t flagMenu = 0;
uint8_t flagLetterM = 0;
uint8_t opAnim = 0;
uint8_t flagAnim = 0;
uint8_t countAnim = 0;

/* Variables para definir el área de escritura de cada letra */
uint8_t E4_Col = 0;
uint8_t E4_Page = 0;
uint8_t B3_Col = 0;
uint8_t B3_Page = 0;
uint8_t G3_Col = 0;
uint8_t G3_Page = 0;
uint8_t D3_Col = 0;
uint8_t D3_Page = 0;
uint8_t A2_Col = 0;
uint8_t A2_Page = 0;
uint8_t E2_Col = 0;
uint8_t E2_Page = 0;

/* Array para figura mostrada durante el proceso de afinación */
uint8_t array_figura[104] = {0};

/* ===== ENCODER ===== */
// Handlers para los pines del encoder
GPIO_Handler_t encoderClk = {0}; // Pin PC8 (Canal 8 del EXTI)
GPIO_Handler_t data = {0};	// Pin PB2
GPIO_Handler_t sw = {0};	// Pin PA0 (Canal 0 del EXTI)

/* Definición de los EXTI para las interrupciones externas del encoder */
EXTI_Config_t exti0 = {0};	// Definimos el EXTI del Switch (estructura -> "objeto")
EXTI_Config_t exti8 = {0};	// Definimos el EXTI del Encoder (estructura -> "objeto")

/* Definicón de la variable para leer la entrada del Data del Encoder */
uint8_t readData = 0;

/* Definición de variables contadoras para el switch
 * y los desplazamientos del encoder
 */
uint8_t contadorSwitch = MODO_MENU_INICIAL;
uint8_t contadorMenu0 = MODO_AUTOMATICO;
uint8_t contadorMenu1 = E4;
uint8_t contadorMenu2 = RESPUESTA_AUTO_SI;

/* Definición de las banderas */
uint8_t flagMode = 0;		// Bandera del EXTI del Switch. Se asigna 0 para indicar que se inicializa en Modo Resolución
uint8_t flagData = 0;		// Bandera del EXTI del Encoder.



/* ===== HEADERS DE LAS FUNCIONES DEL MAIN ===== */
void configPeripherals(void);
void configParameters(void);
void procesamientoFFT(float32_t *array);
void seleccionRango(float32_t frecuencia);
void verificarFrecuencia(float32_t numero);
void seleccionModo(void);
void seleccionAutomatica(void);
void seleccionManual(void);
void evaluate(void);
void animacionApretar(void);
void animacionAflojar(void);
void mensajeAfinado(void);
void muestraNota(uint8_t nota_cuerda);


/*
 * Funcion principal del sistema
 */
int main(void){

	/* Activamos el FPU (Unidad de Punto Flotante) */
	SCB->CPACR |= (0xF << 20);

	/* Aseguramos las banderas */
	configParameters();

	/* Configuramos los periféricos */
	configPeripherals();

	sensores[0] = sensor1;

	/* ===== DEFINICIÓN DE VARIABLES ===== */
	frec_muestreo = MCU_CLOCK_16_MHz / (pwmHandler.config.periodo * pwmHandler.config.prescaler);	// Valor para la frecuencia de muestreo

	factor_correccion = (0.00101 * (frec_muestreo/1000)) + 0.0612; // Factor de corrección de la frecuecnia -> x/1000 Porque se obtuvo a partir de kHz

	frec_corregida = frec_muestreo * (1 - factor_correccion);	// Frecuencia corregida (Aproximación a la Real)

	resolucion_FFT = frec_corregida/ADC_DataSize;	// Resolución de la transformada -> Distancia entre cada intervalo de frecuencia


	/* Cargamos la configuración de los sensores en la función Multicanal del ADC */
	adc_ConfigMultiChannel(sensores, LENGTH);

	/* Configuramos el PWM que será la fuente del Trigger para empezar la conversión multicanal */
	adc_ExternalTrigger(&pwmHandler);

	/* Configuramos la pantalla OLED */
	oled_Config(&i2c_handler);

	/* Limpiamos la pantalla primero */
	oled_clearDisplay(&i2c_handler);

	/* Delay para tener tiempo de ver por la terminal */
	systick_Delay_ms(SYSTICK_3s);

	usart_WriteMsg(&commSerial, "-> Presione 't' para probar USART \r\n");
	usart_WriteMsg(&commSerial, "-> Presione '1' para iniciar el programa \n\r");

	/* Pintamos la interfaz del menú inicial */
	uint8_t bufferString[64] = {0};

	sprintf((char *)bufferString, "BIENVENIDO A");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 12, 28, 1);

	sprintf((char *)bufferString, "GUITAR TUNER");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 12, 28, 3);

	sprintf((char *)bufferString, "EMPEZAR");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 7, 43, 5);


	/* Loop forever*/
	while (1){

		/* Parpadeo de la opción seleccionada */
		if(flagMenuInicial && (contadorSwitch == MODO_MENU_INICIAL)){
			timer_SetState(&blinkString, TIMER_ON);
			uint8_t bufferString[64] = {0};
			if(flagBlinkString == 0){
				sprintf((char *)bufferString, "EMPEZAR");
				oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 7, 43, 5);
			}
			else if(flagBlinkString == 1){
				sprintf((char *)bufferString, "EMPEZAR");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 7, 43, 5);
			}
			flagMenuInicial ^= 1;
			flagBlinkString ^= 1;
		}


		/* Prueba del USART */
		if (usart2DataReceived == 't'){
			usart_WriteMsg(&commSerial, "¿Probando? ¡Funciona! Ouh-Yeah! \n\r");
			usart2DataReceived = '\0';
		}

		/* Iniciamos el programa */
		if ((usart2DataReceived == '1') || (contadorSwitch == MODO_MENU_0)){

			timer_SetState(&blinkString, TIMER_OFF);

			flagMenuInicial = 0;
			flagBlinkString = 0;
			opAnim = 0;
			flagMenu0 = 1;
			contadorMenu0 = 1;
			usart2DataReceived = '\0';

			/* Seleccion de modo por USART */
			seleccionModo();

		}

		/* Reinicia el modo actual */
		if(usart2DataReceived == '2'){
			if(flagModoActual == MODO_AUTOMATICO){
				// Ejecuta el modo de seleccion automatica hasta que esté afinada la cuerda
				contadorSwitch = MODO_MENU_AUTOMATICO;
				seleccionAutomatica();
				usart2DataReceived = '\0';
			}
			else if(flagModoActual == MODO_MANUAL){
				// Ejecuta el modo de afinamiento manual
				contadorSwitch = MODO_MENU_1;
				seleccionManual();
				usart2DataReceived = '\0';
			}
		}

		if (usart2DataReceived == 's'){

			stopPwmSignal(&pwmHandler);

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Pausando conversiones. \r\n");
			usart_WriteMsg(&commSerial, "Presione '1' para reiniciar la ejecución \n\r");
			usart2DataReceived = '\0';
		}

		/* Cuando esté afinada la cuerda, se detiene el programa hasta iniciar
		 * una nueva afinación
		 */
		if(flagAfinado){
			stopPwmSignal(&pwmHandler);
			flagAfinado = 0;
			flagNotaCuerda = 0;
			nota_cuerda = 0;
		}


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
 * Función para limpiar banderas; cargar valores de variables y parámetros
 */
void configParameters(void){

	/* Banderas correspondientes al manejo del micrófono */
	count_ADC_Data = 0;
	nota_cuerda = 0;
	flagStop = 0;
	selecManual = 0;

	flagAfinado = 0;
	flagNotaCuerda = 0;
	flagApretarClav = 0;
	flagAflojarClav = 0;

	/* Banderas para manejo de la OLED */
	flagMenuInicial = 1;
	flagMenu0 = 0;
	flagMenu1 = 0;
	flagMenu2 = 0;
	flagMenu = 0;
	flagLetterM = 0;

	opAnim = 0;
	flagBlinkString = 0;
	flagAnim = 0;
	countAnim = 0;


	/* Definimos los valores de las variables que controlan la muestra del Menu 1 en la OLED */
	E4_Col 	= 4;		// Cuerda 1
	E4_Page = 2;
	B3_Col 	= 58;		// Cuerda 2
	B3_Page = 3;
	G3_Col 	= 4;		// Cuerda 3
	G3_Page = 4;
	D3_Col 	= 58;		// Cuerda 4
	D3_Page = 5;
	A2_Col 	= 4;		// Cuerda 5
	A2_Page = 6;
	E2_Col 	= 58;		// Cuerda 6
	E2_Page = 7;

	/* Figura para mostrar durante la afinación */
	/**/array_figura[0]  = 0b00000000; /**/array_figura[51] = 0b00000000;  /**/array_figura[26] = 0b00000000;
	array_figura[79]  = 0b11111110; /**/array_figura[52] = 0b00000000; array_figura[27] = 0b00000000; array_figura[1] = 0b00000000;
	array_figura[80]  = 0b11111110; array_figura[53] = 0b00000000; array_figura[28] = 0b00000000; array_figura[2] = 0b00000000;
	array_figura[81]  = 0b11111110; array_figura[54] = 0b00000000; array_figura[29] = 0b00000000; array_figura[3] = 0b00000000;
	array_figura[82]  = 0b00000000; array_figura[55] = 0b00000000; array_figura[30] = 0b00000000; array_figura[4] = 0b00000000;
	array_figura[83]  = 0b11111111; array_figura[56] = 0b00000000; array_figura[31] = 0b00000000; array_figura[5] = 0b00000000;
	array_figura[84]  = 0b11111111; array_figura[57] = 0b11111110; array_figura[32] = 0b00000000; array_figura[6] = 0b00000000;
	array_figura[85]  = 0b11111111; array_figura[58] = 0b11111110; array_figura[33] = 0b00000000; array_figura[7] = 0b00000000;
	array_figura[86]  = 0b00000000; array_figura[59] = 0b11111110; array_figura[34] = 0b00000000; array_figura[8] = 0b00000000;
	array_figura[87]  = 0b11111111; array_figura[60] = 0b00000000; array_figura[35] = 0b11111111; array_figura[9] = 0b11111111;
	array_figura[88] = 0b11111111; array_figura[61] = 0b11111111; array_figura[36] = 0b11111111; array_figura[10] = 0b11111111;
	array_figura[89] = 0b11111111; array_figura[62] = 0b11111111; array_figura[37] = 0b11111111; array_figura[11] = 0b11111111;
	array_figura[90] = 0b00000000; array_figura[63] = 0b11111111; array_figura[38] = 0b00000000; array_figura[12] = 0b00000000;
	array_figura[91] = 0b11111111; array_figura[64] = 0b00000000; array_figura[39] = 0b11111110; array_figura[13] = 0b00000000;
	array_figura[92] = 0b11111111; array_figura[65] = 0b11111111; array_figura[40] = 0b11111110; array_figura[14] = 0b00000000;
	array_figura[93] = 0b11111111; array_figura[66] = 0b11111111; array_figura[41] = 0b11111110; array_figura[15] = 0b00000000;
	array_figura[94] = 0b00000000; array_figura[67] = 0b11111111; array_figura[42] = 0b00000000; array_figura[16] = 0b00000000;
	array_figura[95] = 0b11111111; array_figura[68] = 0b00000000; array_figura[43] = 0b11000000; array_figura[17] = 0b00000000;
	array_figura[96] = 0b11111111; array_figura[69] = 0b11111111; array_figura[44] = 0b11000000; array_figura[18] = 0b00000000;
	array_figura[97] = 0b11111111; array_figura[70] = 0b11111111; array_figura[45] = 0b11000000; array_figura[19] = 0b00000000;
	array_figura[98] = 0b00000000; array_figura[71] = 0b11111111; array_figura[46] = 0b00000000; array_figura[20] = 0b00000000;
	array_figura[99] = 0b11111111; array_figura[72] = 0b00000000; array_figura[47] = 0b00000000; array_figura[21] = 0b00000000;
	array_figura[100] = 0b11111111; array_figura[73] = 0b11100000; array_figura[48] = 0b00000000; array_figura[2] = 0b00000000;
	array_figura[101] = 0b11111111; array_figura[74] = 0b11100000; array_figura[49] = 0b00000000; array_figura[23] = 0b00000000;
	array_figura[102] = 0b00000000; array_figura[75] = 0b11100000; array_figura[50] = 0b00000000; array_figura[24] = 0b00000000;
	/**/array_figura[103] = 0b00000000; array_figura[76] = 0b00000000;							   /**/array_figura[25] = 0b00000000;
	/**/array_figura[78] = 0b00000000; /**/array_figura[77] = 0b00000000;


	/* Banderas y contadores del EXTI */
	flagData = 0;
	contadorSwitch = MODO_MENU_INICIAL;
	contadorMenu0 = MODO_AUTOMATICO;
	contadorMenu1 = E4;
	contadorMenu2 = RESPUESTA_AUTO_SI;

}


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


	/* Usamos el PinB8 para SCL */
	pinSCL_I2C.pGPIOx								= GPIOB;
	pinSCL_I2C.pinConfig.GPIO_PinNumber				= PIN_8;
	pinSCL_I2C.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinSCL_I2C.pinConfig.GPIO_PinAltFunMode			= AF4;
	pinSCL_I2C.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	pinSCL_I2C.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
	pinSCL_I2C.pinConfig.GPIO_PinOutputType			= GPIO_OTYPE_OPENDRAIN;

	/* Cargamos la configuración */
	gpio_Config(&pinSCL_I2C);


	/* Usamos el PinB9 para SDA */
	pinSDA_I2C.pGPIOx								= GPIOB;
	pinSDA_I2C.pinConfig.GPIO_PinNumber				= PIN_9;
	pinSDA_I2C.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinSDA_I2C.pinConfig.GPIO_PinAltFunMode			= AF4;
	pinSDA_I2C.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	pinSDA_I2C.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
	pinSDA_I2C.pinConfig.GPIO_PinOutputType			= GPIO_OTYPE_OPENDRAIN;

	/* Cargamos la configuración */
	gpio_Config(&pinSDA_I2C);


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

	/* Configurando el Timer para el parpadeo de la opción actual disponible para seleccionar en el menu*/
	blinkString.pTIMx								= TIM5;
	blinkString.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	blinkString.TIMx_Config.TIMx_Period				= 1000;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (500 ms)
	blinkString.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	blinkString.TIMx_Config.TIMx_InterruptEnable	= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos la configuración del Timer */
	timer_Config(&blinkString);

	/* Apagamos el Timer del parpadeo de opción de OLED */
	timer_SetState(&blinkString, TIMER_OFF);


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


	// 7. ===== ENCODER =====
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


	// 8. ===== I2C =====
	/* Configuramos el I2C */
	i2c_handler.ptrI2Cx				= I2C1;
	i2c_handler.slaveAddress		= OLED_ADDRESS;
	i2c_handler.modeI2C				= I2C_MODE_FM;


	/* Cargamos la configuración del I2C */
	i2c_Config(&i2c_handler);

	// 9. ===== SYSTICK =====
	/* Configuramos el Systick */
	systick.pSystick						= SYSTICK;
	systick.Systick_Config.Systick_Reload	= SYSTICK_PSC_1ms;
	systick.Systick_Config.Systick_IntState = SYSTICK_INT_ENABLE;

	/* Cargamos la configuración del systick */
	systick_Config(&systick);

	/* Encendemos el Systick */
	systick_SetState(&systick, SYSTICK_ON);


	// ===== EXTI's =====
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


}	// Fin de la configuración de los periféricos


/* Función para realizar el cálculo de la FFT para cada sensor */
void procesamientoFFT(float32_t *array){

	/* Obtenemos los valores Máximo y Mínimo de la magnitud de los complejos, y sus índices */
	float32_t   maxValue = 0;
	uint32_t 	maxIndex = 0;
	float32_t   maxValue_r = 0;
	uint32_t 	maxIndex_r = 0;

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

	frec_real = (maxIndex_r * resolucion_FFT);

//	usart_WriteMsg(&commSerial, "\r\n");
//	sprintf(bufferMsg, "Frecuencia (Con max): %.4f Hz\r\n", frec_real);
//	usart_WriteMsg(&commSerial, bufferMsg);


	/* Obtenemos el valor máximo de la magnitud de los complejos, para hallar la frecuencia dominante */
	arm_cmplx_mag_f32(transformedSignal, fft_magnitud, ADC_DataSize/2);

	maxValue = 0;
	maxIndex = 0;
	arm_max_f32(fft_magnitud, ADC_DataSize/2, &maxValue, &maxIndex);

	frec_real_magnitud = (maxIndex * resolucion_FFT);

//	usart_WriteMsg(&commSerial, "\r\n");
//	sprintf(bufferMsg, "Frecuencia (Con magnitud): %.4f Hz\r\n", frec_real_magnitud);
//	usart_WriteMsg(&commSerial, bufferMsg);

	/* Obtenemos el promedio de las dos transformadas obtenidas */
	float32_t result_fft[2] = {frec_real_magnitud, frec_real_magnitud};

	arm_mean_f32(result_fft, 2, &frec_prom);

	frec_prom = frec_real_magnitud;

//	usart_WriteMsg(&commSerial, "\r\n");
//	sprintf(bufferMsg, "Frecuencia (Promedio): %.4f Hz\r\n", frec_prom);
//	usart_WriteMsg(&commSerial, bufferMsg);

	/* Limpiamos el arreglo original */
	for(uint16_t i = 0; i < (ADC_DataSize-1); i++){
		array[i] = 0;
	}

	usart2DataReceived = '\0';

} // Fin FFT


/*
 * Función para determinar cual cuerda se desea afinar, que detecta de manera
 * automática con base a un rango de frecuencias
 */
void seleccionRango(float32_t frecuencia){

	if(LIM_INFERIOR_E2 < frecuencia && frecuencia <= LIM_SUPERIOR_E2){
		nota_cuerda = E2;	// Levantamos una bandera que permite determinar la cuerda específica
		usart_WriteMsg(&commSerial, "Afinando la cuerda N°6 (E2) \r\n");
	}
	else if(LIM_INFERIOR_A2 < frecuencia && frecuencia <= LIM_SUPERIOR_A2){
		nota_cuerda = A2;
		usart_WriteMsg(&commSerial, "Afinando la cuerda N°5 (A2) \r\n");
	}
	else if(LIM_INFERIOR_D3 < frecuencia && frecuencia <= LIM_SUPERIOR_D3){
		nota_cuerda = D3;
		usart_WriteMsg(&commSerial, "Afinando la cuerda N°4 (D3) \r\n");
	}
	else if(LIM_INFERIOR_G3 < frecuencia && frecuencia <= LIM_SUPERIOR_G3){
		nota_cuerda = G3;
		usart_WriteMsg(&commSerial, "Afinando la cuerda N°3 (G3) \r\n");
	}
	else if(LIM_INFERIOR_B3 < frecuencia && frecuencia <= LIM_SUPERIOR_B3){
		nota_cuerda = B3;
		usart_WriteMsg(&commSerial, "Afinando la cuerda N°2 (B3) \r\n");
	}
	else if(LIM_INFERIOR_E4 < frecuencia && frecuencia < LIM_SUPERIOR_E4){
		nota_cuerda = E4;
		usart_WriteMsg(&commSerial, "Afinando la cuerda N°1 (E4) \r\n");
	}
	else{
		nota_cuerda = 0;
	}

} // Fin seleccionRango()



/*
 * Función para verificar si un número es negativo
 */
void verificarFrecuencia(float32_t numero){
	if(numero < -(resolucion_FFT)){
		if(!flagApretarClav){
			usart_WriteMsg(&commSerial, "Aprieta la clavija \n\r");

			/* Mensaje inicial */
			uint8_t bufferString[64] = {0};
			sprintf((char *)bufferString, "APRIETE");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 7, 16, 5);
			sprintf((char *)bufferString, "LA CLAVIJA");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 10, 16, 6);
			flagAnim = 1;
			opAnim = 1;
			countAnim = 0;

			/* Encendemos el Timer */
			timer_SetState(&blinkString, TIMER_ON);

			flagApretarClav = 1;
			flagAflojarClav = 0;
		}

	}
	else if(numero > (resolucion_FFT)){
		if(!flagAflojarClav){
			usart_WriteMsg(&commSerial, "Afloja la clavija \n\r");

			/* Mensaje inicial */
			uint8_t bufferString[64] = {0};
			sprintf((char *)bufferString, "AFLOJE");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 6, 16, 5);
			sprintf((char *)bufferString, "LA CLAVIJA");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 10, 16, 6);
			flagAnim = 1;
			opAnim = 1;
			countAnim = 0;

			/* Encendemos el Timer */
			timer_SetState(&blinkString, TIMER_ON);

			flagAflojarClav = 1;
			flagApretarClav = 0;
		}
	}
	else{
		// Levantamos la bandera para indicar el fin del afinador
		flagAfinado = 1;
		flagAflojarClav = 0;
		flagApretarClav = 0;

		usart_WriteMsg(&commSerial, "¡La cuerda esta afinada! \n\r");

		/* Detenemos las animaciones */
		timer_SetState(&blinkString, TIMER_OFF);

		/* Imprimimos en la OLED el mensaje de finalización */
		mensajeAfinado();
	}
} // Fin función verificarNegativo()


/*
 * Función para que el afinador detecta la cuerda que se desea afinar de forma automática.
 * Esta función también realiza el proceso de afinación.
 */
void seleccionAutomatica(void){

	usart_WriteMsg(&commSerial, "Modo automático seleccionado \n\r");

	oled_clearDisplay(&i2c_handler);
	uint8_t bufferString[64] = {0};

	timer_SetState(&blinkString, TIMER_ON);
	contadorSwitch = MODO_MENU_AUTOMATICO;

	// While de verificación hasta que el sistema detecta la cuerda adecuada
	while(!flagNotaCuerda){

		/* Pintamos la interfaz de las instrucciones */
		oled_clearDisplay(&i2c_handler);
		sprintf((char *)bufferString, "TOQUE LA CUERDA");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 15, 19, 3);

		/* Mandamos las instrucciones por USART */
		usart_WriteMsg(&commSerial, "Por favor, toque la cuerda \n\r");

		// Espera 1 segundo para que el usuario toque la cuerda
		systick_Delay_ms(SYSTICK_2s);

		startPwmSignal(&pwmHandler);

		// Detecta la finalización de una conversión ADC
		while(!flagStop){
			__NOP();
		}
		flagStop = 0;

		// Se procesan los datos de la conversión ADC
		procesamientoFFT(ADC_Data1);
		frec_prom = frec_prom;

		// Se verifica el rango y se asigna un valor a nota_cuerda según el caso
		if(!nota_cuerda){
			seleccionRango(frec_prom);
		}

		/* Mostramos por pantalla OLED cuál nota fue detectada */
		muestraNota(nota_cuerda);

		usart2DataReceived = '\0';
		contadorSwitch = MODO_MENU_AUTOMATICO;

		usart_WriteMsg(&commSerial, "¿La cuerda seleccionada es correcta? \r\n");
		usart_WriteMsg(&commSerial, "Oprima 'y' o 'n' \n\r");

		// Esperamos a recibir una respuesta del usuario
		while(!usart2DataReceived && (contadorSwitch == MODO_MENU_AUTOMATICO)){

			/* Evalúa las interrupciones del ENCODER */
			evaluate();

			/* Animación de parpadeo de las opciones */
			if(flagMenu2 && (contadorMenu2 == RESPUESTA_AUTO_SI)){
				sprintf((char *)bufferString, "NO");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 2, 76, 5);

				if(flagBlinkString == 0){
					sprintf((char *)bufferString, "SI");
					oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 2, 40, 5);
				}
				else if(flagBlinkString == 1){
					sprintf((char *)bufferString, "SI");
					oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 2, 40, 5);
				}
				flagMenu2 ^= 1;
				flagBlinkString ^= 1;
			}

			if(flagMenu2 && (contadorMenu2 == RESPUESTA_AUTO_NO)){
				sprintf((char *)bufferString, "SI");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 2, 40, 5);

				if(flagBlinkString == 0){
					sprintf((char *)bufferString, "NO");
					oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 2, 76, 5);
				}
				else if(flagBlinkString == 1){
					sprintf((char *)bufferString, "NO");
					oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 2, 76, 5);;
				}
				flagMenu2 ^= 1;
				flagBlinkString ^= 1;
			}
		}

		if(usart2DataReceived == 'y' || ((contadorSwitch == MODO_MENU_AFINANDO) && contadorMenu2 == RESPUESTA_AUTO_SI)){
			flagNotaCuerda = 1;
		}
		else if(usart2DataReceived == 'n' || ((contadorSwitch == MODO_MENU_AFINANDO) && contadorMenu2 == RESPUESTA_AUTO_NO)){
			flagNotaCuerda = 0;
			nota_cuerda = 0;
		}

		usart2DataReceived = '\0';

	} // Fin While

	flagMenu2 = 0;
	flagBlinkString = 0;

	/* Limpiamos la OLED */
	oled_clearDisplay(&i2c_handler);

	// Pasamos a la afinación de la cuerda seleccionada
	while(!flagAfinado){

		// Se calcula la diferencia de frecuencia según la cuerda que se está afinando
		switch(nota_cuerda){
		case E2: {
			dif_frecuencias = FREC_E2 - frec_prom; // Obtenemos la diferencia de frecuencias
			break;
		}
		case A2: {
			dif_frecuencias = FREC_A2 - frec_prom;
			break;
		}
		case D3: {
			dif_frecuencias = FREC_D3 - frec_prom;
			break;
		}
		case G3: {
			dif_frecuencias = FREC_G3 - frec_prom;
			break;
		}
		case B3: {
			dif_frecuencias = FREC_B3 - frec_prom;
			break;
		}
		case E4: {
			dif_frecuencias = FREC_E4 - frec_prom;
			break;
		}
		default:{
			__NOP();
			break;
		}
		}

		// Verifica si la frecuencia actual está por encima o por debajo de la frecuencia a afinar
		verificarFrecuencia(dif_frecuencias);

		/* Corremos la animación correspondiente */
		if(flagApretarClav && flagAnim){
			/* Cargamos la animación para indicar al usuario que debe apretar la clavija */
			animacionApretar();
		}
		else if(flagAflojarClav && flagAnim){
			/* Cargamos la animación para indicar al usuario que debe aflojar la clavija */
			animacionAflojar();
		}


	//	systick_Delay_ms(1000);
		if(!flagAfinado){
			startPwmSignal(&pwmHandler);
			// Detecta la finalización de una conversión ADC
			while(!flagStop){
				__NOP();
			}
			flagStop = 0;

			procesamientoFFT(ADC_Data1);
			frec_prom = frec_prom;
		}

		// Pausa de emergencia
		if (usart2DataReceived == 's'){

			stopPwmSignal(&pwmHandler);

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Pausando conversiones. \r\n");
			usart_WriteMsg(&commSerial, "Presione '1' para reiniciar la ejecución \n\r");
			usart2DataReceived = '\0';
		}
	}

	/* Limpiamos las banderas */
	flagAfinado = 0;
	flagNotaCuerda = 0;
	dif_frecuencias = 0;
	nota_cuerda = 0;

	flagAnim = 0;
	opAnim = 0;
	countAnim = 0;

	flagModoActual = MODO_AUTOMATICO;

	timer_SetState(&blinkString, TIMER_OFF);

	usart2DataReceived = '\0';

	usart_WriteMsg(&commSerial, "-> Presione '1' para cambiar de modo \r\n");
	usart_WriteMsg(&commSerial, "-> Presione '2' para reiniciar la afinación en el modo actual \n\r");

	systick_Delay_ms(SYSTICK_2s);

} // Fin seleccionAutomatica()


/*
 * Función para seleccionar de forma Manual cuál cuerda se desea afinar
 */
void seleccionManual(void){

	contadorSwitch = MODO_MENU_1;
	contadorMenu1 = E4;

	/* Limpiamos la pantalla primero */
	oled_clearDisplay(&i2c_handler);

	/* Pintamos la interfaz del menú principal 1 */
	uint8_t bufferString[64] = {0};

	sprintf((char *)bufferString, "SELECCIONE LA CUERDA");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 20, 4, 0);

	sprintf((char *)bufferString, "CUERDA-1 E4");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, E4_Col, E4_Page);

	sprintf((char *)bufferString, "B3 CUERDA-2");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, B3_Col, B3_Page);

	sprintf((char *)bufferString, "CUERDA-3 G3");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, G3_Col, G3_Page);

	sprintf((char *)bufferString, "D3 CUERDA-4");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, D3_Col, D3_Page);

	sprintf((char *)bufferString, "CUERDA-5 A2");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, A2_Col, A2_Page);

	sprintf((char *)bufferString, "E2 CUERDA-6");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, E2_Col, E2_Page);

	/* Imprimimos las instrucciones por comunicación serial */
	usart_WriteMsg(&commSerial, "Modo manual seleccionado \n\r");
	usart_WriteMsg(&commSerial, "Seleccione la cuerda a afinar \r\n");
	usart_WriteMsg(&commSerial, "'1' -> Cuerda 1 (E4) \r\n");
	usart_WriteMsg(&commSerial, "'2' -> Cuerda 2 (B3) \r\n");
	usart_WriteMsg(&commSerial, "'3' -> Cuerda 3 (G3) \r\n");
	usart_WriteMsg(&commSerial, "'4' -> Cuerda 4 (D3) \r\n");
	usart_WriteMsg(&commSerial, "'5' -> Cuerda 5 (A2) \r\n");
	usart_WriteMsg(&commSerial, "'6' -> Cuerda 6 (E2) \r\n");

	usart2DataReceived = '\0';
	contadorSwitch = MODO_MENU_1;
	contadorMenu1 = E4;
	flagBlinkString = 0;
	flagMenu1 = 0;

	timer_SetState(&blinkString, TIMER_ON);

	/* Esperamos que el usuario seleccione una opción */
	while(!usart2DataReceived && (contadorSwitch == MODO_MENU_1)){

		/* Evalúa las interrupciones del ENCODER */
		evaluate();

		switch(contadorMenu1){
		case E4:{
			/* Parpadeo de la opción seleccionada */
			if(flagMenu1){
				sprintf((char *)bufferString, "B3 CUERDA-2");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, B3_Col, B3_Page);

				if(flagBlinkString == 0){
					sprintf((char *)bufferString, "CUERDA-1 E4");
					oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 11, E4_Col, E4_Page);
				}
				else if(flagBlinkString == 1){
					sprintf((char *)bufferString, "CUERDA-1 E4");
					oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, E4_Col, E4_Page);
				}
				flagMenu1 ^= 1;
				flagBlinkString ^= 1;
			}
			break;
		}

		case B3:{
			/* Parpadeo de la opción seleccionada */
			if(flagMenu1){
				sprintf((char *)bufferString, "CUERDA-1 E4");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, E4_Col, E4_Page);
				sprintf((char *)bufferString, "CUERDA-3 G3");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, G3_Col, G3_Page);

				if(flagBlinkString == 0){
					sprintf((char *)bufferString, "B3 CUERDA-2");
					oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 11, B3_Col, B3_Page);
				}
				else if(flagBlinkString == 1){
					sprintf((char *)bufferString, "B3 CUERDA-2");
					oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, B3_Col, B3_Page);
				}
				flagMenu1 ^= 1;
				flagBlinkString ^= 1;
			}
			break;
		}

		case G3: {
			/* Parpadeo de la opción seleccionada */
			if(flagMenu1){
				sprintf((char *)bufferString, "B3 CUERDA-2");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, B3_Col, B3_Page);
				sprintf((char *)bufferString, "D3 CUERDA-4");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, D3_Col, D3_Page);

				if(flagBlinkString == 0){
					sprintf((char *)bufferString, "CUERDA-3 G3");
					oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 11, G3_Col, G3_Page);
				}
				else if(flagBlinkString == 1){
					sprintf((char *)bufferString, "CUERDA-3 G3");
					oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, G3_Col, G3_Page);
				}
				flagMenu1 ^= 1;
				flagBlinkString ^= 1;
			}
			break;
		}

		case D3: {
			/* Parpadeo de la opción seleccionada */
			if(flagMenu1){
				sprintf((char *)bufferString, "CUERDA-3 G3");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, G3_Col, G3_Page);
				sprintf((char *)bufferString, "CUERDA-5 A2");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, A2_Col, A2_Page);

				if(flagBlinkString == 0){
					sprintf((char *)bufferString, "D3 CUERDA-4");
					oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 11, D3_Col, D3_Page);
				}
				else if(flagBlinkString == 1){
					sprintf((char *)bufferString, "D3 CUERDA-4");
					oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, D3_Col, D3_Page);
				}
				flagMenu1 ^= 1;
				flagBlinkString ^= 1;
			}
			break;
		}

		case A2: {
			/* Parpadeo de la opción seleccionada */
			if(flagMenu1){
				sprintf((char *)bufferString, "D3 CUERDA-4");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, D3_Col, D3_Page);
				sprintf((char *)bufferString, "E2 CUERDA-6");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, E2_Col, E2_Page);

				if(flagBlinkString == 0){
					sprintf((char *)bufferString, "CUERDA-5 A2");
					oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 11, A2_Col, A2_Page);
				}
				else if(flagBlinkString == 1){
					sprintf((char *)bufferString, "CUERDA-5 A2");
					oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, A2_Col, A2_Page);
				}
				flagMenu1 ^= 1;
				flagBlinkString ^= 1;
			}
			break;
		}

		case E2: {
			/* Parpadeo de la opción seleccionada */
			if(flagMenu1){
				sprintf((char *)bufferString, "CUERDA-5 A2");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, A2_Col, A2_Page);

				if(flagBlinkString == 0){
					sprintf((char *)bufferString, "E2 CUERDA-6");
					oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 11, E2_Col, E2_Page);
				}
				else if(flagBlinkString == 1){
					sprintf((char *)bufferString, "E2 CUERDA-6");
					oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 11, E2_Col, E2_Page);
				}
				flagMenu1 ^= 1;
				flagBlinkString ^= 1;
			}
			break;
		}

		} // Switch-case
	} // While

	timer_SetState(&blinkString, TIMER_OFF);

	if(usart2DataReceived){
		selecManual = usart2DataReceived;
		switch(selecManual){
		case '1': {
			nota_cuerda = E4;
			break;
		}
		case '2': {
			nota_cuerda = B3;
			break;
		}
		case '3': {
			nota_cuerda = G3;
			break;
		}
		case '4': {
			nota_cuerda = D3;
			break;
		}
		case '5': {
			nota_cuerda = A2;
			break;
		}
		case '6': {
			nota_cuerda = E2;
			break;
		}
		}
	}
	else if(contadorSwitch == MODO_MENU_AUTOMATICO){
		contadorSwitch = MODO_MENU_AFINANDO;
		selecManual = contadorMenu1;
		switch(selecManual){
		case 1: {
			nota_cuerda = E4;
			break;
		}
		case 2: {
			nota_cuerda = B3;
			break;
		}
		case 3: {
			nota_cuerda = G3;
			break;
		}
		case 4: {
			nota_cuerda = D3;
			break;
		}
		case 5: {
			nota_cuerda = A2;
			break;
		}
		case 6: {
			nota_cuerda = E2;
			break;
		}
		}
	}

	usart2DataReceived = '\0';

	/* Limpiamos la pantalla primero */
	oled_clearDisplay(&i2c_handler);

	/* Pintamos la interfaz de las instrucciones */
	sprintf((char *)bufferString, "TOQUE LA CUERDA");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 15, 19, 3);

	/* Enviamos las instrucciones por USART */
	usart_WriteMsg(&commSerial, "Por favor, toque la cuerda \n\r");

	systick_Delay_ms(SYSTICK_2s);

	muestraNota(nota_cuerda);

	startPwmSignal(&pwmHandler);
	// Detecta la finalización de una conversión ADC
	while(!flagStop){
		__NOP();
	}
	flagStop = 0;

	// Se procesan los datos obtenidos por el micrófono
	procesamientoFFT(ADC_Data1);
	frec_prom = frec_prom;

	flagAfinado = 0;

	// Pasamos a la afinación de la cuerda seleccionada
	while(!flagAfinado){

		// Se calcula la diferencia de frecuencia según la cuerda que se está afinando
		switch(nota_cuerda){
		case E2: {
			dif_frecuencias = FREC_E2 - frec_prom; // Obtenemos la diferencia de frecuencias
			break;
		}
		case A2: {
			dif_frecuencias = FREC_A2 - frec_prom;
			break;
		}
		case D3: {
			dif_frecuencias = FREC_D3 - frec_prom;
			break;
		}
		case G3: {
			dif_frecuencias = FREC_G3 - frec_prom;
			break;
		}
		case B3: {
			dif_frecuencias = FREC_B3 - frec_prom;
			break;
		}
		case E4: {
			dif_frecuencias = FREC_E4 - frec_prom;
			break;
		}
		default:{
			__NOP();
			break;
		}
		}

		// Verifica si la frecuencia actual está por encima o por debajo de la frecuencia a afinar
		verificarFrecuencia(dif_frecuencias);

		/* Limpiamos la variable para evitar errores */
		dif_frecuencias = 0;

		if(!flagAfinado){

			/* Corremos la animación correspondiente */
			if(flagApretarClav && flagAnim){
				/* Cargamos la animación para indicar al usuario que debe apretar la clavija */
				animacionApretar();
			}
			else if(flagAflojarClav && flagAnim){
				/* Cargamos la animación para indicar al usuario que debe aflojar la clavija */
				animacionAflojar();
			}

			startPwmSignal(&pwmHandler);
			// Detecta la finalización de una conversión ADC
			while(!flagStop){
				__NOP();
			}
			flagStop = 0;

			procesamientoFFT(ADC_Data1);
			frec_prom = frec_prom;
		}

		// Pausa de emergencia
		if (usart2DataReceived == 's'){

			stopPwmSignal(&pwmHandler);

			usart_WriteMsg(&commSerial, "\r\n");
			usart_WriteMsg(&commSerial, "Pausando conversiones. \r\n");
			usart_WriteMsg(&commSerial, "Presione '1' para reiniciar la ejecución \n\r");
			usart2DataReceived = '\0';
		}
	}

	/* Limpiamos las banderas implicadas en el proceso */
	flagAfinado = 0;
	dif_frecuencias = 0;
	nota_cuerda = 0;

	flagAnim = 0;
	opAnim = 0;
	countAnim = 0;

	flagModoActual = MODO_MANUAL;

	usart2DataReceived = '\0';

	usart_WriteMsg(&commSerial, "-> Presione '1' para cambiar de modo \r\n");
	usart_WriteMsg(&commSerial, "-> Presione '2' para reiniciar la afinación en el modo actual \n\r");

	systick_Delay_ms(SYSTICK_2s);
}

/*
 * Función para seleccionar entre el modo Automático o Manual del Guitar Tuner
 */
void seleccionModo(void){

	/* Limpiamos la pantalla */
	oled_clearDisplay(&i2c_handler);

	/* Pintamos la interfaz del menú principal 1 */
	uint8_t bufferString[64] = {0};

	sprintf((char *)bufferString, "SELECCIONE UN MODO");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 18, 10, 1);

	sprintf((char *)bufferString, "AUTOMATICO");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 10, 34, 4);

	sprintf((char *)bufferString, "MANUAL");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 6, 46, 6);

	timer_SetState(&blinkString, TIMER_ON);

	/* Escribimos en la terminal serial */
	usart2DataReceived = '\0';

	usart_WriteMsg(&commSerial, "Bienvenido a su afinador de guitarra de confianza \n\r");
	usart_WriteMsg(&commSerial, "Presiona A -> Seleccion Automatico \r\n");
	usart_WriteMsg(&commSerial, "Presiona M -> Seleccion Manual \n\r");

	contadorSwitch = MODO_MENU_0;
	contadorMenu0 = MODO_AUTOMATICO;

	while(!usart2DataReceived && (contadorSwitch == MODO_MENU_0)){

		/* Parpadeo de la opción seleccionada */
		if(flagMenu0 && (contadorSwitch == MODO_MENU_0) && (contadorMenu0 == MODO_AUTOMATICO)){
			uint8_t bufferString[64] = {0};

			sprintf((char *)bufferString, "MANUAL");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 6, 46, 6);

			if(flagBlinkString == 0){
				sprintf((char *)bufferString, "AUTOMATICO");
				oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 10, 34, 4);
			}
			else if(flagBlinkString == 1){
				sprintf((char *)bufferString, "AUTOMATICO");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 10, 34, 4);
			}
			flagMenu0 ^= 1;
			flagBlinkString ^= 1;
		}

		if(flagMenu0 && (contadorSwitch == MODO_MENU_0) && (contadorMenu0 == MODO_MANUAL)){
			uint8_t bufferString[64] = {0};

			sprintf((char *)bufferString, "AUTOMATICO");
			oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 10, 34, 4);

			if(flagBlinkString == 0){
				sprintf((char *)bufferString, "MANUAL");
				oled_setString(&i2c_handler, bufferString, INVERSE_DISPLAY, 6, 46, 6);
			}
			else if(flagBlinkString == 1){
				sprintf((char *)bufferString, "MANUAL");
				oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 6, 46, 6);
			}
			flagMenu0 ^= 1;
			flagBlinkString ^= 1;
		}

		evaluate();
	}

	timer_SetState(&blinkString, TIMER_OFF);

	if(usart2DataReceived == 'A' || ((contadorMenu0 == MODO_AUTOMATICO) && (contadorSwitch == MODO_MENU_1))){
		flagModoActual = MODO_AUTOMATICO;
		// Ejecuta el modo de seleccion automatica hasta que esté afinada la cuerda
		seleccionAutomatica();
		usart2DataReceived = '\0';
	}
	else if(usart2DataReceived == 'M' || ((contadorMenu0 == MODO_MANUAL) && (contadorSwitch == MODO_MENU_1))){
		flagModoActual = MODO_MANUAL;
		// Ejectua el modo de seleccion manual, donde elegimos cuál cuerda se desea afinar
		seleccionManual();
		usart2DataReceived = '\0';
	}
	else{
		usart_WriteMsg(&commSerial, "Por favor, seleccione un modo de funcionamiento válido \n\r");
	}

}

// Animación para indicar que hay que apretar la clavija
void animacionApretar(void){

	/* Configuramos la zona donde deseamos pintar la animación */
	uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 6};
	oled_sendCommand(&i2c_handler, array, 6);

	/* Matriz de 0b00000000 */
	uint8_t array_zeros[78] = {0};

	switch(countAnim){
	case 0:{
		oled_sendData(&i2c_handler, array_figura, 104);
		uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 5};
		oled_sendCommand(&i2c_handler, array, 6);
		oled_sendData(&i2c_handler, array_zeros, 78);
		break;
	}
	case 1:{
		oled_sendData(&i2c_handler, array_figura, 104);

		uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 4};
		oled_sendCommand(&i2c_handler, array, 6);
		oled_sendData(&i2c_handler, array_zeros, 52);
		break;
	}
	case 2:{
		oled_sendData(&i2c_handler, array_figura, 104);

		uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 3};
		oled_sendCommand(&i2c_handler, array, 6);
		oled_sendData(&i2c_handler, array_zeros, 26);
		break;
	}
	case 3: {
		oled_sendData(&i2c_handler, array_figura, 104);
		break;
	}
	}

	flagAnim ^= 1;
}

// Animación para indicar que hay que aflojar la clavija
void animacionAflojar(void){

	/* Configuramos el área para la animación */
	uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 6};
	oled_sendCommand(&i2c_handler, array, 6);

	/* Matriz de 0b00000000 */
	uint8_t array_zeros[78] = {0};

	switch(countAnim){
	case 3:{
		oled_sendData(&i2c_handler, array_figura, 104);
		uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 5};
		oled_sendCommand(&i2c_handler, array, 6);
		oled_sendData(&i2c_handler, array_zeros, 78);
		break;
	}
	case 2:{
		oled_sendData(&i2c_handler, array_figura, 104);

		uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 4};
		oled_sendCommand(&i2c_handler, array, 6);
		oled_sendData(&i2c_handler, array_zeros, 52);
		break;
	}
	case 1:{
		oled_sendData(&i2c_handler, array_figura, 104);

		uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 3};
		oled_sendCommand(&i2c_handler, array, 6);
		oled_sendData(&i2c_handler, array_zeros, 26);
		break;
	}
	case 0: {
		oled_sendData(&i2c_handler, array_figura, 104);
		break;
	}
	}

	flagAnim ^= 1;
}


// Mensaje de que terminó la afinación
void mensajeAfinado(void){

	/* Limpiamos la OLED */
	oled_clearDisplay(&i2c_handler);

	/* Mensaje inicial */
	uint8_t bufferString[64] = {0};
	sprintf((char *)bufferString, "CUERDA");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 6, 16, 5);
	sprintf((char *)bufferString, "AFINADA");
	oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 7, 16, 6);
	flagAnim = 0;
	opAnim = 0;
	countAnim = 0;

	/* Cargamos la configuración de la zona donde queremos hacer la figura */
	uint8_t array[6] = 	{0x21, 84, 109, 0x22, 3, 6};
	oled_sendCommand(&i2c_handler, array, 6);

	/* Pintamos la figura */
	oled_sendData(&i2c_handler, array_figura, 104);
}


// Muestra cuál nota estamos afinando actualmente
void muestraNota(uint8_t nota_cuerda){

	/* Limpiamos la pantalla primero */
	oled_clearDisplay(&i2c_handler);

	/* Identificamos la cuerda que se está afinando, para mostrar el mensaje en la OLED */
	uint8_t bufferString[64] = {0};
	switch(nota_cuerda){
	case E4: {
		sprintf((char *)bufferString, "AFINANDO CUERDA-1");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 17, 13, 0);

		sprintf((char *)bufferString, "NOTA: E4");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 8, 16, 2);
		break;
	}
	case B3: {
		sprintf((char *)bufferString, "AFINANDO CUERDA-2");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 17, 13, 0);

		sprintf((char *)bufferString, "NOTA: B3");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 8, 16, 2);
		break;
	}
	case G3: {
		sprintf((char *)bufferString, "AFINANDO CUERDA-3");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 17, 13, 0);

		sprintf((char *)bufferString, "NOTA: G3");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 8, 16, 2);
		break;
	}
	case D3: {
		sprintf((char *)bufferString, "AFINANDO CUERDA-4");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 17, 13, 0);

		sprintf((char *)bufferString, "NOTA: D3");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 8, 16, 2);
		break;
	}
	case A2: {
		sprintf((char *)bufferString, "AFINANDO CUERDA-5");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 17, 13, 0);

		sprintf((char *)bufferString, "NOTA: A2");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 8, 16, 2);
		break;
	}
	case E2: {
		sprintf((char *)bufferString, "AFINANDO CUERDA-6");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 17, 13, 0);

		sprintf((char *)bufferString, "NOTA: E2");
		oled_setString(&i2c_handler, bufferString, NORMAL_DISPLAY, 8, 16, 2);
		break;
	}
	}
}

// Función para evaluar si se aumenta o disminuye el contador
void evaluate(void){

	// Menu para seleccionar afinación manual o automática
	if((contadorSwitch == MODO_MENU_0) && flagData){

		// Bajamos la bandera por interrupción del ENCODER
		flagData = 0;
		if(readData == 1){
			// Giro en sentido horario -> contadorSensor AUMENTA
			if(contadorMenu0 == MODO_MANUAL){
				contadorMenu0--;
			}
			contadorMenu0++;
		}
		else{
			// Giro en sentido anti-horario -> contadorSensor DISMINUYE
			if(contadorMenu0 == MODO_AUTOMATICO){
				contadorMenu0++;
			}
			contadorMenu0--;
		}
	}

	// Menu para seleccionar la cuerda a afinar
	else if((contadorSwitch == MODO_MENU_1) && flagData){

		// Bajamos la bandera por interrupción del ENCODER
		flagData = 0;
		if(readData == 1){
			// Giro en sentido horario -> contadorRes AUMENTA
			if(contadorMenu1 == E2){
				contadorMenu1--;
			}
			contadorMenu1++;
		}
		else{
			// Giro en sentido anti-horario -> contadorRes DISMINUYE
			if(contadorMenu1 == E4){
				contadorMenu1++;
			}
			contadorMenu1--;
		}
	}

	// Menu para interactuar con el afinador en modo automático
	if((contadorSwitch == MODO_MENU_AUTOMATICO) && flagData){

		// Bajamos la bandera por interrupción del ENCODER
		flagData = 0;
		if(readData == 1){
			// Giro en sentido horario -> contadorSensor AUMENTA
			if(contadorMenu2 == RESPUESTA_AUTO_NO){
				contadorMenu2--;
			}
			contadorMenu2++;
		}
		else{
			// Giro en sentido anti-horario -> contadorSensor DISMINUYE
			if(contadorMenu2 == RESPUESTA_AUTO_SI){
				contadorMenu2++;
			}
			contadorMenu2--;
		}
	}

} // Fin Función evaluate()


/* ===== CALLBACKS ===== */

/* Callback de Timer 2 (Controla el userLed) */
void Timer2_Callback(void){
	gpio_TooglePin(&userLed);
}


/*
 * Callback de Timer 5 (Controla la visualización de la opción actual en la OLED)
 */
void Timer5_Callback(void){

	if(contadorSwitch == MODO_MENU_INICIAL){
		flagMenuInicial ^= 1;
	}

	if(contadorSwitch == MODO_MENU_0){
		flagMenu0 ^= 1;
	}

	if(contadorSwitch == MODO_MENU_1){
		flagMenu1 ^= 1;
	}

	if(contadorSwitch == MODO_MENU_AUTOMATICO){
		flagMenu2 ^= 1;
	}

	/* Contador para la animación */
	if(opAnim){
		flagAnim ^= 1;
		if(flagAnim){
			countAnim++;
			if(countAnim >= 4){
				countAnim = 0;
			}
		}
	}

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


/* Función callback para la interrupción del EXTI0, es decir, la interrupción
 * externa debida al switch
 * */
void callback_ExtInt0(void){
	flagMode = 1;	// Aplicamos un XOR a la bandera para que cambie de valor binario
	contadorSwitch++;

	if(contadorSwitch > MODO_MENU_AFINANDO){
		contadorSwitch = MODO_MENU_0;
	}
}


/* Función callback que atiende la interrupción del EXTI8, en este caso, corresponde
 * a la interrupcón externa del encoder (CW o CCW)
 * */
void callback_ExtInt8(void){
	flagData = 1;
	readData = gpio_ReadPin(&data);
}

/*
 * Callback del USART 2 debido a recepción
 */
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
