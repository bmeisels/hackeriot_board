## Hardware
## Hackeriot Board
The Hackeriot board has:
* 4 WS2812B RGB LEDs
* AT24C04C 4Kbit EEPROM
* 2 Functions buttons + Reset button
* SWD + UART (Over [Debug edge](https://debug-edge.io/))

![Hackeriot BoardSchematic](/images/hackeriot_board_schematic.png)

Hackeriot BoardBOM
|Reference|Description|Quantity|
|:---|:---:|:---:|
| C1, C2, C6 | 0805 100nF Capacitor | 3
| C3, C4 | 0805 1uF Capacitor | 2
| C5 | 0805 4.7uF Capacitor | 1
| R1, R5, R7, R8 | 0805 10K Resistor | 4
| R2 | 0805 390 Resistor | 1
| R3 | 0805 680 Resistor | 1
| R4 | 0805 40K Resistor | 1
| R6 | 0805 100 Resistor | 1
| D3, D4, D5, D6 | WS2812B Addressable RGB LED | 4
| D1, D2 | 0805 LED | 2
| U1 | STM32G030F6P6 Microcontroller | 1
| U2 | SN74AHCT1G125 Single Bus Buffer Gate | 1
| U3 | AP2112K-3.3 LDO regulator | 1
| U4 | AT24C04C 4Kbit EEPROM | 1
| SW1, SW2, SW3 | Push switch 6mm THT | 3
| J3, J4, J5, J7 | PinSocket 1x02P 2.54mm | 4
| J2 | Amphenol 10103594-0001LF USB Micro-B Connector | 1
| J6 |  PinSocket 1x05P 2.54mm | 1


![Hackeriot Board](/images/hackeriot_board_layout.png)

![Hackeriot Board](/images/hackeriot_board_preview.png)