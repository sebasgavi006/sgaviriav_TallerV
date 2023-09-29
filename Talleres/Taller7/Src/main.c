/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Yop
 * @brief          : Aquí experimento cosas
 ******************************************************************************
 */

#include <stdint.h>

int main(void)
{

	uint8_t x = 15;		// Se define una variable y se inicializa (se asigna la dirección de memoria y el valor almacenado)

	uint32_t x_memory = &x;	// Se guarda la dirección de la memoria de x en una variable

	uint8_t *puntero = &x;	// Se crea una variable tipo puntero y se le asigna el valor de la dirección de memoria de x

	/* Un puntero es una variable que almacena una dirección de memoria, pudiendo
	 * acceder a ella para leerla y modificarla. Un puntero también puede ser usado
	 * para recorrer diferentes direcciones en la memoria.
	 */

	while(1){

	}
	return 0;

}
