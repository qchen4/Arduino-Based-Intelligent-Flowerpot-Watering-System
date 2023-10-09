# Arduino-Based Intelligent Flowerpot Watering System

This is a simple DIY Arduino-based home automation project that uses soil moisture sensors, pumps, and other electronic components to automatically water plants.

## Components and Materials

- 1 Arduino NANO development board
- 1 IO expansion board
- 1 IO expansion module kit
- App and online services

### Required Software

- Arduino IDE

## Project Background

Create an intelligent irrigation controller using Arduino.

Smartly water your yard or garden with dynamic water circulation. Don't water your yard if it's raining or has rained since the last watering. Detect sunrise time using a light sensor and automatically adjust the water start time. Stop watering the yard if the weather is too cold.

## Features

- Outdoor temperature sensor
- Outdoor rain sensor
- Outdoor light sensor
- Battery-powered RTC supports weekly irrigation schedules
- Non-volatile storage - irrigation settings parameters won't be lost due to power outage
- Sunrise detection
- Smart watering can save on your water bill
- Watering before sunrise to allow for proper penetration
- Reduce fungal growth
- Simple scheduling control

## Parts List for Building the Intelligent Irrigation Controller

- 1 IO expansion board
- 2 One-wire to I2C converters
- 1 One-wire connector
- 1 Optical fiber connector
- 1 Optical splitter
- 1 Arduino Nano development board
- 4-channel DC 5V relay
- TSL2561 light sensor
- DS18B20 waterproof temperature sensor
- Optical infrared water level sensor
- DS3231 AT24C32 IIC precise real-time clock
- I2C SSD1306 OLED 128x64 display
- Transparent waterproof plastic enclosure 200x120x75mm
- Transparent waterproof plastic enclosure 100x68x50mm
- IP68 PG7 waterproof nylon cable gland
- IP68 PG11 waterproof nylon cable gland
- RJ11 screw terminal connector
- 50ft 4C4P RJ11 cable
- 6" 4C4P RJ11 cable
- 2.54mm jumper wires
- 2-pin SPST momentary micro push-button switches (2)
- 12VDC 1A adapter power supply

## Wiring Diagram

*Include wiring diagram here.*

## OLED Display

Click the Menu button to display the menu and continue clicking this button to cycle through all menu options. The menu will automatically return if there is no activity for 30 seconds. Press the Select button to execute the desired menu function.

## Why Use IO Expanders?

- Simplifies the design
- Uses readily available parts
- No need to write one-wire driver code
- No need to write DS3231 RTC driver code
- No need to write EEPROM driver code
- No need to write OLED driver code
- No display fonts occupy Arduino code space
- No need to write temperature sensor driver code
- No need to write optical rain sensor driver code
- Saves Arduino code space; only 12,710 bytes (39%)
- Took only three days to write the code
- Easy to wire using standard RJ11 phone cables
- No sensor cable length issues
- More cost-effective than similar commercial systems
- Easy to modify to suit individual needs
- Single power supply

## System Build

Connect the Arduino Nano to the IO expander and program it with the provided code. The 6-pin header is the software's serial debug port, which is not needed in the final installation.

Ensure you change the addresses defined as ONEWIRE_TO_I2C_ROM1 and ONEWIRE-TO-I2C_ROM2 to match your one-wire address with the I2C address.

Note: If you use the USB port to program the Arduino Nano, you must disconnect it from the IO expander since it also uses the same single serial port. Alternatively, if you want to debug, use the ICSP port to program the ATmega328P. To enable the software debug port, uncomment the SERIAL_DEBUG definition.

The distributor must first be configured to isolate the optical infrared sensor data line from the single-wire remote sensor line. Solder a 0-ohm 0603 resistor at R2.

Drill a 7/16" hole in the small enclosure and an 11/16" hole on the right side of the larger enclosure to accommodate the PG7 and PG11 waterproof cable glands. Use a Dremel tool to slightly enlarge the holes until the glands fit snugly. The PG7 will provide the remote sensor, PG11 will provide 12VDC, 24VAC, multiple wires, and RJ11 remote sensor wires.

Connect the SPST momentary micro push-button switches to the RJ11 screw terminals. Insulate the contacts with heat shrink tubing.

Connect all wires and assemble/fit all components into the larger enclosure. The 50ft RJ11 cable for connecting remote sensors should pass through the PG11 waterproof gland without the need to cut it.

Drill a 9/16-inch hole at the top of the small enclosure for mounting the optical infrared water sensor. Use a Dremel tool to slightly enlarge the hole until the sensor fits in place. The small remote sensor fits snugly with the enclosure, but if the sensor is oriented as recommended, it should fit just right. Keeping the RJ11 cable as short as possible will help it fit into the smaller enclosure. After assembly, it's recommended to add some waterproof adhesive to the nuts on the compression gland before screwing them on for a better seal.

Mount the remote sensor enclosure outdoors and elevate it to the east side of your house, ensuring that the optical infrared rain sensor and light sensor are facing the sky without any obstructions in between.

Drill a 1/4" hole near the top center of the larger enclosure, then install the button. Use a Dremel tool to slightly enlarge the hole until the button fits in place.

Test the system to ensure everything works correctly. To test the relays and sensors, disconnect the Arduino from the IO expander and connect it directly to your computer to control it manually. Once you confirm everything is working fine, assemble all components into the enclosure using double-sided tape and packing foam to protect your circuit board and enjoy the benefits and water savings of the smart irrigation controller.
