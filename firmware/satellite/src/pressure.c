/*
 * pressure.c
 *
 *  Created on: 30 Jan 2017
 *      Author: Siidheesh Theivasigamani
 */
#include "pressure.h"

I2C_HandleTypeDef *hi2c_p;

uint8_t bmp_data[16] = {0};

const uint8_t OSS = 0;  // Oversampling Setting

// Calibration values
int16_t ac1;
int16_t ac2;
int16_t ac3;
uint16_t ac4;
uint16_t ac5;
uint16_t ac6;
int16_t b1;
int16_t b2;
int16_t mb;
int16_t mc;
int16_t md;

uint32_t b5 = 0;
uint16_t ut = 0;
uint32_t up = 0;

uint16_t press_temp = 0;
int32_t pressure = 0;
volatile uint32_t altitude = 0;

volatile uint32_t pressureDelayTicks = 0;
uint32_t pressureDelayState = 0;

void Pressure_Init(I2C_HandleTypeDef* h) {
	hi2c_p = h;
	//while(HAL_I2C_IsDeviceReady(hi2c_p, BMP085_I2C_ADDRESS, 0xfff, 0xffff) == HAL_TIMEOUT);
	Pressure_Calibration();
}

void Pressure_Calibration(void) {
	Pressure_reqData(0xaa, (uint8_t*)&ac1, 2);
	Pressure_reqData(0xac, (uint8_t*)&ac2, 2);
	Pressure_reqData(0xae, (uint8_t*)&ac3, 2);
	Pressure_reqData(0xb0, (uint8_t*)&ac4, 2);
	Pressure_reqData(0xb2, (uint8_t*)&ac5, 2);
	Pressure_reqData(0xb4, (uint8_t*)&ac6, 2);
	Pressure_reqData(0xb6, (uint8_t*)&b1, 2);
	Pressure_reqData(0xb8, (uint8_t*)&b2, 2);
	Pressure_reqData(0xba, (uint8_t*)&mb, 2);
	Pressure_reqData(0xbc, (uint8_t*)&mc, 2);
	Pressure_reqData(0xbe, (uint8_t*)&md, 2);
}

void Pressure_getUnCompTemp(void) {
	//while(HAL_I2C_IsDeviceReady(hi2c_p, BMP085_I2C_ADDRESS, 2, 15) == HAL_TIMEOUT);
	bmp_data[0] = 0xf4;
	bmp_data[1] = 0x2e;
	HAL_I2C_Master_Transmit(hi2c_p, BMP085_I2C_ADDRESS, (uint8_t*)&bmp_data, 2, 0xffff);
	// DELAY 5MS
	pressureDelayTicks = 5;
	pressureDelayState = 1;
}

void Pressure_getUnCompTemp_postDelay(void) {
	if(pressureDelayState!=2) return;
	Pressure_reqData(0xf6, (uint8_t*)&ut, 2);
	Pressure_getUnCompPressure();
}

void Pressure_getUnCompPressure(void) {
	if(pressureDelayState!=2) return;
	//while(HAL_I2C_IsDeviceReady(hi2c_p, BMP085_I2C_ADDRESS, 2, 15) == HAL_TIMEOUT);
	bmp_data[0] = 0xf4;
	bmp_data[1] = 0x34 + (OSS<<6);
	HAL_I2C_Master_Transmit(hi2c_p, BMP085_I2C_ADDRESS, (uint8_t*)&bmp_data, 2, 0xffff);
	// DELAY
	pressureDelayTicks = 2 + (3<<OSS);
}

void Pressure_getUnCompPressure_postDelay(void) {
	if(pressureDelayState!=3) return;
	Pressure_reqData(0xf6, (uint8_t*)&up, 3);
	up >>= 8-OSS;
	Pressure_calculate();
}

void Pressure_calculate(void) {
	if(pressureDelayState!=3) return;
	int32_t x1, x2, x3, b3, b6, p;
	uint32_t b4, b7;

	x1 = (((int32_t)ut - (int32_t)ac6)*(int32_t)ac5) >> 15;
	x2 = ((int32_t)mc << 11)/(x1 + md);
	b5 = x1 + x2;

	ut = ((b5 + 8) >> 4);

	b6 = b5 - 4000;
	// Calculate B3
	x1 = (b2 * (b6 * b6)>>12)>>11;
	x2 = (ac2 * b6)>>11;
	x3 = x1 + x2;
	b3 = (((((int32_t)ac1)*4 + x3)<<OSS) + 2)>>2;

	// Calculate B4
	x1 = (ac3 * b6)>>13;
	x2 = (b1 * ((b6 * b6)>>12))>>16;
	x3 = ((x1 + x2) + 2)>>2;
	b4 = (ac4 * (uint32_t)(x3 + 32768))>>15;

	b7 = ((uint32_t)(up - b3) * (50000>>OSS));
	if (b7 < 0x80000000)
		p = (b7<<1)/b4;
	else
		p = (b7/b4)<<1;

	x1 = (p>>8) * (p>>8);
	x1 = (x1 * 3038)>>16;
	x2 = (-7357 * p)>>16;
	p += (x1 + x2 + 3791)>>4;

	press_temp = ut;
	pressure = p;

	float a = (float)pressure;
	a /= 101325.0f;
	float b = (float)powf(a, (float)(1/5.255f));
	b = 1 - b;
	b *= 44330;
	altitude = (uint32_t)b;

	pressureDelayState = 0;
}


void Pressure_get(void) { //get the ball rolling
	if(pressureDelayState != 0) return;
	Pressure_getUnCompTemp();
}

void Pressure_reqData(uint8_t addr, uint8_t* dsc, uint32_t size) {
	//while(HAL_I2C_IsDeviceReady(hi2c_p, BMP085_I2C_ADDRESS, 2, 15) == HAL_TIMEOUT);
	bmp_data[0] = addr;
	HAL_I2C_Master_Transmit(hi2c_p, BMP085_I2C_ADDRESS, (uint8_t*)&bmp_data, 1, 0xffff);
	HAL_I2C_Master_Receive(hi2c_p, BMP085_I2C_ADDRESS, (uint8_t*)&bmp_data, size, 0xffff);
	while(size) {
		*dsc++ = bmp_data[size-1];
		size--;
	}
}
