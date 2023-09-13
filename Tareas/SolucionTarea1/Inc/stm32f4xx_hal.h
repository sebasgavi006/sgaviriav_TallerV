/*
 * stm32f4xx_hal.h
 *
 *  Created on: Sep 2, 2023
 *      Author: sgaviriav
 *
 *  Este archivo contiene la informacion mas basica del microcontrolador:
 *   - Valores del reloj principal
 *   - Distribucion basica de la memoria (descrito en la Figura 14 de la
 *     hoja de datos del micro)
 *   - Posiciones de memoria de los perifericos disponibles en el micro
 *     descrito en la tabla 1 (Memory Map)
 *   - Incluir los demas drivers de los perifericos
 *   - Definiciones de las constantes mas basicas
 *
 */


#ifndef _STM32F4XX_HAL_H_
#define _STM32F4XX_HAL_H_

#include <stdint.h>
#include <stddef.h>

#define HSI_CLOCK_SPEED		8000000		// Value for the main clock signal (HSI -> High Speed Internal)
#define HSE_CLOCK_SPEED		16000000	// Value for the main clock signal (HSE -> High Speed External)

#define NOP()		asm("NOP")
#define __weak		__attribute__((weak))

/*
 * Base addresses of Flash and SRAM memories
 * Datasheet, Memory Map, Figure 14
 * (Remember, 1KByte = 1024 bytes)
 */

#define FLASH_BASE_ADDR		0x08000000U		// Esta es la memoria del programa, 512KB
#define SRAM_BASE_ADDR		0x20000000U		// Esta es la memoria RAM, 128KB

/* NOTA:
 * Observar que existen unos registros especificos del Cortex M4 en la region
 * 0xE0000000U. Los controladores de las interrupciones se encuentran alli, por ejemplo
 */


/* NOTA:
 * Ahora agregamos la direccion de memoria base para cada uno de los perifericos que
 * posee el micro.
 * En el datasheet del micro, Figrua 14 (Memory Map) encontramos el mapa de los buses:
 * 	- APB1 (Advance Peripheral Bus)
 * 	- APB2
 * 	- AHB1 (Advance High_Performance Bus)
 * 	- AHB2
 */


/*
 * Se muestran las direcciones de memoria de los buses
 * AHBx and APBx Bus Peripherals Base Addresses
 */
#define APB1_BASE_ADDR		0x40000000U
#define APB2_BASE_ADDR		0x40010000U
#define AHB1_BASE_ADDR		0x40020000U
#define AHB2_BASE_ADDR		0x50000000U

/*
 * Ahora hacemos lo mismo para cada una de las posiciones de memoria de cada uno de los
 * perifericos descritos en la Tabla 1 del manual de referencia del micro.
 * Observe que dicha tabla esta a su vez dividida en 4 segmentos, cada uno correspondiente a
 * APB1, APB2, AHB1, AHB2.
 *
 * Comenzar de arriba hacia abajo como se muestra en la tabla.
 * Inicia USB_OTG_FS (AHB2)
 */

/* Posiciones de memoria para perifericos del AHB2 (Bus 2) */
#define USB_OTG_FS_BASE_ADDR	(AHB2_BASE_ADDR + 0x0000U)

/* Posiciones de memoria para perifericos del AHB1 (Bus 1) */
#define DMA2_BASE_ADDR			(AHB1_BASE_ADDR + 0x6400U)
#define DMA1_BASE_ADDR			(AHB1_BASE_ADDR + 0x6000U)
#define FIR_BASE_ADDR			(AHB1_BASE_ADDR + 0x3C00U)
#define RCC_BASE_ADDR			(AHB1_BASE_ADDR + 0x3800U)
#define CRC_BASE_ADDR			(AHB1_BASE_ADDR + 0x3000U)
#define GPIOH_BASE_ADDR			(AHB1_BASE_ADDR + 0x1C00U)
#define GPIOE_BASE_ADDR			(AHB1_BASE_ADDR + 0x1000U)
#define GPIOD_BASE_ADDR			(AHB1_BASE_ADDR + 0x0C00U)
#define GPIOC_BASE_ADDR			(AHB1_BASE_ADDR + 0x0800U)
#define GPIOB_BASE_ADDR			(AHB1_BASE_ADDR + 0x0400U)
#define GPIOA_BASE_ADDR			(AHB1_BASE_ADDR + 0x0000U)


/* Macros Genericos */
#define ENABLE				(1)
#define DISABLE				(0)
#define SET					ENABLE
#define CLEAR				DISABLE
#define RESET				DISABLE
#define FLAG_SET			SET
#define FLAG_RESET			RESET
#define I2C_WRITE			(0)
#define I2C_READ			(1)

/* ========== INICIO de la descripcion de los elementos que componen el periferico ==========
 *
 * Definicion de la estructura de datos que representan a cada uno de los registros que
 * componen el periferico RCC.
 *
 * Debido a los temas que se van a manejar en el curso, solo se deben definir los bits de los
 * registros: 6.3.1 (RCC_CR) hasta el 6.3.12 (RCC_APB2ENR), 6.3.17 (RCC_BDCR) y 6.318 (RCC_CSR)
 *
 * ==================================== ¡¡¡IMPORTANTE!!! =====================================
 * NOTA: La posicion de memoria (offset) debe encajar perfectamente con la posicion de memoria
 * indicada en la hoja de datos del equipo. Observe que los elementos "reservedx" tambien estan
 * presentes alli.
 */

typedef struct
{
	volatile uint32_t CR;			// Clock Control Register				ADDR_OFFSET:	0x00
	volatile uint32_t PLLCFGR;		// PLL Configuration Register			ADDR_OFFSET:	0x04
	volatile uint32_t CFGR;			// Clock Configuration Register			ADDR_OFFSET: 	0x08
	volatile uint32_t CIR;			// Clock Interrupt Register				ADDR_OFFSET:	0x0C
	volatile uint32_t AHB1RSTR;		// AHB1 Peripheral Reset Register		ADDR_OFFSET:	0x10
	volatile uint32_t AHB2RSTR;		// AHB2 Peripheral Reset Register		ADDR_OFFSET:	0x14
	volatile uint32_t reserved0;	//reserved								ADDR_OFFSET:	0x18
	volatile uint32_t reserved1;	//reserved								ADDR_OFFSET:	0x1C
	volatile uint32_t APB1RSTR;		// APB1 Peripheral Reset Register		ADDR_OFFSET:	0x20
	volatile uint32_t APB2RSTR;		// APB2 Peripheral Reset Register		ADDR_OFFSET:	0x24
	volatile uint32_t reserved2;	//reserved								ADDR_OFFSET:	0x28
	volatile uint32_t reserved3;	//reserved								ADDR_OFFSET:	0x2C
	volatile uint32_t AHB1ENR;		// AHB1 Clock Enable Register			ADDR_OFFSET:	0x30
	volatile uint32_t AHB2ENR;		// AHB2 Clock Enable Register			ADDR_OFFSET:	0x34
	volatile uint32_t reserved4;	//reserved								ADDR_OFFSET:	0x38
	volatile uint32_t reserved5;	//reserved								ADDR_OFFSET:	0x3C
	volatile uint32_t APB1ENR;		// APB1 Clock Enable Register			ADDR_OFFSET:	0x40
	volatile uint32_t APB2ENR;		// APB2 Clock Enable Register			ADDR_OFFSET:	0x44
	volatile uint32_t reserved6;	//reserved								ADDR_OFFSET:	0x48
	volatile uint32_t reserved7;	//reserved								ADDR_OFFSET:	0x4C
	volatile uint32_t AHB1LPENR;	//AHB1 Clock Enable Low Power Register	ADDR_OFFSET:	0x50
	volatile uint32_t AHB2LPENR;	//AHB2 Clock Enable Low Power Register	ADDR_OFFSET:	0x54
	volatile uint32_t reserved8;	//reserved								ADDR_OFFSET:	0x58
	volatile uint32_t reserved9;	//reserved								ADDR_OFFSET:	0x5C
	volatile uint32_t APB1LPENR;	//APB1 Clock Enable Low Power Register	ADDR_OFFSET:	0x60
	volatile uint32_t APB2LPENR;	//APB2 Clock Enable Low Power Register	ADDR_OFFSET:	0x64
	volatile uint32_t reserved10;	//reserved								ADDR_OFFSET:	0x68
	volatile uint32_t reserved11;	//reserved								ADDR_OFFSET:	0x6C
	volatile uint32_t BDCR;			//Back Up Domain Register				ADDR_OFFSET:	0x70
	volatile uint32_t CSR;			//Clock Control & Status Register		ADDR_OFFSET:	0x74
	volatile uint32_t reserved12;	//reserved								ADDR_OFFSET:	0x78
	volatile uint32_t reserved13;	//reserved								ADDR_OFFSET:	0x7C
	volatile uint32_t SSCGR;		//Spread Spectrum Clock Generation Reg	ADDR_OFFSET:	0x80
	volatile uint32_t PLLI2SCFGR;	//PLLI2S Configuration Register			ADDR_OFFSET:	0x84
	volatile uint32_t reserved14;	//reserved								ADDR_OFFSET:	0x88
	volatile uint32_t DCKCFGR;		//Dedicated Clocks Configuration Reg	ADDR_OFFSET:	0x8C

} RCC_RegDef_t;

/*
 * Hacemos como un "merge", en el cual ubicamos la estructura RCC_RegDef_t apuntando a la posicion
 * de memoria correspondiente, de forma que cada variable dentro de la estructura coincide con cada
 * uno de los SFR (Special Function Register) en la memoria del microcontrolador
 */

#define RCC				((RCC_RegDef_t *) RCC_BASE_ADDR)	// RECORDAR que " *" significa un puntero



/* Descripcion bit a bit de cada uno de los registros que componen al periferico RCC
 * 6.3.1 RCC_CR
 * 6.3.2 RCC_PLLCFGR
 * 6.3.3 RCC_CFGR
 * 6.3.4 RCC_CIR
 * 6.3.5 RCC_AHB1RSTR
 * 6.3.6 RCC_AHB2RSTR
 * 6.3.7 RCC_APB1RSTR
 * 6.3.8 RCC_APB2RSTR
 */


/* 6.3.9 RCC_AHB1ENR */
#define RCC_AHB1ENR_GPIOAEN			0
#define RCC_AHB1ENR_GPIOBEN			1
#define RCC_AHB1ENR_GPIOCEN			2
#define RCC_AHB1ENR_GPIODEN			3
#define RCC_AHB1ENR_GPIOEEN			4
#define RCC_AHB1ENR_GPIOHEN			7
#define RCC_AHB1ENR_GRCEN			12
#define RCC_AHB1ENR_DMA1EN			21
#define RCC_AHB1ENR_DMA2EN			22

/*
 * 6.3.10 RCC_AHB2ENR
 * 6.3.11 RCC_APB1ENR
 * 6.3.12 RCC_APB2ENR
 * 6.3.17 RCC_BDCR
 * 6.3.18 RCC_CSR
 */

/* ========== FIN de la descripcion de los elementos que componen el periferico ==========


 * Posibles valores de las configuraciones
 * System Clock (main clock) possible options
 * Microcontroller Clock Output1 (MC01) options
 * Microcontroller preescale Clock Output1 (MC01PRE) options
 * Microcontroller preescale Clock Output2 (MC02PRE) options
 * Microcontroller Clock Output2 (MC02) options
 */


/* ========== INICIO de la descripcion de los elementos que componen el periferico ==========
 * Definicion de la estructura de datos que representa a cada uno de los registros que
 * componen el periferico GPIO
 *
 * Debido a que el periferico GPIOx es muy simple, no es muy necesario crear la descripcion
 * bit a bit de cada uno de los registros que componen dicho periferico, pero si es necesario
 * comprender que hace cada registro, para poder cargar correctamente la configuracion.
 */

typedef struct
{
	volatile uint32_t MODER;		//port mode register					ADDR_OFFSET:	0x00
	volatile uint32_t OTYPER;		//port output type register				ADDR_OFFSET:	0x04
	volatile uint32_t OSPEEDR;		//port output speed register			ADDR_OFFSET:	0x08
	volatile uint32_t PUPDR;		//port pull-up/pull-down register		ADDR_OFFSET:	0x0C
	volatile uint32_t IDR;			//port input data register				ADDR_OFFSET:	0X10
	volatile uint32_t ODR;			//port output data register				ADDR_OFFSET:	0X14
	volatile uint32_t BSRR;			//port bit set/reset register			ADDR_OFFSET:	0X18
	volatile uint32_t LCKR;			//port configuration lock register		ADDR_OFFSET:	0X1C
	volatile uint32_t AFR[2];		//port alternate function registers		ADDR_OFFSET:	0x20-0x24
} GPIO_TypeDef;


/* Creamos un objeto de la estructura definida y hacemos que quede ubicada exactamente sobre
 * la posicion de memoria donde se encuentra el registro.
 * debido a que son varios perifericos GPIOx, es necesario hacer la definicion para cada uno.
 *
 * Tener cuidado que cada elemento coincida con su respectiva direccion base
 */

#define GPIOA			((GPIO_TypeDef *) GPIOA_BASE_ADDR)
#define GPIOB			((GPIO_TypeDef *) GPIOB_BASE_ADDR)
#define GPIOC			((GPIO_TypeDef *) GPIOC_BASE_ADDR)
#define GPIOD			((GPIO_TypeDef *) GPIOD_BASE_ADDR)
#define GPIOE			((GPIO_TypeDef *) GPIOE_BASE_ADDR)
#define GPIOH			((GPIO_TypeDef *) GPIOH_BASE_ADDR)


/* For testing assert parameters */
#define IS_GPIO_ALL_INSTANCE(GPIOx)	((GPIOx == GPIOA)	|| \
									 (GPIOx == GPIOB)	|| \
									 (GPIOx == GPIOC)	|| \
									 (GPIOx == GPIOD)	|| \
									 (GPIOx == GPIOE)	|| \
									 (GPIOx == GPIOH))

/*
   typedef struct
   {
   	   volatile uint32_t dummy;			// Dummy Example Register			ADDR_OFFSET: 0x00
   	} DUMMY_RegDef_t
 */

/* Incluimos las librerias de cada periferico */

/* ========== FIN de la descripcion de los elementos que componen el periferico =========== */

#endif /* STM32F4XX_HAL_H_ */
