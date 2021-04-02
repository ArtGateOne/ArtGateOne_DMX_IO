# ArtGateOne_DMX_IO
ArtNet node - DMX Input or Output

This is small code for Arduino Uno.

Can be configured as ArtNet to DMX, or DMX to ArtNet node.


This is the same project as ArtGateOne DMX, but web config & Oled display is removed.


U can use static IP Or DHCP.

ALL config is in code.

.


THIS IS DEFAULT CONFIG - DMX IN --> ARTNET 8 Universe DHCP

int dmx_mode = 0; //DMX_IN set 0, DMX_OUT set 1

int dhcp = 1; //DHCP ON set1, OFF set 0

byte intN = 0; //NET

byte intS = 0; //Subnet

byte intU = 8; //Universe

--------

Required
ArduinoUno &
Max 485 module

---------

WIRING

Arduino - MAX 485 module

GND - GND

+5 - VCC

0 - RO

1 - DI

2 - DE + RE


---------
If U need reprogram Arduino - unplug max module.

