# Wristbandcare
Embedded system like a wearable bracelet to measure health indicators. Based on an 8-bit Atmel AVR microcontroller (ATtiny167). 

This system uses I2C, SPI and UART protocols for the communication between microcontroller and peripherals (sensors, bluetooth module and external flash memory). For the wireless sending of information it utilizes BLE protocol.

Due to the specifications and restriccions for wearable techonology, electronics power consumption its a priority. All components, even the Atmel microcontroller and battery, were choosen with the objective of obtain the minimun size and power consumption.

In the folder "Schematics information" there are Eagle libraries and relevant documentation for circuit design. The file "WristbandCare.zip" is the entire Eagle project of electronic device.

Finally, the file "WristbandCare2.aia" is the executable mobile app developed with AppInventor for present information on health indicators and control the state of the wristbandcare.
