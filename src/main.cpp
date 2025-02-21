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


//TODO: setting of the target positions from control panel
//TODO: implementing the sensors
//TODO: e-stop implementation

//* LIBRARIES: ---------------------------------------------------------------------------------

#include <Arduino.h>

//* DEFINES: -----------------------------------------------------------------------------------

// yes, i know, im a monster (:
#define CW true
#define CCW false

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


//* FUNCTION PROTOTYPES: -----------------------------------------------------------------------

void home();
void moveToPosition(int targetPositionSelection);
void driveMotor(int stepsToTake, int driveSpeed); // driveSpeed is a dividor so the higher number the faster the motor speed
void motorEnable(bool direction), motorDisable();

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

//* FUNCITON DEFINITIONS: ----------------------------------------------------------------------

//* Homing the bridge:

void home() {
    motorEnable(CW);
    while (digitalRead(LIMIT_SW) == HIGH) { // drive motor until it hits the limit switch
        digitalWrite(STEP, HIGH);
        delayMicroseconds(2000);
        digitalWrite(STEP, LOW);
        delayMicroseconds(2000);
    }
    motorDisable();

    stepsFromHome = 0;

    motorEnable(CCW);
    driveMotor(100, 1); // drive the motor back 100 steps
    motorDisable();

    currentBridgePosition = -1;
    // right now the bridge should be at the home position, which is 100 steps back from the limit switch
    Serial.print('H'); // message to the control panel that the bridge is at the home position
}

//* Drive directin and steps to take:

void moveToPosition(int targetPositionSelection) {
    if (targetPositionSelection == currentBridgePosition) {
        Serial.print('X'); // message to the control panel that the bridge is already at the target position
    } else if (targetPositionSelection > currentBridgePosition) {
        motorEnable(CCW);
        driveMotor(targetRailPosition[targetPositionSelection] - stepsFromHome, 2);
        motorDisable();
        currentBridgePosition = targetPositionSelection;
        Serial.print('P');
        Serial.print(currentBridgePosition);// message to the control panel that the bridge is at the target position
    } else if (targetPositionSelection < currentBridgePosition) {
        motorEnable(CW);
        driveMotor(stepsFromHome - targetRailPosition[targetPositionSelection], 2);
        motorDisable();
        currentBridgePosition = targetPositionSelection;
        Serial.print('P');
        Serial.print(currentBridgePosition);// message to the control panel that the bridge is at the target position
    }
}

//* Driving the motor and setting the speed:

void driveMotor(int stepsToTake, int driveSpeed) {
    int stepTime = 2000 / driveSpeed;
    int stepsTaken = 0;

    while (stepsTaken != stepsToTake) {
        digitalWrite(STEP, HIGH);
        delayMicroseconds(stepTime);
        digitalWrite(STEP, LOW);
        delayMicroseconds(stepTime);
        stepsTaken++;
        (digitalRead(DIR) == HIGH) ? stepsFromHome-- : stepsFromHome++;
    }
    digitalWrite(STEP, LOW);
}

//* Enabling/Disabling the motor & setting the direction:

void motorEnable(bool direction) { // enable the motor and set the direction (true for CW, false for CCW)
    digitalWrite(EN_PIN, HIGH);
    digitalWrite(DIR, (direction) ? HIGH : LOW);
}

void motorDisable() { // disable the motor and make sure step pin is low
    digitalWrite(EN_PIN, LOW);
    digitalWrite(DIR, LOW);
    if (digitalRead(STEP) == HIGH) {
        digitalWrite(STEP, LOW);
    }
}