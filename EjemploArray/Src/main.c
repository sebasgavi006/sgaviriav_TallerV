/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Ejemplo de creación y uso de Arrays
 ******************************************************************************
 **/

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32_assert.h"


/*
 * Definimos una variable (clase) con los atributos
 */
struct infoAutomovil {
	uint8_t		numeroPuertas;
	float		maxVel;
	uint8_t		gearNumber;
	uint16_t	priceNew;
	uint16_t	priceSecondHand;
	uint8_t		color;
};

// Definimos un objeto del tipo infoAutomovil, el cual obtiene sus atributos
struct infoAutomovil info = {0};

// Definimos un objeto que al inicializarlo, le daremos la dirección del objeto info
struct infoAutomovil *pInfoAuto;



/* Función principal del programa */
int main(void)
{
	// Le damos un valor a uno de los atributos del objeto
	info.numeroPuertas = 10;

	// Le damos una dirección de memoria al puntero, para que apunte a la estructura info
	pInfoAuto = &info;


}


