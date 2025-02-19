/*
    DMP 2024/25 Rostislav Kaili Otevrel - Railway model turntable

    Total rail positions: 9
    Home position is denoted as -1
    Serial commands:
    H for homing
    P and a number from 0 to 8 for position
    X for alraedy at the target position
    S for stop
*/

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

const int stepsPerRevolution = 3200; // steps per revolution of the stepper motor (200 * 16 for microstepping)
int stepsFromHome = 0; // steps from the home position
int currentBridgePosition = 0; // current position of the bridge (side A)
int targetRailPosition[] = {1000, 1350, 1500, 1600, 2000, 2200, 2250, 2300, 2350}; // positions of the rails in steps from homing position

//* OBJECTS: -----------------------------------------------------------------------------------


//* FUNCTION PROTOTYPES: ----------------------------------------------------------------------

void home(), moveToPosition(int targetPositionSelection), driveMotor(int stepsToTake, int direction);

//* MAIN CODE: ---------------------------------------------------------------------------------

void setup() {
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

void loop() {
    if (Serial.available() > 0) {
        char command = Serial.read();
        if (command == 'H') {
            home();
        } else if (command == 'P') {
            // Wait for the second character
            delay(10); // Small delay to ensure next character arrives (im so sorry)
            if (Serial.available() > 0) {
                char posChar = Serial.read();
                int pos = posChar - '0';
                if (pos >= 0 && pos <= 8) {
                    moveToPosition(pos);
                }
            }
        }
    }
}

void home() {
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
    currentBridgePosition = -1;
    stepsFromHome = 0;
}

void moveToPosition(int targetPositionSelection) { // logic for direction and steps to take to reach the target position
    if (currentBridgePosition < targetPositionSelection || currentBridgePosition > targetPositionSelection) {
        if (currentBridgePosition < targetPositionSelection) {
            driveMotor(targetRailPosition[targetPositionSelection] - stepsFromHome, LOW);
            currentBridgePosition++;
        } else if (currentBridgePosition > targetPositionSelection) {
            driveMotor(stepsFromHome - targetRailPosition[targetPositionSelection], HIGH);
            currentBridgePosition--;
        }
    } else if (currentBridgePosition == targetPositionSelection) {
        Serial.println("X"); // if the bridge is at the target position X for control panel
    }
}

void driveMotor(int stepsToTake, int direction) { // drive the motor a certain number of steps in a certain direction
    digitalWrite(EN_PIN, HIGH);
    digitalWrite(DIR, direction);
    for (int i = 0; i < stepsToTake; i++) {
        digitalWrite(STEP, HIGH);
        delayMicroseconds(500);
        digitalWrite(STEP, LOW);
        delayMicroseconds(500);
        stepsFromHome++;
    }
    digitalWrite(EN_PIN, LOW);
}