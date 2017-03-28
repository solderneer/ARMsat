/*
 * lcd.h
 *
 *  Created on: 30 Jan 2017
 *      Author: sidxb
 */

#ifndef LCD_H_
#define LCD_H_

#include "stm32f4xx_hal.h"
#include "main.h"
#include "hmc.h"

#define LCD_I2C_ADDRESS (0xC6)

#define LCD_clear() LCD_command(0xc)
#define LCD_home() LCD_command(1)
#define LCD_on() LCD_command(0x13)
#define LCD_off() LCD_command(0x14)
#define LCD_cursor_on() LCD_command(5)
#define LCD_cursor_off() LCD_command(4)
#define LCD_blink_on() LCD_command(6)
#define LCD_blink_off() LCD_command(4)

extern int lcdFrame;

extern void LCD_command(uint8_t cmd);
extern void LCD_init(I2C_HandleTypeDef* h);
extern void LCD_print(const char* str);
extern void LCD_printNum(int num, int digits, int radix);
void LCD_loadCustomChar(uint8_t id, uint8_t* bitmap);
void LCD_setCursor(uint8_t line, uint8_t col);
uint8_t LCD_keypad(void);

#ifdef ASSIGN_SYS1
void LCD_updateSensors(uint32_t humid, uint32_t temp, uint32_t pressure, uint32_t alt);
#else
void LCD_updateSensors(hmc_axis_t h, uint32_t alt);
#endif

void LCD_refreshFrame(void);

#endif /* LCD_H_ */
