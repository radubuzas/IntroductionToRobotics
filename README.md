# Introduction To Robotics (2023-2024)

This repository contains <a href="https://www.facebook.com/unibuc.robotics/">"Introduction to Robotics"</a> laboratory homework, taken in the 3rd year (2023-2024) at the <a href="https://fmi.unibuc.ro/en/">Faculty of Mathematics and Computer Science</a>, University of Bucharest. 

Each homework includes requirements, implementation details, code and image files.

Coding environment: <a href="https://www.arduino.cc/en/software">Arduino IDE</a>

<details>

<summary>
Homework #1: Controlling RGB LED
</summary>

# Controlling RGB LED :rotating_light:

## Components
* Arduino Uno
* RGB LED 
* Potentiometers
* Resistors and wires

## Technical Task

* Use a separate potentiometer for controlling each color of the RGB LED: Red, Green, and Blue.  This control must leverage digital electronics.  Specifically, you need to read the potentiometer’s value with Arduino and then write a mapped value to the LED pins.

## Setup

:exclamation: The configuration described below corresponds to an RGB LED with a common cathode.

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/circuit.png)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/front.jpg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/back.jpg)

:exclamation: If you are using a comman anode RGB LED, make sure to connect it to 5V instead of GND :exclamation:

## [DEMO](https://youtu.be/Rtv_YgjhBqY)
[![](https://img.youtube.com/vi/Rtv_YgjhBqY/0.jpg)](https://youtu.be/Rtv_YgjhBqY)

## [Code](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Homework/%231/Homework1.ino)


</details>

<details>

<summary>
Homework #2: Simulating Elevator
</summary>

# Elevator Simulator :arrow_up: :arrow_down:

## Description

For this project, I implemented a 3-floor elevator control system simulation using LEDs, buttons, and a buzzer with an Arduino Uno board. The objective was to create a realistic representation of an elevator control system. This involved utilizing various components and coding to achieve the desired functionality.


## Components

- Arduino Uno
- LEDs (3)
- RGB LED
- Buttons (3)
- Buzzer
- Resistors and wires

## Technical Task

- Design a control system that simulates a 3-floor elevator using the Arduino platform. Here are the specific requirements:

- **LED Indicators:** Each of the 3 LEDs should represent one of the 3 floors. The LED corresponding to the current floor should light up. Additionally, another LED should represent the elevator's operational state. It should blink when the elevator is moving and remain static when stationary.

- **Buttons:** Implement 3 buttons that represent the call buttons from the 3 floors. When pressed, the elevator should simulate movement towards the floor after a short interval (2-3 seconds).

- **Buzzer:** The buzzer should sound briefly during the following scenarios:
  - Elevator arriving at the desired floor (something resembling a "cling").
  - Elevator doors closing and movement.

- **State Change & Timers:** If the elevator is already at the desired floor, pressing the button for that floor should have no effect. Otherwise, after a button press, the elevator should "wait for the doors to close" and then "move" to the corresponding floor. If the elevator is in movement, it should either do nothing or it should stack its decision (get to the first programmed floor, open the doors, wait, close them, and then go to the next desired floor).

- **Debounce:** Remember to implement debounce for the buttons to avoid unintentional repeated button presses.

## Setup

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework2/LED_view.jpg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework2/Buttons_view.jpg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework2/TOP_view.jpg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/Homework2/TOP_view2.jpg)

## [DEMO](https://youtu.be/26ewqoCEFFs)
[![](https://img.youtube.com/vi/26ewqoCEFFs/0.jpg)](https://youtu.be/26ewqoCEFFs)



## [Code](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Homework/Homework2/Homework2.ino)


</details>

<details>

<summary>
Homework #3: drawing on 7 segment display
</summary>

#  drawing on 7 segment display :video_game: :pencil2:

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

## [DEMO](https://youtu.be/26ewqoCEFFs)
[![](https://img.youtube.com/vi/26ewqoCEFFs/0.jpg)]()



## [Code](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Homework/Homework3/Homework3.ino)

</details>

## License
The code in this repo is licensed under the [MIT License](LICENSE)
