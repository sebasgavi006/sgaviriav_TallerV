/*
 * pwm_driver_hal.c
 *
 *  Created on: Nov 03, 2023
 *      Author: sgaviriav
 */

#include "stm32f4xx.h"
#include "pwm_driver_hal.h"

/*
 * Headers de las funciones privadas del PWM
 */
void timer_enable_clock_peripheral(PWM_Handler_t *ptrPwmHandler);


/*
 * Función para cargar las configuraciones desde el Handler del PWM
 */
void pwm_Config(PWM_Handler_t *ptrPwmHandler){

	/* 1. Activar la señal de reloj del Timer requerido */
	timer_enable_clock_peripheral(ptrPwmHandler);

	/* 2. Cargamos la frecuencia deseada */
	setFrequency(ptrPwmHandler);

	/* 3. Cargamos el valor del dutty-Cycle*/
	setDutyCycle(ptrPwmHandler);

	/* 3a. Estamos en UP_Mode, el limite se carga en ARR y se comienza en 0 */

	// Configruamos el contador en Up-Count
	ptrPwmHandler->ptrTIMx->CR1 &= ~TIM_CR1_DIR;

	// Cargamos el límite del contador (periodo) TIMx -> ARR
	ptrPwmHandler->ptrTIMx->ARR = ptrPwmHandler->config.periodo;

	/* 4. Configuramos los bits CCxS del registro TIMy_CCMR1, de forma que sea modo salida
	 * (para cada canal hay un conjunto CCxS)
	 *
	 * 5. Además, en el mismo "case" podemos configurar el modo del PWM, su polaridad...
	 *
	 * 6. Y además activamos el preload bit, para que cada vez que exista un update-event
	 * el valor cargado en el CCRx será recargado en el registro "shadow" del PWM */
	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		// Seleccionamos como salida el canal
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_CC1S;

		// Configuramos el canal como PWM
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC1M; // Limpiamos el registro primero

		/* Configuramos el PWM en mode 1, donde el contador aumenta hasta el valor ARR,
		 * y la señal está en alto (1), y se pone en bajo cuando se alcanza el valor del CCR
		 */
		ptrPwmHandler->ptrTIMx->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos); // PWM - mode 1

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1PE;

		break;
	}

	case PWM_CHANNEL_2:{
		// Seleccionamos como salida el canal
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_CC2S;

		// Configuramos el canal como PWM
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC2M; // Limpiamos el registro primero
		ptrPwmHandler->ptrTIMx->CCMR1 |= (0b110 << TIM_CCMR1_OC2M_Pos); // PWM - mode 1

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2PE;
		break;
	}

	case PWM_CHANNEL_3:{
		// Seleccionamos como salida el canal
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_CC3S;

		// Configuramos el canal como PWM
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_OC3M; // Limpiamos el registro primero
		ptrPwmHandler->ptrTIMx->CCMR2 |= (0b110 << TIM_CCMR2_OC3M_Pos); // PWM - mode 1

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC3PE;
		break;
	}

	case PWM_CHANNEL_4:{
		// Seleccionamos como salida el canal
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_CC4S;

		// Configuramos el canal como PWM
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_OC4M; // Limpiamos el registro primero
		ptrPwmHandler->ptrTIMx->CCMR2 |= (0b110 << TIM_CCMR2_OC4M_Pos); // PWM - mode 1

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC4PE;
		break;
	}

	default:{
		break;
	}

	/* 7. Activamos la salida seleccionada */
	enableOutput(ptrPwmHandler);

	} //Ffin del switch-case

} //Fin de la función de configuración


/* Función para activar la señal de reloj del Timer */
void timer_enable_clock_peripheral(PWM_Handler_t *ptrPwmHandler){

	 /* ===== NOTA IMPORTANTE =====
	  * Los TIMERS tienen diferentes tamaño:
	  *
	  * ===== TIMERS 16 BITS =====
	  * TIM3
	  * TIM4
	  * TIM9
	  * TIM10
	  * TIM11
	  *
	  * ===== TIMERS 32 BITS =====
	  * TIM2 - TIM5
	  * */

	 if(ptrPwmHandler->ptrTIMx == TIM2){
		 RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	 }
	 else if(ptrPwmHandler->ptrTIMx == TIM3){
		 RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	 }
	 else if(ptrPwmHandler->ptrTIMx == TIM4){
		 RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	 }
	 else if(ptrPwmHandler->ptrTIMx == TIM5){
		 RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
	 }
	 else if(ptrPwmHandler->ptrTIMx == TIM9){
		 RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
	 }
	 else if(ptrPwmHandler->ptrTIMx == TIM10){
		 RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	 }
	 else if(ptrPwmHandler->ptrTIMx == TIM11){
		 RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
	 }
	 else{
		 __NOP();
	 }
 } // Fin activar señal reloj Timers


/* Función para activar el Timer y activar todo el módulo PWM */
void startPwmSignal(PWM_Handler_t *ptrPwmHandler) {
	// Activamos el Counter del Timer, el cual activa también el PWM
	ptrPwmHandler->ptrTIMx->CR1 |= TIM_CR1_CEN;

}


/* Función para desactivar el Timer y detener todo el módulo PWM*/
void stopPwmSignal(PWM_Handler_t *ptrPwmHandler) {

	// Desactiva el Counter del Timer, y así mismo el módulo PWM
	ptrPwmHandler->ptrTIMx->CR1 &= ~TIM_CR1_CEN;

}


/* Función encargada de activar cada uno de los canales con los que cuenta el TimerX */
void enableOutput(PWM_Handler_t *ptrPwmHandler) {
	switch (ptrPwmHandler->config.channel) {
	case PWM_CHANNEL_1: {
		// Activamos la salida del canal 1
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC1E;
		break;
	}

	case PWM_CHANNEL_2: {
		// Activamos la salida del canal 2
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC2E;
		break;
	}

	case PWM_CHANNEL_3: {
		// Activamos la salida del canal 3
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC3E;
		break;
	}

	case PWM_CHANNEL_4: {
		// Activamos la salida del canal 4
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC4E;
		break;
	}

	default: {
		break;
	}
	}

} // Fin de la función enableOutput()


/* 
 * La frecuencia es definida por el conjunto formado por el preescaler (PSC)
 * y el valor límite al que llega el Timer (ARR), con estos dos se establece
 * la frecuencia.
 * */
void setFrequency(PWM_Handler_t *ptrPwmHandler){

	// Cargamos el valor del prescaler, nos define la velocidad (en ms) a la cual
	// se incrementa el Timer. (Define la escala de tiempo. 1ms -> PSC = 16000 -> Signal Clock = 16 MHz
	ptrPwmHandler->ptrTIMx->PSC = ptrPwmHandler->config.prescaler;	// Determina un valor de referencia para el tiempo

	// Cargamos el valor del ARR, el cual es el límite de incrementos del Timer
	// antes de hacer un update y reload.
	ptrPwmHandler->ptrTIMx->ARR = ptrPwmHandler->config.periodo;	// Determina cuántas unidades de tiempo pasan (Periodo)

	/*
	 * El producto PSC * ARR determina el periodo del PWM
	 */
}


/* Función para actualizar la frecuencia, funciona de la mano con setFrequency */
void updateFrequency(PWM_Handler_t *ptrPwmHandler, uint16_t newPeriod){

	// Actualizamos el registro que manipula el periodo
    ptrPwmHandler->config.periodo = newPeriod;

	// Llamamos a la fucnión que cambia la frecuencia
	setFrequency(ptrPwmHandler);
}


/* El valor del dutty es una fracción del valor del ARR.
 * Se puede determinar porcentualmente como ( CCRx / ARR * 100 )
 */
void setDutyCycle(PWM_Handler_t *ptrPwmHandler){

	// Seleccionamos el canal para configurar su dutty
	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		// Cargamos el valor del Duty Cycle en el registro CCR1
		ptrPwmHandler->ptrTIMx->CCR1 = ptrPwmHandler->config.dutyCycle;
		break;
	}

	case PWM_CHANNEL_2:{
		// Cargamos el valor del Duty Cycle en el registro CCR2
		ptrPwmHandler->ptrTIMx->CCR2 = ptrPwmHandler->config.dutyCycle;
		break;
	}

	case PWM_CHANNEL_3:{
		// Cargamos el valor del Duty Cycle en el registro CCR3
		ptrPwmHandler->ptrTIMx->CCR3 = ptrPwmHandler->config.dutyCycle;
		break;
	}

	case PWM_CHANNEL_4:{
		// Cargamos el valor del Duty Cycle en el registro CCR4
		ptrPwmHandler->ptrTIMx->CCR4 = ptrPwmHandler->config.dutyCycle;
		break;
	}

	default:{
		break;
	}

	}// fin del switch-case

} // Fin de la función setDutyCycle()


/* Función para actualizar el Dutty, funciona de la mano con setDuttyCycle */
void updateDutyCycle(PWM_Handler_t *ptrPwmHandler, uint16_t newDuty){
	// Actualizamos el registro que manipula el dutty
    ptrPwmHandler->config.dutyCycle = newDuty;

	// Llamamos a la fucnión que cambia el dutty y cargamos el nuevo valor
	setDutyCycle(ptrPwmHandler);
}




