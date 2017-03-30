/*
 * payload.c
 *
 *  Created on: 8 Feb 2017
 *      Author: Siidheesh Theivasigamani
 */

#include "payload.h"


void decodePayload(uint8_t* payload, size_t len) {
	if(len == sizeof(payload_data_rx)) {
		payload_data_rx data;
		uint8_t* ptr = (uint8_t*)&data;
		while(len--) *ptr++ = *payload++;
		if(data.sanity_check1 == PAYLOAD_RX_SANITY_CHECK1 && data.sanity_check2 == PAYLOAD_RX_SANITY_CHECK2) {
			decodePayload_callback(&data);
		}
	}
}

void encodePayload(uint8_t* buf, payload_data_tx* data) {
	data->sanity_check1 = PAYLOAD_TX_SANITY_CHECK1;
	data->sanity_check2 = PAYLOAD_TX_SANITY_CHECK2;
	uint8_t* ptr = (uint8_t*)data;
	for(int i = 0; i < sizeof(payload_data_tx); i++) *buf++ = *ptr++;
}
