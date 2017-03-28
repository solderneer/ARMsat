#include "mpu6050.h"

I2C_HandleTypeDef *hi2c_p;

#define MPU6050_I2C_ADDR			0xD0
#define MPU6050_I_AM				0x68

#define MPU6050_AUX_VDDIO			0x01
#define MPU6050_SMPLRT_DIV			0x19
#define MPU6050_CONFIG				0x1A
#define MPU6050_GYRO_CONFIG			0x1B
#define MPU6050_ACCEL_CONFIG		0x1C
#define MPU6050_MOTION_THRESH		0x1F
#define MPU6050_INT_PIN_CFG			0x37
#define MPU6050_INT_ENABLE			0x38
#define MPU6050_INT_STATUS			0x3A
#define MPU6050_ACCEL_XOUT_H		0x3B
#define MPU6050_ACCEL_XOUT_L		0x3C
#define MPU6050_ACCEL_YOUT_H		0x3D
#define MPU6050_ACCEL_YOUT_L		0x3E
#define MPU6050_ACCEL_ZOUT_H		0x3F
#define MPU6050_ACCEL_ZOUT_L		0x40
#define MPU6050_TEMP_OUT_H			0x41
#define MPU6050_TEMP_OUT_L			0x42
#define MPU6050_GYRO_XOUT_H			0x43
#define MPU6050_GYRO_XOUT_L			0x44
#define MPU6050_GYRO_YOUT_H			0x45
#define MPU6050_GYRO_YOUT_L			0x46
#define MPU6050_GYRO_ZOUT_H			0x47
#define MPU6050_GYRO_ZOUT_L			0x48
#define MPU6050_MOT_DETECT_STATUS	0x61
#define MPU6050_SIGNAL_PATH_RESET	0x68
#define MPU6050_MOT_DETECT_CTRL		0x69
#define MPU6050_USER_CTRL			0x6A
#define MPU6050_PWR_MGMT_1			0x6B
#define MPU6050_PWR_MGMT_2			0x6C
#define MPU6050_FIFO_COUNTH			0x72
#define MPU6050_FIFO_COUNTL			0x73
#define MPU6050_FIFO_R_W			0x74
#define MPU6050_WHO_AM_I			0x75

#define MPU6050_GYRO_SENS_250		((float) 131)
#define MPU6050_GYRO_SENS_500		((float) 65.5)
#define MPU6050_GYRO_SENS_1000		((float) 32.8)
#define MPU6050_GYRO_SENS_2000		((float) 16.4)

#define MPU6050_ACCE_SENS_2			((float) 16384)
#define MPU6050_ACCE_SENS_4			((float) 8192)
#define MPU6050_ACCE_SENS_8			((float) 4096)
#define MPU6050_ACCE_SENS_16		((float) 2048)

MPU6050_Result_t MPU6050_Init(MPU6050_t* DataStruct, MPU6050_Device_t DeviceNumber, MPU6050_Accelerometer_t AccelerometerSensitivity, MPU6050_Gyroscope_t GyroscopeSensitivity, I2C_HandleTypeDef* h){
	uint8_t temp;
	hi2c_p = h;
	uint8_t d[2];

	DataStruct->Address = MPU6050_I2C_ADDR | (uint8_t)DeviceNumber;

	d[0] = MPU6050_PWR_MGMT_1;
	d[1] = 0x00;

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, (uint8_t *)d, 2, 1000);


	//assuming I2C is already initialized
}
MPU6050_Result_t MPU6050_SetGyroscope(MPU6050_t* DataStruct, MPU6050_Gyroscope_t GyroscopeSensitivity, I2C_HandleTypeDef* h){
	uint8_t temp;
	hi2c_p = h;
	uint8_t d[2];

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, &MPU6050_GYRO_CONFIG, 1, 1000);
	HAL_I2C_Master_Receive(h, (uint16_t)DataStruct->Address, &temp, 1, 1000);

	temp = (temp & 0xE7) | (uint8_t)GyroscopeSensitivity << 3;

	d[0] = MPU6050_GYRO_CONFIG;
	d[1] = temp;

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, (uint8_t *)d, 2, 1000);

	switch (GyroscopeSensitivity) {
		case MPU6050_Gyroscope_250s:
			DataStruct->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_250;
			break;
		case MPU6050_Gyroscope_500s:
			DataStruct->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_500;
			break;
		case MPU6050_Gyroscope_1000s:
			DataStruct->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_1000;
			break;
		case MPU6050_Gyroscope_2000s:
			DataStruct->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_2000;
			break;
		default:
			break;
	}

	return MPU6050_Result_Ok;

}
MPU6050_Result_t MPU6050_SetAccelerometer(MPU6050_t* DataStruct, MPU6050_Accelerometer_t AccelerometerSensitivity, I2C_HandleTypeDef* h){
	uint8_t temp;
	hi2c_p = h;
	uint8_t d[2];

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, &MPU6050_ACCEL_CONFIG, 1, 1000);
	HAL_I2C_Master_Receive(h, (uint16_t)DataStruct->Address, &temp, 1, 1000);
	temp = (temp & 0xE7) | (uint8_t)AccelerometerSensitivity << 3;

	d[0] = MPU6050_ACCEL_CONFIG;
	d[1] = temp;

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, (uint8_t *)d, 2, 1000);

	switch (AccelerometerSensitivity) {
		case MPU6050_Accelerometer_2G:
			DataStruct->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_2;
			break;
		case MPU6050_Accelerometer_4G:
			DataStruct->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_4;
			break;
		case MPU6050_Accelerometer_8G:
			DataStruct->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_8;
			break;
		case MPU6050_Accelerometer_16G:
			DataStruct->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_16;
			break;
		default:
			break;
	}

	return MPU6050_Result_Ok;
}
MPU6050_Result_t MPU6050_SetDataRate(MPU6050_t* DataStruct, uint8_t rate, I2C_HandleTypeDef* h){
	hi2c_p = h;
	uint8_t d[2];

	d[0] = MPU6050_SMPLRT_DIV;
	d[1] = rate;
	HAL_I2C_Master_Transmit(h, DataStruct->Address, (uint8_t *)d, 2, 1000);

	return MPU6050_Result_Ok;
}
MPU6050_Result_t MPU6050_EnableInterrupts(MPU6050_t* DataStruct, I2C_HandleTypeDef* h){
	uint8_t temp;
	hi2c_p = h;
	uint8_t d[2];

	d[0] = MPU6050_INT_ENABLE;
	d[1] = 0x21;

	HAL_I2C_Master_Transmit(h, DataStruct->Address, (uint8_t *)d, 2, 1000);

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, &MPU6050_INT_PIN_CFG, 1, 1000);
	HAL_I2C_Master_Receive(h, (uint16_t)DataStruct->Address, &temp, 1, 1000);

	temp |= 0x10;

	d[0] = MPU6050_INT_PIN_CFG;
	d[1] = temp;

	HAL_I2C_Master_Transmit(h, DataStruct->Address, (uint8_t *)d, 2, 1000);

	return MPU6050_Result_Ok;

}
MPU6050_Result_t MPU6050_DisableInterrupts(MPU6050_t* DataStruct, I2C_HandleTypeDef* h){
	hi2c_p = h;
	uint8_t d[2];

	d[0] = MPU6050_INT_ENABLE;
	d[1] = 0x00;

	HAL_I2C_Master_Transmit(h, DataStruct->Address, (uint8_t *)d, 2, 1000);

	return MPU6050_Result_Ok;
}
MPU6050_Result_t MPU6050_ReadInterrupts(MPU6050_t* DataStruct, MPU6050_Interrupt_t* InterruptsStruct, I2C_HandleTypeDef* h){
	uint8_t read;
	hi2c_p = h;

	InterruptsStruct->Status = 0;

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, &MPU6050_INT_STATUS, 1, 1000);
	HAL_I2C_Master_Receive(h, (uint16_t)DataStruct->Address, &read, 1, 1000);

	InterruptsStruct->Status = read;

	return MPU6050_Result_Ok;
}
MPU6050_Result_t MPU6050_ReadAccelerometer(MPU6050_t* DataStruct, I2C_HandleTypeDef* h){
	uint8_t data[6];
	hi2c_p = h;

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, &MPU6050_ACCEL_XOUT_H, 1, 1000);
	HAL_I2C_Master_Receive(h, (uint16_t)DataStruct->Address, data, 6, 1000);

	DataStruct->Accelerometer_X = (int16_t)(data[0] << 8 | data[1]);
	DataStruct->Accelerometer_Y = (int16_t)(data[2] << 8 | data[3]);
	DataStruct->Accelerometer_Z = (int16_t)(data[4] << 8 | data[5]);

	return MPU6050_Result_Ok;
}

MPU6050_Result_t MPU6050_ReadGyroscope(MPU6050_t* DataStruct, I2C_HandleTypeDef* h){
	uint8_t data[6];
	hi2c_p = h;

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, &MPU6050_GYRO_XOUT_H, 1, 1000);
	HAL_I2C_Master_Receive(h, (uint16_t)DataStruct->Address, data, 6, 1000);

	DataStruct->Gyroscope_X = (int16_t)(data[0] << 8 | data[1]);
	DataStruct->Gyroscope_Y = (int16_t)(data[2] << 8 | data[3]);
	DataStruct->Gyroscope_Z = (int16_t)(data[4] << 8 | data[5]);

	return MPU6050_Result_Ok;
}
MPU6050_Result_t MPU6050_ReadTemperature(MPU6050_t* DataStruct, I2C_HandleTypeDef* h){
	uint8_t data[2];
	int16_t temp;
	hi2c_p = h;

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, &MPU6050_TEMP_OUT_H, 1, 1000);
	HAL_I2C_Master_Receive(h, (uint16_t)DataStruct->Address, data, 2, 1000);

	temp = (data[0] << 8 | data[1]);

	DataStruct->Temperature = (float)((int16_t)temp / (float)340.0 + (float)36.53);

	return MPU6050_Result_Ok;
}
MPU6050_Result_t MPU6050_ReadAll(MPU6050_t* DataStruct, I2C_HandleTypeDef* h){
	uint8_t data[14];
	int16_t temp;

	HAL_I2C_Master_Transmit(h, (uint16_t)DataStruct->Address, &MPU6050_ACCEL_XOUT_H, 1, 1000);
	HAL_I2C_Master_Receive(h, (uint16_t)DataStruct->Address, data, 14, 1000);

	DataStruct->Accelerometer_X = (int16_t)(data[0] << 8 | data[1]);
	DataStruct->Accelerometer_Y = (int16_t)(data[2] << 8 | data[3]);
	DataStruct->Accelerometer_Z = (int16_t)(data[4] << 8 | data[5]);

	temp = (data[6] << 8 | data[7]);
	DataStruct->Temperature = (float)((float)((int16_t)temp) / (float)340.0 + (float)36.53);

	DataStruct->Gyroscope_X = (int16_t)(data[8] << 8 | data[9]);
	DataStruct->Gyroscope_Y = (int16_t)(data[10] << 8 | data[11]);
	DataStruct->Gyroscope_Z = (int16_t)(data[12] << 8 | data[13]);
	return MPU6050_Result_Ok;
}


