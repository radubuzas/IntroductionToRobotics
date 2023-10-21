# Introduction To Robotics (2023-2024)

This repository contains <a href="https://www.facebook.com/unibuc.robotics/">"Introduction to Robotics"</a> laboratory homework, taken in the 3rd year (2023-2024) at the <a href="https://fmi.unibuc.ro/en/">Faculty of Mathematics and Computer Science</a>, University of Bucharest. 

Each homework includes requirements, implementation details, code and image files.

Coding environment: <a href="https://www.arduino.cc/en/software">Arduino IDE</a>

<details>

<summary>
Homework #1: Controlling RGB LED
</summary>

## Controlling RGB LED :rotating_light:

### Components
* Arduino Uno
* RGB LED 
* Potentiometers
* Resistors and wires

### Technical Task

* Use a separate potentiometer for controlling each color of the RGB LED:Red,Green, andBlue.  This control must leveragedigital electronics.  Specifically,you  need  to  read  the  potentiometer’s  value  with  Arduino  and  then  write  amapped value to the LED pins.

### Setup

:exclamation: The configuration described below corresponds to an RGB LED with a common cathode.

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/circuit.png)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/front.jpg)

![](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Assets/back.jpg)

:exclamation: If you are using a comman anode RGB LED, make sure to connect it to 5V instead of GND :exclamation:

### [DEMO](https://youtu.be/Rtv_YgjhBqY)
[![](https://img.youtube.com/vi/Rtv_YgjhBqY/0.jpg)](https://youtu.be/Rtv_YgjhBqY)

### [Code](https://github.com/radubuzas/IntroductionToRobotics/blob/master/Homework/%231/Homework1.ino)


</details>

## License
The code in this repo is licensed under the [MIT License](LICENSE)
