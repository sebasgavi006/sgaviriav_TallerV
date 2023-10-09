/*
 * timer_driver_hal.c
 *
 *  Created on: Sep 24, 2023
 *      Author: sgaviriav
 */

#include "stm32f4xx.h"
#include "stm32_assert.h"
#include "timer_driver_hal.h"

/* Variable que guarda la referencia del periférico que se está utilizando */
TIM_TypeDef *ptrTimerUsed;

/* === Headers for private functions === */
static void timer_enable_clock_peripheral(Timer_Handler_t *pTimerHandler);
static void timer_set_mode(Timer_Handler_t *pTimerHandler);
static void timer_set_prescaler(Timer_Handler_t *pTimerHandler);
static void timer_set_period(Timer_Handler_t *pTimerHandler);
static void timer_config_interrupt(Timer_Handler_t *pTimerHandler);

/* Función en la que cargamos la configuración del Timer.
 *
 * Recordar que siempre se debe comenzar con activar la señal
 * de reloj del periférico que se está utilizando.
 * Además, en este caso, debemos ser cuidadosos al momento de
 * utilizar las interrupciones.
 * Los timer están conectados directamente al elemento NVIC del Cortex-Mx.
 *
 * Debemos configurar y/o utilizar:
 *  - TIMx_CR1		(Control Register 1)
 *  - TIMx_SMCR		(slave mode control register) -> Mantener en 0 para modo de Timer Básico
 *  - TIMx_DIER		(DMA and Interrupt enable register)
 *  - TIMx_SR		(Status register)
 *  - TIMx_CNT		(Counter)
 *  - TIMx_PSC		(Pre-scaler)
 *  - TIMx_ARR		(Auto-reload register)
 *
 *  PD: Todos estos modos de configuración vienen con las librerías CMSIS
 *
 *  Como vamos a trabajar con interrupciones, antes de configurar una nueva,
 *  debemos desactivar el sistema global de interrupciones, activar la IRQ específica
 *  y luego volver a encender el sistema
 */

void timer_Config(Timer_Handler_t *pTimerHandler){	// Llamamos la función pública timer_Config
	// Guardamos una referencia al periférico que estamos utilizando...
	ptrTimerUsed = pTimerHandler->pTIMx;

	/* 0. Desactivamos las interrupciones globales mientras configuramos el sistema */
	__disable_irq();

	/* 1. Activar la señal de reloj del periférico requerido */
	timer_enable_clock_peripheral(pTimerHandler);

	/* 2. Configuramos el Pre-scaler */
	timer_set_prescaler(pTimerHandler);

	/* 3. Configuramos si UP_COUNT o DOWN_COUNT (mode) */
	timer_set_mode(pTimerHandler);

	/* 4. Configuramos el Auto-reload */
	timer_set_period(pTimerHandler);

	/* 5. Configuramos la interrupción */
	timer_config_interrupt(pTimerHandler);

	/* X. Volvemos a activar las interrupciones del sistema */
	__enable_irq();

	/* El timer inicia apagado */
	timer_SetState(pTimerHandler, TIMER_OFF);	// Esta es la función pública timer_SetState

}

/*
 * Funciones privadas que realizarán la configuración del Timer
 *
 * La siguiente función activa la señal de reloj según el Timer que estemos configurando
 */
 void timer_enable_clock_peripheral(Timer_Handler_t *pTimerHandler){

	 // Verificamos que es un Timer permitido
	 assert_param(IS_TIME_INSTANCE(pTimerHandler->pTIMx));

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
	 if(pTimerHandler->pTIMx == TIM2){
		 RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	 }
	 else if(pTimerHandler->pTIMx == TIM3){
		 RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	 }
	 else if(pTimerHandler->pTIMx == TIM4){
		 RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	 }
	 else if(pTimerHandler->pTIMx == TIM5){
		 RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
	 }
	 else if(pTimerHandler->pTIMx == TIM9){
		 RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
	 }
	 else if(pTimerHandler->pTIMx == TIM10){
		 RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	 }
	 else if(pTimerHandler->pTIMx == TIM11){
		 RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
	 }
	 else{
		 __NOP();
	 }
 }

 /*
  * El Pre-scaler nos configura la velocidad a la que se incrementa
  * el registro CNT del Timer (Este registro es el que se usa para
  * determinar cuando se dispara la interrupción)
  */
 void timer_set_prescaler(Timer_Handler_t *pTimerHandler){

	 // Verificamos que el valor del prescaler es válido
	 assert_param(IS_TIMER_PRESC(pTimerHandler->TIMx_Config.TIMx_Prescaler));

	 // Configuramos el valor del Pre-scaler
	 pTimerHandler->pTIMx->PSC = pTimerHandler->TIMx_Config.TIMx_Prescaler - 1; // (16000000)
	 	 	 	 	 	 	 	 	 	 // Restamos 1 al valor del prescaler por indicación de la hoja de datos
 }


 /*
  * Upcounter o DownCounter
  */
 void timer_set_mode(Timer_Handler_t *pTimerHandler){

	 // Verificamos que el modo de funcionamiento es correcto
	 assert_param(IS_TIMER_MODE()(pTimerHandler->TIMx_Config.TIMx_mode));

	 // Verificamos cuál es el modo que se desea configurar.
	 if(pTimerHandler->TIMx_Config.TIMx_mode == TIMER_UP_COUNTER){
		 // Configuramos en modo Upcounter DIR = 0
		 pTimerHandler->pTIMx->CR1 &= ~TIM_CR1_DIR;
	 }
	 else{
		 // Configuramos en modo downcounter DIR = 1
		 pTimerHandler->pTIMx->CR1 |= TIM_CR1_DIR;
	 }
 }


 /*
  * Esta función configura el límite hasta donde cuenta el Timer para generar un
  * evento "update" (cuando esta contando de forma ascendente), o configura
  * el valor desde donde se comienza a contar, cuando el sistema funciona de forma
  * descendente.
  */
 void timer_set_period(Timer_Handler_t *pTimerHandler){

	 // Verificamos que el valor que genera el periodo es válido
	 assert_param(IS_TIMER_PERIOD(pTimerHandler->TIMx_Config.TIMx_Period));

	 // Configuramos el valor del autoreload (El periodo, tiene sentido)
	 pTimerHandler->pTIMx->ARR = pTimerHandler->TIMx_Config.TIMx_Period - 1;
 }


 /*
  * Función que permite configurar la interrupción
  */
 void timer_config_interrupt(Timer_Handler_t *pTimerHandler){

	 // Verificamos el posible valor configurado
	 assert_param(IS_TIMER_INTERRUP(pTimerHandler->TIMx_Config.TIMx_InterruptEnable));

	 if(pTimerHandler->TIMx_Config.TIMx_InterruptEnable == TIMER_INT_ENABLE){
		 /* Activamos la interrupción debida al Timerx utilizado */
		 pTimerHandler->pTIMx->DIER |= TIM_DIER_UIE;

		 /* Activamos el canal del sistema NVIC para que lea la interrupción */
		 if(pTimerHandler->pTIMx == TIM2){
			 NVIC_EnableIRQ(TIM2_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM3){
			 NVIC_EnableIRQ(TIM3_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM4){
			 NVIC_EnableIRQ(TIM4_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM5){
			 NVIC_EnableIRQ(TIM5_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM9){
			 NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM10){
			 NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM11){
			 NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
		 }
		 else{
			 __NOP();
		 }
	 }
	 else{
		 /* Desactivamos la interrupción debida al Timerx utilizado */
		 pTimerHandler->pTIMx->DIER &= ~TIM_DIER_UIE;

		 /* Desactivamos el canal del sistema NVIC */
		 if(pTimerHandler->pTIMx == TIM2){
			 NVIC_DisableIRQ(TIM2_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM3){
			 NVIC_DisableIRQ(TIM3_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM4){
			 NVIC_DisableIRQ(TIM4_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM5){
			 NVIC_DisableIRQ(TIM5_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM9){
			 NVIC_DisableIRQ(TIM1_BRK_TIM9_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM10){
			 NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
		 }
		 else if(pTimerHandler->pTIMx == TIM11){
			 NVIC_DisableIRQ(TIM1_TRG_COM_TIM11_IRQn);
		 }
		 else{
			 __NOP();
		 }
	 }

 }

/*
 * Configuramos el encendido o apagado el contador del timer
 */
 void timer_SetState(Timer_Handler_t *pTimerHandler, uint8_t newState){

	 // Verificamos que el estado ingresado es adecuado
	 assert_param(IS_TIMER_STATE(newState));

	 /* 4. Reiniciamos el registro counter */
	 pTimerHandler->pTIMx->CNT = 0;

	 if(newState == TIMER_ON){
		 /* 5a. Activamos el Timer (el CNT debe comenzar a contar) por medio del CR1 (Control Register)  */
		 pTimerHandler->pTIMx->CR1 |= TIM_CR1_CEN;
	 }
	 else{
		 /* 5b. Desactivamos el Timer (el CNT debe detenerse) */
		 pTimerHandler->pTIMx->CR1 &= ~TIM_CR1_CEN;
	 }

 }

 /**/
 __attribute__((weak)) void Timer2_Callback(void){		// El atributo (weak) permite sobreescribir la función Callback
	 __NOP();
 }

 __attribute__((weak)) void Timer3_Callback(void){
	 __NOP();
 }

 __attribute__((weak)) void Timer4_Callback(void){
	 __NOP();
 }

 __attribute__((weak)) void Timer5_Callback(void){
	 __NOP();
 }

 __attribute__((weak)) void Timer9_Callback(void){
	 __NOP();
 }

 __attribute__((weak)) void Timer10_Callback(void){
	 __NOP();
 }

 __attribute__((weak)) void Timer11_Callback(void){
	 __NOP();
 }


 /* Esta es la función a la que apunta el sistema en el vector de interrupciones.
  * Se debe utilizar usando exactamente el mismo nombre definido en el vector de
  * interrupciones.
  * Al hacerlo correctamnete, el sistema apunta a esta función y cuando la interrupción
  * se lanza, el sistema inmediatamente salta a este lugar en la memoria
  */

void TIM2_IRQHandler(void){
	/* Limpiamos la bandera que indica que la interrupción se ha generado */
	TIM2->SR &= ~TIM_SR_UIF;

	/* Llamamos a la función que se debe encargar de hacer algo con esta interrupción */
	Timer2_Callback();
}

void TIM3_IRQHandler(void){
	TIM3->SR &= ~TIM_SR_UIF;
	Timer3_Callback();
}

void TIM4_IRQHandler(void){
	TIM4->SR &= ~TIM_SR_UIF;
	Timer4_Callback();
}

void TIM5_IRQHandler(void){
	TIM5->SR &= ~TIM_SR_UIF;
	Timer5_Callback();
}

void TIM9_IRQHandler(void){
	/* Limpiamos la bandera que indica que la interrupción se ha generado */
	TIM9->SR &= ~TIM_SR_UIF;

	/* Llamamos a la función que se debe encargar de hacer algo con esta interrupción */
	Timer9_Callback();
}

void TIM10_IRQHandler(void){
	TIM10->SR &= ~TIM_SR_UIF;
	Timer10_Callback();
}

void TIM11_IRQHandler(void){
	TIM11->SR &= ~TIM_SR_UIF;
	Timer11_Callback();
}

