/*
 * hmc.h
 *
 *  Created on: 2 Feb 2017
 *      Author: Siidheesh Theivasigamani
 */

#ifndef HMC_H_
#define HMC_H_

#include <math.h>
#include "stm32f4xx_hal.h"
#include "main.h"

#define HMC5883L_ADDRESS              (0x1E<<1)
#define HMC5883L_REG_CONFIG_A         (0x00)
#define HMC5883L_REG_CONFIG_B         (0x01)
#define HMC5883L_REG_MODE             (0x02)
#define HMC5883L_REG_OUT_X_M          (0x03)
#define HMC5883L_REG_OUT_X_L          (0x04)
#define HMC5883L_REG_OUT_Z_M          (0x05)
#define HMC5883L_REG_OUT_Z_L          (0x06)
#define HMC5883L_REG_OUT_Y_M          (0x07)
#define HMC5883L_REG_OUT_Y_L          (0x08)
#define HMC5883L_REG_STATUS           (0x09)
#define HMC5883L_REG_IDENT_A          (0x0A)
#define HMC5883L_REG_IDENT_B          (0x0B)
#define HMC5883L_REG_IDENT_C          (0x0C)

typedef enum
{
    HMC5883L_SAMPLES_8     = 0b11,
    HMC5883L_SAMPLES_4     = 0b10,
    HMC5883L_SAMPLES_2     = 0b01,
    HMC5883L_SAMPLES_1     = 0b00
} hmc5883l_samples_t;

typedef enum
{
    HMC5883L_DATARATE_75HZ       = 0b110,
    HMC5883L_DATARATE_30HZ       = 0b101,
    HMC5883L_DATARATE_15HZ       = 0b100,
    HMC5883L_DATARATE_7_5HZ      = 0b011,
    HMC5883L_DATARATE_3HZ        = 0b010,
    HMC5883L_DATARATE_1_5HZ      = 0b001,
    HMC5883L_DATARATE_0_75_HZ    = 0b000
} hmc5883l_dataRate_t;

typedef enum
{
    HMC5883L_RANGE_8_1GA     = 0b111,
    HMC5883L_RANGE_5_6GA     = 0b110,
    HMC5883L_RANGE_4_7GA     = 0b101,
    HMC5883L_RANGE_4GA       = 0b100,
    HMC5883L_RANGE_2_5GA     = 0b011,
    HMC5883L_RANGE_1_9GA     = 0b010,
    HMC5883L_RANGE_1_3GA     = 0b001,
    HMC5883L_RANGE_0_88GA    = 0b000
} hmc5883l_range_t;

typedef enum
{
    HMC5883L_IDLE          = 0b10,
    HMC5883L_SINGLE        = 0b01,
    HMC5883L_CONTINOUS     = 0b00
} hmc5883l_mode_t;

typedef struct {
	float x;
	float y;
	float z;
} hmc_axis_t;

extern hmc_axis_t normHMC;

extern void HMC_init(I2C_HandleTypeDef* h);
extern void HMC_calculate(void);

float map(float x, float in_min, float in_max, float out_min, float out_max);
hmc_axis_t HMC_readRaw(void);
hmc_axis_t HMC_readNormalize(void);
void HMC_setOffset(int xo, int yo);
void HMC_setRange(hmc5883l_range_t range);
hmc5883l_range_t HMC_getRange(void);
void HMC_setMeasurementMode(hmc5883l_mode_t mode);
hmc5883l_mode_t HMC_getMeasurementMode(void);
void HMC_setDataRate(hmc5883l_dataRate_t dataRate);
hmc5883l_dataRate_t HMC_getDataRate(void);
void HMC_setSamples(hmc5883l_samples_t samples);
hmc5883l_samples_t HMC_getSamples(void);

extern int smoothHeadingDegrees;
#endif /* HMC_H_ */
