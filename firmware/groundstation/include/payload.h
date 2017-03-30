/*
 * payload.h
 *
 *  Created on: 8 Feb 2017
 *      Author: Siidheesh Theivasigamani
 */

#ifndef PAYLOAD_H_
#define PAYLOAD_H_

#include "stm32f4xx_hal.h"
#include "main.h"

#define PAYLOAD_TX_SANITY_CHECK1 0xcafe
#define PAYLOAD_TX_SANITY_CHECK2 0xbabe
#define PAYLOAD_RX_SANITY_CHECK1 0xcafe
#define PAYLOAD_RX_SANITY_CHECK2 0xbabe

typedef struct {
	uint16_t sanity_check1;
	int16_t humidity;
	int16_t temperature;
	int32_t pressure;
	uint16_t altitude;
	uint16_t dust_conc;
	uint16_t wind_speed;
	float hmc_x;
	float hmc_y;
	float hmc_z;
	uint16_t current;
	uint16_t voltage_cell1;
	uint16_t voltage_cell2;
	uint16_t voltage_cell3;
	uint16_t sanity_check2;
} payload_data_rx;

typedef struct {
	uint16_t sanity_check1;
	uint16_t generic_cmd;
	uint16_t pan_pos;
	uint16_t tilt_pos;
	uint16_t sanity_check2;
} payload_data_tx;

void decodePayload(uint8_t* payload, size_t len);
void encodePayload(uint8_t* buf, payload_data_tx* data);

#endif /* PAYLOAD_H_ */
