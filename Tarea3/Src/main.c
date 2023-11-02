/**
 ***************************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Este proyecto consiste en el uso de los periféricos: USART,
 * 					 TIMER, ADC, EXTI y GPIO. Contamos con 3 sensores (potenciómetros),
 * 					 los cuales podrán ser seleccionados uno a uno mediante el encoder,
 * 					 para funcionar como salidas de información analógica para la
 * 					 conversión mediante el módulo ADC del microcontrolador.
 * 					 Mediante comunicación serial por USART, podremos enviar comandos
 * 					 a través de teclado hacia el MPU para variar la Resolución del módulo
 * 					 ADC. Una función igual cumplirá el encoder. Al pulsar el encoder
 * 					 (función switch), cambiaremos a la opción de Variar la Resolución
 * 					 del ADC, y podremos aumentar o disminuir la Resolución entre las
 * 					 disponibles para el ADC. Así mismo, la otra función del encoder nos
 * 					 permitirá seleccionar cuál de los sensores estaremos leyendo para
 * 					 la conversión ADC. En general, para ambas funciones del Encoder,
 * 					 podremos seleccionar las diferentes opciones girando a favor o en
 * 					 contra de las manecillas del reloj (CW o CCW).
 * 					 Finalmente, podremos distinguir entre la función seleccionada
 * 					 mediante el doble display 7 segmentos. Cada display se encargará de
 * 					 mostrar la opción seleccionada dentro de una misma función, y podremos
 * 					 saber cuál es la función seleccionada gracias a la activación del
 * 					 segmento "punto" de cada display.
 *
 ***************************************************************************************
 **/

/* Importando las librerías necesarias desde el PeripheralsDrivers */
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "exti_driver_hal.h"
#include "usart_driver_hal.h"
#include "adc_driver_hal.h"

/* ===== Definición de variables ===== */
/* Definición de variables contadoras para cada modo
 * del Encoder (Resolución y Sensor) */
uint8_t contadorRes = 0;
uint8_t contadorSensor = SENSOR1;

/* Definicón de la variable para leer la entrada del Data del Encoder */

uint8_t readData = 0;

/* Definición de las banderas de los EXTI */
uint8_t flagMode = 0;	// Se asigna 0 para indicar que se inicializa en Modo Resolución

// Constantes para identificar los 3 sensores
enum
{
	SENSOR1	= 1,
	SENSOR2 = 2,
	SENSOR3 = 3
};

// Constantes para los modos del Encoder (Resolución y Sensor)
enum
{
	MODO_RESOLUCION = 0,
	MODO_SENSOR
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

// Pin para el led que indica el modo seleccionado por el switch
GPIO_Handler_t modeLed = {0}; // Pin PA7  (led de modo [Directo o Inverso])


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
GPIO_Handler_t display1 = {0}; // Pin PA4
GPIO_Handler_t display2 = {0}; // Pin PA10



/* Definición de los Timers a utilizar para generar las interrupciones
 * del blinky y del 7 segmentos */
Timer_Handler_t blinkTimer3 = {0};	// Timer 2 para el stateLed
Timer_Handler_t sevenSegmentTimer2 = {0};	// Timer 3 para el 7-segmentos


/* Definición de los EXTI para las interrupciones externas del encoder */
EXTI_Config_t exti0 = {0};	// Definimos el EXTI del Switch (estructura -> "objeto")
EXTI_Config_t exti8 = {0};	// Definimos el EXTI del Encoder (estructura -> "objeto")


/* ===== Headers de las funciones a utilizar en el main ===== */

// Función para cargar las configuraciones de los periféricos
void systemConfig(void);

// Función para mostrar un número ingresado como parámetro, por medios de ambos display
void displayMode(void);

// Función para representar un numero_x en el 7-segmentos
void displayNumeros(uint8_t numero_x);

// Función para controlar los segmentos en el modo Sensores
void modeSensor(void);

// Función para controlar los segmentos en el modo Resolución
void modeRes(void);

// Función para evaluar si se aumenta o disminuye el contador
void evaluate(void);




/*
 * ======== FUNCIÓN PRINCIPAL DEL PROGRAMA ========
 */
int main(void)
{

	// Cargamos la configuración de los periféricos
	systemConfig();



	/* Loop forever */
	while(1){

	}

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
	segmentoC.pinConfig.GPIO_PinNumber			= PIN_3;
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
	segmentoE.pinConfig.GPIO_PinNumber			= PIN_2;
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

	// Falta configurar el punto
//	segmentoG.pGPIOx							= GPIOC;
//	segmentoG.pinConfig.GPIO_PinNumber			= PIN_0;
//	segmentoG.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
//	segmentoG.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
//	segmentoG.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
//	segmentoG.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración de los pines */
	gpio_Config(&segmentoA);
	gpio_Config(&segmentoB);
	gpio_Config(&segmentoC);
	gpio_Config(&segmentoD);
	gpio_Config(&segmentoE);
	gpio_Config(&segmentoF);
	gpio_Config(&segmentoG);
//	gpio_Config(&segmentoPunto);




	/* ===== Configuramos los pines del encoder ===== */

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


	/* ===== Configuramos de últimos los leds de estado, modo y los pines para conmutar los cristales del 7-segmentos ====== */

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


	// Este ya no es necesario (Se usa el punto del 7-segmentos)
//	/* Configuración del pin para el modeLed */
//	modeLed.pGPIOx								= GPIOA;
//	modeLed.pinConfig.GPIO_PinNumber			= PIN_7;
//	modeLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
//	modeLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
//	modeLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
//	modeLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
//
//	/* Cargamos la confiugración del pin */
//	gpio_Config(&modeLed);
//	gpio_WritePin(&modeLed, SET); // Encendemos el Pin para indicar inicialmente el Modo Directo


	/* Configurando el pin del cristal1 (7-segmentos) */
	display1.pGPIOx								= GPIOA;
	display1.pinConfig.GPIO_PinNumber			= PIN_4;
	display1.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	display1.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	display1.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	display1.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&display1);
	gpio_WritePin(&display1, SET); // Encendemos el Pin

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
	blinkTimer3.pTIMx								= TIM3;
	blinkTimer3.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	blinkTimer3.TIMx_Config.TIMx_Period				= 250;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (250 ms)
	blinkTimer3.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuenta ascendente
	blinkTimer3.TIMx_Config.TIMx_InterruptEnable	= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Configurando el TIMER2 para el 7-segmentos (Timer de 32 bits) */
	sevenSegmentTimer2.pTIMx								= TIM2;
	sevenSegmentTimer2.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	sevenSegmentTimer2.TIMx_Config.TIMx_Period				= 8;		// Encendiendo y apagando el cristal cada 16ms, obtenemos aproximadamente 60 FPS
	sevenSegmentTimer2.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuenta ascendente
	sevenSegmentTimer2.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos y encendemos los Timer */
	timer_Config(&blinkTimer3);
	timer_SetState(&blinkTimer3, TIMER_ON);

	timer_Config(&sevenSegmentTimer2);
	timer_SetState(&sevenSegmentTimer2, TIMER_ON);



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
 * Función para controlar los segmentos en el modo Resolución
 */
void modeRes(void){

	// Evaluamos en qué resolución nos encontramos
}


/*
 * Función para mostrar el modo, según el display que esté encendido
 */
void displayMode(void){

	// Determinamos cuál display está encendido
	if(gpio_ReadPin(&display1) == ON){
		// Si es el display izquierdo (1), mostramos el modo Sensor
		modeRes();
	}
	else{
		// Si es el display derecho (2), mostramos el modo Resolución
		modeSensor();
	}

}


// Función para evaluar si se aumenta o disminuye el contador
void evaluate(void){

	// Modo Sensor
	if(flagMode == MODO_SENSOR){
		if(readData == 1){
			// Giro en sentido horario -> contadorSensor AUMENTA
			if(contadorSensor == SENSOR3){
				contadorSensor--;
			}
			contadorSensor++;
		}
		else{
			// Giro en sentido anti-horario -> contadorSensor DISMINUYE
			if(contadorSensor == SENSOR1){
				contadorSensor++;
			}
			contadorSensor--;
		}
	}
	// Modo Resolución
	else{
		if(readData == 1){
			// Giro en sentido horario -> contadorRes AUMENTA
			if(contadorRes == RESOLUTION_6_BIT){
				contadorRes--;
			}
			contadorRes++;
		}
		else{
			// Giro en sentido anti-horario -> contadorRes DISMINUYE
			if(contadorRes == RESOLUTION_12_BIT){
				contadorRes++;
			}
			contadorRes--;
		}
	}

} // Fin Función evaluate()


/*
 * Función para encender los leds del 7-segmentos, para representar el numero_x
 */
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
	default:{
		__NOP();
		break;
	}
	}

} // Fin función displayNumeros()


/*
 * Overwiter function
 */


/* Función callback para la interrupción del EXTI0, es decir, la interrupción
 * externa debida al switch
 * */
void callback_ExtInt0(void){
	flagMode ^= 1;	// Aplicamos un XOR a la bandera para que cambie de valor binario
//	gpio_TooglePin(&modeLed); Cambiar por el segmento del punto

}


/* Función callback que atiende la interrupción del EXTI8, en este caso, corresponde
 * a la interrupcón externa del encoder (CW o CCW)
 * */
void callback_ExtInt8(void){
	readData = gpio_ReadPin(&data);
	evaluate();

}


/* Función que atiende la interrupción debida al TIMER3, es decir, que controla
 * al blinky (Led de estado)
 * */
void Timer3_Callback(void){
	gpio_TooglePin(&stateLed);
}


/* Esta función atiende la interrupción del TIMER2, la cual controla el activación
 * de los cristales del 7-segmentos a una frecuencia que simule que ambos están
 * encendidos continuamente
 * */
void Timer2_Callback(void){
	gpio_TooglePin(&display1);
	displayMode();
	gpio_TooglePin(&display2);
}


/* Función assert para detectar problemas de paŕametros incorrectos */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
