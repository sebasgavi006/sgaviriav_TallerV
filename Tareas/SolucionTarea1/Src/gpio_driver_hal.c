/*
 * gpio_driver_hal.c
 *
 *  Created on: Sep 9, 2023
 *      Author: sgaviriav
 */

#include "gpio_driver_hal.h"
#include "stm32f4xx_hal.h"
#include "stm32_assert.h"

/* Headers for private functions */
void gpio_enable_clock_peripheral(GPIO_Handler_t *pGPIOHandler);
void gpio_config_mode(GPIO_Handler_t *pGPIOHandler);
void gpio_config_output_type(GPIO_Handler_t *pGPIOHandler);
void gpio_config_output_speed(GPIO_Handler_t *pGPIOHandler);
void gpio_config_pullup_pulldown(GPIO_Handler_t *pGPIOHandler);
void gpio_config_alternate_function(GPIO_Handler_t *pGPIOHandler);

/*
 * Para cualquier periférico, hay varios pasos que siempre se deben
 * seguir en un orden estricto para poder que el sistema permita configurar
 * el periférico X.
 * Lo primero y más importante es activar la señal de reloj principal hacia
 * ese elemento específico (relacionado con el periférico RCC, se activa el
 * periférico en dicho registro), a eso llamaremos simplemente "activar el
 * periférico o activar la señal de reloj del periférico".
 */

void gpio_Config (GPIO_Handler_t *pGPIOHandler){	// Función pública

	/* Verificamos que el pin seleccionado es correcto */
	assert_param(IS_GPIO_PIN(pGPIOHandler->pinConfig.GPIO_PinNumber));

	// 1) Activamos el periférico (Señal de reloj)
	gpio_enable_clock_peripheral(pGPIOHandler);

	//Después de activado, podemos comenzar a configurar.

	// 2) Configurando el registro GPIOx_MODER
	gpio_config_mode(pGPIOHandler);

	// 3) Configurando el registro GPIOx_OTYPER
	gpio_config_output_type(pGPIOHandler);

	// 4) Configurando el registro GPIOx_SPEED (velocidad de salida)
	gpio_config_output_speed(pGPIOHandler);

	// 5) Configurando el registro GPIOx_PUPDR o(pull-up, pull-dpwn o flotante)
	gpio_config_pullup_pulldown(pGPIOHandler);

	// 6) Configurando el registro GPIOx_ALTFN de función alternativa
	gpio_config_alternate_function(pGPIOHandler);

} // Fin del GPIO_config



/* Enable Clock Signal for specific GPIOx_Port */
void gpio_enable_clock_peripheral(GPIO_Handler_t *pGPIOHandler){	// Función privada

	// Verificamos que el puerto configurado si es permitido
	assert_param(IS_GPIO_ALL_INSTANCE(pGPIOHandler->pGPIOx));

	// Verificamos para GPIOA
		if(pGPIOHandler->pGPIOx == GPIOA){
			// Escribimos 1 (SET) en la posicion correspondiente al GPIOA
			RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIOAEN);
		}

		// Verificamos para GPIOB
		else if(pGPIOHandler->pGPIOx == GPIOB){
			// Escribimos 1 (SET) en la posicion correspondiente al GPIOB
			RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIOBEN);
		}

		// Verificamos para GPIOC
		else if(pGPIOHandler->pGPIOx == GPIOC){
			// Escribimos 1 (SET) en la posicion correspondiente al GPIOC
			RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIOCEN);
		}

		// Verificamos para GPIOD
		else if(pGPIOHandler->pGPIOx == GPIOD){
			// Escribimos 1 (SET) en la posicion correspondiente al GPIOD
			RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIODEN);
		}

		// Verificamos para GPIOE
		else if(pGPIOHandler->pGPIOx == GPIOE){
			// Escribimos 1 (SET) en la posicion correspondiente al GPIOE
			RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIOEEN);
		}

		// Verificamos para GPIOH
		else if(pGPIOHandler->pGPIOx == GPIOH){
			// Escribimos 1 (SET) en la posicion correspondiente al GPIOH
			RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIOHEN);
		}

} // Fin función pricada Config Enable Clock Peripheral


/*
 * Configures the mode in which the pin will work:
 * - Input
 * - Output
 * - Analog
 * - Alternate function
 */
void gpio_config_mode(GPIO_Handler_t *pGPIOHandler){	// Función privada

	uint32_t auxConfig = 0;

	/* Verificamos si el modo que se ha seleccionado es permitido */
	assert_param(IS_GPIO_MODE(pGPIOHandler->pinConfig.GPIO_PinMode));

	/*
	 * Acá estamos leyendo la config, moviendo "PinNumber" veces hacia la izquierda
	 * ese valor (shift left) y todo eso lo cargamos en la variable auxConfig
	 */
	auxConfig = (pGPIOHandler->pinConfig.GPIO_PinMode << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);

	/*
	 * Antes de cargar el nuevo valor, limpiamos los bits específicos de ese registro
	 * (debemos escribir 0b00), para lo cual aplicamos una máscara y una operación bitwise AND
	 */
	pGPIOHandler->pGPIOx->MODER &= ~(0b11 << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);

	/* Cargamos a auxConfig en el registro MODER */
	pGPIOHandler->pGPIOx->MODER |= auxConfig;

}

/*
 * Se parte de exactamente la misma lógica de la función anterior:
 * 1). Declara e inicializa una variable auxiliar
 * 2). Le carga a la variable auxConfig la configuración que deseamos en el pin
 * 	   (lo hace justo en la posición de memoria exacta para el pin)
 * 3). Limpia únicamente los bits del registro que queremos modificar, lo demás lo
 *     igual (usa dos bits porque habían 4 configuraciones posibles)
 * 4). Carga la configuración que queremos para el pin justo en los bits del registro que modifican dicha configuración
 */

/*
 * Configures which type of output the PinX will use:
 * - Push-Pull (Entrada-Salida)
 * - OpenDrain (Alta Impedancia)
 */
void gpio_config_output_type(GPIO_Handler_t *pGPIOHandler){	//Función privada

	uint32_t auxConfig = 0;

	/* Verificamos si el tipo de salida que se ha seleccionado se puede utilizar */
	assert_param(IS_GPIO_UOTPUT_TYPE(pGPIOHandler->pinConfig.GPIO_PinOutputType));

	/*
	 * Leemos la config, moviendo "PinNumber" veces hacia la izquierda
	 * (shift left) y todo eso lo cargamos en la variable auxConfig
	 */
	auxConfig = (pGPIOHandler->pinConfig.GPIO_PinOutputType << pGPIOHandler->pinConfig.GPIO_PinNumber);

	/*
	 * Limpiamos la congifuración del PIN, para lo cual aplicamos una máscara y una operación bitwise AND
	 */
	pGPIOHandler->pGPIOx->OTYPER &= ~(SET << pGPIOHandler->pinConfig.GPIO_PinNumber);	// SET = 0b1

	/* Cargamos el resultado sobre el registro OTYPER */
	pGPIOHandler->pGPIOx->OTYPER |= auxConfig;

}

/*
 * Selects between four different possible speeds for output PinX
 * - Low
 * - Medium
 * - Fast
 * - HighSpeed
 */
void gpio_config_output_speed(GPIO_Handler_t *pGPIOHandler){	//Función privada

	uint32_t auxConfig = 0;

	/* Verificamos si el tipo de salida que se ha seleccionado se puede utilizar */
	assert_param(IS_GPIO_SPEED(pGPIOHandler->pinConfig.GPIO_PinOutputSpeed));

	/*
	 * Leemos la config, moviendo "PinNumber" veces hacia la izquierda
	 * (shift left) y todo eso lo cargamos en la variable auxConfig
	 */
	auxConfig = (pGPIOHandler->pinConfig.GPIO_PinOutputSpeed << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);

	/*
	 * Limpiamos la congifuración del PIN, para lo cual aplicamos una máscara y una operación bitwise AND
	 */
	pGPIOHandler->pGPIOx->OSPEEDR &= ~(0b11 << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);

	/* Cargamos el resultado sobre el registro OSPEEDR */
	pGPIOHandler->pGPIOx->OSPEEDR |= auxConfig;

}

/*
 * Turns ON/OFF the pull-up pull-down resistor for each PinX in selected GPIO Port
 */
void gpio_config_pullup_pulldown(GPIO_Handler_t *pGPIOHandler){	//Función privada

	uint32_t auxConfig = 0;

	/* Verificamos si el tipo de salida que se ha seleccionado se puede utilizar */
	assert_param(IS_GPIO_PUPDR(pGPIOHandler->pinConfig.GPIO_PinPuPdControl));

	/*
	 * Leemos la config, moviendo "PinNumber" veces hacia la izquierda
	 * (shift left) y todo eso lo cargamos en la variable auxConfig
	 */
	auxConfig = (pGPIOHandler->pinConfig.GPIO_PinPuPdControl << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);

	/*
	 * Limpiamos la congifuración del PIN, para lo cual aplicamos una máscara y una operación bitwise AND
	 */
	pGPIOHandler->pGPIOx->PUPDR &= ~(0b11 << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);

	/* Cargamos el resultado sobre el registro OTYPER */
	pGPIOHandler->pGPIOx->PUPDR |= auxConfig;

}

/*
 * Allows to configure other functions (more specialized) on the selected PinX
 */
void gpio_config_alternate_function(GPIO_Handler_t *pGPIOHandler){	//Función privada

	uint32_t auxPosition = 0;

	if(pGPIOHandler->pinConfig.GPIO_PinMode == GPIO_MODE_ALTFN){

		// Seleccionamos primero si se puede utilizar el registro bajo (AFRL) o el alto (AFRH)
		if(pGPIOHandler->pinConfig.GPIO_PinNumber < 8){
			// Estamos en el registro AFRL, que controla los pines del PIN_0 al PIN_7
			auxPosition = 4 * (pGPIOHandler->pinConfig.GPIO_PinNumber);

			// Limpiamos primero la posición del registro que deseamos escribir a continuación
			pGPIOHandler->pGPIOx->AFR[0] &= ~(0b1111 << auxPosition);

			// Y escribimos el valor configurado en la posición seleccionada
			pGPIOHandler->pGPIOx->AFR[0] |= (pGPIOHandler->pinConfig.GPIO_PinAltFunMode << auxPosition);
		}
		else{
			// Estamos en el registro AFRH, que controla los pines del PIN_8 al PIN_15
			auxPosition = 4 * (pGPIOHandler->pinConfig.GPIO_PinNumber -8);

			// Limpiamos primero la posición del registro que deseamos escribir a continuación
			pGPIOHandler->pGPIOx->AFR[1] &= ~(0b1111 << auxPosition);

			// Y escribimos el valor configurado en la posición seleccionada
			pGPIOHandler->pGPIOx->AFR[1] |= (pGPIOHandler->pinConfig.GPIO_PinAltFunMode << auxPosition);
		}
	}
}


/*
 * Función utilizada para cambiar de estado el pin entregado en el handler,
 * asignando el valor entregado en la variable newState
 */
void gpio_WritePin(GPIO_Handler_t *pPinHandler, uint8_t newState){	// Función pública

	/* Verificamos si la acción que deseamos realizar es permitida */
	assert_param(IS_GPIO_PIN_ACTION(newState));

	/* Limpiams la posición que deseamos */
	if(newState == SET){
		//Trabajando con la parte baja del registro
		pPinHandler->pGPIOx->BSRR |= (SET << pPinHandler->pinConfig.GPIO_PinNumber);
	}
	else{
		//Trabajando con la parte alta del registro
		pPinHandler->pGPIOx->BSRR |= (SET << (pPinHandler->pinConfig.GPIO_PinNumber + 16));
	}

}


/*
 * Función para leer el estado de un pin específico
 */
uint32_t gpio_ReadPin(GPIO_Handler_t *pPinHandler){
	// Creamos unas variable auxiliares para la máscara y para devolver el valor leído del PinX
	uint32_t pinValue = 0;
	uint32_t mask = 0;

	/*
	 * Cargamos una variable con el valor del registro IDR del periférico GPIOx
	 */
	pinValue = pPinHandler->pGPIOx->IDR;

	/*
	 * - Creamos una máscara para obtener un binario que contiene únicamente el valor del pin que nos interesa (PinX).
	 * - Hacemos un AND bitwise entre el valor del registro IDR y la máscara para extraer el valor del PinX
	 */
	mask = SET << pPinHandler->pinConfig.GPIO_PinNumber;
	pinValue &= mask;

	/*
	 * Desplazamos el valor del bit del pin que acabamos obtener, "PinNumber" veces a la derecha para que quede en
	 * el bit 0, y lo asignamos a la variable pinValue
	 */

	pinValue >>= pPinHandler->pinConfig.GPIO_PinNumber;

	/* Devolvemos el valor leído del PinX */
	return (uint8_t)pinValue;	// Se hace un casting de la variable pinValue de 32-bits a 8 bits

}


/* =============== PREGUNTA 2 DE LA PARTE_B DE LA TAREA_1 ================
 *
 * Función para cambiar el estado de un PinX, al estado opuesto del actual
 */
void gpio_TooglePin(GPIO_Handler_t *pPinHandler){
	// Definimos una variable auxiliar para guardar el estado del PinX
	uint8_t auxVal = 0; // Se define de 8-bits para poder utilizarla como parámetro en la función gpio_WritePin, sin que hayan truncamientos

	/*
	 * Leemos el estado del PinX y aplicamos la operación booleana BANG, que devuelve el valor binario justamente
	 * contrario al estado actual (0b1 si el estado del PinX es 0b0, 0b0 si el estado del PinX es 0b1).
	 * Cargamos el estado "negado" en la variable auxVal
	 */
	auxVal = !gpio_ReadPin(pPinHandler);

	/* Se cambia el estado del PinX al estado opuesto */
	gpio_WritePin(pPinHandler, auxVal);

}


/* =============== CONTADOR DE SEGUNDOS BINARIO UP-DOWN ===============
 *
 * La función máscara permíte obtener el valor a escribir en cada bit del reloj,
 * de acuerdo a la representación binaria del segundo correspondiente
 */
uint8_t clock_mask(uint8_t segundos, uint8_t bit){		// La función pública recibe como parámetros el número del segundo que se está
														// contando actualmente, y la posición del bit que deseamos conocer en la representación
														// binaria de dicho segundo.

	uint8_t auxMask = 0; // Definimos una variable auxiliar para las máscaras

	/* La función switch permite seleccionar cuál bit de la representación binaria del
	 * segundo vamos a modificar, y que valor tendrá su estado, de acuerdo a dicha
	 * representación binaria
	 */
	switch(bit){
	case bit_0:
		auxMask = segundos & (SET << bit_0);
		auxMask = auxMask && auxMask;
		break;

	case bit_1:
		auxMask = segundos & (SET << bit_1);
		auxMask = auxMask && auxMask;
		break;

	case bit_2:
		auxMask = segundos & (SET << bit_2);
		auxMask = auxMask && auxMask;
		break;

	case bit_3:
		auxMask = segundos & (SET << bit_3);
		auxMask = auxMask && auxMask;
		break;

	case bit_4:
		auxMask = segundos & (SET << bit_4);
		auxMask = auxMask && auxMask;
		break;

	case bit_5:
		auxMask = segundos & (SET << bit_5);
		auxMask = auxMask && auxMask;
		break;

	case bit_6:
		auxMask = segundos & (SET << bit_6);
		auxMask = auxMask && auxMask;
		break;

	default:
		break;
	}

	return auxMask; // Devuelve la máscara según la posición de bit deseada
}
