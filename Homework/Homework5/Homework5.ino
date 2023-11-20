#include <EEPROM.h>

#define MAIN_MENU       0
#define SENSOR_SETTINGS 1
#define RESET_LOG       2
#define SYSTEM_STAT     3
#define RGB_CTRL        4

const int photocellPin = A0;
const int trigPin      = 9;
const int echoPin      = 10;

const int redPin   = 6;
const int greenPin = 5;
const int bluePin  = 3;

const int samplingPeriodIdx    = 0;
const int distanceThresholdIdx = 1;
const int lightThresholdIdx    = 3;

const int limitOfData = 10;

bool started;
bool waitingForAction;
bool showData;
bool toogleLED;
bool toogleState;

byte samplingPeriodSec = 1;
int  distanceThreshold;
int  lightThreshold;

byte stateSensor;
byte resetState;
byte colorState;

byte byteReceived;
byte menuState;

byte colorIntensity[] = {0, 0, 0};
byte rgbPins[]        = {redPin, greenPin, bluePin};

const int noLeds = 3;

int lightVal;
int distance;

unsigned long lastRecord;

struct recordedData
{
    int lightIntensity = 0;
    int distance       = 0;

    recordedData *next = nullptr;
};

recordedData *head, *tail;

byte countNumberOfRecordedData;

void setup()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    for (byte i = 0; i < noLeds; ++i)
    {
        pinMode(rgbPins[i], OUTPUT);
    }

    EEPROM.get(samplingPeriodIdx, samplingPeriodSec);
    EEPROM.get(distanceThresholdIdx, distanceThreshold);
    EEPROM.get(lightThresholdIdx, lightThreshold);

    Serial.begin(9600);
}

void loop()
{
    start();

    if (waitingForAction)
        actions();
    else
        receive();

    takeSamples();

    lightLED();
}

void start()
{
    if (started == false && millis() > 1000)
    {
        started = true;
        printMainMenu();
    }
}

void takeSamples()
{
    if (millis() - lastRecord > samplingPeriodSec * 1000)
    {
        lightVal = captureLight();
        distance = captureDistance();

        if (showData == true)
        {
            Serial.println(
                "\n\t\tDon't forget you can exit this mode by entering any character!\n");
            Serial.print(F("\t\tLight intensity = ")), Serial.println(lightVal);
            Serial.print(F("\t\tDistance = ")), Serial.println(distance);
        }

        addElement(distance, lightVal);
        lastRecord = millis();
    }
}

int captureLight()
{
    int photocellValue = analogRead(photocellPin);

    return photocellValue;
}

int captureDistance()
{
    long duration = 0;
    int  distance = 0;

    digitalWrite(trigPin, LOW);

    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);

    distance = duration * 0.034 / 2;

    return distance;
}

void lightLED()
{
    if (toogleLED)
    {
        bool acceptable = true;
        if (distance < distanceThreshold)
            acceptable = false;
        if (lightVal < lightThreshold)
            acceptable = false;

        if (acceptable)
        {
            digitalWrite(redPin, LOW);
            digitalWrite(greenPin, HIGH);
            digitalWrite(bluePin, LOW);
        }
        else
        {
            digitalWrite(redPin, HIGH);
            digitalWrite(greenPin, LOW);
            digitalWrite(bluePin, LOW);
        }

        return;
    }

    if (colorState == 0)
        for (byte i = 0; i < noLeds; ++i)
            analogWrite(rgbPins[i], colorIntensity[i]);
}

void receive()
{
    if (Serial.available() > 0)
    {
        byteReceived = Serial.read();
        switch (menuState)
        {
        case MAIN_MENU:
            handleMainMenu(byteReceived);
            break;
        case SENSOR_SETTINGS:
            handleSubmenuSensor(byteReceived);
            break;
        case RESET_LOG:
            handleSubmenuReset(byteReceived);
            break;
        case SYSTEM_STAT:
            handleSubmenuSysStat(byteReceived);
            break;
        case RGB_CTRL:
            handleSubmenuRgbCtrl(byteReceived);
            break;
        default:
            Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            Serial.println("ERROR: MEMORY IS CORRUPTED!");
            Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            break;
        }
    }
}

//  This is used for interactive actions that do not require input from user
void actions()
{
    if (Serial.available() > 0)
    {
        String input = Serial.readString();

        input.trim();

        long readValue = input.toInt();

        bool validInput = true;

        if (input != String(readValue))
        {
            Serial.print(input);
            Serial.println(" is NOT an integer\n");
            validInput = false;
        }

        if (menuState == SENSOR_SETTINGS)
        {
            if (validInput == false)
            {
                handleSubmenuSensor('0' + stateSensor);
                return;
            }
            enterValuesSensors(readValue);
        }
        else if (menuState == RGB_CTRL)
        {
            if (validInput == false)
            {
                return;
            }
            if (readValue < 0 or readValue > 255)
            {
                Serial.println(F("\nValue must be between 0 and 255!\n"));
                printColorInput();
                return;
            }

            Serial.println(readValue);
            colorIntensity[colorState] = readValue;

            ++colorState;
            printColorInput();

            if (colorState == 3)
            {
                colorState = 0;
                Serial.println(F("Input submitted!\n"));
                waitingForAction = false;
                printRgbLed();
            }
        }
    }
}

void printColorInput()
{
    if (colorState == 0)
        Serial.print(F("\t\tRed = "));
    else if (colorState == 1)
        Serial.print(F("\t\tGreen = "));
    else if (colorState == 2)
        Serial.print(F("\t\tBlue = "));
}

void printMainMenu()
{
    Serial.println(F("Main Menu:"));
    Serial.println(F("1. Sensor Settings"));
    Serial.println(F("2. Reset Logger Data"));
    Serial.println(F("3. System Status"));
    Serial.println(F("4. RGB LED Control\n"));
}

void printSensorSettings()
{
    Serial.println(F("Sensor Settings:"));
    Serial.println(F("\t1. Sensors Sampling Interval"));
    Serial.println(F("\t2. Ultrasonic Alert Threshold"));
    Serial.println(F("\t3. LDR Alert Threshold"));
    Serial.println(F("\t4. Back\n"));

    menuState = SENSOR_SETTINGS;
}

void printResetLogger()
{
    Serial.println(F("Reset Logger Data:"));
    Serial.println(F("\t1. Yes"));
    Serial.println(F("\t2. No\n"));

    menuState = RESET_LOG;
}

void printSystemStatus()
{
    Serial.println("System Status:");
    Serial.println("\t1. Current Sensor Readings");
    Serial.println("\t2. Current Sensor Settings");
    Serial.println("\t3. Display Logged Data");
    Serial.println("\t4. Back\n");

    menuState = SYSTEM_STAT;
}

void printRgbLed()
{
    Serial.println(F("RGB LED Control:"));
    Serial.println(F("\t1. Manual Color Control"));
    Serial.println(F("\t2. LED: Toggle Automatic ON/OFF"));
    Serial.println(F("\t3. Back\n"));

    menuState = RGB_CTRL;
}

void handleMainMenu(byte readByte)
{
    switch (readByte)
    {
    case '1':
        printSensorSettings();
        break;
    case '2':
        printResetLogger();
        break;
    case '3':
        printSystemStatus();
        break;
    case '4':
        printRgbLed();
        break;
    default:
        Serial.println(F("[Main Menu] INVALID INPUT!\n"));
        printMainMenu();
        break;
    }
}

void handleSubmenuSensor(byte readByte)
{
    waitingForAction = true; //  suppose that an input is given
    switch (readByte)
    {
    case '1':
        Serial.println(F("Type a value between 1-10 (seconds):\n"));
        stateSensor = 1;
        break;
    case '2':
        Serial.println(F("Type a threshold for minimum distance (in cm), value between 0-100:\n"));
        stateSensor = 2;
        break;
    case '3':
        Serial.println(F("Type a threshold for minimum light intensity, values between 0-1024:\n"));
        stateSensor = 3;
        break;
    case '4':
        printMainMenu();
        menuState        = MAIN_MENU;
        waitingForAction = false; //  back, there will be no input for values
        break;
    default:
        Serial.println(F("[Sensors] INVALID INPUT!\n"));
        printSensorSettings();
        waitingForAction = false; // wrong input
        break;
    }
}

void enterValuesSensors(long value)
{
    waitingForAction = false;
    switch (stateSensor)
    {
    case 1:
        if (value > 10 or value < 1)
        {
            Serial.println(F("[Sample Period] INVALID INPUT!\n"));
            handleSubmenuSensor('1');
            return;
        }
        samplingPeriodSec = value;
        EEPROM.put(samplingPeriodIdx, samplingPeriodSec);
        break;
    case 2:
        if (value > 1023 or value < 0)
        {
            Serial.println(F("[Sound Sensor] INVALID INPUT!\n"));
            handleSubmenuSensor('2');
            return;
        }
        distanceThreshold = value;
        EEPROM.put(distanceThresholdIdx, (int)distanceThreshold);
        break;
    case 3:
        if (value > 1023 or value < 0)
        {
            Serial.println(F("[LDR Sensor] INVALID INPUT!\n"));
            handleSubmenuSensor('3');
            return;
        }
        lightThreshold = value;
        EEPROM.put(lightThresholdIdx, (int)lightThreshold);
        break;
    default:
        Serial.println(F("Imposible?"));
        waitingForAction = true;
        break;
    }
    if (waitingForAction == false)
        printSensorSettings();
}

void handleSubmenuReset(byte readByte)
{
    switch (readByte)
    {
    case '1':
        resetLog();
        break;
    case '2':
        printMainMenu();
        menuState = MAIN_MENU;
        break;
    default:
        Serial.println(F("[Submenu Reset] INVALID INPUT!\n"));
        printResetLogger();
        break;
    }
}

void handleSubmenuSysStat(byte readByte)
{
    if (showData == 1)
    {
        printSystemStatus();
        showData = 0;
        return;
    }
    switch (readByte)
    {
    case '1':
        showData = 1;
        Serial.println(F("\t\tWait for sensors to take sample...\n"));
        break;
    case '2':
        Serial.print(F("Sampling rate: ")), Serial.println(samplingPeriodSec);
        Serial.print(F("Threshold value for Ultrasonic Sensor: ")),
            Serial.println(distanceThreshold);
        Serial.print(F("Threshold value for LDR: ")), Serial.println(lightThreshold),
            Serial.println();
        printSystemStatus();
        break;
    case '3':
        if (countNumberOfRecordedData == 0)
            Serial.println(F("\n\t\t\tThere is no data recorded at the momment!\n"));
        else
            iterateThroughData();
        Serial.println();
        printSystemStatus();
        break;
    case '4':
        printMainMenu();
        menuState = MAIN_MENU;
        break;
    default:
        Serial.println(F("[System Stat]INVALID INPUT!\n"));
        printSystemStatus();
        break;
    }
}

void handleSubmenuRgbCtrl(byte readByte)
{
    if (toogleState)
    {
        dealWithToogle(readByte);
        return;
    }

    switch (readByte)
    {
    case '1':
        Serial.println(F("\t\tIntroduce a value for RGB LED which follows the format 'x = val'"));
        Serial.println(F("\t\twhere x is {red, green, blue} and val is an interger in [0,255]"));
        Serial.print(F("\t\tRed = "));
        waitingForAction = true;
        break;
    case '2':
        printToogleOptions();
        toogleState = true;
        break;
    case '3':
        printMainMenu();
        menuState = MAIN_MENU;
        break;
    default:
        Serial.println(F("INVALID INPUT!\n"));
        printRgbLed();
        break;
    }
}

void printToogleOptions()
{
    Serial.print("\t\tToogle mode is: ");
    if (toogleLED == 1)
        Serial.println("ON!\n");
    else
        Serial.println("OFF!\n");

    Serial.println(F("\t\tChange toogle state to:"));
    Serial.println(F("\t\t\t1. ON"));
    Serial.println(F("\t\t\t2. OFF"));
    Serial.println(F("\t\t\t3. Back (no change will be done)\n"));
}

void dealWithToogle(byte readByte)
{
    toogleState = false;
    switch (readByte)
    {
    case '1':
        toogleLED = true;
        Serial.println(F("\t\t\tToogle mode changed to ON!\n"));
        printRgbLed();
        break;
    case '2':
        toogleLED = false;
        Serial.println(F("\t\t\tToogle mode changed to OFF!\n"));
        printRgbLed();
        break;
    case '3':
        printRgbLed();
        break;
    default:
        toogleState = true;
        Serial.println(F("\n[RGB - TOOGLE] INVALID INPUT!\n"));
        printToogleOptions();
        break;
    }
}

void resetLog()
{
    if (resetState == 0)
    {
        Serial.println(F("\t\tAre you sure you want to delete ALL data?"));
        Serial.println(F("\t\t\t1. Yes"));
        Serial.println(F("\t\t\t2. No\n"));
        resetState = 1;
        return;
    }
    resetState = 0;
    deleteData();
    Serial.println(F("All data has been deleted!\n"));
    printMainMenu();
    menuState = MAIN_MENU;
}

void addElement(int _distance, int _lightIntensity)
{
    if (countNumberOfRecordedData == 0)
    {
        tail = head = (recordedData *)malloc(sizeof(recordedData));

        head->next           = nullptr;
        head->distance       = _distance;
        head->lightIntensity = _lightIntensity;

        ++countNumberOfRecordedData;    //  set it to 1
        return;
    }
    if (++countNumberOfRecordedData > limitOfData)
    {
        countNumberOfRecordedData = limitOfData;

        recordedData *temp;
        temp = tail->next;

        free(tail);

        tail = temp;
    }

    head->next = (recordedData *)malloc(sizeof(recordedData));

    head->next->next           = nullptr;
    head->next->lightIntensity = _lightIntensity;
    head->next->distance       = _distance;

    head = head->next;
}

void iterateThroughData()
{
    byte i = 0;
    for (recordedData *iter = tail; iter != nullptr; iter = iter->next)
    {
        Serial.print("\n\t\t\t#"), Serial.print(++i), Serial.println(":");
        Serial.print("\t\t\tLight intensity: "), Serial.println(iter->lightIntensity);
        Serial.print("\t\t\tDistance: "), Serial.println(iter->distance);
    }
}

void deleteData()
{
    for (recordedData *iter = tail; iter != nullptr;)
    {
        iter = iter->next;
        free(tail);
        tail = iter;
    }
    countNumberOfRecordedData = 0;
}
