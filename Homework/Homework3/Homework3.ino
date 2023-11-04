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

const unsigned long timeToBlink   = 400;
const unsigned long debounceDelay = 200;
const unsigned long timeToPlay    = 2000;
unsigned long       lastDebounceTime;
unsigned long       lastBlinkRecord;

const int segSize = 8;
int       index   = 0;

bool          commonAnode = false; // Modify if you have common anode
const int     noOfDigits  = 10;
volatile bool state       = true;
byte          dpState     = LOW;
byte          swState     = LOW;
int           xValue      = 0;
int           yValue      = 0;

bool joyMoved     = false;
int  digit        = 0;
int  minThreshold = 100;
int  maxThreshold = 900;

bool          buttonState;
volatile bool reset;

volatile int segments[segSize] = {pinA, pinB, pinC, pinD,
                                  pinE, pinF, pinG, pinDP};

bool segmentActivation[segSize] = /*
     0  1  2  3  4  5  6  7
     a  b  c  d  e  f  g  dp   */
    {0, 0, 0, 0, 0, 0, 0, 0};

byte current = 7;

void setup()
{
    Serial.begin(9600);
    // TODO: Initialize joystick pins and commonAnode

    pinMode(pinSW, INPUT_PULLUP);
    pinMode(pinX, INPUT);
    pinMode(pinY, INPUT);

    // Initialize all the pins
    for (int i = 0; i < segSize; i++)
    {
        pinMode(segments[i], OUTPUT);
    }
    // TODO: Check if commonAnode should be modified here

    displayImage();

    attachInterrupt(digitalPinToInterrupt(pinSW), handleInterrupt, FALLING);
}

bool          blinkState;
unsigned long timmer;

void loop()
{
    if (reset)
    {
        current = 7;
        for (int i = 0; i < segSize; ++i)
        {
            segmentActivation[i] = 0;
            digitalWrite(segments[i], LOW);
        }
        reset = false;
    }

    if (millis() - lastBlinkRecord > timeToBlink)
    {
        blinkState      = !blinkState;
        lastBlinkRecord = millis();
        int pinSegment  = segments[current];
        digitalWrite(pinSegment, blinkState);
    }

    if (buttonState)
    {
        timmer                     = millis();
        segmentActivation[current] = !segmentActivation[current];
        buttonState                = false;
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
            lastBlinkRecord =
                millis(); //  just to make the transition smoother!
        }
    }

    displayImage();
}

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

//  decision is the direction of where the current segment is gonna go
//  {Up, Down, Left, Right} = {0, 1, 2, 3}
int makeDecision(byte decision)
{

    switch (current)
    {
        case 0: //  a
            switch (decision)
            {
                case 0: //  NA
                    break;
                case 1:
                    return 6;
                    break;
                case 2:
                    return 5;
                    break;
                case 3:
                    return 1;
                    break;
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
                    break;
                case 1:
                    return 6;
                    break;
                case 2:
                    return 5;
                    break;
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
                    break;
                case 1:
                    return 3;
                    break;
                case 2:
                    return 4;
                    break;
                case 3:
                    return 7;
                    break;
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
                    break;
                case 1: //  NA
                    break;
                case 2:
                    return 4;
                    break;
                case 3:
                    return 2;
                    break;
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
                    break;
                case 1:
                    return 3;
                    break;
                case 2: //  NA
                    break;
                case 3:
                    return 2;
                    break;
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
                    break;
                case 1:
                    return 6;
                    break;
                case 2: //  NA
                    break;
                case 3:
                    return 1;
                    break;
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
                    break;
                case 1:
                    return 3;
                    break;
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
                    break;
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

bool xMoved;
bool yMoved;
int  checkMovement()
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

volatile unsigned long first;

void handleInterrupt()
{
    static unsigned long lastInterrupt = 0;
    unsigned long        now           = micros();

    if (now - lastInterrupt > debounceDelay * 1000)
    {
        first = now;
        buttonState = true;
    }
    lastInterrupt = now;
}