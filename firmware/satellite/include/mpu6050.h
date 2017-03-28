/* mpu6050.h
 *
 * Created by Sudharshan
 */

#ifndef MPU6050_H
#define MPU6050_H

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include "main.h"

#define MPU6050_DataRate_8KHz   0
#define MPU6050_DataRate_4KHz   1
#define MPU6050_DataRate_2KHz   3
#define MPU6050_DataRate_1KHz   7
#define MPU6050_DataRate_500Hz  15
#define MPU6050_DataRate_250Hz  31
#define MPU6050_DataRate_125Hz  63
#define MPU6050_DataRate_100Hz  79

typedef enum MPU6050_Device_t {
	MPU6050_Device_0 = 0x00,
	MPU6050_Device_1 = 0x02
} MPU6050_Device_t;

typedef enum MPU6050_Result_t {
	MPU6050_Result_Ok = 0x00,
	MPU6050_Result_Error,
	MPU6050_Result_DeviceNotConnected,
	MPU6050_Result_DeviceInvalid
} MPU6050_Result_t;

typedef enum MPU6050_Accelerometer_t {
	MPU6050_Accelerometer_2G = 0x00,
	MPU6050_Accelerometer_4G = 0x01,
	MPU6050_Accelerometer_8G = 0x02,
	MPU6050_Accelerometer_16G = 0x03
} MPU6050_Accelerometer_t;

typedef enum MPU6050_Gyroscope_t {
	MPU6050_Gyroscope_250s = 0x00,
	MPU6050_Gyroscope_500s = 0x01,
	MPU6050_Gyroscope_1000s = 0x02,
	MPU6050_Gyroscope_2000s = 0x03,
} MPU6050_Gyroscope_t;

typedef struct MPU6050_t {
	uint8_t Address;
	float Gyro_Mult;
	float Acce_Mult;

	int16_t Accelerometer_X;
	int16_t Accelerometer_Y;
	int16_t Accelerometer_Z;
	int16_t Gyroscope_X;
	int16_t Gyroscope_Y;
	int16_t Gyroscope_Z;
	float Temperature;
} MPU6050_t;

typedef union MPU6050_Interrupt_t {
	struct {
		uint8_t DataReady:1;
		uint8_t reserved2:2;
		uint8_t Master:1;
		uint8_t FifoOverflow:1;
		uint8_t reserved1:1;
		uint8_t MotionDetection:1;
		uint8_t reserved0:1;
	} F;
	uint8_t Status;
} MPU6050_Interrupt_t;

MPU6050_Result_t MPU6050_Init(MPU6050_t* DataStruct, MPU6050_Device_t DeviceNumber, MPU6050_Accelerometer_t AccelerometerSensitivity, MPU6050_Gyroscope_t GyroscopeSensitivity);
MPU6050_Result_t MPU6050_SetGyroscope(MPU6050_t* DataStruct, MPU6050_Gyroscope_t GyroscopeSensitivity);
MPU6050_Result_t MPU6050_SetAccelerometer(MPU6050_t* DataStruct, MPU6050_Accelerometer_t AccelerometerSensitivity);
MPU6050_Result_t MPU6050_SetDataRate(MPU6050_t* DataStruct, uint8_t rate);
MPU6050_Result_t MPU6050_EnableInterrupts(MPU6050_t* DataStruct);
MPU6050_Result_t MPU6050_DisableInterrupts(MPU6050_t* DataStruct);
MPU6050_Result_t MPU6050_ReadInterrupts(MPU6050_t* DataStruct, MPU6050_Interrupt_t* InterruptsStruct);
MPU6050_Result_t MPU6050_ReadAccelerometer(MPU6050_t* DataStruct, I2C_HandleTypeDef* h)
MPU6050_Result_t MPU6050_ReadGyroscope(MPU6050_t* DataStruct);
MPU6050_Result_t MPU6050_ReadTemperature(MPU6050_t* DataStruct);
MPU6050_Result_t MPU6050_ReadAll(MPU6050_t* DataStruct);

#ifdef __cplusplus
}
#endif

#endif

