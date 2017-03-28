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
/*
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
*/

typedef enum {
	SENSORS = 1,
	VC_MONITOR = 2,
	CONTROL = 3
} payload_type_t;

typedef __attribute__((packed)) struct {
	uint16_t sanity_check1;
	payload_type_t type;
	//union {
		//struct {
			int16_t humidity;
			int16_t temperature;
			int32_t pressure;
			uint16_t altitude;
			uint16_t particulate_matter;
			uint16_t ozone_concentration;
			uint16_t wind_speed;
			float hmc_x;
			float hmc_y;
			float hmc_z;
			char keypad;
		//} sensors;
		//struct {
			uint16_t voltage_3v3;
			uint16_t voltage_5v;
			uint16_t current;
			uint16_t voltage_cell1;
			uint16_t voltage_cell2;
			uint16_t voltage_cell3;
			uint16_t voltage_batt;
		//} vc;
		//struct {
			uint16_t generic_cmd;
			uint16_t pan_pos;
			uint16_t tilt_pos;
		//} control;
	//} payload;
	uint16_t crc;
	uint16_t sanity_check2;
} payload_data_t;

void decodePayload(uint8_t* payload, size_t len);
void encodePayload(uint8_t* buf, payload_data_t* data);

#endif /* PAYLOAD_H_ */
