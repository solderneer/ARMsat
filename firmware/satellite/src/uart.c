/*
 * uart.c
 *
 *  Created on: 29 Jan 2017
 *      Author: Siidheesh Theivasigamani
 */
#include "uart.h"

serial_ring_buffer_t rx = {{0}, 0, 0};

volatile uint16_t rx_head = 0;
volatile uint16_t tx_head = 0;
volatile uint16_t rx_timeout = 0;
volatile uint16_t tx_timeout = 0;

HAL_StatusTypeDef SID_UART_Receive_IT_Setup(UART_HandleTypeDef *huart)
{
	SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE);
}

void SID_UART_IRQHandler(UART_HandleTypeDef *huart) {
	uint32_t isrflags   = READ_REG(huart->Instance->SR);
	uint32_t cr1its     = READ_REG(huart->Instance->CR1);
	if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET)) {
		if(huart->Instance == USART6) {
			int i = (unsigned int) (rx.head + 1) & SERIAL_BUF_SIZEOP;
			if (i != rx.tail) {
				rx.buffer[rx.head] = (char)(huart->Instance->DR & 0x00FFU);
				rx.head = i;
			}
			return;
		}
	}
}

void SID_UART_IRQHandler_old(UART_HandleTypeDef *huart) {

	 uint32_t isrflags   = READ_REG(huart->Instance->SR);
	 uint32_t cr1its     = READ_REG(huart->Instance->CR1);
	 uint32_t cr3its     = READ_REG(huart->Instance->CR3);
	 uint32_t errorflags = 0x00U;
	 uint32_t dmarequest = 0x00U;

	 /* If no error occurs */
	 errorflags = (isrflags & (uint32_t)(USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE));
	 if(errorflags == RESET)
	 {
		 /* UART in mode Receiver -------------------------------------------------*/
		 if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
		 {
			 SID_UART_Receive_IT(huart);
			 return;
		 }
	 }

	 /* If some errors occur */
	 if((errorflags != RESET) && (((cr3its & USART_CR3_EIE) != RESET) || ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)))
	 {
		 /* UART parity error interrupt occurred ----------------------------------*/
		 if(((isrflags & USART_SR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
		 {
			 huart->ErrorCode |= HAL_UART_ERROR_PE;
		 }

		 /* UART noise error interrupt occurred -----------------------------------*/
		 if(((isrflags & USART_SR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		 {
			 huart->ErrorCode |= HAL_UART_ERROR_NE;
		 }

		 /* UART frame error interrupt occurred -----------------------------------*/
		 if(((isrflags & USART_SR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		 {
			 huart->ErrorCode |= HAL_UART_ERROR_FE;
		 }

		 /* UART Over-Run interrupt occurred --------------------------------------*/
		 if(((isrflags & USART_SR_ORE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		 {
			 huart->ErrorCode |= HAL_UART_ERROR_ORE;
		 }

		 /* Call UART Error Call back function if need be --------------------------*/
		 if(huart->ErrorCode != HAL_UART_ERROR_NONE)
		 {
			 /* UART in mode Receiver -----------------------------------------------*/
			 if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
			 {
				 SID_UART_Receive_IT(huart);
			 }

			 /* If Overrun error occurs, or if any error occurs in DMA mode reception,
	         consider error as blocking */
			 dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR);
			 if(((huart->ErrorCode & HAL_UART_ERROR_ORE) != RESET) || dmarequest)
			 {
				 /* Blocking error : transfer is aborted
	           Set the UART state ready to be able to start again the process,
	           Disable Rx Interrupts, and disable Rx DMA request, if ongoing */
				 HAL_UART_ErrorCallback(huart);
			 }
			 else
			 {
				 /* Non Blocking error : transfer could go on.
	           Error is notified to user through user error callback */
				 HAL_UART_ErrorCallback(huart);
				 huart->ErrorCode = HAL_UART_ERROR_NONE;
			 }
		 }
		 return;
	 } /* End if some error occurs */

	 /* UART in mode Transmitter ------------------------------------------------*/
	 if(((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
	 {
		 //SID_UART_Transmit_IT(huart);
		 return;
	 }

	 /* UART in mode Transmitter end --------------------------------------------*/
	 if(((isrflags & USART_SR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	 {
		 //SID_UART_EndTransmit_IT(huart);
		 return;
	 }
}

 HAL_StatusTypeDef SID_UART_Receive_IT(UART_HandleTypeDef *huart)
 {
	 if(huart->Instance != USART6) goto rx_end;
	 if(huart->RxState == HAL_UART_STATE_BUSY_RX) {
		 char recv = (uint_fast8_t)(huart->Instance->DR & (uint8_t)0x00FFU);
		 int i = (unsigned int) (rx.head + 1) & SERIAL_BUF_SIZEOP;
		 if (i != rx.tail) {
			 rx.buffer[rx.head] = recv;
			 rx.head = i;
		 }
		 return HAL_OK;
	 }
	 rx_end:
	 return HAL_BUSY;
 }

/*
 static HAL_StatusTypeDef SID_UART_Transmit_IT(UART_HandleTypeDef *huart)
 {
	 if(huart->gState == HAL_UART_STATE_BUSY_TX)
	 {
		 huart->Instance->DR = (uint8_t)(*huart->pTxBuffPtr++ & (uint8_t)0x00FFU);
		 if(--huart->TxXferCount == 0U)
		 {
			 CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);

			 SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);
		 }
		 return HAL_OK;
	 }
	 else return HAL_BUSY;
 }

 static HAL_StatusTypeDef SID_UART_EndTransmit_IT(UART_HandleTypeDef *huart)
 {
	 CLEAR_BIT(huart->Instance->CR1, USART_CR1_TCIE);
	 huart->gState = HAL_UART_STATE_READY;
	 HAL_UART_TxCpltCallback(huart);
	 return HAL_OK;
 }
*/

uint16_t rx_peek(void) {
    if (rx.head == rx.tail) return BUF_EMPTY;
    return rx.buffer[rx.tail];
}

uint16_t tx_peek(void) {
    //if (tx.head == tx.tail) return BUF_EMPTY;
    //return tx.buffer[tx.tail];
	return 0;
}

uint16_t rx_read(uint8_t *buf) {
	// if the head isn't ahead of the tail, we don't have any characters
	if (rx.head == rx.tail) return BUF_EMPTY;
	*buf = rx.buffer[rx.tail];
	rx.tail = (unsigned int) (rx.tail + 1) & SERIAL_BUF_SIZEOP; //fixed bug where & was somehow turned to %... thats why 1023 became 0... @11.21pm 24/5/2013
	return 0;
}

uint16_t tx_read(uint8_t *buf) {
	UNUSED(buf);
    // if the head isn't ahead of the tail, we don't have any characters
    //if (tx.head == tx.tail) return BUF_EMPTY;
    //*buf = tx.buffer[tx.tail];
    //tx.tail = (unsigned int) (tx.tail + 1) & SERIAL_BUF_SIZEOP; //fixed bug where & was somehow turned to %... thats why 1023 became 0... @11.21pm 24/5/2013
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

