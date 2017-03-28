/*
 * hmc.c
 *
 *  Created on: 2 Feb 2017
 *      Author: sidxb
 */

#include "hmc.h"

inline void HMC_writeRegister8(uint8_t reg, uint8_t value);
inline uint8_t HMC_fastRegister8(uint8_t reg);
inline uint8_t HMC_readRegister8(uint8_t reg);
inline int16_t HMC_readRegister16(uint8_t reg);

I2C_HandleTypeDef* hi2c;

hmc_axis_t normHMC = {0};

float mgPerDigit;
int xOffset, yOffset;

float headingDegrees = 0;
float previousDegree = 0;
int smoothHeadingDegrees = 0;
uint8_t hmc_buf[16] = {0};

void HMC_init(I2C_HandleTypeDef* h) {
	h = hi2c;
	if ((HMC_fastRegister8(HMC5883L_REG_IDENT_A) != 0x48)
			|| (HMC_fastRegister8(HMC5883L_REG_IDENT_B) != 0x34)
			|| (HMC_fastRegister8(HMC5883L_REG_IDENT_C) != 0x33))
	{
		return;
	}
	HMC_setRange(HMC5883L_RANGE_1_3GA);
	HMC_setMeasurementMode(HMC5883L_CONTINOUS);
	HMC_setDataRate(HMC5883L_DATARATE_30HZ);
	HMC_setSamples(HMC5883L_SAMPLES_8);
	HMC_setOffset(114,-34);
	HMC_calculate();
	//HMC_calibrate();
}

void HMC_calculate(void) {
#ifdef ASSIGN_SYS1
	hmc_axis_t norm = HMC_readNormalize();
	normHMC.x = norm.x;
	normHMC.y = norm.y;
	normHMC.z = norm.z;
#endif
	float heading = atan2f(normHMC.y, normHMC.x);
	// Set declination angle on your location and fix heading
	// You can find your declination on: http://magnetic-declination.com/
	// (+) Positive or (-) for negative
	// SG declination angle is 0'14E (positive)
	// Formula: (deg + (min / 60.0)) / (180 / M_PI);
	float declinationAngle = (14.0 / 60.0) / (180 / M_PI);
	heading += declinationAngle;
	// Correct for heading < 0deg and heading > 360deg
	if (heading < 0) {
		heading += 2 * M_PI;
	}
	if (heading > 2 * M_PI) {
		heading -= 2 * M_PI;
	}
	// Convert to degrees
	headingDegrees = heading * 180/M_PI;

	float fixedHeadingDegrees;
	if (headingDegrees >= 1 && headingDegrees < 240) {
		fixedHeadingDegrees = map(headingDegrees, 0, 239, 0, 179);
	} else if (headingDegrees >= 240){
		fixedHeadingDegrees = map(headingDegrees, 240, 360, 180, 360);
	}
	// Smooth angles rotation for +/- 3deg
	smoothHeadingDegrees = round(fixedHeadingDegrees);
	if (smoothHeadingDegrees < (previousDegree + 3) && smoothHeadingDegrees > (previousDegree - 3)) {
		smoothHeadingDegrees = previousDegree;
	}
	previousDegree = smoothHeadingDegrees;
}

float map(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void HMC_calibrate(void) {
	while(1) {
	hmc_axis_t mag = HMC_readRaw();

	int minX = 0;
	int maxX = 0;
	int minY = 0;
	int maxY = 0;
	int offX = 0;
	int offY = 0;

	// Determine Min / Max values
	if (mag.x < minX) minX = mag.x;
	if (mag.x > maxX) maxX = mag.x;
	if (mag.y < minY) minY = mag.y;
	if (mag.y > maxY) maxY = mag.y;

	// Calculate offsets
	offX = (maxX + minX)/2;
	offY = (maxY + minY)/2;

	trace_printf("offX: %d\n offY: %d", offX, offY);
	}
}

hmc_axis_t HMC_readRaw(void)
{
	hmc_axis_t a = {0};
    a.x = HMC_readRegister16(HMC5883L_REG_OUT_X_M) - xOffset;
    a.y = HMC_readRegister16(HMC5883L_REG_OUT_Y_M) - yOffset;
    a.z = HMC_readRegister16(HMC5883L_REG_OUT_Z_M);
    return a;
}

hmc_axis_t HMC_readNormalize(void)
{
	hmc_axis_t a = {0};
	a.x = ((float)HMC_readRegister16(HMC5883L_REG_OUT_X_M) - xOffset) * mgPerDigit;
	a.y = ((float)HMC_readRegister16(HMC5883L_REG_OUT_Y_M) - yOffset) * mgPerDigit;
	a.z = (float)HMC_readRegister16(HMC5883L_REG_OUT_Z_M) * mgPerDigit;
    return a;
}

void HMC_setOffset(int xo, int yo)
{
    xOffset = xo;
    yOffset = yo;
}


void HMC_setRange(hmc5883l_range_t range) {
	switch(range) {
		case HMC5883L_RANGE_0_88GA:
			mgPerDigit = 0.073f;
			break;
		case HMC5883L_RANGE_1_3GA:
			mgPerDigit = 0.92f;
			break;
		case HMC5883L_RANGE_1_9GA:
			mgPerDigit = 1.22f;
			break;
		case HMC5883L_RANGE_2_5GA:
			mgPerDigit = 1.52f;
			break;
		case HMC5883L_RANGE_4GA:
			mgPerDigit = 2.27f;
			break;
		case HMC5883L_RANGE_4_7GA:
			mgPerDigit = 2.56f;
			break;
		case HMC5883L_RANGE_5_6GA:
			mgPerDigit = 3.03f;
			break;
		case HMC5883L_RANGE_8_1GA:
			mgPerDigit = 4.35f;
			break;
		default:
			break;
	}
	HMC_writeRegister8(HMC5883L_REG_CONFIG_B, range << 5);
}

hmc5883l_range_t HMC_getRange(void)
{
    return (hmc5883l_range_t)((HMC_readRegister8(HMC5883L_REG_CONFIG_B) >> 5));
}

void HMC_setMeasurementMode(hmc5883l_mode_t mode)
{
    uint8_t value;

    value = HMC_readRegister8(HMC5883L_REG_MODE);
    value &= 0b11111100;
    value |= mode;

    HMC_writeRegister8(HMC5883L_REG_MODE, value);
}

hmc5883l_mode_t HMC_getMeasurementMode(void)
{
    uint8_t value;

    value = HMC_readRegister8(HMC5883L_REG_MODE);
    value &= 0b00000011;

    return (hmc5883l_mode_t)value;
}

void HMC_setDataRate(hmc5883l_dataRate_t dataRate)
{
    uint8_t value;

    value = HMC_readRegister8(HMC5883L_REG_CONFIG_A);
    value &= 0b11100011;
    value |= (dataRate << 2);

    HMC_writeRegister8(HMC5883L_REG_CONFIG_A, value);
}

hmc5883l_dataRate_t HMC_getDataRate(void)
{
    uint8_t value;

    value = HMC_readRegister8(HMC5883L_REG_CONFIG_A);
    value &= 0b00011100;
    value >>= 2;

    return (hmc5883l_dataRate_t)value;
}

void HMC_setSamples(hmc5883l_samples_t samples)
{
    uint8_t value;

    value = HMC_readRegister8(HMC5883L_REG_CONFIG_A);
    value &= 0b10011111;
    value |= (samples << 5);

    HMC_writeRegister8(HMC5883L_REG_CONFIG_A, value);
}

hmc5883l_samples_t HMC_getSamples(void)
{
    uint8_t value;

    value = HMC_readRegister8(HMC5883L_REG_CONFIG_A);
    value &= 0b01100000;
    value >>= 5;

    return (hmc5883l_samples_t)value;
}

inline void HMC_writeRegister8(uint8_t reg, uint8_t value)
{
	hmc_buf[0] = reg;
	hmc_buf[1] = value;
	HAL_I2C_Master_Transmit(hi2c, HMC5883L_ADDRESS, (uint8_t*)&hmc_buf, 2, 0xffff);
}

// Read byte to register
inline uint8_t HMC_fastRegister8(uint8_t reg)
{
	hmc_buf[0] = reg;
	HAL_I2C_Master_Transmit(hi2c, HMC5883L_ADDRESS, (uint8_t*)&hmc_buf, 1, 0xffff);
	HAL_I2C_Master_Receive(hi2c, HMC5883L_ADDRESS, (uint8_t*)&hmc_buf, 1, 0xffff);
    return hmc_buf[0];
}

// Read byte from register
inline uint8_t HMC_readRegister8(uint8_t reg)
{
	hmc_buf[0] = reg;
	HAL_I2C_Master_Transmit(hi2c, HMC5883L_ADDRESS, (uint8_t*)&hmc_buf, 1, 0xffff);
	HAL_I2C_Master_Receive(hi2c, HMC5883L_ADDRESS, (uint8_t*)&hmc_buf, 1, 0xffff);
	return hmc_buf[0];
}

// Read word from register
inline int16_t HMC_readRegister16(uint8_t reg)
{
	hmc_buf[0] = reg;
	HAL_I2C_Master_Transmit(hi2c, HMC5883L_ADDRESS, (uint8_t*)&hmc_buf, 1, 0xffff);
	HAL_I2C_Master_Receive(hi2c, HMC5883L_ADDRESS, (uint8_t*)&hmc_buf, 2, 0xffff);
    return hmc_buf[0] << 8 | hmc_buf[1];
}
