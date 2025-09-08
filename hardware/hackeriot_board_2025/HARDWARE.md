## Hardware
## Hackeriot Board
The Hackeriot board has:
* 8x8 LED matrix display with a HT16K33A led driver
* AT24C256C 256Kbit EEPROM
* 6 Functions buttons (Directional + A + B) + Reset button
* SWD + UART (Over [Debug edge](https://debug-edge.io/))

![Hackeriot Board Schematic](/images/2025/hackeriot_board_schematic.png)

Hackeriot Board BOM
|Reference|Description|Quantity|
|:---|:---:|:---:|
| C1, C6, C8 | 100 nF Capacitor | 3
| C2, C3 | 0805 1uF Capacitor | 2
| C7 | 0805 4.7 uF | 1
| R1, R2 | 0805 10K Resistor | 2
| R3 | 0805 500 Resistor | 1
| R4 | 0805 1K Resistor | 1 
| R5 | 0805 200K Resistor | 1
| R7, R8, R9, R10, R11 | 0805 10k Resistor | 5
| D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18, D19, D20, D21, D22, D23, D24, D25, D26, D27, D28, D29, D30, D31, D32, D33, D34, D35, D36, D37, D38, D39, D40, D41, D42, D43, D44, D45, D46, D47, D48, D49, D50, D51, D52, D53, D54, D55, D56, D57, D58, D59, D60, D61, D62, D63, D64, D65, D66| 0805 LED | 66
| U1 | STM32G030C8T6 | 1
| U2 | HT16K33A Led Driver | 1
| U3 | AP2112K-3.3 LDO regulator | 1
| U4 | PCA9306DCUR I2C voltage translator | 1
| U5 | AT24C256C 256Kbit EEPROM | 1
| SW1, SW2, SW3, SW4, SW5, SW6, SW7 | Push switch 6mm SMD | 7
| J1 | Amphenol 10103594-0001LF USB Micro-B Connector | 1
| J2 | PinSocket_2x08P 2.54mm | 1
| J3, J4 | PinSocket 1x03P 2.54mm | 2
| J5 | PinSocket 1x05P 2.54mm | 1







![Hackeriot Board](/images/2025/hackeriot_board_layout.png)

![Hackeriot Board](/images/2025/hackeriot_board_preview.png)