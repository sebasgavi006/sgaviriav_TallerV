/*
 * pll_driver_hal.h
 *
 *  Created on: Nov 17, 2023
 *      Author: sgaviriav
 */

#ifndef PLL_DRIVER_HAL_H_
#define PLL_DRIVER_HAL_H_



#define MAIN_CLOCK

/* Headers de las funciones publicas del PLL */
void pll_Config_100MHz(void);
void pll_Config_MCO1(uint8_t prescalerMCO, uint8_t channelMCO);
uint8_t pll_Get_MainClock(void);


#endif /* PLL_DRIVER_HAL_H_ */
