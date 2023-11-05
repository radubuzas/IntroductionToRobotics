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