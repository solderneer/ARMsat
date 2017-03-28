/*
 * servo.h
 *
 *  Created on: 2 Feb 2017
 *      Author: Siidheesh Theivasigamani
 */

#ifndef SERVO_H_
#define SERVO_H_

#include "stm32f4xx_hal.h"
#include "main.h"

#define SERVO_PAN_CHAN TIM_CHANNEL_1
#define SERVO_TILT_CHAN TIM_CHANNEL_2

void Servo_Init(TIM_HandleTypeDef* h, uint32_t initPanPos, uint32_t initTiltPos);
void Servo_write(uint32_t Channel, uint32_t pos);

#endif /* SERVO_H_ */
