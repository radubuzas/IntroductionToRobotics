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

const unsigned long timeToBlink   = 400;
const unsigned long debounceDelay = 200;
const unsigned long timeToPlay    = 1200;
unsigned long       lastBlinkRecord;
unsigned long       timeOfResetMicroSec;

volatile unsigned long timeOfFirstButtonPressMicroSec;

byte current = 7;

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

    if (direction >= 0)
    {
        Serial.println(direction);
        int lastSegmentPin = segments[current];
        int go_to          = makeDecision(direction);
        Serial.println(go_to);
        if (go_to >= 0)
        {
            digitalWrite(lastSegmentPin, LOW);
            blinkState     = true;
            current        = go_to;
            int pinSegment = segments[current];
            digitalWrite(pinSegment, blinkState);
            lastBlinkRecord = millis(); //  just to make the transition is smoother!
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
        current             = 7;
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
    case 0: //  a
        switch (decision)
        {
        case 0: //  NA
            break;
        case 1:
            return 6;
        case 2:
            return 5;
        case 3:
            return 1;
        default:
            Serial.println("BIG ERROR!");
            break;
        }
        return -1;
    case 1: //  b
        switch (decision)
        {
        case 0:
            return 0;
        case 1:
            return 6;
        case 2:
            return 5;
        case 3: //  NA
            break;
        default:
            Serial.println("BIG ERROR!");
            break;
        }
        return -1;
    case 2: //  c
        switch (decision)
        {
        case 0:
            return 6;
        case 1:
            return 3;
        case 2:
            return 4;
        case 3:
            return 7;
        default:
            Serial.println("BIG ERROR!");
            break;
        }
        return -1;
    case 3: //  d
        switch (decision)
        {
        case 0:
            return 6;
        case 1: //  NA
            break;
        case 2:
            return 4;
        case 3:
            return 2;
        default:
            Serial.println("BIG ERROR!");
            break;
        }
        return -1;
    case 4: //  e
        switch (decision)
        {
        case 0:
            return 6;
        case 1:
            return 3;
        case 2: //  NA
            break;
        case 3:
            return 2;
        default:
            Serial.println("BIG ERROR!");
            break;
        }
        return -1;
    case 5: //  f
        switch (decision)
        {
        case 0:
            return 0;
        case 1:
            return 6;
        case 2: //  NA
            break;
        case 3:
            return 1;
        default:
            Serial.println("BIG ERROR!");
            break;
        }
        return -1;
    case 6: //  g
        switch (decision)
        {
        case 0:
            return 0;
        case 1:
            return 3;
        case 2: //  NA
            break;
        case 3: //  NA
            break;
        default:
            Serial.println("BIG ERROR!");
            break;
        }
        return -1;
    case 7: //  dp
        switch (decision)
        {
        case 0: //  NA
            break;
        case 1: //
            break;
        case 2:
            return 2;
        case 3: //  NA
            break;
        default:
            Serial.println("BIG ERROR!");
            break;
        }
        return -1;
    default:
        Serial.println("BIG BIG ERROR!");
        return -1;
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
    return -1;
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