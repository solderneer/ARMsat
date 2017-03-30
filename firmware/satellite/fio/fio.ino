#include <SPI.h>
#include <SD.h>

const int chipSelect = 17;
bool x = true;
uint8_t cmdbuf = 0;
uint8_t rx_i = 0;

uint16_t temperature;
uint16_t humidity;
uint32_t pressure;
uint16_t altitude;
uint16_t dust_conc;
uint16_t heading;
uint16_t voltage_cell1;
uint16_t voltage_cell2;
uint16_t voltage_cell3;
uint16_t current;

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH); 
	Serial1.begin(57600);
	while(!SD.begin(chipSelect)) {
		;
	}
	digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
	if (Serial1.available()) {
		cmdbuf = Serial1.read();
		switch(rx_i) {
			default:
			case 0:
				if(cmdbuf==0xab) rx_i++;
				break;
			case 1:
				rx_i = cmdbuf == 0xcd ? 2 : 0;
				break;
			case 2:
				humidity = cmdbuf;
				rx_i++;
				break;
			case 3:
				humidity <<= 8;
				humidity |= cmdbuf;
				rx_i++;
				break;
			case 4:
				temperature = cmdbuf;
				rx_i++;
				break;
			case 5:
				temperature <<= 8;
				temperature |= cmdbuf;
				rx_i++;
				break;
			case 6:
				pressure = cmdbuf;
				rx_i++;
				break;
			case 7:
				pressure <<= 8;
				pressure |= cmdbuf;
				rx_i++;
				break;
			case 8:
				pressure <<= 8;
				pressure |= cmdbuf;
				rx_i++;
				break;
			case 9:
				pressure <<= 8;
				pressure |= cmdbuf;
				rx_i++;
				break;
			case 10:
				altitude = cmdbuf;
				rx_i++;
				break;
			case 11:
				altitude <<= 8;
				altitude |= cmdbuf;
				rx_i++;
				break;
			case 12:
				dust_conc = cmdbuf;
				rx_i++;
				break;
			case 13:
				dust_conc <<= 8;
				dust_conc |= cmdbuf;
				rx_i++;
				break;
			case 14:
				heading = cmdbuf;
				rx_i++;
				break;
			case 15:
				heading <<= 8;
				heading |= cmdbuf;
				rx_i++;
				break;
			case 16:
				voltage_cell1 = cmdbuf;
				rx_i++;
				break;
			case 17:
				voltage_cell1 <<= 8;
				voltage_cell1 |= cmdbuf;
				rx_i++;
				break;
			case 18:
				voltage_cell2 = cmdbuf;
				rx_i++;
				break;
			case 19:
				voltage_cell2 <<= 8;
				voltage_cell2 |= cmdbuf;
				rx_i++;
				break;
			case 20:
				voltage_cell3 = cmdbuf;
				rx_i++;
				break;
			case 21:
				voltage_cell3 <<= 8;
				voltage_cell3 |= cmdbuf;
				rx_i++;
				break;
			case 22:
				current = cmdbuf;
				rx_i++;
				break;
			case 23:
				current <<= 8;
				current |= cmdbuf;
				rx_i++;
				break;
			case 24:
				rx_i = cmdbuf == 0x55 ? 25 : 0;
				break;
			case 25: //IDLE till ready to rx again
				break;
		}
	}
	if(rx_i == 25) {
		File dataFile = SD.open("datalog.txt", FILE_WRITE);
		dataFile.print("Humidity: ");
		dataFile.println(humidity, DEC);
		dataFile.print("Temp: ");
		dataFile.println(temperature, DEC);
		dataFile.print("Pressure: ");
		dataFile.println(pressure, DEC);
		dataFile.print("Altitude: ");
		dataFile.println(altitude, DEC);
		dataFile.print("Heading: ");
		dataFile.println(heading, DEC);
		dataFile.print("Dust conc: ");
		dataFile.println(dust_conc, DEC);
		dataFile.print("V1: ");
		dataFile.println(voltage_cell1, DEC);
		dataFile.print("V2: ");
		dataFile.println(voltage_cell2, DEC);
		dataFile.print("V3: ");
		dataFile.println(voltage_cell3, DEC);
		dataFile.print("Current: ");
		dataFile.println(current, DEC);
		dataFile.println();
		dataFile.close();
		rx_i = 0;
		digitalWrite(LED_BUILTIN, x?HIGH:LOW);
		x = !x;
	}
}
