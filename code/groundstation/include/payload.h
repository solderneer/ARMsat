/*
 * payload.h
 *
 *  Created on: 8 Feb 2017
 *      Author: sidxb
 */

#ifndef PAYLOAD_H_
#define PAYLOAD_H_

#include "stm32f4xx_hal.h"
#include "main.h"

#define PAYLOAD_SANITY_CHECK1 0xcafe
#define PAYLOAD_SANITY_CHECK2 0xbabe

typedef enum {
	TEMPERATURE,
	HUMIDITY,
	PRESSURE,
	ALTITUDE,
	X,
	Y,
	Z,
	SERVO_1,
	SERVO_2,
	COMMAND,
	STATUS
} payload_type_t;

typedef struct {
	uint16_t sanity_check1;
	int16_t humidity;
	int16_t temperature;
	int32_t pressure;
	uint16_t altitude;
	float x;
	float y;
	float z;
	payload_type_t type;
	uint16_t sanity_check2;
} payload_data_t;

void decodePayload(uint8_t* payload, size_t len);
void encodePayload(uint8_t* buf, payload_data_t* data);

#endif /* PAYLOAD_H_ */
