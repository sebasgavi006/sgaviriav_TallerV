
#include <stdio.h>
#include <stdint.h>

uint8_t variableGlobal = 0; /* Es una variable globlal, esta fuera de la funcion "main". Es global para el programa.
 	 	 	 	 	 	 	   Todas las funciones pueden acceder a esta variable */
uint8_t num1 = 2;
uint8_t num2 = 3;

/* Estos se conocen como Protocolos. Ponemos los headers de la funcion. Asi el compilador saber que las funciones existen*/
void sumaGlobal(void);
void sumaLocal(void);

// void SIGNIFICA QUE LA FUNCION NO RETORNA NADA!


int main(void)	// Se usa el "void" como parametro de la funcion main para evitar errores a la
{				// salida del programa, es decir, que la salida sea 0 (Return 0) (Buena practica de programacion)

	for(uint8_t i = 0; i < 10; i++){	// Definicion de un for desde 0 hasta 9, con paso de 1 en 1. (Se define la variable
		sumaGlobal();								// i dentro del mismo ciclo
	}

	sumaGlobal();	// Aqui llamamos las funciones a traves del main
	sumaLocal();

	return 0;
}

/* Funcion para sumar una variable global*/
void sumaGlobal(void) {
	variableGlobal += 1;
}

/* Funcion para sumar una variable local */
void sumaLocal(void) {
	uint8_t variableLocal = 0; // Esta es una variable local
	variableLocal += 1;
}

/* Funcion que SI retorna un valor */
uint8_t sumar(void){
	uint8_t res = num1 + num2;
	return res;
}
