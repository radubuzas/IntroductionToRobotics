/*
    Controlling RGB LED via analog input

    In this sketch I am going to light up the 3 colors from a RGB LED with different intesities.
    On the analog input pins there will be 3 'intensity' sensors attached to it, in my case I will
    use potentiometers. I will use 3 analog output pins (in actuality there are no analog output 
    pins on a Arduino board, so I will use the pins with the "~" symbol, which are PWM pins) 
    for transmiting the current with the desired intensity. In this example I am going to use a 
    RGB LED with a comman cathode (the longest pin )

    The circuit:
    * Attach a potentiometer to each input: A0, A1, A2
    * Connect the RGB LED's pins to the corresponding output pins: 11 - red, 10 - green, 9 - blue

    Created 20 October 2023
    By Radu-Gabriel Buzas
    Modified 21 October 2023
    By Radu-Gabriel Buzas

    https://github.com/radubuzas/IntroductionToRobotics/Homework1
*/

//  defining each led its pin
#define redLedPin   11
#define greenLedPin 10
#define blueLedPin  9


//  constants for the pins of input analog signal which will define the intensity of each led
#define signalIntensityLevelForRedLedInput    A0
#define signalIntensityLevelForGreenLedInput  A1
#define signalIntensityLevelForBlueLedInput   A2


//  the function analogRead returns a 10 bit unsignet integer, so its minimum value is 0 and its maximum value its 1023
#define minimumIntensityForInput 0
#define maximumIntensityForInput  1023

//  the function analogWrite receives as parameter an 8 bit unsigned integer, min val = 0, max val = 255
#define minimumIntensityForOutput 0
#define maximumIntensityForOutput 255

uint8_t redLedIntensity;
uint8_t greenLedIntensity;
uint8_t blueLedIntensity;

int signalValueForRedLed;
int signalValueForGreenLed;
int signalValueForBlueLed;

unsigned long last_record;

void setup() {
  Serial.begin(9600);

  pinMode(redLedPin,   OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin,  OUTPUT);

  pinMode(signalIntensityLevelForRedLedInput,   INPUT);
  pinMode(signalIntensityLevelForGreenLedInput, INPUT);
  pinMode(signalIntensityLevelForBlueLedInput,  INPUT);
}

void loop() {
  signalValueForRedLed    = analogRead(signalIntensityLevelForRedLedInput);
  signalValueForGreenLed  = analogRead(signalIntensityLevelForGreenLedInput);
  signalValueForBlueLed   = analogRead(signalIntensityLevelForBlueLedInput);


  //  maping values from [0, 1023] to [0, 255] (only integers)
  redLedIntensity   = map(signalValueForRedLed, minimumIntensityForInput, maximumIntensityForInput, minimumIntensityForOutput, maximumIntensityForOutput);
  greenLedIntensity = map(signalValueForGreenLed, minimumIntensityForInput, maximumIntensityForInput, minimumIntensityForOutput, maximumIntensityForOutput);
  blueLedIntensity  = map(signalValueForBlueLed, minimumIntensityForInput, maximumIntensityForInput, minimumIntensityForOutput, maximumIntensityForOutput);

  analogWrite(redLedPin, redLedIntensity);
  analogWrite(greenLedPin, greenLedIntensity);
  analogWrite(blueLedPin, blueLedIntensity);

  if (millis() - last_record > 1000) {
    Serial.print("Red: ");
    Serial.println(signalValueForRedLed);

    Serial.print("Green: ");
    Serial.println(signalValueForGreenLed);

    Serial.print("Blue: ");
    Serial.println(signalValueForBlueLed);

    Serial.print("\n-----------------\n");
    last_record = millis();
  }

}
