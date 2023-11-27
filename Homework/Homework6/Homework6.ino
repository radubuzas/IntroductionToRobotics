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

const unsigned long blinkingDelayPlayer = 500;
const unsigned long blinkingDelayBullet = 75;

const unsigned long debounceDelay = 200;
const unsigned long travelTime    = 200;

unsigned long lastBlinkPlayer;
unsigned long lastBlinkBullet;
unsigned long lastBulletTravel;

bool bulletRequested;

struct position
{
    byte row;
    byte column;
};

struct movingObject
{
    struct position position;
    byte            headingTo;
};

struct movingObject player;

struct movingObject *bullet;

// int TAMPON;

bool matrix[matrixSize][matrixSize];

void setup()
{
    // the zero refers to the MAX7219 number, it is zero for 1 chip
    lc.shutdown(0, false); // turn off power saving, enables display
    lc.setIntensity(0, 2); // sets brightness (0~15 possible values)
    lc.clearDisplay(0);    // clear screen

    pinMode(pinSW, INPUT_PULLUP);
    pinMode(pinX, INPUT);
    pinMode(pinY, INPUT);

    initBorder();

    player.position.row    = 6;
    player.position.column = 1;

    player.headingTo = RIGHT;

    matrix[player.position.row][player.position.column] = 1;

    attachInterrupt(digitalPinToInterrupt(pinSW), handleInterrupt, FALLING);

    Serial.begin(9600);
}

void loop()
{
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

void initBorder()
{
    for (int i = 0; i < matrixSize; ++i)
    {
        matrix[0][i]              = 1; //  first row
        matrix[matrixSize - 1][i] = 1; //  last row

        matrix[i][0]              = 1; //  first column
        matrix[i][matrixSize - 1] = 1; //  last column
    }
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

void movePlayer(byte move)
{
    player.headingTo = move;

    byte newPosRow    = player.position.row + (move == UP) - (move == DOWN);
    byte newPosColumn = player.position.column + (move == RIGHT) - (move == LEFT);

    if (newPosRow < 1 || newPosRow > 6 || newPosColumn < 1 || newPosColumn > 6) //  invalid action
        return;

    matrix[player.position.row][player.position.column] = 0;

    player.position.row    = newPosRow;
    player.position.column = newPosColumn;

    matrix[player.position.row][player.position.column] = 1; //  smooth transition

    lastBlinkPlayer = millis(); //  smooth transition
}

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

    if (checkCollision(tempRow, tempColumn))
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

        // Serial.print("Row: "), Serial.println(bullet->position.row);
        // Serial.print("Column: "), Serial.println(bullet->position.column);
        // Serial.println();

        byte tempRow    = bullet->position.row + (headingTo == UP) - (headingTo == DOWN);
        byte tempColumn = bullet->position.column + (headingTo == RIGHT) - (headingTo == LEFT);

        lastBulletTravel = millis();

        if (tempRow < 0 || tempRow > 7 || tempColumn < 0 || tempColumn > 7) //  invalid action
        {
            free(bullet);
            bullet = nullptr;
            return;
        }

        if (checkCollision(tempRow, tempColumn))
            return;

        matrix[bullet->position.row][bullet->position.column] = 0;

        bullet->position.row    = tempRow;
        bullet->position.column = tempColumn;

        matrix[bullet->position.row][bullet->position.column] = 1;
    }
}

bool checkCollision(byte row, byte column)
{
    if (matrix[row][column] == 1) //  invalid action
    {
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
