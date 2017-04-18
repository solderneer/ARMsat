# ARMsat

## Quick start
The best way to get what this project is about in a few seconds is to open the ARMsat-poster.pdf. This project is now officially "complete" so don't expect much more work done on it.

To set up the app: 
```
npm install
npm run start
```
To build the distributable version after installing dependencies
```
npm run prerelease
npm run release
```

## Introduction
A nano satellite platform built using a STM32F4 and numerous additional sensors and peripherals. This is for the module project for the [EG1310](https://myaces.nus.edu.sg/cors/jsp/report/ModuleDetailedInfo.jsp?acad_y=2015/2016&sem_c=1&mod_c=EG1310) module in the National University of Singapore. I'll just be documenting the progress for actually building a 32-bit architecture based BalloonSat here in hopes that it might be useful to someone. The module's final project requires us to fly the BalloonSat on a weather balloon to test its reliability and functionality.

## System specifications and components

| Specifications (BalloonSat)  |                                                                                                                                                                                                            |
|---------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|            Mass           | 1 kg max                                                                                                                                                                                                   |
| Dimensions                | 15 x 15 x 15 cm max                                                                                                                                                                                        |
| Chassis material          | 3D Printed LCP plastic                                                                                                                                                                                     |
| Battery Life              | 60-80 minutes                                                                                                                                                                                              |
| System Current Draw       | 1.5A maximum                                                                                                                                                                                               |
| System Voltage Input      | 11.1V nominal                                                                                                                                                                                              |
| Onboard Sensors           | Pressure: BMP085 Temperature: TMP102 Humidity: HIH-4030 Particulate Matter: GP2Y1010AU0F Wind speed: MD0550 Rev C wind speed sensor Air Quality: MiCS-2714 NO2 gas sensor Camera: 5.8Ghz AOMWAY FPV camera |
| Primary Microcontroller   | Nucleo-411RE board                                                                                                                                                                                         |
| Secondary Microcontroller | Arduino FIO                                                                                                                                                                                                |
| Additional Peripherials   | MicroSD card reader, XBee Pro S2B Wireless                                                                                                                                                                 |
| Servos                    | SpringRC SR-403P (2)                                                                                                                                                                                       |
| Battery                   | 2200 mAh 20C LiPo battery                                                                                                                                                                                  |

| Specifications (Groundstation) |                                                    |
|--------------------------------|----------------------------------------------------|
| Mass                           | 1.00 kg                                            |
| Dimensions                     | 15 x 10 x 15 cm                                    |
| Chassis Material               | 3D Printed LCP plastic with Aluminium 6061 flanges |
| Microcontroller                | Nucleo-411RE board                                 |
| Sensors                        | Ground-level pressure: BMP085                      |
| Display                        | LCD03-I2C/Serial 20x4 LCD display                  |
| Inputs                         | 12 Button Keypad, Joystick                         |

## System architecture

### BalloonSat system
The BalloonSat system is intended to monitor atmospheric parameters such as wind speed, air quality, humidity, pressure and temperature. Accordingly, sensors meant to measure such parameters have been included in the design. They include a particulate matter detector, an ozone gas sensor, and pressure, temperature and humidity sensors. The data is then wireless transmitted back to the base station, via the XBee and a pre-specified protocol, for formatting and display. Since the STM32F4 microcontroller has problems interfacing with the SD card, an Arduino Fio slaved to it will be used to log data to the SD card instead. Since the Nucleo-F411RE only has 12 analog inputs available, the Fio can be used as an external ADC if needed. A TL431 shunt regulator is included for a stable voltage reference.

### Groundstation system
The ground station wirelessly receives the telemetry data from the BalloonSat and displays it on the LCD. It also has a joystick to pan and tilt the camera. However, the LCD and joystick provide an unsatisfactory user experience as the data is poorly formatted. Hence, the ground station is further equipped with an USB-UART adapter for interfacing with a PC. A desktop app will be coded to read the serial data and present it to the user in a visually intuitive way with an interactive GUI containing the camera stream and graph plots of the sensor readings, and allowing the user finer control of the BalloonSat.

## Mechanical Design
From the mechanical standpoint, it is imperative that the following considerations are made in order to ensure that the chassis and its electrical modules will not fail during the ascent:

Shock and Acceleration: Due to the rapid transfer of energy to the chassis from the considerable increase in acceleration of the system, the natural frequency of the structure may be excited beyond its limit. This may cause the electrical components to malfunction and the structure to break due to high displacement. From kinematics, the shock intensity can be estimated to be 5G while a safety factor is applied upfront for the calculation. 

Vibration: Vibration due to turbulence experienced by the chassis may cause fasteners and screws to loosen from the screw holes which would result in the assembly falling apart. So, the vibration of the assembly has to be accounted for. Initial analysis being done on the chassis system has shown that the structure should be able to survive the displacement caused by the shock and vibration. Also, the Octave Rule was utilised to ensure that the resonant frequencies of the chassis and the electronic substructures are separated by a ratio more than 2:1.

Wind Speeds: From the National Environmental Agency’s (NEA) website, the average wind speed in Singapore during April is 1.7m/s. Assuming an elastic collision with the wind and the mass of the air in contact with the chassis to be negligible by comparison, the force acting on the structure can thus be determined. This calculated force will then be taken into account during the Static Stress Analysis of the chassis to ensure it can survive the force exerted by the wind

Temperature: The maximum operating temperature is expected to be 40 degrees celsius, well below the operating limits for the electrical components. Furthermore, sufficient ventilation is provided for passive cooling

Humidity: The relative humidity levels range from 63% to 97% and condensation may result when the structure is hoisted up quickly to 60m in altitude. As 60m is still within the same layer of atmosphere as the atmosphere at ground level, we would not expect an extreme change of temperature that would lead to condensation occurring. However, ventilation holes will be made on the chassis to ensure there is sufficient heat transfer between the environment and the interior of the chassis such, reducing the risk of condensation

Center of Gravity: The structure has a centre of gravity close to its midpoint so as to avoid net torque whilst accelerating upwards

### Material choice
Due to the lightweight and transparency of the materials, Acrylonitrile Butadiene Styrene (ABS) and Polyester Liquid Crystal Polymer (LCP), a cost benefit analysis was done to determine which material was more suited to be part of our chassis. As the chassis has to undergo massive physical stress, tensile strength of the material is an important factor to take into account. Thus, the benefits of choosing LCP far outweigh the cost of such a material, which is the reason why LCP was chosen as the material for our chassis.

## Electronics Design

### BalloonSat
The figure shows the proposed board for the BalloonSat, with most of the components on a perfboard that plugs into the ST Morpho pins of the Nucleo-F411RE board. A fuse is included for safety in case the current draw exceeds safe levels. An LM358 dual op-amp is used to measure the battery cell voltages. The headers are for gas sensor, wind sensor, particulate matter detector and servos that are external to the board. The linear regulators shown will be replaced by switch-mode power supply modules for the 3.3V and 5V rails.The XBee module will be standing upright to make space for the Arduino Fio which will be plugged into headers on the board.

### Groundstation
The figure shows the proposed board for the ground station, which the Nucleo-F411RE board plugs into from below as well. It contains the LCD display, pressure sensor and keypad above. The USB hub and receiver will be attached below the board.

## Software Design
The software for the STM32F4 microcontrollers will be coded in C with a GNU ARM toolchain and Eclipse. The software for the Arduino Fio will be coded in C with the Arduino IDE. The desktop app will be coded in Javascript with Electron.

### Protocol design
The proposed protocol format is outlined to the left. Each data packet sent to the XBee is 36 bytes long and includes sanity checks and a CRC hash. These help to ensure that the data sent/received is not compromised. As both the BalloonSat and ground station share the same STM32F4 microcontroller architecture, the struct can be easily packed and unpacked, resulting in a simple protocol with a constant payload size.

The sensor packet containing sensor data is transmitted back to the ground station at regular intervals. The control packet is sent to the ground station to move the pan and tilt servos and to issue other commands, possibly to reset or change the frequency of data transmission. The voltage/current monitor packet, containing the internal voltage and current readings, is sent out at longer intervals to the ground station to avoid making the sensor packet too big.

### Why electron?
Electron was chosen for the desktop app as it allows us to code in conventional HTML, CSS and JavaScript while providing cross-platform compatibility. We are able to program once and build it to run in Mac, Windows and Linux. Electron also comes with a host of different node modules which allow for it to interface with the camera and the serial port.

## Conclusion
In conclusion, our system seeks to implement a user friendly implementation of a BalloonSat which is capable of quantifying properties of the atmosphere through the use of a multitude of sensors.

## Credits to group members
Thanks to my groupmates Siidheesh and Marcus.
