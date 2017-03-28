/*
 * xbee.h
 *
 *  Created on: 30 Jan 2017
 *      Author: Siidheesh Theivasigamani
 */

#ifndef XBEE_H_
#define XBEE_H_

#include "stm32f4xx_hal.h"
#include "main.h"

#define START_BYTE 0x7e
#define ESCAPE 0x7d
#define XON 0x11
#define XOFF 0x13

#define XBEE_FRAMEBUF_MAX_SIZE 0xff
#define XBEE_PAYLOAD_MAX_SIZE (XBEE_FRAMEBUF_MAX_SIZE>>1)

#define XBEE_RX_BUFFER_SIZE 128
#define XBEE_RX_BUF_SIZEOP (XBEE_RX_BUFFER_SIZE - 1)//1023

#define xbee_available ((XBEE_RX_BUFFER_SIZE+xbeerx.head-xbeerx.tail) & XBEE_RX_BUF_SIZEOP)
#define xbee_empty (xbeerx.head == xbeerx.tail)

#ifdef ASSIGN_SYS1
#define XBEE_COORD_ADDR				0x0013a20040b9d07e
#else
#define XBEE_COORD_ADDR				0x0013a20040a9dc0a
#endif
#define XBEE_BROADCAST_ADDR 		0x000000000000FFFF
#define XBEE_BROADCAST_NETWORK_ADDR 0xFFFE

typedef struct {
    uint8_t* buffer[XBEE_RX_BUFFER_SIZE];
    uint16_t lenbuf[XBEE_RX_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
} xbee_rx_buffer_t;

typedef struct {
	union {
		uint16_t word;
		struct {
			uint8_t _0;
			uint8_t _1;
		};
	} length;
	uint8_t frameType;
	uint8_t frameId;
	union {
		uint64_t word;
		struct {
			uint8_t _0;
			uint8_t _1;
			uint8_t _2;
			uint8_t _3;
			uint8_t _4;
			uint8_t _5;
			uint8_t _6;
			uint8_t _7;
		};
	} destAddr;
	union {
		uint16_t word;
		struct {
			uint8_t _0;
			uint8_t _1;
		};
	} destNetworkAddr;
	uint8_t broadcastRadius;
	uint8_t options;
	uint8_t* payload;
	size_t payloadSize;
	uint8_t checksum;
	uint8_t* frameBuffer;
	size_t frameBufferSize;
} Xbee_Tx_Packet;

typedef struct {
	union {
		uint16_t word;
		struct {
			uint8_t _0;
			uint8_t _1;
		};
	} length;
	uint8_t frameType;
	union {
		uint64_t word;
		struct {
			uint8_t _0;
			uint8_t _1;
			uint8_t _2;
			uint8_t _3;
			uint8_t _4;
			uint8_t _5;
			uint8_t _6;
			uint8_t _7;
		};
	} srcAddr;
	union {
		uint16_t word;
		struct {
			uint8_t _0;
			uint8_t _1;
		};
	} srcNetworkAddr;
	uint8_t options;
	uint8_t payload[XBEE_PAYLOAD_MAX_SIZE];
	uint32_t payloadSize;
	uint8_t checksum;
	uint8_t frameBuffer[XBEE_FRAMEBUF_MAX_SIZE];
	uint32_t frameBufferSize;
} Xbee_Rx_Packet;


typedef enum {
	XBEE_RX_IDLE,
	XBEE_FRAME_START,
	XBEE_LENGTH_1,
	XBEE_LENGTH_2,
	XBEE_FRAME_TYPE,
	XBEE_ADDR_7,
	XBEE_ADDR_6,
	XBEE_ADDR_5,
	XBEE_ADDR_4,
	XBEE_ADDR_3,
	XBEE_ADDR_2,
	XBEE_ADDR_1,
	XBEE_ADDR_0,
	XBEE_NET_ADDR_1,
	XBEE_NET_ADDR_0,
	XBEE_RX_OPT,
	XBEE_RX_DATA,
	XBEE_CHECKSUM
} xbee_rx_state;

extern void Xbee_init(UART_HandleTypeDef* h);
void Xbee_checksum(void);
void Xbee_prepareTxFrame(void);
void Xbee_createTxPacket(uint8_t* payload, size_t payloadSize);
void Xbee_sendTxPacket(void);
void Xbee_sendPayload(uint8_t* payload, size_t payloadSize);
void Xbee_rxStateMachine(uint8_t cmdbuf);
uint16_t xrx_read(uint8_t **buf, size_t* len);

xbee_rx_buffer_t xbeerx;

#endif /* XBEE_H_ */
