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
#define ENABLE				1
#define DISABLE				0
#define SET					ENABLE
#define CLEAR				DISABLE
#define RESET				DISABLE
#define GPIO_PIN_SET		SET
#define GPIO_PIN_RESET		RESET
#define FLAG_SET			SET
#define FLAG_RESET			RESET
#define I2C_WRITE			0
#define I2C_READ			1

/* ========== INICIO de la descripcion de los elementos que componen el periferico ==========
 *
 * Definicion de la estructura de datos que representan a cada uno de los registros que
 * componen el periferico RCC.
 *
 * Debido a los temas que se van a manejar en el curso, solo se deben definir los bits de los
 * registros: 6.3.1 (RCC_CR) hasta el 6.3.12 (RCC_APB2ENR), 6.3.17 (RCC:BDCR) y 6.318 (RCC_CSR)
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
	volatile uint32_t SSCGR;		// Spread Spectrum Clock Generation Reg	ADDR_OFFSET:	0x80
	volatile uint32_t PLLI2SCFGR;	// PLLI2S Configuration Register		ADDR_OFFSET:	0x84
	volatile uint32_t reserved14;	//reserved								ADDR_OFFSET:	0x88
	volatile uint32_t DCKCFGR;		// Dedicated Clocks Configuration Reg	ADDR_OFFSET:	0x8C

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
#define RCC_AHB1ENR_GPIOA_EN			0
#define RCC_AHB1ENR_GPIOB_EN			1
#define RCC_AHB1ENR_GPIOC_EN			2
#define RCC_AHB1ENR_GPIOD_EN			3
#define RCC_AHB1ENR_GPIOE_EN			4
#define RCC_AHB1ENR_GPIOH_EN			7
#define RCC_AHB1ENR_GRCEN				12
#define RCC_AHB1ENR_DMA1_EN				21
#define RCC_AHB1ENR_DMA2_EN				22

/*
 * 6.3.10 RCC_AHB2ENR
 * 6.3.11 RCC_APB1ENR
 * 6.3.12 RCC_APB2ENR
 * 6.3.17 RCC_BDCR
 * 6.3.18 RCC_CSR
 */

/* ========== FIN de la descripcion de los elementos que componen el periferico ==========


/* ++++++++++ Posibles valores de las configuraciones
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


/* Descripcion de cada uno de los registros del periferico */

/* Valores estandar para las configuraciones */
/* 8.4.1 GPIOx_MODER (dos bits por cada PIN) */
#define GPIO_MODE_IN			0
#define GPIO_MODE_OUT			1
#define GPIO_MODE_ATLFN			2
#define GPIO_MODE_ANALOG		3

/* 8.4.2 GPIOx_OTYPER (un bit por cada PIN) */
#define GPIO_OTYPE_PUSHPULL			0
#define GPIO_OTYPE_OPENDRAIN		1

/* 8.4.3 GPIOx_OSPEEDR (dos bits por cada PIN) */
#define GPIO_OSPEED_LOW			0
#define GPIO_OSPEED_MEDIU		1
#define GPIO_OSPEED_FAST		2
#define GPIO_OSPEED_HIGH		3

/* 8.4.4 GPIOx_PUPDR (dos bits por cada PIN) */
#define GPIO_PUPDR_NOTHING			0
#define GPIO_PUPDR_PULLUP			1
#define GPIO_PUPDR_PULLDOWN			2
#define GPIO_PUPDR_RESERVED			3

/* 8.4.5 GPIOx_IDR (un bit por cada PIN) - este es el registro para leer el estado de un PIN */

/* 8.4.6 GPIOx_ODR (un bit por cada PIN) - este es el registro para escribir el estado de un
 * PIN (1 o 0). Este registro puede ser escrito y leido desde el software, pero no garantiza
 * una escritura "atomica", por lo cual es preferible usar el registro BSRR
 */

/* Definicion de los nombres de los pines */
#define PIN_0			0
#define PIN_1			1
#define PIN_2			2
#define PIN_3			3
#define PIN_4			4
#define PIN_5			5
#define PIN_6			6
#define PIN_7			7
#define PIN_8			8
#define PIN_9			9
#define PIN_10			10
#define PIN_11			11
#define PIN_12			12
#define PIN_13			13
#define PIN_14			14
#define PIN_15			15

/* Definicion de las funciones alternativas */
#define AF0			0b0000
#define AF1			0b0001
#define AF2			0b0010
#define AF3			0b0011
#define AF4			0b0100
#define AF5			0b0101
#define AF6			0b0110
#define AF7			0b0111
#define AF8			0b1000
#define AF9			0b1001
#define AF10		0b1010
#define AF11		0b1011
#define AF12		0b1100
#define AF13		0b1101
#define AF14		0b1110
#define AF15		0b1111

/*
   typedef struct
   {
   	   volatile uint32_t dummy;			// Dummy Example Register			ADDR_OFFSET: 0x00
   	} DUMMY_RegDef_t
 */

/* Incluimos las librerias de cada periferico */

/* ========== FIN de la descripcion de los elementos que componen el periferico =========== */

#endif /* STM32F4XX_HAL_H_ */
