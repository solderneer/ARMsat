/*
 * xbee.c
 *
 *  Created on: 30 Jan 2017
 *      Author: Siidheesh Theivasigamani
 */
#include "xbee.h"
#include "stdlib.h"

UART_HandleTypeDef* huart;

xbee_rx_buffer_t xbeerx = {{0}, {0}, 0, 0};

Xbee_Tx_Packet p;
uint8_t xbee_framebuf[XBEE_FRAMEBUF_MAX_SIZE] = {0};

Xbee_Rx_Packet rp;
xbee_rx_state state = XBEE_RX_IDLE;
uint8_t rx_length_expected = 0;
uint8_t rx_length = 0;
uint8_t rx_escape_flag = 0;
uint8_t rx_checksum = 0;

void Xbee_init(UART_HandleTypeDef* h) {
	huart = h;
}

void Xbee_prepareTxFrame(void) {
	p.length.word = 14 + p.payloadSize;
	if(p.length.word + 4 > XBEE_FRAMEBUF_MAX_SIZE) return;

	p.frameBufferSize = p.length.word + 4;
	p.frameBuffer[0] = START_BYTE;
	p.frameBuffer[1] = p.length._1;
	p.frameBuffer[2] = p.length._0;
	p.frameBuffer[3] = p.frameType;
	p.frameBuffer[4] = p.frameId;
	p.frameBuffer[5] = p.destAddr._7;
	p.frameBuffer[6] = p.destAddr._6;
	p.frameBuffer[7] = p.destAddr._5;
	p.frameBuffer[8] = p.destAddr._4;
	p.frameBuffer[9] = p.destAddr._3;
	p.frameBuffer[10] = p.destAddr._2;
	p.frameBuffer[11] = p.destAddr._1;
	p.frameBuffer[12] = p.destAddr._0;
	p.frameBuffer[13] = p.destNetworkAddr._1;
	p.frameBuffer[14] = p.destNetworkAddr._0;
	p.frameBuffer[15] = p.broadcastRadius;
	p.frameBuffer[16] = p.options;
	for(size_t i = 0; i < p.payloadSize; i++) p.frameBuffer[17+i] = p.payload[i];
	Xbee_checksum();
}

void Xbee_checksum(void) {
	uint32_t checksum = 0;
	for(size_t i = 3; i < p.frameBufferSize - 1; i++) {
		checksum += p.frameBuffer[i];
	}
	p.frameBuffer[p.frameBufferSize-1] = (0xff - (checksum&0xff)) & 0xff;
}

void Xbee_sendTxPacket(void) {
	uint8_t xbee_buf[XBEE_FRAMEBUF_MAX_SIZE] = {0};
	uint8_t* ptr = (uint8_t*)&xbee_buf;
	int new_size = p.frameBufferSize;
	*ptr++ = p.frameBuffer[0];
	for(size_t i = 1; i < p.frameBufferSize; i++) {
		if (1 && (p.frameBuffer[i] == START_BYTE || p.frameBuffer[i] == ESCAPE || p.frameBuffer[i] == XON || p.frameBuffer[i] == XOFF)) {
			*ptr++ = ESCAPE;
			*ptr++ = p.frameBuffer[i] ^ 0x20;
			new_size++;
		} else *ptr++ = p.frameBuffer[i];
	}
	HAL_UART_Transmit(huart, (uint8_t*)&xbee_buf, new_size, 0xffff);
}

void Xbee_createTxPacket(uint8_t* payload, size_t payloadSize) {
	p.frameType = 0x10;
	p.frameId = 0;
	p.destAddr.word = XBEE_COORD_ADDR;
	p.destNetworkAddr.word = XBEE_BROADCAST_NETWORK_ADDR;
	p.broadcastRadius = 0;
	p.options = 0;
	p.payload = payload;
	p.payloadSize = payloadSize;
	p.frameBuffer = xbee_framebuf;
	p.frameBufferSize = 0;
}

void Xbee_sendPayload(uint8_t* payload, size_t payloadSize) {
	Xbee_createTxPacket(payload, payloadSize);
	Xbee_prepareTxFrame();
	Xbee_sendTxPacket();
}

void Xbee_rxStateMachine(uint8_t cmd) {
	if(cmd == START_BYTE) {
		state = XBEE_FRAME_START;
		return;
	} else if(cmd == ESCAPE) {
		rx_escape_flag = 1;
		return;
	}
	if(rx_escape_flag) {
		rx_escape_flag = 0;
		cmd ^= 0x20;
	}
	switch(state) {
	case XBEE_RX_IDLE:
		if(cmd == START_BYTE) state = XBEE_FRAME_START;
		break;
	case XBEE_FRAME_START:
		rp.length._1 = cmd;
		state = XBEE_LENGTH_1;
		break;
	case XBEE_LENGTH_1:
		rp.length._0 = cmd;
		state = XBEE_LENGTH_2;
		break;
	case XBEE_LENGTH_2:
		rp.frameType = cmd;
		rx_checksum = cmd & 0xff;
		if(rp.frameType == 0x90)
			state = XBEE_FRAME_TYPE;
		else
			state = XBEE_RX_IDLE;
		break;
	case XBEE_FRAME_TYPE:
		rp.srcAddr._7 = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_ADDR_7;
		break;
	case XBEE_ADDR_7:
		rp.srcAddr._6 = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_ADDR_6;
		break;
	case XBEE_ADDR_6:
		rp.srcAddr._5 = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_ADDR_5;
		break;
	case XBEE_ADDR_5:
		rp.srcAddr._4 = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_ADDR_4;
		break;
	case XBEE_ADDR_4:
		rp.srcAddr._3 = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_ADDR_3;
		break;
	case XBEE_ADDR_3:
		rp.srcAddr._2 = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_ADDR_2;
		break;
	case XBEE_ADDR_2:
		rp.srcAddr._1 = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_ADDR_1;
		break;
	case XBEE_ADDR_1:
		rp.srcAddr._0 = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_ADDR_0;
		break;
	case XBEE_ADDR_0:
		rp.srcNetworkAddr._1 = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_NET_ADDR_1;
		break;
	case XBEE_NET_ADDR_1:
		rp.srcNetworkAddr._0 = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_NET_ADDR_0;
		break;
	case XBEE_NET_ADDR_0:
		rp.options = cmd;
		rx_checksum += cmd & 0xff;
		state = XBEE_RX_OPT;
		rx_length = 0;
		rx_length_expected = rp.length.word - 12;
		break;
	case XBEE_RX_OPT:
	case XBEE_RX_DATA:
		rp.payload[rx_length++] = cmd;
		rx_checksum += cmd & 0xff;
		if(rx_length == rx_length_expected) state = XBEE_CHECKSUM;
		break;
	case XBEE_CHECKSUM: {
			uint8_t check = (uint8_t)(0xffu - (rx_checksum & 0xffu)) & 0xffu;
			if(check == cmd) {
				uint8_t* buf = (uint8_t*)malloc(rx_length);
				for(int i = 0; i < rx_length; i++) buf[i] = rp.payload[i];
				int i = (unsigned int) (xbeerx.head + 1) & XBEE_RX_BUF_SIZEOP;
				if (i != xbeerx.tail) {
					xbeerx.buffer[xbeerx.head] = buf;
					xbeerx.lenbuf[xbeerx.head] = rx_length;
					xbeerx.head = i;
				}
			}
		}
		state = XBEE_RX_IDLE;
		break;
	}
}

uint16_t xrx_read(uint8_t **buf, size_t* len) {
	// if the head isn't ahead of the tail, we don't have any characters
	if (xbeerx.head == xbeerx.tail) return 0x100;
	*buf = xbeerx.buffer[xbeerx.tail];
	*len = xbeerx.lenbuf[xbeerx.tail];
	xbeerx.tail = (unsigned int) (xbeerx.tail + 1) & XBEE_RX_BUF_SIZEOP; //fixed bug where & was somehow turned to %... thats why 1023 became 0... @11.21pm 24/5/2013
	return 0;
}
