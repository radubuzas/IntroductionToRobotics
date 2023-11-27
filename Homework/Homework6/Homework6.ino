#include "LedControl.h" // need the library

#define LEFT  0
#define RIGHT 1
#define UP    2
#define DOWN  3

#define NO_MOVEMENT 255

const int dinPin   = 12;
const int clockPin = 11;
const int loadPin  = 10;

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

const unsigned long debounceDelay = 200;
const unsigned long travelTime    = 200;

const unsigned long timeForInitialImageDisplay = 2000;

unsigned long lastBlinkPlayer;
unsigned long lastBlinkBullet;
unsigned long lastBulletTravel;
unsigned long lastRecordStart;

bool bulletRequested;
bool showStartImage = true;

int score = 0;

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

    Serial.begin(9600);
}

void loop()
{
    if (showStartImage)
    {
        showImage();
        return;
    }

    if (score == 0){
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


//  Displays initial image
void showImage()
{
    if (millis() - lastRecordStart > timeForInitialImageDisplay || bulletRequested == true)
    {
        showStartImage  = false;
        bulletRequested = false;
        initMap();
    }
    for (int row = 0; row < matrixSize; ++row)
    {
        lc.setRow(0, row, IMAGE[row]);
    }
}


//  Upon pressing the button, the map will reset
void reset()
{
    if (bulletRequested)
    {
        showStartImage  = true;
        lastRecordStart = millis();
        bulletRequested = false;
        return;
    }

    for (int row = 0; row < matrixSize; ++row)
    {
        lc.setRow(0, row, HEART[row]);
    }
}

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

// Generates random structures 
void initMap()
{
    for (int i = 0; i < matrixSize; ++i)
    {
        for (int j = 0; j < matrixSize; ++j)
        {
            matrix[i][j] = random(10) % 2;
            score += matrix[i][j];
        }
    }

    player.position.row    = random(40) % 8;
    player.position.column = random(40) % 8;

    player.headingTo = RIGHT;

    score -= matrix[player.position.row][player.position.column];

    matrix[player.position.row][player.position.column] = 1;
}

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
    if (millis() - lastBulletTravel > travelTime)
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
        --score;    //  trying to scount the broken walls
        free(bullet);
        bullet = nullptr;

        matrix[row][column] = 0;
        return true;
    }

    return false;
}

//  When the button is pressed, this ISR will be triggered
void handleInterrupt()
{
    static unsigned long lastInterrupt = 0;

    unsigned long now = micros();

    if (now - lastInterrupt > debounceDelay * 1000)
    {
        bulletRequested = true;
        lastInterrupt   = now;
    }
}