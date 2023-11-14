# Stopwatch Timer :watch:

## Description

Creating a project that utilizes a 4-digit 7-segment display and three buttons to implement a stopwatch timer counting in tenths of a second. The project includes a save lap functionality, resembling basic stopwatch features commonly found on mobile phones.

## Components

- Arduino Uno
- Shift Register
- 4-digit 7-segment display
- 3 buttons
- resistors and wires

## Technical Task

### Requirements

- 4-digit 7-segment display initialized to "000.0".
- Button functionalities:
  - Button 1: Start / Pause.
  - Button 2: Reset (if in pause mode), Reset saved laps (if in lap viewing mode).
  - Button 3: Save lap (if in counting mode), cycle through the last saved laps (up to 4 laps).

### Workflow

1. **Display Initialization:**
   - The display shows "000.0".

2. **Start Functionality:**
   - Pressing the Start button initiates the timer.

3. **Lap Saving:**
   - During timer operation, pressing the lap button saves the current timer value in memory.
   - Up to 4 laps can be saved, with the 5th press overriding the 1st saved lap.

4. **Reset Operation:**
   - If the reset button is pressed during timer operation, no action is taken.
   - If the reset button is pressed in pause mode, the timer is reset, and saved laps are cleared.

5. **Pause Functionality:**
   - Pressing the pause button stops the timer.

## Setup

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework4/top.jpeg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework4/right_side.jpeg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework4/left_side.jpeg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework4/front.jpeg)

## [DEMO](https://youtu.be/KPTe3z1YQpI)
[![](https://img.youtube.com/vi/KPTe3z1YQpI/0.jpg)](https://youtu.be/KPTe3z1YQpI)

## [Code](https://github.com/radubuzas/IntroductionToRobotics/tree/master/Homework/Homework4)