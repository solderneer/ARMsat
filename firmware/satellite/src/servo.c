/*
 * servo.c
 *
 *  Created on: 2 Feb 2017
 *      Author: Siidheesh Theivasigamani
 */

#include "servo.h"

TIM_HandleTypeDef* htim;

void Servo_Init(TIM_HandleTypeDef* h, uint32_t initPanPos, uint32_t initTiltPos) {
	htim = h;
	HAL_TIM_PWM_Start(htim, SERVO_PAN);
	HAL_TIM_PWM_Start(htim, SERVO_TILT);

	Servo_write(SERVO_PAN, initPanPos);
	Servo_write(SERVO_TILT, initTiltPos);
}

int mapi(int x, int in_min, int in_max, int out_min, int out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Servo_write(uint32_t Channel, uint32_t pos) {
	//assert(Channel == SERVO_PAN_CHAN || Channel == SERVO_TILT_CHAN);
	int pwm = mapi(pos, 0, 179, 1751, 8079);
	__HAL_TIM_SetCompare(htim, Channel, pwm);
}
