/*
 * i2c_driver_hal.c
 *
 *  Created on: Oct 31, 2023
 *      Author: sgaviriav
 */

#include <stdint.h>
#include "i2c_driver_hal.h"

/*
 * Recordar que se debe coonfigurar los pines parael I2C (SDA y SCL),
 * para lo cual se necesita el módulo GPIO y los pines configurados
 * en el modo Alternate Function.
 * Admás, estos pines deben ser configurados como salidas open-drain
 * y con la resistencia en modo pull-up
 */

/*
 * Función para carfar la configuración del I2C
 */
void i2c_Config(I2C_Handler_t *ptrHandlerI2C){

	/* 1. Activamos la señal de reloj para el módulo I2C seleccionado */
	if(ptrHandlerI2C->ptrI2Cx == I2C1){
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	}
	else if(ptrHandlerI2C->ptrI2Cx == I2C2){
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
	}
	else if(ptrHandlerI2C->ptrI2Cx == I2C3){
		RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
	}

	/* 2. Reiniciamos el periférico, de forma que inicia en un estado conocido */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_SWRST;
	__NOP();
	ptrHandlerI2C->ptrI2Cx->CR1 &= ~I2C_CR1_SWRST;

	/* 3. Indicamos cuál es la velocidad del reloj principal, que es la señal
	 * utilizada por el periférico para generar la señal de reloj para el
	 * bus I2C
	 */
	ptrHandlerI2C->ptrI2Cx->CR2 &= ~I2C_CR2_FREQ; //Borramos la configuración prestablecida
	ptrHandlerI2C->ptrI2Cx->CR2 |= (MAIN_CLOCK_16_MHz_FOR_I2C << I2C_CR2_FREQ_Pos); // Ponemos la frecuencia del CPU como la que usará el periférico

	/* 4. Configuramos el modo I2C en el que el sistema funciona.
	 * En esta configuración se incluye también la velocidad del reloj y el tiempo
	 * máximo para el cambio de la señal (TIME RISE -> T-Rise).
	 *
	 * Todo comienza con los dos registros correspondientes en 0
	 * Registros:
	 * 		- Clock Control Register (CCR)
	 * 		- Time Rise Register (TRISE)
	 */

	// Limpiamos los registros
	ptrHandlerI2C->ptrI2Cx->CCR = 0;
	ptrHandlerI2C->ptrI2Cx->TRISE = 0;

	if(ptrHandlerI2C->modeI2C == I2C_MODE_SM){
		// Estamos en el modo "standar" (SM Mode)
		ptrHandlerI2C->ptrI2Cx->CCR &= ~I2C_CCR_FS;

		// Configuramos el registro que se encarga de generar la señal de reloj
		ptrHandlerI2C->ptrI2Cx->CCR |= (I2C_MODE_SM_SPEED_100KHz << I2C_CCR_CCR_Pos);

		// Configuramos el registro que controla el tiempo T-Rise máximo
		ptrHandlerI2C->ptrI2Cx->TRISE |= I2C_MAX_RISE_TIME_SM;
	}
	else{
		// Estamos en el modo "fast" (SM Mode)
		ptrHandlerI2C->ptrI2Cx->CCR |= I2C_CCR_FS;

		// Configuramos el registro que se encarga de generar la señal de reloj
		ptrHandlerI2C->ptrI2Cx->CCR |= (I2C_MODE_SM_SPEED_400KHz << I2C_CCR_CCR_Pos);

		// Configuramos el registro que controla el tiempo T-Rise máximo
		ptrHandlerI2C->ptrI2Cx->TRISE |= I2C_MAX_RISE_TIME_FM;
	}

	/* 5. Activamos el módulo I2C */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_PE;


} // Fin de la configuración del I2C
