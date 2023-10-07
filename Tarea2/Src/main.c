/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          :
 ******************************************************************************
 **/

/* Importando las librerías necesarias desde el PeripheralsDrivers */
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "exti_driver_hal.h"


/* Definición de una variable contadora global */
uint8_t contador = 0;

/* Definición de una variable global para guardar el valor de data */
uint8_t readData = 1;

/* Definición de una variable para leer la dirección */
uint8_t readModeLed = 1;

/* Definición de los pines a utilizar */

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

// Pines para controlar la activación-desactivación de los transistores para alternar
// entre los cristales del 7-segmentos
GPIO_Handler_t cristal1 = {0}; // Pin PA4
GPIO_Handler_t cristal2 = {0}; // Pin PA10



/* Definición de los Timers a utilizar para generar las interrupciones
 * del blinky y del 7 segmentos */
Timer_Handler_t blinkTimer2 = {0};	// Timer 2 para el stateLed
Timer_Handler_t sevenSegmentTimer3 = {0};	// Timer 3 para el 7-segmentos


/* Definición de los EXTI para las interrupciones externas del encoder */
EXTI_Config_t exti_0 = {0};	// Definimos el EXTI del Switch (estructura -> "objeto")
EXTI_Config_t exti_8 = {0};	// Definimos el EXTI del Encoder (estructura -> "objeto")


/* ===== Headers de las funciones a utilizar en el main ===== */

// Función para mostrar un número ingresado como parámetro, por medios de ambos display
void displayNumber(uint8_t numero);

// Función para encender los leds del 7-segmentos, para representar el numero_x
void numeros(uint8_t numero_x);

// Función para evaluar si se aumenta o disminuye el contador
void evaluate(void);


/* Función principal del programa */
int main(void)
{

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

	/* Cargamos la configuración de los pines */
	gpio_Config(&segmentoA);
	gpio_Config(&segmentoB);
	gpio_Config(&segmentoC);
	gpio_Config(&segmentoD);
	gpio_Config(&segmentoE);
	gpio_Config(&segmentoF);
	gpio_Config(&segmentoG);


	/* Configuramos los pines del encoder */

	/* Pin del Encoder Clock */
	encoderClk.pGPIOx							= GPIOC;
	encoderClk.pinConfig.GPIO_PinNumber			= PIN_8;
	encoderClk.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;

	/* Pin del Switch */
	sw.pGPIOx							= GPIOA;
	sw.pinConfig.GPIO_PinNumber			= PIN_0;
	sw.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;

	/* Pin del de la salida de datos del encoder */
	data.pGPIOx							= GPIOB;
	data.pinConfig.GPIO_PinNumber		= PIN_2;
	data.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;

	/* Se cargan las configuraciones de los respectivos pines */
	gpio_Config(&encoderClk);
	gpio_Config(&sw);
	gpio_Config(&data);

	/* ===== Configuramos de últimos los leds de estado, modo y los pines para conmutar los cristales del 7-segmentos ====== */

	/* Configuración del pin para el modeLed */
	modeLed.pGPIOx								= GPIOA;
	modeLed.pinConfig.GPIO_PinNumber			= PIN_7;
	modeLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	modeLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	modeLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	modeLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la confiugración del pin */
	gpio_Config(&modeLed);
	gpio_WritePin(&modeLed, SET); // Encendemos el Pin para indicar inicialmente el Modo Directo

	/* Configurando el pin del stateLed */
	stateLed.pGPIOx								= GPIOA;
	stateLed.pinConfig.GPIO_PinNumber			= PIN_5;
	stateLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	stateLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	stateLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	stateLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&stateLed);
	gpio_WritePin(&stateLed, SET); // Encendemos el Pin para indicar inicialmente el Modo Directo


	/* Configurando el pin del cristal1 (7-segmentos) */
	cristal1.pGPIOx								= GPIOA;
	cristal1.pinConfig.GPIO_PinNumber			= PIN_4;
	cristal1.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	cristal1.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	cristal1.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	cristal1.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&cristal1);
	gpio_WritePin(&cristal1, SET); // Encendemos el Pin

	/* Configurando el pin del cristal2 (7-segmentos) */
	cristal2.pGPIOx								= GPIOA;
	cristal2.pinConfig.GPIO_PinNumber			= PIN_10;
	cristal2.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	cristal2.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	cristal2.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	cristal2.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&cristal2);
	gpio_WritePin(&cristal2, RESET); // Apagamos el Pin


	/* ====== Configuramos las interrupciones externas (EXTI) ===== */
	/* Condigurando EXTI0 */
	exti_0.pGPIOHandler				= &sw;
	exti_0.edgeType					= EXTERNAL_INTERRUPT_FALLING_EDGE;	/* Configurando el pin del cristal1 (7-segmentos) */

	/* Cargamos la configuración del EXTI */
	exti_Config(&exti_0);

	/* Condigurando EXTI8 */
	exti_8.pGPIOHandler				= &encoderClk;
	exti_8.edgeType					= EXTERNAL_INTERRUPT_FALLING_EDGE;

	/* Cargamos la configuración del EXTI */
	exti_Config(&exti_8);


	/* ===== Configurando los TIMER ===== */
	/* Configurando el TIMER2 para el Blinky*/
	blinkTimer2.pTIMx								= TIM2;
	blinkTimer2.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	blinkTimer2.TIMx_Config.TIMx_Period				= 250;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (250 ms)
	blinkTimer2.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuenta ascendente
	blinkTimer2.TIMx_Config.TIMx_InterruptEnable	= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Configurando el TIMER3 para el 7-segmentos */
	sevenSegmentTimer3.pTIMx								= TIM3;
	sevenSegmentTimer3.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	sevenSegmentTimer3.TIMx_Config.TIMx_Period				= 8;		// Encendiendo y apagando el cristal cada 16ms, obtenemos aproximadamente 60 FPS
	sevenSegmentTimer3.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuenta ascendente
	sevenSegmentTimer3.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos y encendemos los Timer */
	timer_Config(&blinkTimer2);
	timer_SetState(&blinkTimer2, TIMER_ON);

	timer_Config(&sevenSegmentTimer3);
	timer_SetState(&sevenSegmentTimer3, TIMER_ON);




	/* Loop forever */
	while(1){

	}

}


/*
 * Overwiter function
 */


/* Función callback para la interrupción del EXTI0, es decir, la interrupción
 * externa debida al switch
 * */
void callback_ExtInt0(void){
	gpio_TooglePin(&modeLed);
}


/* Función callback que atiende la interrupción del EXTI8, en este caso, corresponde
 * a la interrupcón externa del encoder
 * */
void callback_ExtInt8(void){
	readData = gpio_ReadPin(&data);
	readModeLed = gpio_ReadPin(&modeLed);
	evaluate();

}


/* Función que atiende la interrupción debida TIMER2, es decir, que controla
 * al blinky (Led de estado)
 * */
void Timer2_Callback(void){
	gpio_TooglePin(&stateLed);
}


/* Esta función atiende la interrupción del TIMER3, la cual controla el activación
 * de los cristales del 7-segmentos a una frecuencia que simule que ambos están
 * encendidos continuamente
 * */
void Timer3_Callback(void){
	gpio_TooglePin(&cristal1);
	displayNumber(contador);
	gpio_TooglePin(&cristal2);
}


/* ===== Funciones auxiliares ===== */

// Función para encender los leds del 7-segmentos, para representar el numero_x
void numeros(uint8_t numero_x){

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
		gpio_WritePin(&segmentoB, SET);
		gpio_WritePin(&segmentoC, SET);
		gpio_WritePin(&segmentoD, SET);
		gpio_WritePin(&segmentoE, RESET);
		gpio_WritePin(&segmentoF, RESET);
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


// Función para mostrar un número ingresado como parámetro, por medios de ambos display
void displayNumber(uint8_t numero){

	/* Lo primero que debemos hacer, es diferenciar las decenas de las unidades dentro
	 * del número decimal. Para ello, podemos usar el operador módulo
	 */
	uint8_t unidades = numero%10;
	uint8_t decenas = (numero-unidades)/10;

	if( gpio_ReadPin(&cristal1) == 1){
		numeros(decenas);
	}
	else if(gpio_ReadPin(&cristal2) == 1){
		numeros(unidades);
	}

}


// Función para evaluar si se aumenta o disminuye el contador
void evaluate(void){

	// Modo directo
	if(readModeLed == 1){
		if(readData == 1){
			// Giro en sentido horario -> Contador aumenta
			if(contador < 99){
				contador++;
			}
			else{
				contador = 99;
			}
		}
		else{
			// Giro en sentido anti-horario -> Contador disminuye
			if(contador > 0){
				contador--;
			}
			else{
				contador = 0;
			}
		}
	}
	// Modo inverso
	else{
		if(readData == 1){
			// Giro en sentido horario -> Contador disminuye
			if(contador > 0){
				contador--;
			}
			else{
				contador = 0;
			}
		}
		else{
			// Giro en sentido anti-horario -> Contador aumenta
			if(contador < 99){
				contador++;
			}
			else{
				contador = 99;
			}
		}
	}

} // Fin Función


/* Función assert para detectar problemas de paŕametros incorrectos */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
