# Smart Environment Monitor and Logger :robot::ledger:

## Description

This project involves the development of a Smart Environment Monitor and Logger using Arduino. The system is designed to gather environmental data through various sensors, log the data into EEPROM, provide visual feedback via an RGB LED, and enable user interaction through a Serial Menu.

## Components

- Arduino Uno
- Ultrasonic Sensor (HC-SR04)
- LDR (Light-Dependent Resistor) or Light Sensor
- RGB LED
- Resistors and wires

## Technical Task

### Requirements

- **Sensor Settings:**
  - Set the sampling interval for sensors (1-10 seconds).
  - Ultrasonic Alert Threshold setting.
  - LDR Alert Threshold setting.
  - Return to the main menu.

- **Reset Logger Data:**
  - Confirm deletion of all data.
  - Submenu with YES or NO options.

- **System Status:**
  - Display current sensor readings.
  - Display current sensor settings.
  - Display the last 10 sensor readings.
  - Return to the main menu.

- **RGB LED Control:**
  - Manual color control.
  - Toggle automatic mode ON/OFF.
  - Return to the main menu.

### Workflow

1. **Sensor Settings:**
   - User sets sampling intervals and alert thresholds.
   - Persistan settings, saved in EEPROM
   - Alerts triggered based on sensor values exceeding thresholds.

2. **Reset Logger Data:**
   - User prompted to confirm data deletion.

3. **System Status:**
   - Display real-time sensor readings.
   - Show current sensor settings.
   - Display the last 10 sensor readings.

4. **RGB LED Control:**
   - Manually set RGB colors.
   - Toggle automatic mode for LED based on sensor alerts.

## Setup

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework5/top.jpeg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework5/front_side.jpeg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework5/back_side.jpeg)


## [DEMO](https://youtube_link_here)
[![](https://img.youtube.com/vi/baLxZFGrtDU/0.jpg)](https://youtu.be/baLxZFGrtDU)

## [Code](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Homework/Homework5/Homework5.ino)
