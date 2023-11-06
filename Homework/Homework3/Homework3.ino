/*
    Controlling RGB LED via analog input

    This code controls a 7-segment display using a joystick and a button. The 7-segment display is
    composed of eight segments, and each segment can be individually toggled on and off. The
    joystick is used to navigate and select the active segment, while the button is used to toggle
    the segment's state between on and off. A long press of the button resets the display, turning
    off all segments and setting the initial position to the decimal point. The code also handles
    segment blinking and ensures smooth transitions between segments. The code uses digital and
    analog pins for input and output.

    The circuit connections are as follows:
    * Connect each of the 8 non-ground pins of the 7-segment display according to the constants
    specified below. In this example, a 7-segment display with a common cathode is used, and the
   pins are mapped to the constants pinA, pinB, pinC, pinD, pinE, pinF, pinG, and pinDP.
    * Connect the joystick as indicated by the constants below: pinSW is the digital pin linked to
    the switch output, pinX is the analog pin linked to the X output, and pinY is the analog pin
    associated with the Y output.



    Created 5 November 2023
    By Radu-Gabriel Buzas

    https://github.com/radubuzas/IntroductionToRobotics/Homework3
*/

#define SEGMENT_A  0
#define SEGMENT_B  1
#define SEGMENT_C  2
#define SEGMENT_D  3
#define SEGMENT_E  4
#define SEGMENT_F  5
#define SEGMENT_G  6
#define SEGMENT_DP 7

#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

#define ERROR -1

// Declare all the joystick pins
const int pinSW = 2;  // Digital pin connected to switch output
const int pinX  = A0; // A0 - Analog pin connected to X output
const int pinY  = A1; // A1 - Analog pin connected to Y output

// Declare all the segments pins
const int pinA  = 12;
const int pinB  = 10;
const int pinC  = 9;
const int pinD  = 8;
const int pinE  = 7;
const int pinF  = 6;
const int pinG  = 5;
const int pinDP = 4;

const int segSize      = 8;
const int minThreshold = 100;
const int maxThreshold = 900;
const int startOnDP    = SEGMENT_DP;

const unsigned long timeToBlink   = 400;
const unsigned long debounceDelay = 200;
const unsigned long timeToPlay    = 1200;
unsigned long       lastBlinkRecord;
unsigned long       timeOfResetMicroSec;

volatile unsigned long timeOfFirstButtonPressMicroSec;

byte current = startOnDP;

bool commonAnode = false; // Modify if you have common anode
bool buttonState;
bool blinkState;
bool verifyLongPress;
bool xMoved;
bool yMoved;

volatile bool reset;

volatile int segments[segSize] = {pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP};

bool segmentActivation[segSize] = /*
     0  1  2  3  4  5  6  7
     a  b  c  d  e  f  g  dp   */
    {0, 0, 0, 0, 0, 0, 0, 0};

void setup()
{
    Serial.begin(9600);

    pinMode(pinSW, INPUT_PULLUP);
    pinMode(pinX, INPUT);
    pinMode(pinY, INPUT);

    // Initialize all the pins
    for (int i = 0; i < segSize; ++i)
    {
        pinMode(segments[i], OUTPUT);
    }

    displayImage();

    //  If the button is pressed, an interrupt will be triggered
    attachInterrupt(digitalPinToInterrupt(pinSW), handleInterrupt, FALLING);
}

void loop()
{
    if (verifyLongPress)
    {
        detectLongPress();
    }

    if (reset)
    {
        initReset();
    }

    blink();

    if (buttonState)
    {
        if (timeOfResetMicroSec - timeOfFirstButtonPressMicroSec > debounceDelay * 1000)
        //  Just a small debounce to not consider double press after the long press
        {
            verifyLongPress            = true;
            segmentActivation[current] = !segmentActivation[current];
            buttonState                = false;
        }
    }

    //  {Up, Down, Left, Right} = {0, 1, 2, 3}
    int direction = checkMovement();

    if (direction != ERROR) //  joystick has been moved
    {
        Serial.println(direction);
        int lastSegmentPin = segments[current];       // The segment that was selected
        int go_to          = makeDecision(direction); // Where to move current segment
        Serial.println(go_to);
        if (go_to != ERROR) // if the request is valid
        {
            digitalWrite(lastSegmentPin, LOW); // just stop the blinking
            blinkState     = true;             // the current segment should start blinking
            current        = go_to;
            int pinSegment = segments[current];
            digitalWrite(pinSegment, HIGH); // the new current segment should start blinking
            lastBlinkRecord = millis();     // just to make the transition is smoother!
        }
    }

    displayImage();
}

//  Iterates through the segments and writes the appropriate value
void displayImage()
{
    for (int i = 0; i < segSize; ++i)
    {
        if (i != current)
        {
            digitalWrite(segments[i], segmentActivation[i]);
        }
    }
}

//  Turns the current segment on and off to make it blink at constant time
//  interval (timeToBlink)
void blink()
{
    if (millis() - lastBlinkRecord > timeToBlink)
    {
        blinkState      = !blinkState;
        lastBlinkRecord = millis();
        int pinSegment  = segments[current];
        digitalWrite(pinSegment, blinkState);
    }
}

//  Function that reads the value from the button pin, if the value is LOW/ 0
//  (because of INPUT_PULLUP) then the button is still being pressed upon. The
//  function checks in a cascadong effect if the button is still being pressed
//  down from the momment of the first push.
void detectLongPress()
{
    bool valueOfButton = digitalRead(pinSW);
    if (valueOfButton == HIGH)
    {
        verifyLongPress = false; // No need to continue the 'cascading effect',
                                 // so it won't enter this function next
                                 // iteration if the button will not be pressed
    }
    else
    {
        if (micros() - timeOfFirstButtonPressMicroSec > timeToPlay * 1000)
        {
            reset           = true;
            verifyLongPress = false; // no need to check anymore
        }
    }
}

//  Every segment will get value 0, the 'current' segment will be 'DP'
void initReset()
{
    bool value = digitalRead(pinSW);
    if (value == HIGH) //  The button is not pressed HIGH/ 1 (INPUT_PULLUP)
    {
        timeOfResetMicroSec = micros();
        current             = startOnDP;
        for (int i = 0; i < segSize; ++i)
        {
            segmentActivation[i] = 0;
            digitalWrite(segments[i], LOW);
        }
        reset = false; //  Will not enter this function in next iteration
    }
}

//  decide what direction the current segment is gonna go
//  {Up, Down, Left, Right} = {0, 1, 2, 3}
int makeDecision(byte decision)
{
    // You can look it up, 'switch' instructions are WAY faster
    // than 'if - else if' instructions.
    switch (current)
    {
    case SEGMENT_A: //  a
        switch (decision)
        {
        case UP: //  No Change
            break;
        case DOWN:
            return SEGMENT_G;
        case LEFT:
            return SEGMENT_F;
        case RIGHT:
            return SEGMENT_B;
        default:
            Serial.println("decision has wrong value");
            break;
        }
        return ERROR;
    case SEGMENT_B: //  b
        switch (decision)
        {
        case UP:
            return SEGMENT_A;
        case DOWN:
            return SEGMENT_G;
        case LEFT:
            return SEGMENT_F;
        case RIGHT: //  No Change
            break;
        default:
            Serial.println("decision has wrong value");
            break;
        }
        return ERROR;
    case SEGMENT_C: //  c
        switch (decision)
        {
        case UP:
            return SEGMENT_G;
        case DOWN:
            return SEGMENT_D;
        case LEFT:
            return SEGMENT_E;
        case RIGHT:
            return SEGMENT_DP;
        default:
            Serial.println("decision has wrong value");
            break;
        }
        return ERROR;
    case SEGMENT_D: //  d
        switch (decision)
        {
        case UP:
            return SEGMENT_G;
        case DOWN: //  No Change
            break;
        case LEFT:
            return SEGMENT_E;
        case RIGHT:
            return SEGMENT_C;
        default:
            Serial.println("decision has wrong value");
            break;
        }
        return ERROR;
    case SEGMENT_E: //  e
        switch (decision)
        {
        case UP:
            return SEGMENT_G;
        case DOWN:
            return SEGMENT_D;
        case LEFT: //  No Change
            break;
        case RIGHT:
            return SEGMENT_C;
        default:
            Serial.println("decision has wrong value");
            break;
        }
        return ERROR;
    case SEGMENT_F: //  f
        switch (decision)
        {
        case UP:
            return SEGMENT_A;
        case DOWN:
            return SEGMENT_G;
        case LEFT: //  No Change
            break;
        case RIGHT:
            return SEGMENT_B;
        default:
            Serial.println("decision has wrong value");
            break;
        }
        return ERROR;
    case SEGMENT_G: //  g
        switch (decision)
        {
        case UP:
            return SEGMENT_A;
        case DOWN:
            return SEGMENT_D;
        case LEFT: //  No Change
            break;
        case RIGHT: //  No Change
            break;
        default:
            Serial.println("decision has wrong value");
            break;
        }
        return ERROR;
    case SEGMENT_DP: //  dp
        switch (decision)
        {
        case UP: //  No Change
            break;
        case DOWN: //
            break;
        case LEFT:
            return SEGMENT_C;
        case RIGHT: //  No Change
            break;
        default:
            Serial.println("decision has wrong value");
            break;
        }
        return ERROR;
    default:
        Serial.println("current has wrong value");
        return ERROR;
        break;
    }
}

//  This function checks the movement of the joystick
int checkMovement()
{
    int xRead = analogRead(pinY);
    int yRead = analogRead(pinX);

    if (xRead < minThreshold && xMoved == false)
    {
        xMoved = true;
        return 2;
    }

    if (xRead > maxThreshold && xMoved == false)
    {
        xMoved = true;
        return 3;
    }

    if (yRead < minThreshold && yMoved == false)
    {
        yMoved = true;
        return 1;
    }

    if (yRead > maxThreshold && yMoved == false)
    {
        yMoved = true;
        return 0;
    }

    if (xRead >= minThreshold && xRead <= maxThreshold)
    {
        xMoved = false;
    }

    if (yRead >= minThreshold && yRead <= maxThreshold)
    {
        yMoved = false;
    }

    delay(1);
    return ERROR;
}

//  When the button is pressed, this ISR will be triggered
void handleInterrupt()
{
    static unsigned long lastInterrupt = 0;
    unsigned long        now           = micros();

    if (now - lastInterrupt > debounceDelay * 1000)
    {
        timeOfFirstButtonPressMicroSec = now;
        buttonState                    = true;
    }
    lastInterrupt = now;
}