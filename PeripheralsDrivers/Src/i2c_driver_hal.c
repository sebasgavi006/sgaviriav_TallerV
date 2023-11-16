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
		// Estamos en el modo "Standar" (SM Mode)
		ptrHandlerI2C->ptrI2Cx->CCR &= ~I2C_CCR_FS;

		// Configuramos el registro que se encarga de generar la señal de reloj
		ptrHandlerI2C->ptrI2Cx->CCR |= (I2C_MODE_SM_SPEED_100KHz << I2C_CCR_CCR_Pos);

		// Configuramos el registro que controla el tiempo T-Rise máximo
		ptrHandlerI2C->ptrI2Cx->TRISE |= I2C_MAX_RISE_TIME_SM;
	}
	else{
		// Estamos en el modo "Fast" (SM Mode)
		ptrHandlerI2C->ptrI2Cx->CCR |= I2C_CCR_FS;

		// Configuramos el registro que se encarga de generar la señal de reloj
		ptrHandlerI2C->ptrI2Cx->CCR |= (I2C_MODE_SM_SPEED_400KHz << I2C_CCR_CCR_Pos);

		// Configuramos el registro que controla el tiempo T-Rise máximo
		ptrHandlerI2C->ptrI2Cx->TRISE |= I2C_MAX_RISE_TIME_FM;
	}

	/* 5. Activamos el módulo I2C */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_PE;


} // Fin de la configuración del I2C


/*
 * Función para iniciar la transmisión del I2C
 */
void i2c_StartTransaction(I2C_Handler_t *ptrHandlerI2C){

	/* 1. Verificamos que la línea no está ocupada - bit "Busy" del reg CR2 */
	while(ptrHandlerI2C->ptrI2Cx->SR2 & I2C_SR2_BUSY){	// El ciclo se mantiene hasta que esté desocupada
		__NOP();
	}

	/* 2. Generamos la señal "start" */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_START;

	/* 2a. Esperamos a que la bandera del evento "start" se levante.
	 * Mientras esperamos, el valor de SB es 0, entonces la negación (!) es 1.
	 */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_SB)){
		__NOP();
	}

} // Fin función i2c_StartTransaction


/*
 * Función para empezar de nuevo la transacción
 */
void i2c_ReStartTransaction(I2C_Handler_t *ptrHandlerI2C){

	/* 1. Generamos la señal de "start" */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_START;

	/* 2. Esperamos que se levante la bandera del evento "start" */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_SB)){
		__NOP();
	}

} // Fin de la función i2c_ReStartTransaction


/*
 * Función para la parada de la transmisión por I2C
 */
void i2c_StopTransaction(I2C_Handler_t *ptrHandlerI2C){
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_STOP;
}


/*
 * Indicación de ACK (Indicación para el Slave para iniciar)
 */
void i2c_SendAck(I2C_Handler_t *ptrHandlerI2C){

	/* Debemos escribir 1 en la posición ACK del CR1 */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_ACK;
}


/*
 * Indicación de No-ACK (Indicación para el Slave para iniciar)
 */
void i2c_SendNoAck(I2C_Handler_t *ptrHandlerI2C){

	/* Debemos escribir 0 en la posición ACK del CR1 */
	ptrHandlerI2C->ptrI2Cx->CR1 &= ~I2C_CR1_ACK;
}


/*
 * Función para enviar la dirección del Slave con el que nos queremos
 * comunicar, y se indica si queremos Leer o Escribir en el dispositivo
 * con el que nos comunicamos a través del I2C.
 */
void i2c_SendSlaveAddressRW(I2C_Handler_t *ptrHandlerI2C, uint8_t slaveAddress, uint8_t readOrWrite){

	/* 0. Definimos una variable auxiliar para leer los
	 * registros para la secuencia de la bandera del ADDR
	 */
	uint8_t auxByte = 0;
	(void) auxByte;

	/* 1. Enviamos la dirección del Slave. En el bit menos significativo ponemos
	 * el valor Lectura (1) o Escritura (0)
	 */
	ptrHandlerI2C->ptrI2Cx->DR = ((slaveAddress << 1) | readOrWrite);

	/* 1a. Esperamos hasta que la bandera del evento ADDR se levante
	 * (esto nos indica que la dirección fue enviada satisfactoriamente,
	 * junto con el bit de Lectura o Escritura)
	 */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_ADDR)){
		__NOP();
	}

	/* 2. Debemos limpiar la bandera de la recepción de ACK del ADDR, para
	 * lo cual debemos leer en secuencia el I2C_SR1 y luego el I2C_SR2
	 */
	auxByte = ptrHandlerI2C->ptrI2Cx->SR1;
	auxByte = ptrHandlerI2C->ptrI2Cx->SR2;

} // Fin del i2c_SendSlaveAddressRW


/*
 * Función para enviar la dirección de memoria
 */
void i2c_SendMemoryAddress(I2C_Handler_t *ptrHandlerI2C, uint8_t memAddress){
	/* Enviamos la dirección de memoria que deseamos leer */
	ptrHandlerI2C->ptrI2Cx->DR = memAddress;

	/* Esperamos hasta que el byte sea transmitido */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_TXE)){
		__NOP();
	}
}


/*
 * Función para enviar un solo Byte al dispositivo Slave
 */
void i2c_SendDataByte(I2C_Handler_t *ptrHandlerI2C, uint8_t dataToWrite){
	/* Cargamos el valor que deseamos escribir */
	ptrHandlerI2C->ptrI2Cx->DR = dataToWrite;

	/* Esperamos hasta que el byte sea transmitido */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_BTF)){
		__NOP();
	}
}


/*
 * Función para leer el byte recibido
 */
uint8_t i2c_ReadDataByte(I2C_Handler_t *ptrHandlerI2C){
	/* Esperamos hasat que el byte entrante sea recibido */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_RXNE)){
		__NOP();
	}

	/* Devolvemos como salida de la función lo almacenado en el DR */
	ptrHandlerI2C->dataI2C = ptrHandlerI2C->ptrI2Cx->DR;
	return ptrHandlerI2C->dataI2C;

}


/*
 * Función para leer un registro específico del dispositivo Slave
 */
uint8_t i2c_ReadSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead){

	/* 0. Creamos una variable auxiliar para recibir el dato que leemos */
	uint8_t auxRead = 0;

	/* 1. Generamos la condición de "Start" */
	i2c_StartTransaction(ptrHandlerI2C);

	/* 2. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_WRITE_DATA);

	/* 3. Enviamos la dirección de memoria que deseamos leer */
	i2c_SendMemoryAddress(ptrHandlerI2C, regToRead);

	/* 4. Creamos una condición de "Re-Start" */
	i2c_ReStartTransaction(ptrHandlerI2C);

	/* 5. Enviamos la dirección del esclavo y la indicación de LEER */
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_READ_DATA);

	/* 6. Leemos el dato que envia el esclavo */
	auxRead = i2c_ReadDataByte(ptrHandlerI2C);

	/* 7. Generamos la condición de NoACK, para que el Master no responda y el slave
	 * solo envíe 1 byte.
	 */
	i2c_SendNoAck(ptrHandlerI2C);

	/* 8. Generamos la condición Stop, para que el slave se detenga después de 1 byte */
	i2c_StopTransaction(ptrHandlerI2C);

	/* 9. La función devuelve el dato enviado por el slave */
	return auxRead;

} // Fin de i2c_ReadSingleRegister


/*
 * Función para escribir en un registro del dispositivo Slave
 */
void i2c_WriteSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead, uint8_t newValue){

	/* 1. Generamos la condición de "Start" */
	i2c_StartTransaction(ptrHandlerI2C);

	/* 2. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_WRITE_DATA);

	/* 3. Enviamos la dirección de memoria que deseamos escribir */
	i2c_SendMemoryAddress(ptrHandlerI2C, regToRead);

	/* 4. Enviamos el valor que deseamos escribir en el registro seleccionado */
	i2c_SendDataByte(ptrHandlerI2C, newValue);

	/* 5. Generamos la condición Stop, para que el slave se detenga después de 1 byte */
	i2c_StopTransaction(ptrHandlerI2C);


} // Fin de i2c_WriteSingleRegister
