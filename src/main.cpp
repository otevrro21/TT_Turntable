//* LIBRARIES: ---------------------------------------------------------------------------------

#include <Arduino.h>

//* PINS: --------------------------------------------------------------------------------------

//* Sensors:

const int BRIDGE_SENS_A = 9;
const int BRIDGE_SENS_B = 10;
const int TRACK_SENS_1 = 11;
const int LIMIT_SW = 12;

//* Stepper Motor:

const int EN_PIN = 14;
const int DIR = 15;
const int STEP = 16;

//! GLOBAL VARIABLES: --------------------------------------------------------------------------

const int stepsPerRevolution = 3200; // It's actually 3200 but the library is broken...
int stepCount = 0;

//* OBJECTS: -----------------------------------------------------------------------------------


//* FUNCTION PROTOTYPES: ----------------------------------------------------------------------

void home(), test(int dir);

//* MAIN CODE: ---------------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);

    pinMode(EN_PIN, OUTPUT);
    pinMode(DIR, OUTPUT);
    pinMode(STEP, OUTPUT);

    pinMode(BRIDGE_SENS_A, INPUT);
    pinMode(BRIDGE_SENS_B, INPUT);
    pinMode(TRACK_SENS_1, INPUT);
    pinMode(LIMIT_SW, INPUT);

    digitalWrite(EN_PIN, LOW);

}

void loop()
{
    if (Serial.available() > 0) {
        char incomingByte = Serial.read();
        Serial.println(incomingByte);
        if (incomingByte == 'A' || incomingByte == 'B') {
            test(incomingByte);
        }
        else
        if (incomingByte == 'H') {
            home();
        }
    }
}

void home()
{
    digitalWrite(EN_PIN, HIGH);
    digitalWrite(DIR, HIGH);
    while (digitalRead(LIMIT_SW) == HIGH) {
        digitalWrite(STEP, HIGH);
        delayMicroseconds(1000);
        digitalWrite(STEP, LOW);
        delayMicroseconds(1000);
    }
    digitalWrite(STEP, LOW);
    digitalWrite(EN_PIN, LOW);
    stepCount = 0;
}

void test(int dir)
{
    if (dir == 'A') {
        digitalWrite(DIR, HIGH);
    }
    else if (dir == 'B') {
        digitalWrite(DIR, LOW);
    }
    digitalWrite(EN_PIN, HIGH);
    for (int i = 0; i < stepsPerRevolution/4; i++) {
        digitalWrite(STEP, HIGH);
        delayMicroseconds(500);
        digitalWrite(STEP, LOW);
        delayMicroseconds(500);
    }
    digitalWrite(EN_PIN, LOW);
}