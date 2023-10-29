/*
    Simulating Elevator with Arduino

    In this Arduino sketch, I have created a simulation of a 3-floor elevator
    control system. The project involves using various components, including
    LEDs, buttons, and a buzzer, to mimic the operation of a real-world
elevator.

    Components Used:

    * LEDs: I've utilized LEDs to represent the different floors of the elevator
    and indicate the operational state of the elevator.

    * Buttons: Buttons are employed to simulate the call buttons from each of the three floors, 
    allowing users to request the elevator. 
    
    * Buzzer: An optional buzzer is integrated into the system to provide audio feedback for various events,
    such as the elevator arriving at a desired floor or the doors closing and the elevator's movement.

    Created 28 October 2023
    By Radu-Gabriel Buzas
    Modified 29 October 2023
    By Radu-Gabriel Buzas

    https://github.com/radubuzas/IntroductionToRobotics/Homework2
*/

#define NOTE_C4 262
#define NOTE_A3 220
#define NOTE_B3 247

//  START OF PINS

const int buttonPinFloor1 = 2;
const int LEDPinFloor1    = 3;

const int buttonPinFloor2 = 5;
const int LEDPinFloor2    = 6;

const int buttonPinFloor3 = 11;
const int LEDPinFloor3    = 12;

const int controlLEDRed   = 10;
const int controlLEDGreen = 13;

const int buzzerPin = 9;

//  END OF PINS

const unsigned long debounceDelay     = 10;
const unsigned long timeAtLevel       = 1400;
const unsigned long timeBetweenLevels = 1000;
const unsigned long blinkRate         = 200;
const unsigned long arriveAtLevel     = 800;

unsigned long startElevator;
unsigned long blinkTimmer;

unsigned long lastDebounceRecord;

bool buttonStateFloor1;
bool buttonStateFloor2;
bool buttonStateFloor3;

bool lastButtonStateFloor1;
bool lastButtonStateFloor2;
bool lastButtonStateFloor3;

bool controlLEDGreenState = 1;
bool controlLEDRedState;

bool state = 1;

byte elevatorCurrentFloor = 1;
byte elevatorDesiredFloor;
byte counterInUpdateVariables;

bool working;

bool closingGates;

bool updateVariables = true;

bool reachedLevel = false;

struct floorNode
{
    floorNode *next        = NULL;
    byte       floorNumber = 0;
};

floorNode *head = NULL;

void setup()
{
    Serial.begin(9600);

    pinMode(LEDPinFloor1, OUTPUT);
    pinMode(LEDPinFloor2, OUTPUT);
    pinMode(LEDPinFloor3, OUTPUT);

    pinMode(controlLEDRed, OUTPUT);
    pinMode(controlLEDGreen, OUTPUT);

    pinMode(buttonPinFloor1, INPUT_PULLUP);
    pinMode(buttonPinFloor2, INPUT_PULLUP);
    pinMode(buttonPinFloor3, INPUT_PULLUP);

    digitalWrite(LEDPinFloor1, HIGH);
    digitalWrite(controlLEDGreen, HIGH);

    // head = (floorNode *)malloc(sizeof(floorNode));
}

void loop()
{

    int readingButtonFloor1 = !digitalRead(buttonPinFloor1);
    int readingButtonFloor2 = !digitalRead(buttonPinFloor2);
    int readingButtonFloor3 = !digitalRead(buttonPinFloor3);

    if (readingButtonFloor1 != lastButtonStateFloor1 ||
        readingButtonFloor2 != lastButtonStateFloor2 ||
        readingButtonFloor3 != lastButtonStateFloor3)
    {
        lastDebounceRecord = millis();
    }

    if (millis() - lastDebounceRecord > debounceDelay && working == false)
    {
        if (readingButtonFloor1 != buttonStateFloor1)
        {
            buttonStateFloor1 = readingButtonFloor1;

            if (buttonStateFloor1 == HIGH)
            {
                elevatorDesiredFloor = 1;
                working              = true;
                closingGates         = true;
                reachedLevel         = false;
            }
        }

        if (readingButtonFloor2 != buttonStateFloor2)
        {
            buttonStateFloor2 = readingButtonFloor2;

            if (buttonStateFloor2 == HIGH)
            {
                elevatorDesiredFloor = 2;
                working              = true;
                closingGates         = true;
                reachedLevel         = false;
            }
        }

        if (readingButtonFloor3 != buttonStateFloor3)
        {
            buttonStateFloor3 = readingButtonFloor3;

            if (buttonStateFloor3 == HIGH)
            {
                elevatorDesiredFloor = 3;
                working              = true;
                closingGates         = true;
                reachedLevel         = false;
            }
        }
    }
    if (working)
    {
        if (elevatorDesiredFloor == elevatorCurrentFloor)
        {
            tone(buzzerPin, NOTE_B3);
            if (reachedLevel == false)
            {
                startElevator = millis();
                reachedLevel  = true;
            }
            if (millis() - startElevator > arriveAtLevel)
            {
                working = false;
                noTone(buzzerPin);
            }
            activateLED(elevatorCurrentFloor);
            if (closingGates == false)
            {
                controlLEDGreenState = true;
            }
            controlLEDRedState = false;
        }
        else
        {
            if (closingGates)
            {
                tone(buzzerPin, NOTE_A3);
                if (millis() - blinkTimmer > blinkRate)
                {
                    blinkTimmer          = millis();
                    controlLEDGreenState = !controlLEDGreenState;
                }
            }
            else
            {
                tone(buzzerPin, NOTE_C4);
                if (millis() - blinkTimmer > blinkRate)
                {
                    blinkTimmer        = millis();
                    controlLEDRedState = !controlLEDRedState;
                }
            }
            if (updateVariables)
            {
                startElevator   = millis();
                updateVariables = false;
            }
            if (elevatorCurrentFloor < elevatorDesiredFloor)
            {
                Serial.println(millis() - startElevator);
                if (state)
                {
                    activateLED(elevatorCurrentFloor);
                    if (millis() - startElevator > timeAtLevel)
                    {
                        deactivateLED(elevatorCurrentFloor);
                        startElevator   = millis();
                        state           = !state;
                        updateVariables = true;
                        closingGates    = false;
                    }
                }
                else
                {
                    if (millis() - startElevator > timeBetweenLevels)
                    {
                        startElevator   = millis();
                        state           = !state;
                        updateVariables = true;
                        ++elevatorCurrentFloor;
                    }
                }
            }
            else
            {
                if (state)
                {
                    activateLED(elevatorCurrentFloor);
                    if (millis() - startElevator > timeAtLevel)
                    {
                        deactivateLED(elevatorCurrentFloor);
                        startElevator   = millis();
                        state           = !state;
                        updateVariables = true;
                        closingGates    = false;
                    }
                }
                else
                {
                    if (millis() - startElevator > timeBetweenLevels)
                    {
                        --elevatorCurrentFloor;
                        startElevator   = millis();
                        state           = !state;
                        updateVariables = true;
                    }
                }
            }
        }
    }

    digitalWrite(controlLEDRed, controlLEDRedState);
    digitalWrite(controlLEDGreen, controlLEDGreenState);

    Serial.print("Etaj comandat: ");
    Serial.println(elevatorCurrentFloor);

    lastButtonStateFloor1 = readingButtonFloor1;
    lastButtonStateFloor2 = readingButtonFloor2;
    lastButtonStateFloor3 = readingButtonFloor3;

    //     Serial.print("Elevator floor: ");
    //     Serial.println(elevatorFloor);

    //   Serial.print("b1: ");
    //   Serial.print(buttonStateFloor1);
    //   Serial.print('\t');

    //   Serial.print("b2: ");
    //   Serial.print(buttonStateFloor2);
    //   Serial.print('\t');

    //   Serial.print("b3: ");
    //   Serial.print(buttonStateFloor3);
    //   Serial.print('\n');
    //   delay(1000);
}

void activateLED(byte floor)
{
    switch (floor)
    {
        case 1:
            digitalWrite(LEDPinFloor1, HIGH);
            break;
        case 2:
            digitalWrite(LEDPinFloor2, HIGH);
            break;
        case 3:
            digitalWrite(LEDPinFloor3, HIGH);
            break;
        default:
            break;
    }
}

void deactivateLED(byte floor)
{
    switch (floor)
    {
        case 1:
            digitalWrite(LEDPinFloor1, LOW);
            break;
        case 2:
            digitalWrite(LEDPinFloor2, LOW);
            break;
        case 3:
            digitalWrite(LEDPinFloor3, LOW);
            break;
        default:
            break;
    }
}