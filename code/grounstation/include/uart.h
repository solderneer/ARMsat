#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"

#define SERIAL_BUFFER_SIZE 1024
#define SERIAL_BUF_SIZEOP (SERIAL_BUFFER_SIZE - 1)//1023

#define rx_available ((SERIAL_BUFFER_SIZE+rx.head-rx.tail) & SERIAL_BUF_SIZEOP)
#define tx_available ((SERIAL_BUFFER_SIZE+tx.head-tx.tail) & SERIAL_BUF_SIZEOP)
#define rx_empty (pcrx.head == pcrx.tail)
#define tx_empty (camrx.head == camrx.tail)

#define BUF_EMPTY 0x100

typedef struct {
    char buffer[SERIAL_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
} serial_ring_buffer_t;

extern serial_ring_buffer_t rx;

extern void SID_UART_IRQHandler(UART_HandleTypeDef *huart);
HAL_StatusTypeDef SID_UART_Receive_IT_Setup(UART_HandleTypeDef *huart);
HAL_StatusTypeDef SID_UART_Receive_IT(UART_HandleTypeDef *huart);
HAL_StatusTypeDef SID_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef SID_UART_TransmitChar(UART_HandleTypeDef *huart, uint8_t *pData);

uint16_t rx_peek(void);
uint16_t tx_peek(void);
uint16_t rx_read(uint8_t *buf);
uint16_t tx_read(uint8_t *buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32F4xx_H */
