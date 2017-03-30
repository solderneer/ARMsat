/**
 ******************************************************************************
 * File Name          : main.c
 * Description        : Main program body
 ******************************************************************************
 *
 * COPYRIGHT(c) 2017 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "adc.h"
#include "crc.h"
#include "i2c.h"
#include "usart.h"
#include "uart.h"
#include "gpio.h"
#include "lcd.h"
#include "pressure.h"
#include "hmc.h"
#include "xbee.h"
#include "payload.h"
#include "stdlib.h"

void SystemClock_Config(void);
void Error_Handler(void);

volatile uint32_t currTick = 0;
uint32_t lcdTick = 0;
uint32_t sensorTick = 0;
uint32_t reInitTick = 0;
uint32_t pcWriteTick = 0;
uint32_t secondTick = 0;

int humidity = 0;
int temperature = 0;

uint8_t lcdKey = ' ';
uint8_t newKey = ' ';

uint8_t cmdbuf = 0;

uint32_t vref = 1500; //1.2v nominal, temp-dependent
uint32_t vref_avg = 0;
uint32_t vref_i = 0;

int32_t joystick_x;
int32_t joystick_x_avg = 0;
uint32_t joystick_x_i = 0;

int32_t joystick_y;
int32_t joystick_y_avg = 0;
uint32_t joystick_y_i = 0;

GPIO_PinState joystick_btn;

uint32_t voltage_cell1 = 0;
uint32_t voltage_cell2 = 0;
uint32_t voltage_cell3 = 0;
uint32_t current = 0;
uint32_t dust_conc = 0;

int main(void)
{
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_ADC1_Init();
	MX_I2C1_Init();
	MX_USART2_UART_Init();
	MX_USART6_UART_Init();
	MX_CRC_Init();
	SID_UART_Receive_IT_Setup(&XBEE_UART);
	SID_UART_Receive_IT_Setup(&PC_UART);
	Xbee_init(&XBEE_UART);
	LCD_init(&hi2c1);
	Pressure_Init(&hi2c1);

	HAL_Delay(500); //for lcd

	currTick = HAL_GetTick();
	lcdTick = currTick;
	sensorTick = currTick;
	reInitTick = currTick;
	pcWriteTick = currTick;
	secondTick = currTick;

	while (1)
	{
		if(currTick - secondTick > 1000) {
			secondTick = currTick;
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		}
		if(!pressureDelayTicks && pressureDelayState) switch(pressureDelayState) {
		case 1:
			pressureDelayState++;
			Pressure_getUnCompTemp_postDelay();
			break;
		case 2:
			pressureDelayState++;
			Pressure_getUnCompPressure_postDelay();
			break;
		}
		if(currTick - sensorTick > 50) {
			sensorTick = currTick;
			Pressure_get();
			HMC_calculate();
			newKey = LCD_keypad();
			if(newKey != lcdKey) {
				switch(newKey) {
					case '1':
						HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
						break;
				}
				lcdKey = newKey;
			}
		}
		if(currTick - pcWriteTick > 1000) {
			pcWriteTick = currTick;
			uint8_t data[25] = {0xab,0xcd,
					((uint16_t)humidity)>>8,
					((uint16_t)humidity&0xff),
					((uint16_t)temperature>>8),
					((uint16_t)temperature&0xff),
					((uint32_t)pressure>>24),
					(((uint32_t)pressure)>>16)&0xff,
					(((uint32_t)pressure)>>8)&0xff,
					((uint32_t)pressure)&0xff,
					(uint16_t)altitude>>8,
					(uint16_t)altitude&0xff,
					(uint16_t)dust_conc>>8,
					(uint16_t)dust_conc&0xff,
					(uint16_t)smoothHeadingDegrees>>8,
					(uint16_t)smoothHeadingDegrees&0xff,
					((uint16_t)voltage_cell1>>8),
					((uint16_t)voltage_cell1&0xff),
					((uint16_t)voltage_cell2>>8),
					((uint16_t)voltage_cell2&0xff),
					((uint16_t)voltage_cell3>>8),
					((uint16_t)voltage_cell3&0xff),
					((uint16_t)current>>8),
					((uint16_t)current&0xff),
					0x55};
			HAL_UART_Transmit(&PC_UART, data, sizeof(data), 1000);
		}
		if(adc_available) {
			adc_available = 0;
			uint32_t val = HAL_ADC_GetValue(&hadc1);
			switch(adc_chan) {
			default:
			case ADC_CHANNEL_VREFINT:
				vref_avg += val;
				if(++vref_i>=1024) {
					vref = vref_avg>>10;
					vref_avg = 0;
					vref_i = 0;
				}
				adc_chan = ADC_JOYSTICK_X_CHAN;
				break;
			case ADC_JOYSTICK_X_CHAN:
				joystick_x_avg += val;
				if(++joystick_x_i>=16) {
					joystick_x = joystick_x_avg>>4;
					joystick_x_avg = 0;
					joystick_x_i = 0;
				}
				adc_chan = ADC_JOYSTICK_Y_CHAN;
				break;
			case ADC_JOYSTICK_Y_CHAN:
				joystick_y_avg += val;
				if(++joystick_y_i>=16) {
					joystick_y = joystick_y_avg>>4;
					joystick_y_avg = 0;
					joystick_y_i = 0;
				}
				adc_chan = ADC_CHANNEL_VREFINT;
				break;
			}
			ADC_ChannelConfTypeDef c = {adc_chan,1,ADC_SAMPLETIME_3CYCLES,0};
			HAL_ADC_ConfigChannel(&hadc1, &c);
			HAL_ADC_Start_IT(&hadc1);
		}
		if(joystick_btn != HAL_GPIO_ReadPin(JoystickBtn_Port, JoystickBtn_Pin)) {
			joystick_x = 0;
			joystick_y = 0;
			joystick_btn = HAL_GPIO_ReadPin(JoystickBtn_Port, JoystickBtn_Pin);
		}
		if(xbrx_available && xbrx_read(&cmdbuf) == 0) Xbee_rxStateMachine(cmdbuf);
		if(xbee_available) {
			uint8_t* payload = 0;
			size_t length = 0;
			if(xrx_read(&payload, &length) == 0) decodePayload(payload, length);
			free(payload);
		}
		if(pcrx_available && pcrx_read(&cmdbuf) == 0) switch(cmdbuf) {
			default:
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				break;
			case 'l':
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				break;
			case 'r':
				NVIC_SystemReset();
				break;
			case 28: //left
				joystick_x -= 10;
				break;
			case 29: //right
				joystick_x += 10;
				break;
			case 30: //up
				joystick_y += 10;
				break;
			case 31: //down
				joystick_y -= 10;
				break;
		}
	}
}

void decodePayload_callback(payload_data_rx* p) {
	temperature = (typeof(temperature))p->temperature;
	humidity = (typeof(humidity))p->humidity;
	pressure = (typeof(pressure))p->pressure;
	altitude = (typeof(altitude))p->altitude;
	normHMC.x = (typeof(normHMC.x))p->hmc_x;
	normHMC.y = (typeof(normHMC.y))p->hmc_y;
	normHMC.z = (typeof(normHMC.z))p->hmc_z;
	dust_conc = (typeof(dust_conc))p->dust_conc;
	voltage_cell1 = (typeof(voltage_cell1))p->voltage_cell1;
	voltage_cell2 = (typeof(voltage_cell2))p->voltage_cell2;
	voltage_cell3 = (typeof(voltage_cell3))p->voltage_cell3;
	current = (typeof(current))p->current;
}

void HAL_SYSTICK_Callback(void) {
	currTick = HAL_GetTick();
	if(pressureDelayTicks) pressureDelayTicks--;
}


/** System Clock Configuration
 */
void SystemClock_Config(void)
{

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	/**Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}

	/**Configure the Systick interrupt time
	 */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/**Configure the Systick
	 */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler */
	/* User can add his own implementation to report the HAL error return state */
	while(1)
	{
	}
	/* USER CODE END Error_Handler */
}
/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
