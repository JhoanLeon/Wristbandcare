# Wristbandcare
Embedded system like a wearable bracelet to measure health indicators. Based on an 8-bit Atmel AVR microcontroller (ATtiny167). This project was done in the first semester of 2021.

This system uses I2C, SPI and UART protocols for the communication between microcontroller and peripherals (sensors, bluetooth module and external flash memory). For the wireless sending of information it utilizes BLE protocol.

Due to the specifications and restriccions for wearable techonology, electronics power consumption its a priority. All components, even the Atmel microcontroller and battery, were choosen with the objective of obtain the minimun size and power consumption.

In the folder "Schematics information" there are Eagle libraries and relevant documentation for circuit design. The file "WristbandCare.zip" is the entire Eagle project of electronic device.

Finally, the file "WristbandCare2.aia" is the executable mobile app developed with AppInventor for present information on health indicators and control the state of the wristbandcare.

The other folders are codes to test every single sensor and peripheral (with arduino) and integration of these codes for global functionality in ATtiny167.

Here are some photos of final electronic system assembly and final device presentation.
![electronicstop](https://user-images.githubusercontent.com/55003151/147157598-c81c0bc0-abfa-4338-a947-afc1e4ce780d.jpg)

![electronicsbottom](https://user-images.githubusercontent.com/55003151/147157595-d8717856-6d7f-4562-9afe-19bd46683108.jpg)

![img1](https://user-images.githubusercontent.com/55003151/147157600-9de734b2-0e98-4e05-bb90-ff5ff928405c.jpg)

![img2](https://user-images.githubusercontent.com/55003151/147157601-0f155b92-3322-4322-b705-3d38088f0c3b.jpg)
