/*
 * pressure.h
 *
 *  Created on: 30 Jan 2017
 *      Author: Siidheesh Theivasigamani
 */

#ifndef PRESSURE_H_
#define PRESSURE_H_

#include <math.h>
#include "stm32f4xx_hal.h"
#include "main.h"

#define BMP085_I2C_ADDRESS (0x77<<1)

extern void Pressure_Init(I2C_HandleTypeDef* h);
void Pressure_Calibration(void);
void Pressure_reqData(uint8_t addr, uint8_t* dsc, uint32_t size);
void Pressure_calculate(void);
void Pressure_getUnCompTemp(void);
void Pressure_getUnCompPressure(void);

extern void Pressure_get(void);
extern void Pressure_getUnCompTemp_postDelay(void);
extern void Pressure_getUnCompPressure_postDelay(void);

extern volatile uint32_t pressureDelayTicks;
extern uint32_t pressureDelayState;

extern int32_t pressure;
extern volatile uint32_t altitude;

#endif /* PRESSURE_H_ */
