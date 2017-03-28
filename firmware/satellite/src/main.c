/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "adc.h"
#include "crc.h"
#include "fatfs.h"
#include "i2c.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "uart.h"
#include "gpio.h"
#include "pressure.h"
#include "lcd.h"
#include "xbee.h"
#include "hmc.h"
#include "payload.h"
#include "stdlib.h"
#include "diag/Trace.h"
#include "mpu6050.h"

volatile uint32_t currTick = 0;
uint32_t adcTick = 0;
uint32_t lcdTick = 0;
uint32_t sensorTick = 0;
uint32_t pwmTick = 0;
uint32_t rxTick = 0;
uint32_t xbeeTick = 0;
uint32_t reInitTick = 0;
uint32_t pcWriteTick = 0;
uint32_t secondTick = 0;

int humidity = 0;
int temperature = 0;

uint32_t pwm = 500;
uint8_t lcdKey = ' ';
uint8_t newKey = ' ';

uint8_t cmdbuf = 0;

MPU6050_t* mpud = {0};

void SystemClock_Config(void);
void Error_Handler(void);
void decodePayload_callback(payload_data_t* p);

uint8_t tempbuf[30] = {0};
int rx_index = 0;

int main(void)
{

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_CRC_Init();
  MX_I2C1_Init();
  MX_SDIO_SD_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_FATFS_Init();
  MX_TIM10_Init();

  HAL_Delay(500); //wait 0.5 sec for lcd display to turn on

  SID_UART_Receive_IT_Setup(&huart6);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  pwm = 1783;
  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 1783);

  Xbee_init(&XBEE_UART, &hcrc);

  LCD_init(&hi2c1);

  MPU6050_Init(mpud, MPU6050_Device_0, MPU6050_Accelerometer_2G, MPU6050_Gyroscope_250s, &hi2c1);

#ifdef XBEE_TX_END
#ifdef ASSIGN_SYS2
  HAL_ADC_Start_IT(&hadc1);
  Pressure_Init(&hi2c1);
#else
  HMC_init(&hi2c1);
#endif
#endif

  currTick = HAL_GetTick();
  pwmTick = currTick;
  lcdTick = currTick;
  sensorTick = currTick;
  rxTick = currTick;
  xbeeTick = currTick;
  reInitTick = currTick;
  pcWriteTick = currTick;
  secondTick = currTick;

  GPIO_PinState btn;
  UNUSED(btn);

#ifdef _DEBUG
  trace_puts("Hello!");

  payload_data_t data;/*
  data.sanity_check1 = PAYLOAD_SANITY_CHECK1;
  data.sanity_check2 = PAYLOAD_SANITY_CHECK2;
  data.temperature = 0x1234;
  data.pressure = 0x5678;
  data.humidity = 0x9abc;
  data.altitude = 0xde;
  data.x = 1;
  data.y = 2;
  data.z = 3;
  uint8_t* ptr = (uint8_t*)&data;
  for(int i = 0; i < sizeof(data); i++) trace_printf("%02X ", *ptr++);
  trace_putchar('\n');
 */
  /*uint8_t arr[] = {0xFE, 0xCA, 0x9D, 0x0C, 0x22, 0x0C, 0x78, 0x57, 0x01, 0x88, 0x01, 0x00, 0x51, 0x00, 0x35, 0x7D, 0xAE, 0x47, 0x29, 0xC2, 0x9A, 0x19, 0x03, 0xC4, 0x29, 0x5C, 0x11, 0xC3, 0xBE, 0xBA, 0x38, 0x3D};
  uint8_t* ptr = (uint8_t*)&data;
  for(int i = 0; i < sizeof(data); i++) *ptr++ = arr[i];
  trace_printf("data.temperature: %d.%02uC  ", data.temperature/100, data.temperature%100);*/

#endif
  trace_printf("sizeof payload_data_t = %d bytes\n", sizeof(payload_data_t));

  float a = -10.0f;
  int b = -10;
  trace_printf("-10.0f=%d -10=%d\n",(int)a,b);

  for(;;)
  {
	  btn = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
	  if(currTick - secondTick > 1000) {
		  secondTick = currTick;
		  //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	  }
#ifdef ASSIGN_SYS2
	  if(!pressureDelayTicks && pressureDelayState)
		  switch(pressureDelayState) {
			  case 1:
				  pressureDelayState++;
				  Pressure_getUnCompTemp_postDelay();
				  break;
			  case 2:
				  pressureDelayState++;
				  Pressure_getUnCompPressure_postDelay();
				  break;
		  }
#endif
	  /*if(currTick - pwmTick > 1) {
		  pwmTick = currTick;
		  if(pwm >= 7864)
			  pwm = 1783;
		  else
			  pwm += 1;
		  //__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, pwm);
	  }*/
	  if(currTick - lcdTick > 250) {
		  lcdTick = currTick;
		  switch(lcdFrame) {
			  case 0:
#ifdef ASSIGN_SYS1
				  LCD_updateSensors(humidity, temperature, pressure, altitude);
#else
				  LCD_updateSensors(normHMC, smoothHeadingDegrees);
#endif
				  break;
		  }
	  }
#ifdef XBEE_TX_END
	  if(currTick - xbeeTick > 100) {
		  xbeeTick = currTick;
		  payload_data_t data;
		  data.type = (payload_type_t)SENSORS;
#ifdef ASSIGN_SYS2
		  data.temperature = (typeof(data.temperature)) temperature;
		  data.pressure = (typeof(data.pressure)) pressure;
		  data.humidity = (typeof(data.humidity)) humidity;
		  data.altitude = (typeof(data.altitude)) altitude;
#else
		  data.hmc_x = (typeof(data.hmc_x)) normHMC.x;
		  data.hmc_y = (typeof(data.hmc_y)) normHMC.y;
		  data.hmc_z = (typeof(data.hmc_z)) normHMC.z;
#endif
		  data.crc = 0xd00d;
		  uint8_t buf[sizeof(payload_data_t)] = {0};
		  encodePayload((uint8_t*)&buf, &data);
		  Xbee_sendPayload((uint8_t*)&buf, sizeof(data));
		  //HAL_UART_Transmit(&XBEE_UART, (uint8_t*)&buf, sizeof(data), 0xffff);
	  }
#else
	  UNUSED(xbeeTick);
#endif
	  if(currTick - sensorTick > 50) {
		  sensorTick = currTick;
#ifdef XBEE_TX_END
#ifdef ASSIGN_SYS2
		  uint8_t tmp_data[2] = {0};
		  HAL_I2C_Master_Transmit(&hi2c1, TMP102_I2C_ADDRESS, (uint8_t*)&tmp_data, 1, 1000);
		  HAL_I2C_Master_Receive(&hi2c1, TMP102_I2C_ADDRESS, (uint8_t*)&tmp_data, 2, 1000);
		  int16_t val = 0;
		  val = tmp_data[0]<<4;
		  val |= tmp_data[1]>>4;
		  float convertedTemp = val * 0.0625f;
		  temperature = (int)(convertedTemp*100);
		  Pressure_get();
#endif
#ifdef ASSIGN_SYS1
		  HMC_calculate();
		  newKey = LCD_keypad();
#endif
		  if(newKey != lcdKey) {
			  switch(newKey) {
			  case '1':
				  NVIC_SystemReset();
				  break;
			  case '2':
				  break;
			  case '3':
				  lcdFrame++;
				  if(lcdFrame >= 3) lcdFrame = 0;
				  LCD_refreshFrame();
				  break;
			  case '#':
				  pwm += 0xfff;
				  pwm &= 0xffff;
				  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, pwm);
				  break;
			  case '4':
				  LCD_on();
				  break;
			  case '5':
				  LCD_off();
				  break;
			  case '7':
				  pwm = 1751;
				  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, pwm);
				  break;
			  case '8':
				  pwm = 4900;
				  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, pwm);
				  break;
			  case '9':
				  pwm = 8079;
				  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, pwm);
				  break;
			  case '*':
				  pwm += 16;
				  //if(pwm > 7864) pwm = 7864;
				  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, pwm);
				  break;
			  case '0':
				  pwm -= 16;
				  //if(pwm < 1783) pwm = 1783;
				  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, pwm);
				  break;
			  }
#ifdef ASSIGN_SYS1
			  payload_data_t data;
			  data.type = (payload_type_t)SENSORS;
			  data.keypad = (typeof(data.keypad))newKey;
			  uint8_t buf[sizeof(payload_data_t)] = {0};
			  encodePayload((uint8_t*)&buf, &data);
			  HAL_UART_Transmit(&XBEE_UART, (uint8_t*)&buf, sizeof(data), 1000);
			  Xbee_sendPayload((uint8_t*)&buf, sizeof(data));
#endif
			  lcdKey = newKey;
		  }
#endif //XBEE_TX_END
	  }
	  if(currTick - reInitTick > 20000) {
		  reInitTick = currTick;
		  LCD_init(&hi2c1);
	  }
#ifdef XBEE_TX_END
#ifdef ASSIGN_SYS2
	  //CURRENT SENSOR: (2.98-2.95)/(40/1000) = 0.75

	  if(adc_available) {
		  adc_available = 0;
		  float val = (float)adc_val;
		  HAL_ADC_Start_IT(&hadc1);
		  val /= 4096.0f;
		  val *= 3.3f;
		  val /= 2.5f;
		  val -= 0.16f;
		  val /= 0.0062f;
		  val /= (float)(1.0546f - 0.0000216f*temperature);
		  humidity = (int)(val*100);
		  if(val < 0) humidity = 0;
	  }
#endif
#endif
	  if(rx_available) {
		  if(rx_read(&cmdbuf) == 0) Xbee_rxStateMachine(cmdbuf);
	  }
	  if(xbee_available) {
		  uint8_t* payload = 0;
		  size_t length = 0;
		  if(xrx_read(&payload, &length) == 0) decodePayload(payload, length);
		  free(payload);
	  }
	  if(currTick - pcWriteTick > 1000) {
		  pcWriteTick = currTick;
		  char buf[100] = {0};
		  size_t len = 0;
#ifdef ASSIGN_SYS1
		  len = sprintf(&buf, "Sys2 -> Temp: %d.%02uC || Pressure: %uPa || Humid: %u.%02u%%\n", temperature/100, temperature%100, pressure, humidity/100, humidity%100);
#else
		  int x = (int)(normHMC.x * 100);
		  int y = (int)(normHMC.y * 100);
		  int z = (int)(normHMC.z * 100);
		  len = sprintf(&buf, "Sys1 -> Mag X: %d.%02u || Mag Y: %d.%02u || Mag Z: %d.%02u || Heading: %d", x/100, x%100, y/100, y%100, z/100, z%100, smoothHeadingDegrees);
#endif
		  HAL_UART_Transmit(&PC_UART, (uint8_t*)&buf, len, 0xffff);
	  }
  }
}

int rcv = 0;

void decodePayload_callback(payload_data_t* p) {
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	rcv++;
#ifdef XBEE_RX_END
	if(p->type == (payload_type_t)SENSORS) {
#ifdef ASSIGN_SYS1
		temperature = (typeof(temperature))p->temperature;
		humidity = (typeof(humidity))p->humidity;
		pressure = (typeof(pressure))p->pressure;
		altitude = (typeof(altitude))p->altitude;
#else
		normHMC.x = (typeof(normHMC.x))p->hmc_x;
		normHMC.y = (typeof(normHMC.y))p->hmc_y;
		normHMC.z = (typeof(normHMC.z))p->hmc_z;
		newKey = (typeof(p->keypad))p->keypad;
		HMC_calculate();
#endif
	}
#else
	UNUSED(p);
#endif
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif
