# Drawing on 7 segment display :video_game: :pencil2:

## Description

This task focuses on using a joystick to control the position of a 7-segment display with natural movement. The segment will smoothly transition to neighboring segments, creating a user-friendly experience, and avoiding any unintended "walls" or invalid moves. Users can interact with the display by drawing or selecting specific segments using the joystick input.

## Components

- Arduino Uno
- 7-segment display
- Joystick
- Resistors and wires

## Technical Task

This task involves implementing a segment display controlled by a joystick. The display initializes with the decimal point (DP) segment blinking. The system provides movement and segment state control using the joystick and button, following specific rules:

- **Initial Position**: The display starts with the decimal point (DP) segment blinking.

- **Joystick Control**: Use the joystick to move the current blinking segment to its neighboring segments, following the provided movement table.

- **Blinking Segment**: The current segment always blinks, regardless of its ON or OFF state, creating a visually engaging display.

- **Button Functionality**:
  - **Short Press**: Toggles the state of the current segment between ON and OFF.
  - **Long Press**: Resets the entire display, turning all segments OFF, and moving the current position back to the decimal point (DP).


* Mapping each segment to its neighbours

| Current Segment     | UP          | DOWN        | LEFT        | RIGHT       |
| ----------- | ----------- | ----------- | ----------- | ----------- |
| a           | N/A         | g           | f           | b           |
| b           | a           | g           | f           | N/A         |
| c           | g           | d           | e           | dp          |
| d           | g           | N/A         | e           | c           |
| e           | g           | d           | N/A         | c           |    
| f           | a           | g           | N/A         | b           |
| g           | a           | d           | N/A         | N/A         |
| dp          | N/A         | N/A         | c           | N/A         |


## Setup

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homewrok3/top.jpeg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homewrok3/side.jpeg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homewrok3/front.jpeg)

## [DEMO](https://youtu.be/dUXgIo1wGho)
[![](https://img.youtube.com/vi/dUXgIo1wGho/0.jpg)](https://youtu.be/dUXgIo1wGho)



## [Code](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Homework/Homework3/Homework3.ino)