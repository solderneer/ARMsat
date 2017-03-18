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

volatile uint32_t currTick = 0;
uint32_t adcTick = 0;
uint32_t lcdTick = 0;
uint32_t sensorTick = 0;
uint32_t pwmTick = 0;
uint32_t rxTick = 0;
uint32_t xbeeTick = 0;
uint32_t reInitTick = 0;

int humidity = 0;
int temperature = 0;

uint32_t pwm = 500;
uint8_t lcdKey = ' ';

uint8_t cmdbuf = 0;

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

  HAL_Delay(2000); //wait 0.5 sec for lcd display to turn on

  SID_UART_Receive_IT_Setup(&huart6);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 0xffff>>8);

  Xbee_init(&huart6, &hcrc);

  LCD_init(&hi2c1);

#ifdef XBEE_TX_END
  HAL_ADC_Start_IT(&hadc1);
  Pressure_Init(&hi2c1);
  HMC_init(&hi2c1);
#endif

  currTick = HAL_GetTick();
  pwmTick = currTick;
  lcdTick = currTick;
  sensorTick = currTick;
  rxTick = currTick;
  xbeeTick = currTick;
  reInitTick = currTick;

  GPIO_PinState btn;
  UNUSED(btn);

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

  FATFS fs;
  FIL fil;

  FRESULT res = f_mount(&fs, "0:/", 1);

  if(res == FR_OK) {
	  trace_puts("mount ok!");
	  if (f_open(&fil, "file.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) == FR_OK) {
		  //File opened, turn off RED and turn on GREEN led
		  //If we put more than 0 characters (everything OK)
		  trace_puts("file open ok!");
		  if (f_puts("First string in my file\n", &fil) > 0) {
			  trace_puts("write ok!");
		  }
		  //Close file, don't forget this!
		  f_close(&fil);
	  }
	  //Unmount drive, don't forget this!
	  f_mount(0, "0:/", 1);
  }
/*
  FRESULT result;
  FATFS fs;
  FIL file;

  const char string[] = "Hallo world\0";
  uint16_t written = 0;
  FILINFO fno;

  printf("mount:%d",f_mount(&fs, "", 1));
  printf("open: %d", f_open(&file, "log.txt", FA_WRITE  | FA_OPEN_ALWAYS ));
  printf("stat: %d", f_stat("log3.txt", &fno));
  printf("write: %d wr: %d", f_write(&file, string, 11, &written), written);
  printf("flush: %d", f_sync(&file));
  printf("close: %d", f_close(&file));
*/

  for(;;)
  {
	  btn = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
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
	  if(currTick - pwmTick > 1000) {
		  pwmTick = currTick;
		  if(pwm == 0x2000)
			  pwm = 0x666;
		  else
			  pwm = 0x2000;
		  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, pwm);
	  }
	  if(currTick - lcdTick > 250) {
		  lcdTick = currTick;
		  switch(lcdFrame) {
			  case 0:
			  case 1:
				  LCD_updateSensors(humidity, temperature, pressure, altitude, smoothHeadingDegrees);
				  break;
		  }
	  }
#ifdef XBEE_TX_END
	  if(currTick - xbeeTick > 3000) {
		  xbeeTick = currTick;
		  payload_data_t data;
		  data.temperature = (typeof(data.temperature)) temperature;
		  data.pressure = (typeof(data.pressure)) pressure;
		  data.humidity = (typeof(data.humidity)) humidity;
		  data.altitude = (typeof(data.altitude)) altitude;
		  data.x = (typeof(data.x)) normHMC.x;
		  data.y = (typeof(data.y)) normHMC.y;
		  data.z = (typeof(data.z)) normHMC.z;
		  uint8_t buf[sizeof(payload_data_t)] = {0};
		  encodePayload((uint8_t*)&buf, &data);
		  Xbee_sendPayload((uint8_t*)&buf, sizeof(data));
	  }
#else
	  UNUSED(xbeeTick);
#endif
	  if(currTick - sensorTick > 50) {
		  sensorTick = currTick;
#ifdef XBEE_TX_END
		  uint8_t tmp_data[2] = {0};
		  HAL_I2C_Master_Transmit(&hi2c1, TMP102_I2C_ADDRESS, (uint8_t*)&tmp_data, 1, 0xffff);
		  HAL_I2C_Master_Receive(&hi2c1, TMP102_I2C_ADDRESS, (uint8_t*)&tmp_data, 2, 0xffff);
		  int16_t val = 0;
		  val = tmp_data[0]<<4;
		  val |= tmp_data[1]>>4;
		  float convertedTemp = val * 0.0625f;
		  temperature = (int)(convertedTemp*100);
		  Pressure_get();
		  HMC_calculate();
#endif
		  uint8_t newKey = LCD_keypad();
		  if(newKey != lcdKey) {
			  switch(newKey) {
			  case '1':
				  NVIC_SystemReset();
				  break;
			  case '2': {
				  payload_data_t data;
				  data.temperature = (int16_t)temperature;
				  data.pressure = (int32_t) pressure;
				  data.humidity = (int16_t) humidity;
				  data.altitude = (int16_t) altitude;
				  data.x = 1;
				  data.y = 2;
				  data.z = 3;
				  uint8_t buf[sizeof(payload_data_t)] = {0};
				  encodePayload((uint8_t*)&buf, &data);
				  Xbee_sendPayload((uint8_t*)&buf, sizeof(data));
				  break;
			  }
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
			  }
			  lcdKey = newKey;
		  }
	  }
	  if(currTick - reInitTick > 20000) {
		  reInitTick = currTick;
		  LCD_init(&hi2c1);
	  }
#ifdef XBEE_TX_END
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
	  if(rx_available) {
		  if(rx_read(&cmdbuf) == 0) Xbee_rxStateMachine(cmdbuf);
	  }
	  if(xbee_available) {
		  uint8_t* payload = 0;
		  size_t length = 0;
		  if(xrx_read(&payload, &length) == 0) decodePayload(payload, length);
		  free(payload);
	  }
  }
}

void decodePayload_callback(payload_data_t* p) {
#ifdef XBEE_RX_END
	temperature = (typeof(temperature))p->temperature;
	humidity = (typeof(humidity))p->humidity;
	pressure = (typeof(pressure))p->pressure;
	altitude = (typeof(altitude))p->altitude;
	normHMC.x = (typeof(normHMC.x))p->x;
	normHMC.y = (typeof(normHMC.x))p->y;
	normHMC.z = (typeof(normHMC.x))p->z;
	HMC_calculate();
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
