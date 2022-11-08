/*
 * Final Sprint 2 Firmware Implementation
 * State Machine has 4 States: Ready/Idle; Selecting; Dispensing; Done.
 * These states are indicated through LEDs and an I2C display
 * 
 * Current Implementation uses 2 motors to control 2 pumps.
 * For future implementation with more pumps, we would consolidate some of the LEDs and consolidate
 * the confirm button to be the same as the current select button
 */
 
// Stepper Motor Library
#include <AccelStepper.h>
// Libraries for User Interface (i2c LCD)
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// User interface setup
LiquidCrystal_I2C lcd(0x26,16,1); 

// State LEDs
const int RED = 13;
const int YELLOW = 12;
const int GREEN = 11;
// Pump LEDs - To be replaced by the stepper motor pinouts
const int BLUE = 10;
const int WHITE = 9;
// Pump Motors (2)
const int dirPin = 2;
const int stepPin = 3;
const int dir2Pin = 4;
const int step2Pin = 5;
// Motor Parameters
const int stepsPerRevolution = 200;
const int mit = 1;
// Buttons
const int SELECT = 8;
const int CONFIRM = 7;
const int RESET = 6;
// Selection Potentiometer
const int POTSELECT = 1;

// State timings
uint32_t blink_time;
uint32_t dispense_time;
uint16_t BLINK_INT = 500;

// Choice instantiation
int choice_raw;
int drink_choice;

//duration of pumps being turned on based on drink choice
int d1p1dur = 10000;
int d1p2dur = 0;
int d2p1dur = 0;
int d2p2dur = 10000;
int d3p1dur = 10000;
int d3p2dur = 10000;

AccelStepper pump1 = AccelStepper(mit, stepPin, dirPin);
AccelStepper pump2 = AccelStepper(mit, step2Pin, dir2Pin);

enum states{
  NONE,
  READY,
  SELECTING,
  DISPENSING,
  DONE
};

states prior_state, state;
uint32_t sel_time;
uint16_t sel_count;

void idle(){
  // Wait for the SELECT button press, in the meantime, hold RED on.
  if (state != prior_state){
    prior_state = state;
    digitalWrite(RED, HIGH);
    lcd.setCursor(4,0);
    lcd.print("READY...");
  }
  if(digitalRead(SELECT) == HIGH){
    while(digitalRead(SELECT) == HIGH){}
    state = SELECTING;
    lcd.clear();
  }
}

void selecting(){
  // Read the analog once, and blink the LED mapping to the choice.
  // Give user 10 seconds to confirm selection.
  // if confirm -> DISPENSING, else -> READY.
  uint32_t t;
  // initialize selecting state
  if (state != prior_state){
    prior_state = state;
    choice_raw = analogRead(POTSELECT);
    drink_choice = choice_raw/257;
    lcd.setCursor(0,0);
    lcd.print("Current Drink:");
    lcd.print(drink_choice);
    lcd.setCursor(0,1);
    lcd.print("Please Confirm");
    sel_time = millis();
    sel_count = 0;
  }

  // Blink LEDs representing choice
  t = millis();
  if(t >= sel_time + BLINK_INT){
    if(drink_choice == 1){
      digitalWrite(RED, !digitalRead(RED));
    } else if(drink_choice == 2){
      digitalWrite(RED, LOW);
      digitalWrite(YELLOW, !digitalRead(YELLOW));
    } else{
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, !digitalRead(GREEN));
    }
    sel_time = t;
    sel_count++;
  }

  // Check for state transition
  if (digitalRead(CONFIRM) == HIGH){
    while(digitalRead(CONFIRM) == HIGH) {}
    state = DISPENSING;
  } else if(sel_count == 20 || drink_choice == 0){
    state = READY;
  } else if(digitalRead(RESET) == HIGH){
    while(digitalRead(RESET) == HIGH){}
    state = READY;
  }
  
  //reset LEDs to switch out of selecting state
  if(state != prior_state){
    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, LOW);
    digitalWrite(GREEN, LOW);
    lcd.clear();
  }
}
/*
void step_1(){
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(2000);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(2000);
}

void step_2(){
  digitalWrite(step2Pin, HIGH);
  delayMicroseconds(2000);
  digitalWrite(step2Pin, LOW);
  delayMicroseconds(2000);
}
*/

void dispensing(){
  // What to do while dispensing
  uint32_t t;
  
  // initialize dispensing state
  if(state != prior_state){
    prior_state = state;
    Serial.println(drink_choice);
    digitalWrite(YELLOW, HIGH);
    digitalWrite(dirPin, HIGH);
    digitalWrite(dir2Pin, HIGH);
    lcd.setCursor(2,0);
    lcd.print("DISPENSING..");
    dispense_time = millis();
  }

  // Dispense drinks according to drink_choice
  t = millis();
  Serial.println(t);
  if(drink_choice == 1){
    if(t < dispense_time + d1p1dur){
      digitalWrite(BLUE, HIGH);
      pump1.run();
      //step_1();
      }
    if(t < dispense_time + d1p2dur){
      digitalWrite(WHITE, HIGH);
      pump2.run();
      //step_2();
    }
    if(t > dispense_time + d1p1dur && t > dispense_time + d1p2dur){
      state = DONE;
    }
  }else if(drink_choice == 2){
    if(t < dispense_time + d2p1dur){
      digitalWrite(BLUE, HIGH);
      pump1.run();
      //step_1();
    }
    if(t < dispense_time + d2p2dur){
      digitalWrite(WHITE, HIGH);
      pump2.run();
      //step_2();
    }
    if(t > dispense_time + d2p1dur && t > dispense_time + d2p2dur){
      state = DONE;
    }
  }else{
    if(t < dispense_time + d3p1dur){
      digitalWrite(BLUE, HIGH);
      pump1.run();
      //step_1();
    }
    if(t < dispense_time + d3p2dur){
      digitalWrite(WHITE, HIGH);
      pump2.run();
      //step_2();
    }
    if(t > dispense_time + d3p1dur && t > d3p2dur){
      state = DONE;
    }
  }

  //if RESET button is hit, will abort action and return to READY
  if(digitalRead(RESET) == HIGH){
    while(digitalRead(RESET) == HIGH){}
    state = READY;
  }
  
  // Turns off motors to move on to the next state
  if(state != prior_state){
    digitalWrite(YELLOW, LOW);
    digitalWrite(BLUE, LOW);
    digitalWrite(stepPin, LOW);
    digitalWrite(WHITE, LOW);
    //digitalWrite(step2Pin, LOW);
    lcd.clear();
  }
}

void done(){
  // What to do when the barbot has finished.
  if(state != prior_state){
    prior_state = state;
    lcd.setCursor(5,0);
    lcd.print("DONE!!!");
    digitalWrite(GREEN, HIGH);
  }
  delay(5000);
  digitalWrite(GREEN, LOW);
  lcd.clear();
  state = READY;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(WHITE, OUTPUT);

  pump1.setMaxSpeed(800);
  pump1.setAcceleration(50);
  pump1.setSpeed(200);
  pump2.setMaxSpeed(800);
  pump2.setAcceleration(50);
  pump2.setSpeed(200);
  //pinMode(stepPin, OUTPUT);
  //pinMode(dirPin, OUTPUT);
  //pinMode(step2Pin, OUTPUT);
  //pinMode(dir2Pin, OUTPUT);

  pinMode(SELECT, INPUT);
  pinMode(CONFIRM, INPUT);
  pinMode(RESET, INPUT);
  pinMode(POTSELECT, INPUT);

  digitalWrite(RED, HIGH);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
  digitalWrite(WHITE, LOW);

  prior_state = NONE;
  state = READY;

}

void loop() {
  // A state machine that loops through the process of dispensing a drink.
  switch(state){
    case READY:
    idle();
    break;
    case SELECTING:
    selecting();
    break;
    case DISPENSING:
    dispensing();
    break;
    case DONE:
    done();
    break;
  }
}
