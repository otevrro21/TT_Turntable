/*
    DMP 2024/25 Rostislav Kaili Otevrel - Railway model turntable

    Total rail positions: 9
    Home position is denoted as -1
    Serial commands:
    H for homing
    P and a number from 0 to 8 for position
    X for alraedy at the target position
    S for stop

    E for reseting program after estop
    C for continuing after estop
*/


//TODO: setting of the target positions from control panel
//TODO: implementing the sensors (track DONE, bridge NOT DONE)
//TODO: e-stop implementation
//TODO: PLP

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

unsigned long bridgeSensorPreviousTime = 0; // for timer function

const int stepsPerRevolution = 3200; // steps per revolution of the stepper motor (200 * 16 for microstepping)
int stepsFromHome = 0; // steps from the home position
int currentBridgePosition = 0; // current position of the bridge (side A)
int targetRailPosition[] = {951, 1235, 1387, 1529, 1911, 2169, 2293, 2409, 2560}; // positions of the rails
bool bridgeInMotion = false; // for checking if the bridge is in motion and saving it to EEPROM for PLP purposes
// bool trainOnBridge = false; // for checking if the train is on the bridge and saving it to EEPROM for PLP purposes
bool autoPilotEnabled = false; // to not have X sent to the control panel when track sensor gets triggered more than once

//* FUNCTION PROTOTYPES: -----------------------------------------------------------------------

void home();
void moveToPosition(int targetPositionSelection);
void driveMotor(int stepsToTake, int driveSpeed); // driveSpeed is a dividor so the higher number the faster the motor speed
void motorEnable(bool direction), motorDisable();
void estop(int stepsTaken, int stepsToTake);

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
    // Listening for incoming commands from the control panel
    if (Serial.available() > 0) {
        char command = Serial.read();
        if (command == 'H' && bridgeInMotion == false && autoPilotEnabled == false) {
            home();
        } else if (command == 'P' && bridgeInMotion == false && autoPilotEnabled == false) {
            // Wait for the second character
            delay(10); // Small delay to ensure next character arrives (im so sorry)
            if (Serial.available() > 0) {
                char posChar = Serial.read();
                int pos = posChar - '0';
                if (pos >= 0 && pos <= 8) {
                    moveToPosition(pos);
                }
            }
        } else if (command == 'S') {
            estop(NULL, NULL);
        }
    }
    // Automatic turning to track 1 if a train is detected
    if (digitalRead(TRACK_SENS_1) == LOW && bridgeInMotion == false) {
        if (bridgeInMotion == false) {
            autoPilotEnabled = true;
            moveToPosition(0);
            autoPilotEnabled = false;
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
    driveMotor(100, 2); // drive the motor back 100 steps
    motorDisable();

    currentBridgePosition = -1;
    // right now the bridge should be at the home position, which is 100 steps back from the limit switch
    Serial.print('H'); // message to the control panel that the bridge is at the home position
}

//* Drive direction and steps to take:

void moveToPosition(int targetPositionSelection) {
    if (targetPositionSelection == currentBridgePosition && autoPilotEnabled == false) {
        Serial.print('X'); // message to the control panel that the bridge is already at the target position
    } else if (targetPositionSelection > currentBridgePosition) {
        motorEnable(CCW);
        driveMotor(targetRailPosition[targetPositionSelection] - stepsFromHome, 4); //! slow down after testing
        motorDisable();
        currentBridgePosition = targetPositionSelection;
        Serial.print('P');
        Serial.print(currentBridgePosition);// message to the control panel that the bridge is at the target position
    } else if (targetPositionSelection < currentBridgePosition) {
        motorEnable(CW);
        driveMotor(stepsFromHome - targetRailPosition[targetPositionSelection], 4); //! slow down after testing
        motorDisable();
        currentBridgePosition = targetPositionSelection;
        Serial.print('P');
        Serial.print(currentBridgePosition);// message to the control panel that the bridge is at the target position
    }
}

//* Driving the motor and setting the speed:

void driveMotor(int stepsToTake, int driveSpeed) {
    int stepTime = 4000 / driveSpeed;
    int stepsTaken = 0;

    bridgeInMotion = true;

    while (stepsTaken != stepsToTake) {
        digitalWrite(STEP, HIGH);
        delayMicroseconds(stepTime);
        digitalWrite(STEP, LOW);
        delayMicroseconds(stepTime);
        stepsTaken++;
        (digitalRead(DIR) == HIGH) ? stepsFromHome-- : stepsFromHome++;
        if (Serial.read() == 'S') {
            estop(stepsTaken, stepsToTake);
            break;
        }
    }
    digitalWrite(STEP, LOW);

    bridgeInMotion = false;
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

//* Logic for E-Stop

void estop(int stepsTaken, int stepsToTake) {
    digitalWrite(EN_PIN, LOW);
    digitalWrite(STEP, LOW);
    Serial.print('S'); // message to the control panel that the bridge is stopped
    while (Serial.available() == 0) {
        // do nothing until the control panel sends a command
    }
    char afterStopCommand = Serial.read();
    if (afterStopCommand == 'C') {
        if (bridgeInMotion == true) {
            if (digitalRead(DIR) == HIGH) {
                motorEnable(CW);
                driveMotor(stepsToTake - stepsTaken, 4); //! slow down after testing
                motorDisable();
            } else {
                motorEnable(CCW);
                driveMotor(stepsToTake - stepsTaken, 4); //! slow down after testing
                motorDisable();
            }
        }
    } else if (afterStopCommand == 'E') {
        home();
    }
}