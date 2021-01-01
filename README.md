# arduino-canmon-sniffer

Copyright (c) Krzysztof Strehlau

This code has been written to sniff and emulate CAN packets
by using CAN Monitor-qt project avaiable at github:
https://github.com/tixiv/CAN-Monitor-qt

It emulates only basic packets of CanAdapterChina packets and
it does not support changing any CAN parameters on demand.

CAN bus speed can be changed below - see CAN BUS SETUP section.

Keep in mind that some ESP32 devkits do not support 2000000
serial speed. CP21XX based ones can be reconfigured to support
faster rates by using CP21xxCustomizationUtility from S. Labs.

# Requirements:
[Arduino-CAN]	https://github.com/sandeepmistry/arduino-CAN