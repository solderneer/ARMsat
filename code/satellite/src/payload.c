/*
 * payload.c
 *
 *  Created on: 8 Feb 2017
 *      Author: sidxb
 */

#include "payload.h"


void decodePayload(uint8_t* payload, size_t len) {
	if(len == sizeof(payload_data_t)) {
		payload_data_t data;
		uint8_t* ptr = (uint8_t*)&data;
		while(len--) *ptr++ = *payload++;
		if(data.sanity_check1 == PAYLOAD_SANITY_CHECK1 && data.sanity_check2 == PAYLOAD_SANITY_CHECK2) {
			//trace_printf("payload.temperature: %d.%02uC\r\n", data.temperature/100, data.temperature%100);
			decodePayload_callback(&data);
		}
	}
}

void encodePayload(uint8_t* buf, payload_data_t* data) {
	data->sanity_check1 = PAYLOAD_SANITY_CHECK1;
	data->sanity_check2 = PAYLOAD_SANITY_CHECK2;
	uint8_t* ptr = (uint8_t*)data;
	for(int i = 0; i < sizeof(payload_data_t); i++) *buf++ = *ptr++;
}
