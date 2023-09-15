/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria
 * @brief          : Main program body
 ******************************************************************************
 */

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32_assert.h"
#include "gpio_driver_hal.h"


// Header definition
int add(int x, int y);


/* ========================= PREGUNTA 1 ========================= */
/* ===== A continuación se muestra una COPIA del código mostrado en los videos (código con errores) =====

uint32_t gpio_ReadPin(GPIO_Handler_t *pPinHandler){
	uint32_t pinValue = 0;
	pinValue = (pPinHandler->pGPIOx->IDR << pPinHandler->pinConfig.GPIO_PinNumber);
	pinValue = pinValue;

	return pinValue;

}

  ========================= ERRORES DEL CÓDIGO =========================
	* El primer error está en la definición de la variable PinValue. Esta variable almacena el valor de
	* todo el registro IDR del puerto GPIOx correspondiente, y desplaza el valor completo del registro
	* "PinNumber" veces a la izquierda (shift left), por lo que no se obtiene un número binario sin sentido
	* para el propósito de la función. Finalmente se retorna ese valor, y por ende la lectura del PinX es
	* incorrecta.
	*
	* El segundo error, es que se escribe una línea redundante donde se le asigna a la variable pinValue su mismo valor.
	*
	* El tercer error es que no se hace uso en ningún momento de una máscara para obtener el valor del PinX que queremos leer.
	*
	* ADICIONAL: Para mejorar las práctias de programación, cambiamos el valor de la función gpio_ReadPin a una variable de
	* 8-bits, pues el valor que retorna la función es simplemente 0 ó 1 y no es prácticp almacenar este valor en un tipo de dato
	* de mayor tamaño.

  ========================= SOLUCIÓN DE ERRORES =========================
	* El primer error se soluciona cargando únicamente el registro IDR del puerto GPIOx en la variable pinValue.
	* Este número binario contiene el estado de todos los pines pertenecientes al puerto GPIOx.
	*
	* Posteriormente creamos una máscara que nos permite ubicar la posición del bit específico del PinX dentro del registro.
	* Luego se hace una operación AND entre la máscara y el valor guardado en la variable pinValue, lo que permite obtener un
	* número binario donde los bits de todas las posiciones son 0, y la posición que corresponde al pin específico PinX tendrá
	* el valor del estado de dicho pin, que estaba almacenado en el registro IDR.
	*
	* Finalmente, se hace un right shift para ubicar el valor leído del PinX en la primera posición del número binario y se almacena
	* nuevamente en la variable pinValue, y así poder retornar únicamente un número cuyo valor sea 0 ó 1, según el caso.

*/

// Definimos un Pin de pruebas
/*
 * En esta parte, creamos un "objeto" que obtiene los "atributos" de la
 * "clase" (realmente es una estructura aquí en C) GPIO_Handler_t.
 * A su vez, la estructura "hereda" los "atributos y métodos" de las clases
 * que conforman la estructura: GPIO_TypeDef y GPIO_PinConfig_t
 */
GPIO_Handler_t userLed = {0}; // PinA5 -> LD2 de la board del microcontrolador (Led verde)


/*
 * Iniciamos configurando los pines que se usarán como Salidas de propósito general
 * para poder encender y apagar los leds que irán conectados a ellos
 */

// Se definen e inicializan los pines que vamos a utilizar
// Se describen además los números de los pines físicos, según doc. STM32-nucleo64-boards
//							// (6.12 - Table 29. ST morpho connector on NUCLEO-F411RE)
GPIO_Handler_t PA7 = {0};	// bit-0 (CN10 - Pin 15)
GPIO_Handler_t PC8 = {0};	// bit-1 (CN10 - Pin 2)
GPIO_Handler_t PC7 = {0};	// bit-2 (CN10 - Pin 19)
GPIO_Handler_t PA6 = {0};	// bit-3 (CN10 - Pin 13)
GPIO_Handler_t PB8 = {0};	// bit-4 (CN10 - Pin 3)
GPIO_Handler_t PC6 = {0};	// bit-5 (CN10 - Pin 4)
GPIO_Handler_t PC9 = {0};	// bit-6 (CN10 - Pin 1)
GPIO_Handler_t PC13 = {0};	// USER_BUTTON (6.5 Push_buttons) (CN7 - Pin 23)



/* The main function, where everything happens */
int main(void)
{
	/* Configuramos el pin del Led LD2 */
	userLed.pGPIOx							= GPIOA;
	userLed.pinConfig.GPIO_PinNumber		= PIN_5;
	userLed.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	userLed.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	userLed.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	userLed.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración en los registros que gobiernan el puerto */
	gpio_Config(&userLed); // Esta es la función pública de configuración


	/* =============== CONTADOR DE SEGUNDOS BINARIO UP-DOWN ===============
	 *
	 * Configuramos los pines a sus respectivos puertos, números y modos de funcionamiento
	 * y cargamos dichas configuraciones en sus respectivos registros con la función
	 * pública gpio_Config()
	 */

	// Configurando el PA7 -> bit-0
	PA7.pGPIOx							= GPIOA;
	PA7.pinConfig.GPIO_PinNumber		= PIN_7;
	PA7.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	PA7.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	PA7.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	PA7.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&PA7);

	// Configurando el PC8 -> bit-1
	PC8.pGPIOx							= GPIOC;
	PC8.pinConfig.GPIO_PinNumber		= PIN_8;
	PC8.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	PC8.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	PC8.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	PC8.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&PC8);

	// Configurando el PC7 -> bit-2
	PC7.pGPIOx							= GPIOC;
	PC7.pinConfig.GPIO_PinNumber		= PIN_7;
	PC7.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	PC7.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	PC7.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	PC7.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&PC7);

	// Configurando el PA6 -> bit-3
	PA6.pGPIOx							= GPIOA;
	PA6.pinConfig.GPIO_PinNumber		= PIN_6;
	PA6.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	PA6.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	PA6.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	PA6.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&PA6);

	// Configurando el PB8 -> bit-4
	PB8.pGPIOx							= GPIOB;
	PB8.pinConfig.GPIO_PinNumber		= PIN_8;
	PB8.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	PB8.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	PB8.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	PB8.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&PB8);

	// Configurando el PC6 -> bit-5
	PC6.pGPIOx							= GPIOC;
	PC6.pinConfig.GPIO_PinNumber		= PIN_6;
	PC6.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	PC6.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	PC6.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	PC6.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&PC6);

	// Configurando el PC9 -> bit-6
	// Este led subutilizado, será usado para indicar el curso del reloj (avanzando = encendido o retrocediendo = apagado)
	PC9.pGPIOx							= GPIOC;
	PC9.pinConfig.GPIO_PinNumber		= PIN_9;
	PC9.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	PC9.pinConfig.GPIO_PinOutputType	= GPIO_OTYPE_PUSHPULL;
	PC9.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_MEDIUM;
	PC9.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&PC9);

	// Configurando el PC13 -> USER_BUTTON
	PC13.pGPIOx							= GPIOC;
	PC13.pinConfig.GPIO_PinNumber		= PIN_13;
	PC13.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	PC13.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_PULLUP;
	gpio_Config(&PC13);

	/* Ciclo for para Delay de 1 segundo
	 *
	 * Dado que la velcoidad de reloj del micricontrolador es de 8 MHz y 16 MHz (idealmente),
	 * podemos crear un ciclo for "vacío" que se repita 8-16 millones de veces y que al finalizar,
	 * aumenta un contador en uno (1). Este contador sería nuestro reloj.
	 */

	uint32_t signalClock = 1600000;	// Variable con la frecuencia de reloj del micro (aproximación)

	while(1){ //La función main queda activa indefinidamente (infinite loop) dentro del ciclo for y del ciclo while

		/* Definimos un ciclo for, donde cada repetición del ciclo cuenta un segundo y actualiza el estado
		 * de los leds para que representen el número de segundos que se van contando, de forma binaria
		 */
		for(uint8_t segundos = 1; segundos <= 60; segundos++){

			uint8_t userButton = gpio_ReadPin(&PC13); // Se lee el estado del USER_BUTTON

			for(uint32_t i = 0; i < signalClock; i++ ){		// Este ciclo permite obtener el delay de 1 segundo, que utilizaremos para tener una referencia de tiempo
			}
			gpio_TooglePin(&userLed);	// Usamos el led LD2 (led verde) del microcontrolador como contador de segundos, cambiando de estado cada segundo

			// Reloj avanzando -> USER_BOTTON NO está presionado
			if(userButton == 1){

				/* Se actualiza el estado de cada LED según el segundo que indique el reloj.
				 *
				 * La función clock_mask() devuelve el valor que debe tener cada led (1 = ON, 0 = OFF)
				 * de acuerdo a la representación binaria del segundo que se está contando actualmente
				 */

				gpio_WritePin(&PA7, clock_mask(segundos, bit_0));
				gpio_WritePin(&PC8, clock_mask(segundos, bit_1));
				gpio_WritePin(&PC7, clock_mask(segundos, bit_2));
				gpio_WritePin(&PA6, clock_mask(segundos, bit_3));
				gpio_WritePin(&PB8, clock_mask(segundos, bit_4));
				gpio_WritePin(&PC6, clock_mask(segundos, bit_5));
				gpio_WritePin(&PC9, SET);

			} // Fin if (Reloj avanzando)


			else if(userButton == 0){

				segundos = segundos - 2; // Esto nos permite ir hacia atrás en la cuenta de segundos (cuenta regresiva)

				gpio_WritePin(&PA7, clock_mask(segundos, bit_0));
				gpio_WritePin(&PC8, clock_mask(segundos, bit_1));
				gpio_WritePin(&PC7, clock_mask(segundos, bit_2));
				gpio_WritePin(&PA6, clock_mask(segundos, bit_3));
				gpio_WritePin(&PB8, clock_mask(segundos, bit_4));
				gpio_WritePin(&PC6, clock_mask(segundos, bit_5));
				gpio_WritePin(&PC9, RESET);

				if(segundos == 1){		// Nos aseguramos que la cuenta regresiva no se convierta en un número negativo
					segundos = 59 ;		// y actualiza el contador "segundos" para empezar nuevamente desde el segundo 60
				}

			} // Fin else (Reloj en cuenta regresiva)


		} // Fin for

	} // Fin While


} // Fin main


/*
 * Esta función sirve para detectar problemas de parámetros
 * incorrectos al momento de ejecutar un programa
 */

void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}

