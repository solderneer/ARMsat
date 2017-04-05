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
#include "six_axis_comp_filter.h"
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

uint8_t cmdbuf = 0;

uint32_t genericCmd = 0;
uint32_t newGenericCmd = 0;
uint32_t panPos = 90;
uint32_t newPanPos = 90;
uint32_t tiltPos = 90;
uint32_t newTiltPos = 90;

uint32_t vref = 1500; //1.2v nominal, temp-dependent
uint32_t vref_avg = 0;
uint32_t vref_i = 0;

uint32_t voltage_cell1 = 0;
uint32_t voltage_cell2 = 0;
uint32_t voltage_cell3 = 0;
uint32_t current = 0;
uint32_t dust_conc = 0;
uint32_t wind_speed = 0;

MPU6050_t* mpud = {0};
SixAxis* sxf = {0};

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

	SID_UART_Receive_IT_Setup(&XBEE_UART);
	Servo_Init(&htim1, panPos, tiltPos);
	MPU6050_Init(mpud, MPU6050_Device_0, MPU6050_Accelerometer_2G, MPU6050_Gyroscope_250s, &hi2c1);
	HAL_ADC_Start_IT(&hadc1);
	Pressure_Init(&hi2c1);
	HMC_init(&hi2c1);
	Xbee_init(&XBEE_UART);
	CompInit(sxf, 0.05f, 2.0f);
	CompStart(sxf);

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
			data.dust_conc = (typeof(data.dust_conc)) dust_conc;
			data.wind_speed = (typeof(data.wind_speed)) wind_speed;
			data.voltage_cell1 = (typeof(data.voltage_cell1)) voltage_cell1;
			data.voltage_cell2 = (typeof(data.voltage_cell2)) (voltage_cell2-voltage_cell1);
			data.voltage_cell3 = (typeof(data.voltage_cell3)) (voltage_cell3-voltage_cell2);
			data.current = (typeof(data.current)) current;
			uint8_t buf[sizeof(payload_data_tx)] = {0};
			encodePayload((uint8_t*)&buf, &data);
			Xbee_sendPayload((uint8_t*)&buf, sizeof(data));
		}
		if(currTick - sensorTick > 50) {
			sensorTick = currTick;
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
			MPU6050_ReadAll(mpud);
			CompAccelUpdate(sxf,(float)mpud->Accelerometer_X,(float)mpud->Accelerometer_Y,(float)mpud->Accelerometer_Z);
			CompGyroUpdate(sxf,(float)mpud->Gyroscope_X,(float)mpud->Gyroscope_Y,(float)mpud->Gyroscope_Z);
			CompUpdate(sxf);
			//TODO: use sxf->compAngleX and sxf->compAngleY to modify servo pos
		}
		if(currTick - dataLogTick > 1000) {
			dataLogTick = currTick;
			uint8_t data[27] = {0xab,0xcd,
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
					(uint16_t)smoothHeadingDegrees>>8,
					(uint16_t)smoothHeadingDegrees&0xff,
					(uint16_t)dust_conc>>8,
					(uint16_t)dust_conc&0xff,
					(uint16_t)wind_speed>>8,
					(uint16_t)wind_speed&0xff,
					((uint16_t)voltage_cell1>>8),
					((uint16_t)voltage_cell1&0xff),
					((uint16_t)voltage_cell2>>8),
					((uint16_t)voltage_cell2&0xff),
					((uint16_t)voltage_cell3>>8),
					((uint16_t)voltage_cell3&0xff),
					((uint16_t)current>>8),
					((uint16_t)current&0xff),
					0x55};
			HAL_UART_Transmit(&FIO_UART, data, sizeof(data), 1000);
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
			Servo_write(SERVO_PAN, panPos);
		}
		if(tiltPos != newTiltPos) {
			tiltPos = newTiltPos;
			Servo_write(SERVO_TILT, tiltPos);
		}
		if(adc_available) {
			adc_available = 0;
			uint32_t val = HAL_ADC_GetValue(&hadc1);
			float volts = (float)val;
			volts /= (float)vref;
			volts *= 1.2f;
			switch(adc_chan) {
			case ADC_CHANNEL_VREFINT:
				vref_avg += val;
				if(++vref_i>=1024) {
					vref = vref_avg>>10;
					voltage_cell1 = (uint32_t)((vref/vref)*1200);
					vref_avg = 0;
					vref_i = 0;
				}
				adc_chan = ADC_CHANNEL_0;
				break;
			case ADC_CHANNEL_0:
				adc_chan = ADC_CHANNEL_1;
				break;
			case ADC_CHANNEL_1:
				adc_chan = ADC_CHANNEL_4;
				break;
			case ADC_CHANNEL_4:
				volts *= 2000.0f;
				voltage_cell1 = (uint32_t)volts;
				adc_chan = ADC_CHANNEL_6;
				break;
			case ADC_CHANNEL_6:
				adc_chan = ADC_CHANNEL_7;
				break;
			case ADC_CHANNEL_7:
				adc_chan = ADC_CHANNEL_8;
				break;
			case ADC_CHANNEL_8:
				volts *= 5700.0f;
				voltage_cell2 = (uint32_t)volts;
				adc_chan = ADC_CHANNEL_9;
				break;
			case ADC_CHANNEL_9:
				adc_chan = ADC_HUMIDITY_CHAN;
				break;
			case ADC_HUMIDITY_CHAN: {
				float v = (float)val;
				v /= (float)vref;
				v *= 1.2f;
				v /= 2.5f;
				v -= 0.16f;
				v /= 0.0062f;
				v /= (float)(1.0546f - 0.0000216f*temperature);
				humidity = (int)(v*100);
				if(v < 0) humidity = 0;
				adc_chan = ADC_CHANNEL_11;
				break;
			}
			case ADC_CHANNEL_11:
				volts *= 5700.0f;
				voltage_cell3 = (uint32_t)volts;
				adc_chan = ADC_CHANNEL_14;
				break;
			case ADC_CHANNEL_14:
				adc_chan = ADC_CHANNEL_15;
				break;
			default:
			case ADC_CHANNEL_15:
				adc_chan = ADC_CHANNEL_VREFINT;
				break;
			}
			ADC_ChannelConfTypeDef c = {adc_chan,1,ADC_SAMPLETIME_3CYCLES,0};
			HAL_ADC_ConfigChannel(&hadc1, &c);
			HAL_ADC_Start_IT(&hadc1);
		}
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
