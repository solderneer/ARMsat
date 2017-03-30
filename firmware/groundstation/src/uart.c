/*
 * uart.c
 *
 *  Created on: 29 Jan 2017
 *      Author: Siidheesh Theivasigamani
 */
#include "uart.h"

serial_ring_buffer_t xbrx = {{0}, 0, 0};
serial_ring_buffer_t pcrx = {{0}, 0, 0};

HAL_StatusTypeDef SID_UART_Receive_IT_Setup(UART_HandleTypeDef *huart)
{
	SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE);
}

void SID_UART_IRQHandler(UART_HandleTypeDef *huart) {
	uint32_t isrflags   = READ_REG(huart->Instance->SR);
	uint32_t cr1its     = READ_REG(huart->Instance->CR1);
	if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET)) {
		if(huart->Instance == USART6) {
			int i = (unsigned int) (xbrx.head + 1) & SERIAL_BUF_SIZEOP;
			if (i != xbrx.tail) {
				xbrx.buffer[xbrx.head] = (char)(huart->Instance->DR & 0x00FFU);
				xbrx.head = i;
			}
			return;
		} else if(huart->Instance == USART2) {
			int i = (unsigned int) (pcrx.head + 1) & SERIAL_BUF_SIZEOP;
			if (i != pcrx.tail) {
				pcrx.buffer[pcrx.head] = (char)(huart->Instance->DR & 0x00FFU);
				pcrx.head = i;
			}
			return;
		}
	}
}

uint16_t xbrx_peek(void) {
    if (xbrx.head == xbrx.tail) return BUF_EMPTY;
    return xbrx.buffer[xbrx.tail];
}

uint16_t xbrx_read(uint8_t *buf) {
	// if the head isn't ahead of the tail, we don't have any characters
	if (xbrx.head == xbrx.tail) return BUF_EMPTY;
	*buf = xbrx.buffer[xbrx.tail];
	xbrx.tail = (unsigned int) (xbrx.tail + 1) & SERIAL_BUF_SIZEOP; //fixed bug where & was somehow turned to %... thats why 1023 became 0... @11.21pm 24/5/2013
	return 0;
}

uint16_t pcrx_peek(void) {
    if (pcrx.head == pcrx.tail) return BUF_EMPTY;
    return pcrx.buffer[pcrx.tail];
}

uint16_t pcrx_read(uint8_t *buf) {
	// if the head isn't ahead of the tail, we don't have any characters
	if (pcrx.head == pcrx.tail) return BUF_EMPTY;
	*buf = pcrx.buffer[pcrx.tail];
	pcrx.tail = (unsigned int) (pcrx.tail + 1) & SERIAL_BUF_SIZEOP; //fixed bug where & was somehow turned to %... thats why 1023 became 0... @11.21pm 24/5/2013
	return 0;
}

HAL_StatusTypeDef SID_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	UNUSED(Timeout);
	/* Check that a Tx process is not already ongoing */
	if(huart->gState == HAL_UART_STATE_READY) {
		if((pData == NULL ) || (Size == 0U)) {
			return  HAL_ERROR;
		}
		/* Process Locked */
		__HAL_LOCK(huart);
		huart->ErrorCode = HAL_UART_ERROR_NONE;
		huart->gState = HAL_UART_STATE_BUSY_TX;
		huart->TxXferSize = Size;
		huart->TxXferCount = Size;
		while(huart->TxXferCount-- > 0U) {
			huart->Instance->DR = (*pData++ & (uint8_t)0xFFU);
		}
		/* At end of Tx process, restore huart->gState to Ready */
		huart->gState = HAL_UART_STATE_READY;
		/* Process Unlocked */
		__HAL_UNLOCK(huart);
		return HAL_OK;
	} else return HAL_BUSY;
}

HAL_StatusTypeDef SID_UART_TransmitChar(UART_HandleTypeDef *huart, uint8_t *pData)
{
	/* Check that a Tx process is not already ongoing */
	if(huart->gState == HAL_UART_STATE_READY) {
		if(pData == NULL) return  HAL_ERROR;
		__HAL_LOCK(huart);
		while((__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE) ? SET : RESET) == RESET);
		huart->Instance->DR = (*pData & (uint8_t)0xFFU);
		__HAL_UNLOCK(huart);
		return HAL_OK;
	} else return HAL_BUSY;
}

