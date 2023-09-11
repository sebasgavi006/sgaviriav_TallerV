
#include <stdint.h>

/*
 * Verificación del ciclo for
 */
#define SET		1
#define RESET	0

uint8_t flag3 = 0; //Variables globales del programa
uint8_t flag6 = 0;
uint8_t flag9 = 0;

enum{
	SUMA	= 0,
	RESTA,
	MULTIPLICACION,
	DIVISION,
	MODULO
};


void verificarFor(void); //Declaramos la función verificarFor

calculadora(uint16_t int1, uint8_t operacion, uint16_t int2);

variablel(uint8_t var);

Factorial(uint16_t var)



/* Función main del programa */
int main(void){

	verificarFor();

	uint16_t resSuma = calculadora(4, SUMA, 9);
	(void) resSuma;

	uint16_t var = variablel(25);
	(void) var;

	uint32_t factorial = Factorial(4);
	(void) factorial;


	while(1);	//Este ciclo onfinito ejecuta la función main indefinidamente
}


void verificarFor(void){
	uint8_t contador = 0;

	/*
	 * Aquí ubicamos flags que se "activan" cuando el contador alcance
	 * un valor específico (una especie de activación y desactivación).
	 */

	for(uint8_t i = 0; 1 < 10; i += 1){
		contador += 1;

		if(contador < 3){
			flag3 = 1;
		}
		else if(contador >= 3 && contador < 6){
			flag3 = 0;
			flag6 = 1;
		}
		else{
			flag6 = 0;
			flag9 = 1;
		}
	}
}

uint16_t calculadora(uint16_t int1, uint8_t operacion, uint16_t int2){
	uint16_t res = 0;

	/*
	 * A la operación Switch le pasamos una variable y evaluamos sobre ella
	 * diferentes casos (case) y realizar una opción cuando se alcance uno
	 * de los casos propuestos
	 */
	switch(operacion){
	case SUMA:
		res = int1 + int2;
		break;	//Usamos el break para terminar la evaluación del case

	case RESTA:
		res = int1 - int2;
		break;

	case MULTIPLICACION:
		res = int1 * int2;
		break;

	case DIVISION:
		res = int1 / int2;
		break;

	case MODULO:
		res = int1 % int2;
		break;
	}

	return res;
}

uint8_t variablel(uint8_t var){
	uint8_t contador = 0;

	/*
	 * Uso del ciclo While.
	 * Importante que la variable del contador sí pueda salir del ciclo
	 */
	while(contador < var){
		contador += 1;
	}
	return contador;
}


uint32_t Factorial(uint16_t var){
	uint16_t contador = 0;
	uint16_t suma = var;

	while(contador < var){
		contador += 1;
		suma += var - 1;
	}
	return suma;
}
