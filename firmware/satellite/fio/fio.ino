#include <SPI.h>
#include <SD.h>

const int chipSelect = 17;
bool x = false;
uint8_t cmdbuf = 0;
uint8_t rx_i = 0;

uint16_t temperature;
uint16_t humidity;
uint32_t pressure;
uint16_t altitude;
uint16_t wind_speed;
uint16_t dust_conc;
uint16_t heading;
uint16_t voltage_cell1;
uint16_t voltage_cell2;
uint16_t voltage_cell3;
uint16_t current;

void(* resetFunc) (void) = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(11, INPUT);
  digitalWrite(LED_BUILTIN, LOW); 
  Serial1.begin(57600);
  while(digitalRead(11) != LOW && !SD.begin(chipSelect)) {
    ;
  }
  digitalWrite(LED_BUILTIN, HIGH);
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
        heading = cmdbuf;
        rx_i++;
        break;
      case 13:
        heading <<= 8;
        heading |= cmdbuf;
        rx_i++;
        break;
      case 14:
        dust_conc = cmdbuf;
        rx_i++;
        break;
      case 15:
        dust_conc <<= 8;
        dust_conc |= cmdbuf;
        rx_i++;
        break;
      case 16:
        wind_speed = cmdbuf;
        rx_i++;
        break;
      case 17:
        wind_speed <<= 8;
        wind_speed |= cmdbuf;
        rx_i++;
        break;
      case 18:
        voltage_cell1 = cmdbuf;
        rx_i++;
        break;
      case 19:
        voltage_cell1 <<= 8;
        voltage_cell1 |= cmdbuf;
        rx_i++;
        break;
      case 20:
        voltage_cell2 = cmdbuf;
        rx_i++;
        break;
      case 21:
        voltage_cell2 <<= 8;
        voltage_cell2 |= cmdbuf;
        rx_i++;
        break;
      case 22:
        voltage_cell3 = cmdbuf;
        rx_i++;
        break;
      case 23:
        voltage_cell3 <<= 8;
        voltage_cell3 |= cmdbuf;
        rx_i++;
        break;
      case 24:
        current = cmdbuf;
        rx_i++;
        break;
      case 25:
        current <<= 8;
        current |= cmdbuf;
        rx_i++;
        break;
      case 26:
        rx_i = cmdbuf == 0x55 ? 27 : 0;
        break;
      case 27: //IDLE till ready to rx again
        break;
    }
  }
  if(rx_i == 27) {
      File dataFile = SD.open("data.csv", FILE_WRITE);
      dataFile.print(humidity, DEC);
      dataFile.print(',');
      dataFile.print(temperature, DEC);
      dataFile.print(',');
      dataFile.print(pressure, DEC);
      dataFile.print(',');
      dataFile.print(altitude, DEC);
      dataFile.print(',');
      dataFile.print(heading, DEC);
      dataFile.print(',');
      dataFile.print(wind_speed, DEC);
      dataFile.print(',');
      dataFile.print(dust_conc, DEC);
      dataFile.print(',');
      dataFile.print(voltage_cell1, DEC);
      dataFile.print(',');
      dataFile.print(voltage_cell2, DEC);
      dataFile.print(',');
      dataFile.print(voltage_cell3, DEC);
      dataFile.print(',');
      dataFile.println(current, DEC);
      dataFile.close();
      digitalWrite(LED_BUILTIN, x?HIGH:LOW);
      x = !x;
    rx_i = 0;
  }
}
