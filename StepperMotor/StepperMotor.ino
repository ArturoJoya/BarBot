/*Example sketch to control a stepper motor with A4988 stepper motor driver and Arduino without a library. More info: https://www.makerguides.com */
#include <AccelStepper.h>

// Define stepper motor connections and steps per revolution:
#define dirPin 2
#define stepPin 3
#define dir2Pin 4
#define step2Pin 5
#define dir3Pin 7
#define step3Pin 6
#define dir4Pin 8
#define step4Pin 9
#define mit 1
#define reset 8

#define stepsPerRevolution 200

AccelStepper pump1 = AccelStepper(mit, stepPin, dirPin);
AccelStepper pump2 = AccelStepper(mit, step2Pin, dir2Pin);
AccelStepper pump3 = AccelStepper(mit, step3Pin, dir3Pin);
AccelStepper pump4 = AccelStepper(mit, step4Pin, dir4Pin);

void setup() {
  // Declare pins as output:
  Serial.begin(115200);
  pump1.setMaxSpeed(1000);
  pump1.setAcceleration(50);
  pump1.setSpeed(200);
  pump1.moveTo(5000000);
  pump2.setMaxSpeed(1000);
  pump2.setAcceleration(50);
  pump2.setSpeed(200);
  pump2.moveTo(5000000);
  pump3.setMaxSpeed(1000);
  pump3.setAcceleration(50);
  pump3.setSpeed(200);
  pump3.moveTo(5000000);
  pump4.setMaxSpeed(1000);
  pump4.setAcceleration(50);
  pump4.setSpeed(200);
  pump4.moveTo(5000000);

  pinMode(reset, INPUT);
  
}

void loop() {
  // Accelerate Motors to 1000 and run for some amount of time.
  if (pump1.distanceToGo() == 0) {
    pump1.moveTo(-pump1.currentPosition());
    Serial.println("Rotating Motor in opposite direction...");
  }
  if (pump2.distanceToGo() == 0) {
    pump2.moveTo(-pump2.currentPosition());
    Serial.println("Rotating Motor in opposite direction...");
  }
  if (pump3.distanceToGo() == 0) {
    pump3.moveTo(-pump3.currentPosition());
    Serial.println("Rotating Motor in opposite direction...");
  }
  if (pump4.distanceToGo() == 0) {
    pump4.moveTo(-pump4.currentPosition());
    Serial.println("Rotating Motor in opposite direction...");
  }
  if(digitalRead(reset) == HIGH){
    while(digitalRead(reset) == HIGH){}
    pump1.setSpeed(0);
    pump2.setSpeed(0);
  }
  pump1.run();
  //pump2.run();
  pump3.run();
  //pump4.run();

}
