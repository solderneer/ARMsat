/*
 * lcd.c
 *
 *  Created on: 30 Jan 2017
 *      Author: sidxb
 */

#include "lcd.h"

I2C_HandleTypeDef *hi2c;

int lcdFrame = 0;

uint8_t data[83] = {0};
uint8_t bitmap_up_arrow[8] = {128,132,142,149,132,132,132,128};
uint8_t bitmap_down_arrow[8] = {128,132,132,132,149,142,132,128};
uint8_t bitmap_smiley[8] = {0x80,0x8a,0x8a,0x80,0x91,0x8e,0x80,0x80};
uint8_t bitmap_thing[8] = {0x84,0x84,0x8a,0x95,0x8a,0x95,0x8a,0x91};
uint8_t bitmap_sad[8] = {0x80,0x8a,0x8a,0x80,0x8e,0x91,0x80,0x80};
uint8_t bitmap_shocked[8] = {0x80,0x8a,0x8a,0x80,0x8e,0x91,0x91,0x8e};
uint8_t bitmap_meh[8] = {0x80,0x8a,0x8a,0x80,0x9f,0x80,0x80,0x80};

void LCD_command(uint8_t cmd) {
	data[0] = 0;
	data[1] = cmd;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, (uint8_t*)&data, 2, 0xfff);
	HAL_Delay(1);
}

void LCD_setCursor(uint8_t line, uint8_t col) {
	data[0] = 0;
	data[1] = 3;
	data[2] = line+1;
	data[3] = col+1;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, (uint8_t*)&data, 4, 0xfff);
	HAL_Delay(1);
}

uint8_t LCD_keypad(void) {
	//return ' '; // TODO: the i2c receive request here causes the module to trap and reset if the reset was sporadic or something, make the data requests more robust
	data[0] = 0;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, (uint8_t*)&data, 1, 0xff);
	HAL_Delay(2);
	if(HAL_OK != HAL_I2C_Master_Receive(hi2c, LCD_I2C_ADDRESS, (uint8_t*)&data, 3, 0xff)) return 0;
	if (data[1] & 1)   return '1'; // 49
	if (data[1] & 2)   return '2'; // 50
	if (data[1] & 4)   return '3'; // 51
	if (data[1] & 8)   return '4'; // 52
	if (data[1] & 16)  return '5'; // 53
	if (data[1] & 32)  return '6'; // 54
	if (data[1] & 64)  return '7'; // 55
	if (data[1] & 128) return '8'; // 56
	if (data[2] & 1)   return '9'; // 57
	if (data[2] & 2)   return '*'; // 42
	if (data[2] & 4)   return '0'; // 48
	if (data[2] & 8)   return '#'; // 35
	return ' '; // 32
}

void LCD_init(I2C_HandleTypeDef* h) {
	hi2c = h;
	//lcdFrame = 0;

	LCD_on();
	LCD_command(21);
	LCD_clear();
	LCD_blink_off();
	LCD_cursor_off();
	LCD_home();

	LCD_loadCustomChar(0, (uint8_t*)&bitmap_smiley);
	LCD_loadCustomChar(1, (uint8_t*)&bitmap_sad);
	LCD_loadCustomChar(2, (uint8_t*)&bitmap_shocked);
	LCD_loadCustomChar(3, (uint8_t*)&bitmap_meh);

	LCD_refreshFrame();
}

void LCD_print(const char* str) {
	int i = 1;
	data[0] = 0;
	while(*str != '\0') data[i++] = *str++;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, data, i, 0xfff);
	HAL_Delay(1);
}

void LCD_refreshFrame(void) {
	LCD_clear();
	LCD_home();
	switch(lcdFrame) {
	case 0:
		LCD_setCursor(0,0);
		LCD_print("Humidity:");
		LCD_setCursor(1,0);
		LCD_print("Temp:");
		LCD_setCursor(2,0);
		LCD_print("Pressure:");
		LCD_setCursor(3,0);
		LCD_print("Altitude:");
		break;
	case 1:
		LCD_setCursor(0,0);
		LCD_print("Heading:");
		break;
	default:
		LCD_setCursor(0,0);
		LCD_print("Hello World!");
		LCD_setCursor(1,5);
		LCD_command('0'+lcdFrame);
		break;
	}
}

void LCD_updateSensors(uint32_t humid, uint32_t temp, uint32_t pressure, uint32_t alt, uint32_t heading) {
	uint8_t buf[10] = {0};
	switch(lcdFrame) {
		case 0:
			LCD_setCursor(0,10);
			sprintf(&buf, "%u.%02u%%  ", humid/100, humid%100);
			LCD_print(&buf);
			LCD_setCursor(0,19);
			LCD_command(humid<5000?128:129);

			LCD_setCursor(1,10);
			sprintf(&buf, "%d.%02uC  ", temp/100, temp%100);
			LCD_print(&buf);
			LCD_setCursor(1,19);
			LCD_command(temp<2500?128:129);

			LCD_setCursor(2,10);
			sprintf(&buf, "%uPa  ", pressure);
			LCD_print(&buf);
			LCD_setCursor(2,19);
			LCD_command(131);

			LCD_setCursor(3,10);
			sprintf(&buf, "%um  ", alt);
			LCD_print(&buf);
			LCD_setCursor(3,19);
			LCD_command(130);
			break;
		case 1:
			LCD_setCursor(0,9);
			sprintf(&buf, "%03u deg", heading);
			LCD_print(&buf);
			LCD_setCursor(0,19);
			break;
	}
}

void LCD_loadCustomChar(uint8_t id, uint8_t* bitmap) {
	data[0] = 0;
	data[1] = 27;
	data[2] = 128+id;
	for(int i = 3; i < 11; i++) data[i] = *bitmap++;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, (uint8_t*)&data, 11, 0xfff);
	HAL_Delay(1);
}
