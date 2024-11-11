#include <Arduino.h>
/* #include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
    lcd.init();
    lcd.backlight();
}

void loop() {
    for (int i = 0; i < 4; i++) {
        for (int y = 0; y < 20; y++) {
            lcd.setCursor(y, i);
            lcd.print("X");
            delay(500);
            lcd.clear();
        }
    }
} */

#include <Stepper.h>

const int stepsPerRevolution = 200; // Change this to match your motor's steps per revolution
const int dirPin = 2;
const int stepPin = 3;

Stepper myStepper(stepsPerRevolution, stepPin, dirPin);

void setup() {
  myStepper.setSpeed(500); // Adjust the speed as needed
}

void loop() {
  // Turn the motor left
  myStepper.step(stepsPerRevolution);
  delay(1000);
}