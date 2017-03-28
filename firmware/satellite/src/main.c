/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  * Author			   : Siidheesh Theivasigamani (siidheesh@gmail.com)
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "adc.h"
#include "crc.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "uart.h"
#include "gpio.h"
#include "pressure.h"
#include "xbee.h"
#include "hmc.h"
#include "payload.h"
#include "mpu6050.h"
#include "servo.h"
#include "stdlib.h"
#include "diag/Trace.h"

void SystemClock_Config(void);
void Error_Handler(void);

int humidity = 0;
int temperature = 0;

volatile uint32_t currTick = 0;
uint32_t secondTick = 0;
uint32_t xbeeTick = 0;
uint32_t sensorTick = 0;
uint32_t dataLogTick = 0;

uint16_t genericCmd = 0;
uint16_t newGenericCmd = 0;
uint16_t panPos = 0;
uint16_t newPanPos = 0;
uint16_t tiltPos = 0;
uint16_t newTiltPos = 0;

MPU6050_t* mpud = {0};

int main(void) {

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_CRC_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();

  SID_UART_Receive_IT_Setup(&huart6);
  Servo_Init(&htim1, panPos, tiltPos);
  MPU6050_Init(mpud, MPU6050_Device_0, MPU6050_Accelerometer_2G, MPU6050_Gyroscope_250s, &hi2c1);
  HAL_ADC_Start_IT(&hadc1);
  Pressure_Init(&hi2c1);
  HMC_init(&hi2c1);
  Xbee_init(&XBEE_UART);

  currTick = HAL_GetTick();

  for(;;) {
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
	  if(currTick - xbeeTick > 100) {
		  xbeeTick = currTick;
		  payload_data_tx data;
		  data.temperature = (typeof(data.temperature)) temperature;
		  data.pressure = (typeof(data.pressure)) pressure;
		  data.humidity = (typeof(data.humidity)) humidity;
		  data.altitude = (typeof(data.altitude)) altitude;
		  data.hmc_x = (typeof(data.hmc_x)) normHMC.x;
		  data.hmc_y = (typeof(data.hmc_y)) normHMC.y;
		  data.hmc_z = (typeof(data.hmc_z)) normHMC.z;
		  uint8_t buf[sizeof(payload_data_tx)] = {0};
		  encodePayload((uint8_t*)&buf, &data);
		  Xbee_sendPayload((uint8_t*)&buf, sizeof(data));
	  }
	  if(currTick - sensorTick > 50) {
		  sensorTick = currTick;
		  uint8_t tmp_data[2] = {0};
		  HAL_I2C_Master_Transmit(&hi2c1, TMP102_I2C_ADDRESS, (uint8_t*)&tmp_data, 1, 1000);
		  HAL_I2C_Master_Receive(&hi2c1, TMP102_I2C_ADDRESS, (uint8_t*)&tmp_data, 2, 1000);
		  int16_t val = 0;
		  val = tmp_data[0]<<4;
		  val |= tmp_data[1]>>4;
		  float convertedTemp = val * 0.0625f;
		  temperature = (int)(convertedTemp*100);
		  Pressure_get();
		  HMC_calculate();
		  MPU6050_ReadAll(mpud);
	  }
	  if(currTick - dataLogTick > 2000) {
		  dataLogTick = currTick;
	  }
	  if(genericCmd != newGenericCmd) {
		  switch(genericCmd) {
		  	  default: //short ze battery hier
		  		  break;
		  }
		  genericCmd = newGenericCmd;
	  }
	  if(panPos != newPanPos) {
		  panPos = newPanPos;
	  }
	  if(tiltPos != newTiltPos) {
		  tiltPos = newTiltPos;
	  }
  }
}

void decodePayload_callback(payload_data_rx* p) {
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	newGenericCmd = (typeof(newGenericCmd))p->generic_cmd;
	newPanPos = (typeof(newPanPos))p->pan_pos;
	newTiltPos = (typeof(newTiltPos))p->tilt_pos;
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

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
}


/*****END OF FILE****/
