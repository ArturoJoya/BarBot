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
LiquidCrystal_I2C lcd(0x26,16,3); 

// State LEDs
const int RED = 23;
const int YELLOW = 25;
const int GREEN = 27;
// Pump LEDs - To be replaced by the stepper motor pinouts
const int BLUE = 29;
const int WHITE = 31;
// Pump Motors (6)
const int dirPin = 22;
const int stepPin = 2;
const int dir2Pin = 24;
const int step2Pin = 3;
const int dir3Pin = 26;
const int step3Pin = 4;
const int dir4Pin = 28;
const int step4Pin = 5;
const int dir5Pin = 30;
const int step5Pin = 6;
const int dir6Pin = 32;
const int step6Pin = 7;
// Motor Parameters
const int stepsPerRevolution = 200;
const int mit = 1;
const int decel_time = 18000;
// Buttons
const int SELECT = 33;
const int CONFIRM = 35;
const int RESET = 37;
// Selection Potentiometer
const int POTSELECT = 1;
// Lists of Drink Strings
const char* drink_list[] = {"NULL","Soda Water","Gin","Gin Fizz"};
//const char* dis_drink_list[] = {
//  "NULL","Dispensing Soda Water","Dispensing Gin","Dispensing Gin Fizz"};

// State timings
uint32_t blink_time;
uint32_t dispense_time;
uint16_t BLINK_INT = 500;
// Setup timings
uint32_t clean_time;
uint32_t setup_time;

// Choice instantiation
int choice_raw;
int drink_choice;
int prior_choice; 

//duration of pumps being turned on based on drink choice
long int d1p1dur = 30000;
long int d1p2dur = 0;
long int d2p1dur = 0;
long int d2p2dur = 30000;
long int d3p1dur = 60000;
long int d3p2dur = 45000;

long int cleandur = 90000;
long int setupdur = 45000;

//duration of time used to set liquids in ms
//long int settime = 4206969;

// pump initialization
AccelStepper pump1 = AccelStepper(mit, stepPin, dirPin);
AccelStepper pump2 = AccelStepper(mit, step2Pin, dir2Pin);
AccelStepper pump3 = AccelStepper(mit, step2Pin, dir2Pin);
AccelStepper pump4 = AccelStepper(mit, step2Pin, dir2Pin);
AccelStepper pump5 = AccelStepper(mit, step2Pin, dir2Pin);
AccelStepper pump6 = AccelStepper(mit, step2Pin, dir2Pin);


// user FSM states
enum use_states{
  NONE,
  READY,
  SELECTING,
  DISPENSING,
  DONE
};
use_states prior_state, state;

// mode states
enum modes{
  OFF,
  USER,
  SETUP
};
modes prior_mode, mode;

// setup FSM states
enum set_states{
  NOTSET,
  DISABLED,
  CLEAN,
  SET
};
set_states prev_state, curr_state;


uint32_t sel_time;
uint16_t sel_count;
uint32_t dis_time;
uint16_t dis_count;

// MAINTENANCE  FUNCTIONS 

void disabled(){
  // Menu mode for maintenance 
  uint32_t t;
  // Initialization mode
  if (curr_state != prev_state){
    prev_state = curr_state;
    lcd.clear();
    dis_time = millis();
    dis_count = 0;
    lcd.setCursor(0,0);
    lcd.print("- BARBOT -");
    lcd.setCursor(0,1);
    lcd.print("- MAINTENANCE -");
    lcd.setCursor(0,2);
    lcd.print("- MODE -");
    delay(1000);
    lcd.setCursor(0,0);
    lcd.print("Set-Up    Clean");
    lcd.setCursor(0,1);
    lcd.print("VV           VV");
  }

  // Blink LEDs representing choice 
  t = millis();
  if(t >= dis_time + BLINK_INT){
      digitalWrite(RED, !digitalRead(RED));
      dis_time = t;
      dis_count++;
  }

/*
  // When timeout, display maintenance options
  if (dis_count > 6){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Select -> Clean");
    lcd.setCursor(0,1);
    lcd.print("Confirm -> Set");
  }
  */
  
  // Allow switch to maintenance options
  if (digitalRead(CONFIRM) == HIGH){
    while(digitalRead(CONFIRM) == HIGH) {}
    pump1.moveTo(1000000);
    pump2.moveTo(1000000);
    pump3.moveTo(1000000);
    pump4.moveTo(1000000);
    pump5.moveTo(1000000);
    pump6.moveTo(1000000);
    curr_state = CLEAN;
  } 
  if(digitalRead(SELECT) == HIGH){
    while(digitalRead(SELECT) == HIGH){}
    pump1.moveTo(1000000);
    pump2.moveTo(1000000);
    pump3.moveTo(1000000);
    pump4.moveTo(1000000);
    pump5.moveTo(1000000);
    pump6.moveTo(1000000);
    curr_state = SET;
  }
  if(digitalRead(RESET) == HIGH){
    while(digitalRead(RESET) == HIGH) {}
    mode = USER;
    pump1.setCurrentPosition(0);
    pump2.setCurrentPosition(0);
    pump3.setCurrentPosition(0);
    pump4.setCurrentPosition(0);
    pump5.setCurrentPosition(0);
    pump6.setCurrentPosition(0);
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("Ready...");
  }
}

void clean(){
  // Pumpline Cleaning Mode
  uint32_t t;

  // Initialize 
  if (curr_state != prev_state){
    prev_state = curr_state;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Cleaning...");
    lcd.setCursor(0,1);
    lcd.print("RESET to Stop");
    clean_time = millis();
  }

  t = millis();
  if(t < cleandur + clean_time){
    digitalWrite(BLUE, HIGH);
    digitalWrite(WHITE, HIGH);
  }
  if(t > cleandur + clean_time - decel_time){
    pump1.stop();
    pump2.stop(); 
    pump3.stop();
    pump4.stop(); 
    pump5.stop();
    pump6.stop(); 
  }

  pump1.run();
  pump2.run(); 
  pump3.run();
  pump4.run(); 
  pump5.run();
  pump6.run(); 

  if(digitalRead(RESET) == HIGH){
    while(digitalRead(RESET) == HIGH){}
    pump1.stop();
    pump2.stop(); 
    pump3.stop();
    pump4.stop(); 
    pump5.stop();
    pump6.stop(); 
    pump1.setCurrentPosition(0);
    pump2.setCurrentPosition(0);
    pump3.setCurrentPosition(0);
    pump4.setCurrentPosition(0);
    pump5.setCurrentPosition(0);
    pump6.setCurrentPosition(0);
    digitalWrite(BLUE, LOW);
    digitalWrite(WHITE, LOW);
    curr_state = DISABLED;
  }
}

void set(){
  // Liquid Setting Mode
  uint32_t t;
  // Initialize
  if (curr_state != prev_state){
    prev_state = curr_state;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Setting Liquids");
    lcd.setCursor(0,1);
    lcd.print("Please Wait...");
    setup_time = millis();
  }

  t = millis();
  if(t < setupdur + setup_time){
    digitalWrite(BLUE, HIGH);
    digitalWrite(WHITE, HIGH);
  }
  if(t > setupdur + setup_time - decel_time){
    pump1.stop();
    pump2.stop(); 
    pump3.stop();
    pump4.stop(); 
    pump5.stop();
    pump6.stop(); 

  }

  pump1.run();
  pump2.run(); 
  pump3.run();
  pump4.run(); 
  pump5.run();
  pump6.run(); 

  // Stop motors and switch stage after set amount of time.
  t = millis();
  if (t > setupdur + setup_time){
    curr_state = DISABLED;
    pump1.setCurrentPosition(0);
    pump2.setCurrentPosition(0);
    pump3.setCurrentPosition(0);
    pump4.setCurrentPosition(0);
    pump5.setCurrentPosition(0);
    pump6.setCurrentPosition(0);
  }
  
  // delete later
  if(digitalRead(RESET) == HIGH){
    while(digitalRead(RESET) == HIGH){}
    pump1.stop();
    pump2.stop(); 
    pump3.stop();
    pump4.stop(); 
    pump5.stop();
    pump6.stop(); 
    pump1.setCurrentPosition(0);
    pump2.setCurrentPosition(0);
    pump3.setCurrentPosition(0);
    pump4.setCurrentPosition(0);
    pump5.setCurrentPosition(0);
    pump6.setCurrentPosition(0);
    digitalWrite(BLUE, LOW);
    digitalWrite(WHITE, LOW);
    curr_state = DISABLED;
  }
}


void idle(){
  // Wait for the SELECT button press, in the meantime, hold RED on.
  if (state != prior_state){
    prior_state = state;
    digitalWrite(RED, HIGH);
    lcd.setCursor(4,0);
    lcd.print("Ready...");
  }
  if(digitalRead(SELECT) == HIGH){
    while(digitalRead(SELECT) == HIGH){}
    state = SELECTING;
    lcd.clear();
  }
  if(digitalRead(RESET) == HIGH){
    while(digitalRead(RESET) == HIGH){}
    mode = SETUP;
    prev_state = NOTSET;
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
    lcd.setCursor(0,1);
    lcd.print(drink_list[drink_choice]);
    // lcd.setCursor(0,1);
    // lcd.print("Please Confirm");
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
    pump1.moveTo(1000000);
    pump2.moveTo(1000000);
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
void step_1(int dels){
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(dels);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(dels);
}

void step_2(int dels){
  digitalWrite(step2Pin, HIGH);
  delayMicroseconds(dels);
  digitalWrite(step2Pin, LOW);
  delayMicroseconds(dels);
}
*/

void dispensing(){
  // What to do while dispensing
  uint32_t t;
  
  // initialize dispensing state
  if(state != prior_state){
    prior_state = state;
    digitalWrite(YELLOW, HIGH);
    lcd.setCursor(0,0);
    lcd.print("Dispensing");
    lcd.setCursor(0,1);
    lcd.print(drink_list[drink_choice]);
    dispense_time = millis();
  }

  // Dispense drinks according to drink_choice
  t = millis();
  if(drink_choice == 1){
    //pump 1
    if(t < dispense_time + d1p1dur){
      digitalWrite(BLUE, HIGH);
      } else{
        digitalWrite(BLUE, LOW);
      }
    if(t > (dispense_time + d1p1dur - decel_time)){
      pump1.stop();
    }
    //pump 2
    if(t < dispense_time + d1p2dur){
      digitalWrite(WHITE, HIGH);
    } else {
      digitalWrite(WHITE, LOW);
    }
    if(t > (dispense_time + d1p2dur - decel_time)){
      pump2.stop();
    }
    //both done
    if(t > dispense_time + d1p1dur && t > dispense_time + d1p2dur){
      state = DONE;
    }
  }
  
  else if(drink_choice == 2){
    //pump 1
    if(t < dispense_time + d2p1dur){
      digitalWrite(BLUE, HIGH);
    } else{
      digitalWrite(BLUE, LOW);
    }
    if(t > (dispense_time + d2p1dur - decel_time)){
      pump1.stop();
    }
    //pump2
    if(t < dispense_time + d2p2dur){
      digitalWrite(WHITE, HIGH);
    } else {
      digitalWrite(WHITE, LOW);
    }
    if(t > (dispense_time + d2p2dur - decel_time)){
      pump2.stop();
    }
    //both done
    if(t > dispense_time + d2p1dur && t > dispense_time + d2p2dur){
      state = DONE;
    }
  }
  
  else{
    //pump 1
    if(t < dispense_time + d3p1dur){
      digitalWrite(BLUE, HIGH);
    } else{
      digitalWrite(BLUE, LOW);
    }
    if(t > (dispense_time + d3p1dur - decel_time)){
      pump1.stop();
    }
    //pump 2
    if(t < dispense_time + d3p2dur){
      digitalWrite(WHITE, HIGH);
    } else{
      digitalWrite(WHITE, LOW);
    }
    if(t > (dispense_time + d3p2dur - decel_time)){
      pump2.stop();
    }
    //both done
    if(t > dispense_time + d3p1dur && t > dispense_time + d3p2dur){
      state = DONE;
    }
  }
  pump1.run();
  pump2.run();

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
    pump1.setCurrentPosition(0);
    pump2.setCurrentPosition(0);
    lcd.clear();
  }
}

void done(){
  // What to do when the barbot has finished.
  if(state != prior_state){
    prior_state = state;
    lcd.setCursor(5,0);
    lcd.print("Enjoy (:");
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
  // LCD/LED setup
  lcd.begin();
  lcd.backlight();
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(WHITE, OUTPUT);
  digitalWrite(RED, HIGH);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
  digitalWrite(WHITE, LOW);

  // motor setup
  pump1.setMaxSpeed(850);
  pump1.setAcceleration(50);
  pump1.setSpeed(400);
  pump2.setMaxSpeed(850);
  pump2.setAcceleration(50);
  pump2.setSpeed(400);

  // Button Setup
  pinMode(SELECT, INPUT);
  pinMode(CONFIRM, INPUT);
  pinMode(RESET, INPUT);
  pinMode(POTSELECT, INPUT);

  //FSM setup
  prior_state = NONE;
  state = READY;
  prior_mode = OFF;
  mode = USER;
  prev_state = NOTSET;
  curr_state = DISABLED;
}

void loop() {
  // A state machine that loops through the process of dispensing a drink.
  // nested switch statements designed for user/setup modes
  switch(mode){
    case USER:
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
    break;
    case SETUP:
      switch(curr_state){
        case DISABLED:
        disabled();
        break;
        case CLEAN:
        clean();
        break;
        case SET:
        set();
        break;
      }
    break;
  }
}
