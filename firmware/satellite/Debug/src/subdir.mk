################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/adc.c \
../src/crc.c \
../src/gpio.c \
../src/hmc.c \
../src/i2c.c \
../src/main.c \
../src/mpu6050.c \
../src/payload.c \
../src/pressure.c \
../src/servo.c \
../src/spi.c \
../src/stm32f4xx_hal_msp.c \
../src/stm32f4xx_it.c \
../src/tim.c \
../src/uart.c \
../src/usart.c \
../src/xbee.c 

OBJS += \
./src/adc.o \
./src/crc.o \
./src/gpio.o \
./src/hmc.o \
./src/i2c.o \
./src/main.o \
./src/mpu6050.o \
./src/payload.o \
./src/pressure.o \
./src/servo.o \
./src/spi.o \
./src/stm32f4xx_hal_msp.o \
./src/stm32f4xx_it.o \
./src/tim.o \
./src/uart.o \
./src/usart.o \
./src/xbee.o 

C_DEPS += \
./src/adc.d \
./src/crc.d \
./src/gpio.d \
./src/hmc.d \
./src/i2c.d \
./src/main.d \
./src/mpu6050.d \
./src/payload.d \
./src/pressure.d \
./src/servo.d \
./src/spi.d \
./src/stm32f4xx_hal_msp.d \
./src/stm32f4xx_it.d \
./src/tim.d \
./src/uart.d \
./src/usart.d \
./src/xbee.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F411xE -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f4xx" -I"../system/include/cmsis/device" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


