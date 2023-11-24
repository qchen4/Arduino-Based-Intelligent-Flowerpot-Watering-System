
# Arduino-Based Intelligent Flowerpot Watering System

**Introduction:** 
This DIY Arduino-based home automation project introduces an intelligent watering system for plants. Utilizing soil moisture sensors, pumps, and other electronic components, this system not only automates the watering process but also contributes to water conservation and plant health.

## Components and Materials
The system comprises categorized components for streamlined assembly:
- **Control Unit:** 1 Arduino NANO development board, 1 IO expansion board, 1 IO expansion module kit.
- **Connectivity:** App and online services for remote management.
- **Power Supply:** 12VDC 1A adapter power supply.

### Required Software
- Arduino IDE for programming the control unit.

## Project Background
The project aims to replace inefficient traditional watering systems with a smart, environmentally friendly alternative. It dynamically adjusts watering schedules based on weather conditions and soil moisture levels.

## Features
Enhanced features grouped for clarity:
- **Sensors:** Outdoor temperature, rain, and light sensors for environmental monitoring.
- **Automation and Control:** Battery-powered RTC for weekly irrigation schedules, sunrise detection, and non-volatile storage ensuring settings retention.
- **Efficiency:** Smart watering to optimize water usage and reduce fungal growth by watering before sunrise.

## Parts List for Building the Intelligent Irrigation Controller
Each part is listed with a brief description:
- **Control and Sensing Components:** Arduino Nano, IO expansion board, various sensors (TSL2561 light sensor, DS18B20 temperature sensor, etc.).
- **Connectivity and Power:** RJ11 cables, 4-channel DC 5V relay, 12VDC power supply.
- **Enclosures and Accessories:** Two sizes of waterproof enclosures, waterproof glands, and connectors for robust outdoor installation.

## Wiring Diagram
A detailed wiring diagram is included to guide the assembly process.

## OLED Display
Simplified usage instructions: The Menu button cycles through options, with an auto-return feature after 30 seconds of inactivity. The Select button activates chosen functions.

## Why Use IO Expanders?
IO Expanders streamline the design, save Arduino code space, and simplify wiring with standard cables, making the system more cost-effective and easy to modify.

## System Build
Follow these step-by-step instructions for assembly:
1. Connect the Arduino Nano to the IO expander and upload the provided software.
2. Modify addresses for one-wire to I2C conversion as per your setup.
3. Prepare the enclosures by drilling holes for the cable glands and sensors, ensuring a snug fit.
4. Assemble the remote sensor unit, paying attention to the orientation and cable management for optimal performance.
5. Test the system comprehensively before final assembly.

## Testing and Final Assembly
Manual control via direct connection to a computer is recommended for initial testing. Once confirmed, assemble all components into the enclosure using double-sided tape and foam for protection.

## Conclusion
This intelligent watering system not only brings convenience but also promotes sustainable gardening practices. Its impact extends beyond mere automation, contributing to water conservation and plant health.
