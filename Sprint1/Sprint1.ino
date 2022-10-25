/* Sprint 1 firmware that carries the 3 main states - idle/ready, selecting, and creating.
 * In case we run out of time, instead of actuating the motors, we will turn on LEDs that will simulate the pump motors running.
 * A red LED will represent the READY state. This will blink for 10 seconds in the sSELECTING state before returning to READY.
 * A yellow LED will turn on when the barbot is mixing the drink.
 * A Green LED wiil turn on symbolizing that the drink has been dispensed. This will remain lit for 5 seconds before returning to READY.
 * 
 * For a future design with 4 pumps, we may consolidate the Yellow LED and the Confirm Button, and instead press the select twice to confirm. 
 * Since the Blue and White LEDs (our drinks for now) won't be needed, these consolidations would open up 2 motors.
 * Since we already have 2 motors hooked up, we would be able to use up to 4 pumps with the two LEDs and two buttons on the UNO R3
 */

// State LEDs
const int RED = 13;
const int YELLOW = 12;
const int GREEN = 11;
// Pump LEDs - To be replaced by the stepper motor pinouts
const int BLUE = 10;
const int WHITE = 9;
// Buttons
const int SELECT = 8;
const int CONFIRM = 7;
const int RESET = 6;
// Selection Potentiometer
const int POTSELECT = 1;

// State timings
uint32_t blink_time;
uint32_t dispense_time;
//uint32_t debounce_time;
uint16_t BLINK_INT = 500;
//uint16_t debounce_int = 100;

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
  }
  if(digitalRead(SELECT) == HIGH){
    while(digitalRead(SELECT) == HIGH){}
    state = SELECTING;
  }
}

//void parse_choice(raw){
  // Interpret analog to fit choices 0 (NONE), 1, 2, or 3
//  drink_choice = raw / 255;
//}

void selecting(){
  // Read the analog once, and blink the LED mapping to the choice.
  // Give user 10 seconds to confirm selection.
  // if confirm -> DISPENSING, else -> READY.
  uint32_t t;
  // initialize selecting state
  if (state != prior_state){
    prior_state = state;
    choice_raw = analogRead(POTSELECT);
    drink_choice = choice_raw/255;
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
  }
}


void dispensing(){
  // What to do while dispensing
  uint32_t t;
  
  // initialize dispensing state
  if(state != prior_state){
    prior_state = state;
    Serial.println(drink_choice);
    digitalWrite(YELLOW, HIGH);
    dispense_time = millis();
  }

  // Dispense drinks according to drink_choice
  t = millis();
  Serial.println(t);
  if(drink_choice == 1){
    if(t < dispense_time + d1p1dur){
      digitalWrite(BLUE, HIGH);
    }
    if(t < dispense_time + d1p2dur){
      digitalWrite(WHITE, HIGH);
    }
    if(t > dispense_time + d1p1dur && t > dispense_time + d1p2dur){
      state = DONE;
    }
  }else if(drink_choice == 2){
    if(t < dispense_time + d2p1dur){
      digitalWrite(BLUE, HIGH);
    }
    if(t < dispense_time + d2p2dur){
      digitalWrite(WHITE, HIGH);
    }
    if(t > dispense_time + d2p1dur && t > dispense_time + d2p2dur){
      state = DONE;
    }
  }else{
    if(t < dispense_time + d3p1dur){
      digitalWrite(BLUE, HIGH);
    }
    if(t < dispense_time + d3p2dur){
      digitalWrite(WHITE, HIGH);
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
    digitalWrite(BLUE, LOW); // The actual implementation will simply turn off the steppers
    digitalWrite(WHITE, LOW);
  }
}

void done(){
  // What to do when the barbot has finished.
  if(state != prior_state){
    prior_state = state;
    digitalWrite(GREEN, HIGH);
  }
  delay(5000);
  digitalWrite(GREEN, LOW);
  //int drink_choice;
  state = READY;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(WHITE, OUTPUT);

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
