/*
 * microphone_driver.h
 *
 *  Created on: Nov 27, 2023
 *      Author: sgaviriav
 */

#ifndef MICROPHONE_DRIVER_H_
#define MICROPHONE_DRIVER_H_

/*
 * ===== CIFRADO INTERNACIONAL =====
 * Nota		Cifrado
 * ----------------
 * Do		C
 * Re		D
 * Mi		E
 * Fa		F
 * Sol		G
 * La		A
 * Si		B
 *
 *
 * Nota		Octava		Cifrado
 * ----------------------------
 * Mi		Cuarta		E4
 * Si		Tercera		B3
 * Sol		Tercera		G3
 * Re		Tercera		D3
 * La		Segunda		A2
 * Mi		Segunda		E2
 */

/* Frecuencias fundamentales de las distintas notas de cada
 * unas de las cuerdas de la guitarra
 */

#define	FREC_E4 	329.6	// Cuerda N°1 -> Cuerda inferior
#define	FREC_B3 	246.9	// Cuerda N°2
#define	FREC_G3 	196		// Cuerda N°3
#define	FREC_D3 	146.8	// Cuerda N°4
#define	FREC_A2		110		// Cuerda N°5
#define FREC_E2 	82.4	// Cuerda N°6 -> Cuerda Superior

/*
 * Lista para seleccionar la nota a afinar
 */
enum{
	E4 = 1,
	B3,
	G3,
	D3,
	A2,
	E2
};

/* Límites superior e inferior de frecuencias para considerar
 * cula es la cuerda que se está afinando
 */
enum{
	LIM_SUPERIOR_E4 = 371,				// Cuerda N°1 -> Cuerda inferior
	LIM_INFERIOR_E4 = 288,

	LIM_SUPERIOR_B3 = LIM_INFERIOR_E4,	// Cuerda N°2
	LIM_INFERIOR_B3 = 221,

	LIM_SUPERIOR_G3 = LIM_INFERIOR_B3,	// Cuerda N°3
	LIM_INFERIOR_G3 = 171,

	LIM_SUPERIOR_D3 = LIM_INFERIOR_G3,	// Cuerda N°4
	LIM_INFERIOR_D3 = 128,

	LIM_SUPERIOR_A2 = LIM_INFERIOR_D3,	// Cuerda N°5
	LIM_INFERIOR_A2 = 96,

	LIM_SUPERIOR_E2 = LIM_INFERIOR_A2,	// Cuerda N°6 -> Cuerda Superior
	LIM_INFERIOR_E2 = 67
};



#endif /* MICROPHONE_DRIVER_H_ */
