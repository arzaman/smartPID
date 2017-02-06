# schematic

this folder contains the SmartPID electric schema and PCB

the controller is composed by 3 PCBs

Front Board
contains the SAMD21 processor, the EEPROM, the OLED display and push button. there are 2 sockets one for the ESP8266 module and the other one for SWD programming.

Base Board
it connects the 3 PCBs and contains all the linear switching 220AC/12VDC power supply. On the board there are also the 2 mechanical relays and the buzzer

Rear Board
provides the I/O lines via terminal blocks and contains all the power driver and protection electronic

CAD file are in altium designer format


![smartpid HW](https://github.com/arzaman/smartPID/blob/master/Picture/smartPID%20HW.jpg)

![smartpid assembling](https://github.com/arzaman/smartPID/blob/master/Picture/smartPID%20assembling.jpg)

