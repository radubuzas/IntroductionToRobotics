//  START OF PINS

const int buttonPinFloor1 = 2;
const int LEDPinFloor1    = 3;

const int buttonPinFloor2 = 5;
const int LEDPinFloor2    = 6;

const int buttonPinFloor3 = 11;
const int LEDPinFloor3    = 12;

const int controlLEDRed   = 10;
const int controlLEDGreen = 13;

//  END OF PINS

const unsigned long debounceDelay     = 10;
const unsigned long timeAtLevel       = 1600;
const unsigned long timeBetweenLevels = 600;
const unsigned long blinkRate         = 200;

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

struct floorNode{
    floorNode * next = NULL;
    byte floorNumber = 0;
};

floorNode * head = NULL;

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

    head = (floorNode *) malloc(sizeof(floorNode));
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
            }
        }
    }

    if (working)
    {
        if (elevatorDesiredFloor == elevatorCurrentFloor)
        {
            working = false;
            activateLED(elevatorCurrentFloor);
            if (closingGates == false) {
                // digitalWrite(controlLEDGreen, HIGH);
                controlLEDGreenState = true;
            }
            controlLEDRedState = LOW;
        }
        else
        {
            if (closingGates)
            {
                if (millis() - blinkTimmer > blinkRate) {
                    blinkTimmer = millis();
                    // controlLEDGreenState = !controlLEDGreenState;
                    // digitalWrite(controlLEDGreen, controlLEDGreenState);
                }
            }
            else 
            {
                controlLEDGreenState = false;
                if (millis() - blinkTimmer > blinkRate) {
                    blinkTimmer = millis();
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
                        closingGates = false;
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