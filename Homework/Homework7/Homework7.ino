#include "LedControl.h"
#include <EEPROM.h>
#include <LiquidCrystal.h>

#define LEFT  0
#define RIGHT 1
#define UP    2
#define DOWN  3

#define NO_MOVEMENT 255

#define PLAY_GAME 0
#define SETTINGS  1
#define ABOUT     2

#define FIRST_MENU      -10
#define NO_SELECTION    -1
#define EXIT_MENU_STATE -2

const int dinPin   = 12;
const int clockPin = 11;
const int loadPin  = 10;

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;

const byte backLight = 3;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int matrixSize = 8;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); // DIN, CLK, LOAD, No. DRIVER
const int  n  = 8;

const int pinX  = A0;
const int pinY  = A1;
const int pinSW = 2;

const int minThreshold = 100;
const int maxThreshold = 900;

const unsigned long blinkingDelayPlayer = 700;
const unsigned long blinkingDelayBullet = 75;

const unsigned long debounceDelayMs = 200;
const unsigned long travelTimeMs    = 200;

const unsigned long timeForInitialImageDisplayMs = 2000;
const unsigned long debounceDelayFunctionMs      = 800;

const unsigned long waitUntilScrollingTextMs = 2500;

const unsigned long delayBlinkCursor = 500;

unsigned long lastBlinkPlayer;
unsigned long lastBlinkBullet;
unsigned long lastBulletTravel;
unsigned long lastRecordStart;
unsigned long lastCallToFunction;
unsigned long lastBlinkCursor;
unsigned long timeTheGameStartedMs;

bool bulletRequested;
bool showStartImage = true;
bool buttonPressed;
bool skipImage;
bool chooseName;
bool stateSelect;

bool printedFirstHalf;
bool printedSecondHalf;

bool cursorState;
bool playGame;

int numberOfWalls;
int initialNumberOfWalls;

int navMenu      = FIRST_MENU;
int settingsMenu = NO_SELECTION;

char username[16] = {'A' - 1, 'A' - 1, 'A' - 1, 'A' - 1, 'A' - 1, 'A' - 1, 'A' - 1, 'A' - 1,
                     'A' - 1, 'A' - 1, 'A' - 1, 'A' - 1, 'A' - 1, 'A' - 1, 'A' - 1, 'A' - 1};

const char *optionStrings[]  = {"1. Start Game", "2. Settings", "3. About"};
const char *settingStrings[] = {"1.LCD Brightness", "2.Matrix Brightness", "3.Back"};
const char *about[]          = {
             "Game Name:", "Battle Tank", "Author:", "Radu Buzas", "GitHub user:", "radubuzas"};

const int maxLenght = 19; // "2.Matrix Brightness"
const int sizeOfLCD = 16;
const int diff      = maxLenght - sizeOfLCD;

const byte idxLCDBrightness    = 16;
const byte idxMatrixBrightness = 17;
const byte idxBestUser         = 0;
const byte idxHighScore        = 18;

int score;

bool resetState;

struct position
{
    byte row;
    byte column;
};

struct movingObject
{
    struct position position;

    byte headingTo;
};

struct movingObject player;

struct movingObject *bullet;

// int TAMPON;

bool matrix[matrixSize][matrixSize];

const byte IMAGE[matrixSize] = {0b00000000, 0b10010111, 0b10010010, 0b11110010,
                                0b11110010, 0b10010010, 0b10010111, 0b00000000};

const byte HEART[matrixSize] = {0b01100110, 0b11111111, 0b11111111, 0b11111111,
                                0b11111111, 0b01111110, 0b00111100, 0b00011000};

void setup()
{
    // the zero refers to the MAX7219 number, it is zero for 1 chip
    lc.shutdown(0, false); // turn off power saving, enables display
    lc.setIntensity(0, 2); // sets brightness (0~15 possible values)
    lc.clearDisplay(0);    // clear screen

    pinMode(pinSW, INPUT_PULLUP);
    pinMode(pinX, INPUT);
    pinMode(pinY, INPUT);

    randomSeed(analogRead(0));

    attachInterrupt(digitalPinToInterrupt(pinSW), handleInterrupt, FALLING);

    lastRecordStart = millis();

    lcd.begin(16, 2);
    lcd.print(F("Hello, Welcome"));
    lcd.setCursor(0, 1);
    lcd.print((F("To TANK BATTLE!!")));

    pinMode(backLight, OUTPUT);

    byte brightness = EEPROM.read(idxLCDBrightness);
    analogWrite(backLight, brightness);

    brightness = EEPROM.read(idxMatrixBrightness);
    lc.setIntensity(0, brightness);

    Serial.begin(9600);
}

void loop()
{
    if (playGame == false)
    {
        showImage();
        setupGame();
        return;
    }

    showCurrentStatus();

    if (numberOfWalls == 0)
    {
        reset();
        return;
    }

    blinkPlayer();
    showMatrix();

    byte ret = checkMovement();
    if (ret != NO_MOVEMENT)
        movePlayer(ret);

    if (bullet != nullptr)
    {
        bulletTravel();
        blinkBullet();
    }
    if (bulletRequested)
        generateBullet();
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// SETUP GAME LCD

void setupGame()
{
    if (showStartImage)
    {
        if (buttonPressed || millis() > timeForInitialImageDisplayMs)
        {
            showStartImage = false;
            lcd.clear();
            printChooseName();
            lastCallToFunction = millis();
        }
        return;
    }

    if (chooseName)
    {
        chooseName_function();
        return;
    }

    if (stateSelect)
    {
        displayOptions();
        return;
    }
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// SHOW LIVE GAME STATS

void showCurrentStatus()
{
    static bool      initialized     = false;
    static const int messageLenLine0 = 7;
    static const int messageLenLine1 = 6;
    static int       numberOfWallsLocal;
    if (resetState)
        return;
    if (initialized == false)
    {
        initialized        = true;
        numberOfWallsLocal = numberOfWalls;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Score: "));
        lcd.setCursor(0, 1);
        lcd.print(F("Time: "));
    }

    if (numberOfWalls == 0)
    {
        initialized = false;
        score += getScore();
        resetState = true;
    }

    if (numberOfWalls + 1 == numberOfWallsLocal)
        score += getScore(), --numberOfWallsLocal;

    lcd.setCursor(messageLenLine0, 0);
    lcd.print(score);
    lcd.setCursor(messageLenLine1, 1);

    unsigned long timeInGameMs = millis() - timeTheGameStartedMs;

    lcd.print(timeInGameMs / 1000);
    lcd.write('.');
    lcd.print(timeInGameMs % 1000 / 10);
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// PRINT LCD

void printEndOfLevel()
{
    static bool initialized = false;
    if (initialized == false)
    {
        initialized = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Congratulations!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Score: "));
        lcd.print(score);
    }
}

void printFirstHalf()
{
    lcd.clear();
    for (byte i = 0; i < 2; ++i)
    {
        lcd.setCursor(0, i);
        lcd.print(optionStrings[i]);
        printedFirstHalf  = true;
        printedSecondHalf = false;
        lcd.setCursor(0, 0);
    }
}

void printAbout1_4()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(about[0]);
    lcd.setCursor(0, 1);
    lcd.print(about[1]);
}

void printAbout2_4()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("HighScore: "));
    int scoreEEPROM;
    lcd.print(EEPROM.get(idxHighScore, scoreEEPROM));
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; ++i) {
        char c = EEPROM.read(idxBestUser + i);
        if (c == '\0')  break;
        lcd.write(c);
        Serial.print(c);
    }
}

void printAbout3_4()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(about[2]);
    lcd.setCursor(0, 1);
    lcd.print(about[3]);
}

void printAbout4_4()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(about[4]);
    lcd.setCursor(0, 1);
    lcd.print(about[5]);
}

void printSecondHalf()
{
    lcd.clear();
    for (byte i = 1; i <= 2; ++i)
    {
        lcd.setCursor(0, i - 1);
        lcd.print(optionStrings[i]);
    }
    printedFirstHalf  = false;
    printedSecondHalf = true;
}

void printChooseName()
{
    lcd.setCursor(0, 0);
    lcd.print(F("Choose username:"));
    lcd.setCursor(0, 1);
    chooseName = true;
}

void printFirstHalfSettings()
{
    lcd.clear();
    for (byte i = 0; i < 2; ++i)
    {
        lcd.setCursor(0, i);
        lcd.print(settingStrings[i]);
    }
    lcd.setCursor(0, 0);
    printedFirstHalf  = true;
    printedSecondHalf = false;
}

void printSecondHalfSettings()
{
    lcd.clear();
    for (byte i = 1; i <= 2; ++i)
    {
        lcd.setCursor(0, i - 1);
        lcd.print(settingStrings[i]);
    }
    lcd.setCursor(0, 0);
    printedFirstHalf  = false;
    printedSecondHalf = true;
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// CHOOSE USERNAME

void chooseName_function()
{
    static byte thisColumn = 0;

    if (buttonPressed)
    {
        if (millis() - lastCallToFunction > debounceDelayFunctionMs)
        {
            chooseName = false;
            for (byte i = 0; i < 16; ++i)
            {
                if (username[i] == 'A' - 1)
                    username[i] = ' ';
                // EEPROM.update(0, username[i]);
            }
            stateSelect = true;

            cursorState = false;
            lcd.noCursor();
            printFirstHalf();
            lastCallToFunction = millis();
        }
        buttonPressed = false;
    }

    if (millis() - lastBlinkCursor > delayBlinkCursor)
    {
        lastBlinkCursor = millis();

        if (cursorState)
        {
            lcd.noCursor();
            cursorState = false;
        }
        else
        {
            lcd.cursor();
            cursorState = true;
        }
    }
    byte direction = checkMovement();

    if (direction == NO_MOVEMENT)
        return;

    char *thisChar = &username[thisColumn];

    if (direction == RIGHT)
    {
        if (thisColumn + 1 > 15)
            return;
        ++thisColumn;
        if (cursorState)
            lcd.noCursor();
        lcd.setCursor(thisColumn, 1);
        lcd.cursor();
        return;
    }
    else if (direction == LEFT)
    {
        if (thisColumn - 1 < 0)
            return;
        --thisColumn;
        if (cursorState)
            lcd.noCursor();
        lcd.setCursor(thisColumn, 1);
        lcd.cursor();
        return;
    }
    else if (direction == UP)
    {
        *thisChar = *thisChar - 1;
        if (*thisChar < 'A')
            *thisChar = 'z';
        else if (*thisChar < 'a' && *thisChar > 'Z')
            *thisChar = 'Z';
    }
    else
    {
        *thisChar = *thisChar + 1;
        if (*thisChar > 'z')
            *thisChar = 'A';
        else if (*thisChar > 'Z' && *thisChar < 'a')
            *thisChar = 'a';
    }

    lcd.write(*thisChar);
    lcd.setCursor(thisColumn, 1);
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// FIRST LEVEL MENU

// Displays the first level of options and routes the subsections
void displayOptions()
{
    static byte option = 0;

    if (navMenu != FIRST_MENU)
    {
        switch (navMenu)
        {
        case PLAY_GAME:
            return;
        case SETTINGS:
            handdleSettings();
            return;
        case ABOUT:
            handdleAbout();
            return;
        case EXIT_MENU_STATE:
            option  = 0;
            navMenu = FIRST_MENU;
            Serial.println(F("Returned to settings menu"));
            break;
        default:
            Serial.println(F("Bad memory leak!"));
            return;
        }
    }

    lcd.blink();

    if (buttonPressed)
    {
        buttonPressed = false;
        if (millis() - lastCallToFunction > debounceDelayFunctionMs)
        {
            unsigned long temp = lastCallToFunction;
            lastCallToFunction = millis();
            switch (option)
            {
            case 0:
                playGame = true;
                initMap();
                timeTheGameStartedMs = millis();
                return;
            case 1:
                navMenu = SETTINGS;
                return;
            case 2:
                navMenu = ABOUT;
                return;
            default:
                Serial.println(F("Option has wrong value!"));
                lastCallToFunction = temp;
                return;
            }
        }
    }

    byte direction = checkMovement();

    if (direction == NO_MOVEMENT || direction == LEFT || direction == RIGHT)
        return;

    if (direction == UP)
    {
        if (++option > 2)
            option = 2;
    }
    else
    {
        if (option-- == 0)
            option = 0;
    }

    if (option == 0)
    {
        if (printedFirstHalf == false)
        {
            printFirstHalf();
        }
        lcd.setCursor(0, 0);
    }
    else if (option == 2)
    {
        if (printedSecondHalf == false)
        {
            printSecondHalf();
        }
        lcd.setCursor(0, 1);
    }
    else
    {
        if (printedSecondHalf == true)
            lcd.setCursor(0, 0);
        else
            lcd.setCursor(0, 1);
    }
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// HANDDLE MENU

void handdleAbout()
{
    static byte option      = 0;
    static bool initialized = false;
    if (initialized == false)
    {
        initialized = true;
        printAbout1_4();
    }

    if (buttonPressed)
    {
        buttonPressed = false;
        if (millis() - lastCallToFunction > debounceDelayFunctionMs)
        {
            lastCallToFunction = millis();

            initialized = false;
            navMenu     = EXIT_MENU_STATE;
            printFirstHalf();
            return;
        }
    }

    byte direction = checkMovement();

    if (direction == NO_MOVEMENT || direction == LEFT || direction == RIGHT)
        return;

    if (direction == UP)
    {
        if (++option > 3)
            option = 3;
    }
    else
    {
        if (option-- == 0)
            option = 0;
    }

    switch (option)
    {
    case 0:
        printAbout1_4();
        return;
    case 1:
        printAbout2_4();
        return;
    case 2:
        printAbout3_4();
        return;
    case 3:
        printAbout4_4();
        return;
    default:
        Serial.println(F("Lol?"));
        return;
    }
}

void handdleSettings()
{
    static unsigned long lastMove = millis();

    static byte option      = 0;
    static bool check       = false;
    static byte counter     = 0;
    static bool initialized = false;
    if (initialized == false)
    {
        initialized = true;
        lastMove    = millis();
        printFirstHalfSettings();
    }

    if (check)
    {
        if (settingsMenu == NO_SELECTION)
        {
            settingsMenu = option;
        }

        switch (settingsMenu)
        {
        case 0:
            settingsMenu = 0;
            LCDBrightness();
            return;
        case 1:
            settingsMenu = 1;
            matrixBrightness();
            return;
        case 2: //  back
            check = false;
            printFirstHalf();
            navMenu      = EXIT_MENU_STATE;
            settingsMenu = NO_SELECTION;
            initialized  = false;
            option       = 0;
            return;
        case EXIT_MENU_STATE:
            check = false;
            printFirstHalfSettings();
            settingsMenu = NO_SELECTION;
            initialized  = false;
            option       = 0;
            return;
        default:
            Serial.println(F("WHAT?!"));
            return;
        }
    }

    if (buttonPressed)
    {
        buttonPressed = false;
        if (millis() - lastCallToFunction > debounceDelayFunctionMs)
        {
            lastCallToFunction = millis();
            check              = true;
            return;
        }
    }

    if (millis() - lastMove > waitUntilScrollingTextMs)
    {
        const unsigned long smoothness = 1000;
        if (millis() - lastMove > smoothness * counter + waitUntilScrollingTextMs)
        {
            if (++counter <= diff)
            {
                lcd.scrollDisplayLeft();
            }
            else
            {
                lcd.scrollDisplayRight();
                if (counter == 2 * diff)
                {
                    counter  = 0;
                    lastMove = millis();
                }
            }
        }
    }

    byte direction = checkMovement();

    if (direction == NO_MOVEMENT || direction == LEFT || direction == RIGHT)
        return;

    if (direction == UP)
    {
        if (++option > 2)
            option = 2;
    }
    else
    {
        if (option-- == 0)
            option = 0;
    }

    if (counter <= 3)
    {
        for (; counter > 0; --counter)
            lcd.scrollDisplayRight();
    }
    else
    {
        for (; counter < 6; ++counter)
            lcd.scrollDisplayRight();
        counter = 0;
    }

    if (option == 0)
    {
        if (printedFirstHalf == false)
        {
            printFirstHalfSettings();
        }
        lcd.setCursor(0, 0);
    }
    else if (option == 2)
    {
        if (printedSecondHalf == false)
        {
            printSecondHalfSettings();
        }
        lcd.setCursor(0, 1);
    }
    else
    {
        if (printedSecondHalf == true)
            lcd.setCursor(0, 0);
        else
            lcd.setCursor(0, 1);
    }

    lastMove = millis();
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// SETTINGS SUBMENU

void LCDBrightness()
{
    static byte brightness  = EEPROM.read(idxLCDBrightness);
    static byte no_chars    = (brightness >= 100) ? 3 : (brightness >= 10 ? 2 : 1);
    static bool initialized = false;
    if (initialized == false)
    {
        initialized = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("LCD"));
        lcd.setCursor(0, 1);
        lcd.print(F("Brightness: "));
        lcd.print(brightness);
        lcd.setCursor(0 + 12 + no_chars - 1, 1);
    }

    if (buttonPressed)
    {
        buttonPressed = false;
        if (millis() - lastCallToFunction > debounceDelayFunctionMs)
        {
            unsigned long temp = lastCallToFunction;
            lastCallToFunction = millis();
            EEPROM.update(idxLCDBrightness, brightness);
            analogWrite(backLight, brightness);
            settingsMenu = EXIT_MENU_STATE;
            initialized  = false;
            return;
        }
    }

    byte direction = checkMovement();

    if (direction == NO_MOVEMENT)
        return;

    if (direction == DOWN || direction == RIGHT)
    {
        if (brightness++ == 255)
            brightness = 255;
    }
    else
    {
        if (brightness-- == 0)
            brightness = 0;
    }

    no_chars = (brightness >= 100) ? 3 : (brightness >= 10 ? 2 : 1);

    lcd.setCursor(0 + 12, 1);
    lcd.print(brightness);
    lcd.setCursor(0 + 12 + no_chars, 1);
    if (brightness == 99 || brightness == 9)
        lcd.write(' ');
    lcd.setCursor(0 + 12 + no_chars - 1, 1);
}

void matrixBrightness()
{
    static byte brightness = EEPROM.read(idxMatrixBrightness);
    static byte no_chars   = (brightness >= 10) ? 2 : 1;
    static bool initialized;
    if (initialized == false)
    {
        initialized = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Matrix"));
        lcd.setCursor(0, 1);
        lcd.print(F("Brightness: "));
        lcd.print(brightness);
        lcd.setCursor(0 + 12 + no_chars - 1, 1);
    }

    if (buttonPressed)
    {
        buttonPressed = false;
        if (millis() - lastCallToFunction > debounceDelayFunctionMs)
        {
            unsigned long temp = lastCallToFunction;
            lastCallToFunction = millis();
            EEPROM.update(idxMatrixBrightness, brightness);
            lc.setIntensity(0, brightness);
            settingsMenu = EXIT_MENU_STATE;
            initialized  = false;
            return;
        }
    }

    byte direction = checkMovement();

    if (direction == NO_MOVEMENT)
        return;

    if (direction == DOWN || direction == RIGHT)
    {
        if (brightness++ == 15)
            brightness = 15;
    }
    else
    {
        if (brightness-- == 0)
            brightness = 0;
    }

    no_chars = (brightness >= 10) ? 2 : 1;

    lcd.setCursor(0 + 12, 1);
    lcd.print(brightness);
    lcd.setCursor(0 + 12 + no_chars, 1);
    if (brightness == 9)
        lcd.write(' ');
    lcd.setCursor(0 + 12 + no_chars - 1, 1);
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// CALCULATE SCORE

int getScore()
{
    static const int   initialScore = 150;
    static const float maxTimeSec   = 40;

    // Linear interpolation using integer arithmetic
    int timeSec = (millis() - timeTheGameStartedMs) / 1000;

    if (maxTimeSec - timeSec < 0)
        return 0;

    Serial.println((maxTimeSec - timeSec) / maxTimeSec);

    return initialScore * ((maxTimeSec - timeSec) / maxTimeSec);
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// ENVIRONMENT

//  Displays initial image
void showImage()
{
    // if (millis() - lastRecordStart > timeForInitialImageDisplayMs || skipImage == true)
    // {
    //     showStartImage = false;
    //     skipImage      = false;
    //     initMap();
    // }
    for (int row = 0; row < matrixSize; ++row)
    {
        lc.setRow(0, row, IMAGE[row]);
    }
}

//  Upon pressing the button, the map will reset
void reset()
{
    // if (skipImage)
    // {
    //     showStartImage  = true;
    //     lastRecordStart = millis();
    //     skipImage       = false;
    //     return;
    // }

    static unsigned long timeEnteredMs;
    static bool          initialized = false;
    if (initialized == false)
    {
        initialized   = true;
        timeEnteredMs = millis();
        int highScore = EEPROM.read(idxHighScore);
        if (score > highScore)
        {
            for (byte i = 0; i < 16; ++i)
            {
                EEPROM.update(idxBestUser + i, username[i]);
            }
            EEPROM.put(idxHighScore, score);
        }
        printEndOfLevel();
    }

    if (buttonPressed)
    {
        buttonPressed = false;
        if (millis() - timeEnteredMs > debounceDelayFunctionMs)
        {
            playGame    = false;
            initialized = false;
            resetState  = false;
            score       = 0;
            printFirstHalf();
            return;
        }
    }

    for (int row = 0; row < matrixSize; ++row)
    {
        lc.setRow(0, row, HEART[row]);
    }
}

// Generates random structures
void initMap()
{
    for (int i = 0; i < matrixSize; ++i)
    {
        for (int j = 0; j < matrixSize; ++j)
        {
            matrix[i][j] = random(10) % 2;
            numberOfWalls += matrix[i][j];
        }
    }

    player.position.row    = random(40) % 8;
    player.position.column = random(40) % 8;

    player.headingTo = RIGHT;

    numberOfWalls -= matrix[player.position.row][player.position.column];

    initialNumberOfWalls = numberOfWalls;

    matrix[player.position.row][player.position.column] = 1;
}

//  Will display to current configuration of the matrix
void showMatrix()
{
    for (int i = 0; i < matrixSize; i++)
    {
        for (int j = 0; j < matrixSize; j++)
        {
            lc.setLed(0, i, j, matrix[i][j]);
        }
    }
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// BLINKING

//  Makes the player blink ON and OFF
void blinkPlayer()
{
    if (millis() - lastBlinkPlayer > blinkingDelayPlayer)
    {
        int playerX              = player.position.row;
        int playerY              = player.position.column;
        matrix[playerX][playerY] = !matrix[playerX][playerY];

        lastBlinkPlayer = millis();
    }
}

//  Makes the bullet blink ON and OFF. Remember that there is only one bullet
void blinkBullet()
{
    if (bullet == nullptr)
        return;

    if (millis() - lastBlinkBullet > blinkingDelayBullet)
    {
        int bulletX              = bullet->position.row;
        int bulletY              = bullet->position.column;
        matrix[bulletX][bulletY] = !matrix[bulletX][bulletY];

        lastBlinkBullet = millis();
    }
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//

// Will go according to the controller
void movePlayer(byte move)
{
    player.headingTo = move;

    byte newPosRow    = player.position.row + (move == UP) - (move == DOWN);
    byte newPosColumn = player.position.column + (move == RIGHT) - (move == LEFT);

    if (newPosRow < 0 || newPosRow > 7 || newPosColumn < 0 || newPosColumn > 7) //  invalid action
        return;

    if (playerCollision(newPosRow, newPosColumn))
        return;

    matrix[player.position.row][player.position.column] = 0;

    player.position.row    = newPosRow;
    player.position.column = newPosColumn;

    matrix[player.position.row][player.position.column] = 1; //  smooth transition

    lastBlinkPlayer = millis(); //  smooth transition
}

//  check is the player will hit a wall
bool playerCollision(byte row, byte column)
{
    if (matrix[row][column] == 1) //  invalid action
    {
        return true;
    }
    return false;
}

// creating a bullet object based on the player's position and orientation
void generateBullet()
{
    bulletRequested = false;
    if (bullet != nullptr)
        return;

    bullet = (movingObject *)malloc(sizeof(movingObject));

    byte headingTo    = player.headingTo;
    byte tempRow      = player.position.row + (headingTo == UP) - (headingTo == DOWN);
    byte tempColumn   = player.position.column + (headingTo == RIGHT) - (headingTo == LEFT);
    bullet->headingTo = headingTo;

    if (tempRow < 0 || tempRow > 7 || tempColumn < 0 || tempColumn > 7) //  invalid action
    {
        free(bullet);
        bullet = nullptr;
        return;
    }

    if (bulletCollision(tempRow, tempColumn))
        return;

    bullet->position.row    = tempRow;
    bullet->position.column = tempColumn;

    matrix[bullet->position.row][bullet->position.column] = 1;
}

void bulletTravel()
{
    if (millis() - lastBulletTravel > travelTimeMs)
    {

        byte headingTo = bullet->headingTo;

        matrix[bullet->position.row][bullet->position.column] = 0;

        byte tempRow    = bullet->position.row + (headingTo == UP) - (headingTo == DOWN);
        byte tempColumn = bullet->position.column + (headingTo == RIGHT) - (headingTo == LEFT);

        lastBulletTravel = millis();

        if (tempRow < 0 || tempRow > 7 || tempColumn < 0 || tempColumn > 7) //  invalid action
        {
            free(bullet);
            bullet = nullptr;
            return;
        }

        if (bulletCollision(tempRow, tempColumn))
            return;

        matrix[bullet->position.row][bullet->position.column] = 0;

        bullet->position.row    = tempRow;
        bullet->position.column = tempColumn;

        matrix[bullet->position.row][bullet->position.column] = 1;
    }
}

// the bullet will hit a wall
bool bulletCollision(byte row, byte column)
{
    if (matrix[row][column] == 1) //  invalid action
    {
        --numberOfWalls; //  trying to scount the broken walls
        free(bullet);
        bullet = nullptr;

        matrix[row][column] = 0;
        return true;
    }

    return false;
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//
// JOYSTICK

//  This function checks the movement of the joystick
byte checkMovement()
{
    static bool xMoved = false;
    static bool yMoved = false;

    int xRead = analogRead(pinY);
    int yRead = analogRead(pinX);

    if (xRead < minThreshold && xMoved == false)
    {
        xMoved = true;
        return LEFT;
    }

    if (xRead > maxThreshold && xMoved == false)
    {
        xMoved = true;
        return RIGHT;
    }

    if (yRead < minThreshold && yMoved == false)
    {
        yMoved = true;
        return UP;
    }

    if (yRead > maxThreshold && yMoved == false)
    {
        yMoved = true;
        return DOWN;
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
    return NO_MOVEMENT;
}

//--------------------------------------------------------------------//--------------------------------------------------------------------//

//  When the button is pressed, this ISR will be triggered
void handleInterrupt()
{
    static unsigned long lastInterrupt = 0;

    unsigned long now = micros();

    if (now - lastInterrupt > debounceDelayMs * 1000)
    {
        bulletRequested = true;
        if (numberOfWalls == 0)
            skipImage = true;
        buttonPressed = true;
        lastInterrupt = now;
    }
}