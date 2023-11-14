// DS = [D]ata [S]torage - data
// STCP = [ST]orage [C]lock [P]in latch
// SHCP = [SH]ift register [C]lock [P]in clock

#define NOT_STARTED 0
#define RUNNING     1
#define PAUSED      2

const byte latchPin = 11; // STCP to 12 on Shift Register const byte
const byte clockPin = 10; // 
const byte dataPin  = 12;

const byte startBtnPin = 3;
const byte resetBtnPin = 8;
const byte lapBtnPin   = 2;

const byte segD1 = 4;
const byte segD2 = 5;
const byte segD3 = 6;
const byte segD4 = 7;

byte displayDigits[] = {segD1, segD2, segD3, segD4};

const byte displayCount = 4;

const int encodingsNumber = 16; // Number of different character encodings
// Encoding array representing the segments of the display for numbers 0-9 and letters A-F
const int encodingArray[encodingsNumber] = {
    // Binary representations for each character
    // A B C D E F G DP
    B11111100, // 0
    B01100000, // 1
    B11011010, // 2
    B11110010, // 3
    B01100110, // 4
    B10110110, // 5
    B10111110, // 6
    B11100000, // 7
    B11111110, // 8
    B11110110, // 9
    B11101110, // A
    B00111110, // b
    B10011100, // C
    B01111010, // d
    B10011110, // E
    B10001110  // F
};

const unsigned long delayCount      = 100; // Delay between updates (milliseconds)
const unsigned long debounceDelay   = 50;
const unsigned long commandDebounce = 200;
const unsigned long lapDebounce     = 500;

unsigned long lastIncrement;
unsigned long number; // The number being displayed
unsigned long lastDebounceRecord;
unsigned long lastCommandRecord;
unsigned long lastLapRecord;

bool startLastReading;
bool resetLastReading;
bool lapLastReading;

bool startState;
bool resetState;
bool lapState;

bool enteredLap;

const int maxSizeLap = 4;

int actualSizeLap;
int iteratorLap;

long lapQueue[maxSizeLap];
long numberToDisplay;

byte runningState;

void setup()
{
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);

    for (byte i = 0; i < displayCount; i++)
    {
        pinMode(displayDigits[i], OUTPUT);
        digitalWrite(displayDigits[i], LOW);
    }

    writeNumber(0);

    pinMode(startBtnPin, INPUT_PULLUP);
    pinMode(resetBtnPin, INPUT_PULLUP);
    pinMode(lapBtnPin, INPUT_PULLUP);

    Serial.begin(9600);
}

void loop()
{

    // Serial.println(startState);
    bool readStart = !digitalRead(startBtnPin);
    bool readReset = !digitalRead(resetBtnPin);
    bool readLap   = !digitalRead(lapBtnPin);

    debounceStart(readStart);
    debounceReset(readReset);
    debounceLap(readLap);

    if (startState)
    {
        startStop();
    }

    if (runningState == NOT_STARTED)
    {
        if (lapState == true)
        {
            numberToDisplay = iterateThroughLaps(numberToDisplay);
            enteredLap      = true;
        }

        if (resetState == true && enteredLap == true)
        {
            numberToDisplay = 0;

            iteratorLap   = 0;
            actualSizeLap = 0;
        }

        writeNumber(numberToDisplay);
        return;
    }

    if (runningState == PAUSED)
    {
        if (resetState == true)
        {
            runningState = NOT_STARTED;

            numberToDisplay = 0;
            number          = 0;
            iteratorLap     = 0;
        }
        writeNumber(number);
        return;
    }

    if (lapState == true)
    {
        if (runningState == RUNNING)
        {
            countLap(number);
        }
    }

    if (millis() - lastIncrement > delayCount)
    {
        ++number;

        if (number > 9999)
            number = 0;

        lastIncrement = millis();
    }

    writeNumber(number);
}

// Function to write the contents of the 'encoding' array to the shift register
void writeReg(byte encoding)
{

    digitalWrite(latchPin, LOW); // Begin the transmission by pulling the latch low

    shiftOut(dataPin, clockPin, MSBFIRST, encoding);

    digitalWrite(latchPin, HIGH); // Latch the data into the output register to update the display
}

void activateDisplay(byte displayNumber)
{
    for (byte i = 0; i < displayCount; ++i)
    {
        digitalWrite(displayDigits[i], HIGH); //  turn off leds
    }

    // Activate only the digit corresponding to 'displayNumber' parameter.
    digitalWrite(displayDigits[displayNumber], LOW);
}

void writeNumber(long number)
{

    for (int i = displayCount - 1; i >= 0; --i)
    {
        byte leastSignificantDigit = number % 10; //  we will use the reversal number algorithm
        activateDisplay(i);

        //  on digit 3 flip the bit that activates the dot
        writeReg(encodingArray[leastSignificantDigit] | (i == 2));
        number /= 10;

        writeReg(B00000000); // Clear the register to avoid ghosting
    }
}

void debounceStart(bool reading)
{
    if (reading != startLastReading)
    {
        lastDebounceRecord = millis();
    }
    if (millis() - lastDebounceRecord > debounceDelay)
    {
        if (reading != startState)
        {
            startState = reading;
        }
    }

    startLastReading = reading;
}

void debounceReset(bool reading)
{
    if (reading != resetLastReading)
    {
        lastDebounceRecord = millis();
    }
    if (millis() - lastDebounceRecord > debounceDelay)
    {
        if (reading != resetState)
        {
            resetState = reading;
        }
    }
    resetLastReading = reading;
}

void debounceLap(bool reading)
{
    if (reading != lapLastReading)
    {
        lastDebounceRecord = millis();
    }
    if (millis() - lastDebounceRecord > debounceDelay)
    {
        if (reading != lapState)
        {
            lapState = reading;
        }
    }
    lapLastReading = reading;
}

//  Function that deals with the start/stop button
void startStop()
{
    if (runningState == NOT_STARTED)
    {
        runningState = RUNNING;

        iteratorLap   = 0;  //  reset interator
        actualSizeLap = 0;  //  'delete' values in lap queue
        enteredLap = false; //  variable that facilitates the CORRECT use of reset inside lap mode

        lastCommandRecord = millis(); //  debounce
        return;                       //  exit the function
    }

    if (millis() - lastCommandRecord > commandDebounce) //  debounce
    {
        if (runningState == RUNNING) //  this means the pause button was pressed
        {
            runningState      = PAUSED;
            lastCommandRecord = millis();
            return;
        }

        if (runningState == PAUSED) //  this means UNpause button was pressed
        {
            runningState = RUNNING;
        }
        lastCommandRecord = millis();
    }
}

//  This function taeks as parameter the time the lap button was pressed and it puts it in the queue
void countLap(long timeToRemember)
{
    if (millis() - lastLapRecord > commandDebounce)
    {
        if (iteratorLap == maxSizeLap) //  This is WAY faster than modulo
        {
            iteratorLap = 0;
        }

        actualSizeLap = max(actualSizeLap, iteratorLap + 1); //  You can cycle through them

        lapQueue[iteratorLap++] = timeToRemember;

        Serial.println(timeToRemember);
    }

    lastLapRecord = millis();
}

//  Gets as a parameter the number that is currently displayed, it can be zero or a lap record
long iterateThroughLaps(long numberToDisplay)
{
    if (actualSizeLap == 0)
    {
        return 0;
    }
    if (millis() - lastLapRecord > lapDebounce)
    {
        if (iteratorLap == actualSizeLap)
        {
            iteratorLap = 0;
        }

        numberToDisplay = lapQueue[iteratorLap++];

        lastLapRecord = millis();
    }
    return numberToDisplay;
}