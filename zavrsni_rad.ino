#include <Servo.h>
#define OFF 0
#define ON 1
#define SERVO1_MIN 50
#define SERVO1_MAX 130
#define SERVO2_MIN 90
#define SERVO2_MAX 130

Servo servo1, servo2, servo3;  // servo objects (for Servo library/header)

// Constant variables for pin allocation on microcontroller
const int base_x = A0;        // control for servo at the base of the hand
const int joint1_y = A1;      // control for servo at the first joint
const int joint2_y = A2;      // control for servo at the second joint
const int servo1_pin = 9;     // base servo pin
const int servo2_pin = 6;     // first joint servo pin
const int servo3_pin = 5;     // second joint servo pin
const int mosfet_signal = 10; // output to EPM MOSFET
const int sw1 = 8;            // switch that controls EPM ON/OFF
const int sw2 = 7;            // switch that changes between automatic and manual mode
const int servo1_mosfet = 4;  // output to servo 1 MOSFET gate
const int servo2_mosfet = 3;  // output to servo 2 MOSFET gate
const int servo3_mosfet = 2;  // output to servo 3 MOSFET gate

// Changeable variables
int val1, val2, val3;                                       // for reading analog values from potentiometers for servo control
int val1_def, val2_def, val3_def;                           // stationary (default) values for servos
int val1_inc, val2_inc, val3_inc;                           // increases servo speed based on analog input (joystick)
int servo1_mov = OFF, servo2_mov = OFF, servo3_mov = OFF;   // checks for movement on servos
int servo1_pot, servo2_pot, servo3_pot;                     // potentiometer neutral (stationary) values
bool mosfet_power = OFF;                                    // EPM power ON/OFF control
bool manual_mode = ON;                                      // choosing between manual and automatic mode/control
int sw1_current, sw1_prev = OFF;                            // checking for state change on the first switch
int sw2_current, sw2_prev = OFF;                            // checking for state change on the second switch
int SERVO3_MIN, SERVO3_MAX;                                 // min and max angle values for servo 3 that change based on the position of servo 2
int heat_up_magnet = 0;                                     // in automatic mode runs current through EPM for 15 seconds once to heat up magnet for use

// with this function we put arm into default/idle position
void default_position(int pos1, int pos2, int pos3){
  if (servo1.read() != pos1){
    servo1_mov = ON;
    digitalWrite(servo1_mosfet, servo1_mov);
    servo1.write(pos1);
    delay(500);
    servo1_mov = OFF;
    digitalWrite(servo1_mosfet, servo1_mov);
    delay(50);
  }

  if (servo2.read() != pos2){
    servo2_mov = ON;
    digitalWrite(servo2_mosfet, servo2_mov);
    servo2.write(pos2);
    delay(500);
    servo2_mov = OFF;
    digitalWrite(servo2_mosfet, servo2_mov);
    delay(50);
  }

  if (servo3.read() != pos3){
    servo3_mov = ON;
    digitalWrite(servo3_mosfet, servo3_mov);
    servo3.write(pos3);
    delay(500);
    servo3_mov = OFF;
    digitalWrite(servo3_mosfet, servo3_mov);
    delay(50);
  }

  val1_inc = pos1;
  val2_inc = pos2;
  val3_inc = pos3;

  Serial.print(servo1.read());
  Serial.print("\n");
  Serial.print(servo2.read());
  Serial.print("\n");
  Serial.print(servo3.read());
  Serial.print("\n");
}

void setup() {
  Serial.begin(9600);  // serial connection baud rate

  // attaches the servos on defined pins to servo objects
  servo1.attach(servo1_pin);
  servo2.attach(servo2_pin);
  servo3.attach(servo3_pin);

  // defines (output/input) modes for pins
  pinMode(base_x, INPUT);
  pinMode(joint1_y, INPUT);
  pinMode(joint2_y, INPUT);
  pinMode(sw1, INPUT_PULLUP);
  pinMode(sw2, INPUT_PULLUP);
  pinMode(mosfet_signal, OUTPUT);
  pinMode(servo1_mosfet, OUTPUT);
  pinMode(servo2_mosfet, OUTPUT);
  pinMode(servo3_mosfet, OUTPUT);

  servo1_pot = analogRead(base_x);
  servo2_pot = analogRead(joint1_y);
  servo3_pot = analogRead(joint2_y);

  val1_def = 90;
  val2_def = 95;
  val3_def = 45;

  default_position(val1_def, val2_def, val3_def);
}

void loop() {
  val1 = analogRead(base_x); 
  val2 = analogRead(joint1_y);
  val3 = analogRead(joint2_y);
  sw1_current = digitalRead(sw1);
  sw2_current = digitalRead(sw2);

  if (sw1_prev == ON && sw1_current == OFF){
    delay(150);  //debounce
    mosfet_power = !mosfet_power;
    digitalWrite(mosfet_signal, mosfet_power);
  }
  sw1_prev = sw1_current;

  if (sw2_prev == ON && sw2_current == OFF){
    delay(150);  //debounce
    manual_mode = !manual_mode;
  }
  sw2_prev = sw2_current;
  
  // MANUAL MODE - Joystick scales our speed instead of determining position (based on distance from neutral and given direction)
  if (manual_mode == ON){
    // Servo 1
    if ((mosfet_power == OFF) && (servo2_mov == OFF) && (servo3_mov == OFF)) {
      if (servo1_pot+10 < val1) { // move left
        if (val1_inc > SERVO1_MIN){
          servo1_mov = ON;
          digitalWrite(servo1_mosfet, servo1_mov);
          val1_inc = val1_inc + ((servo1_pot-val1)/100);
          servo1.write(val1_inc);
          delay(40);
        }

        else if (val1_inc < SERVO1_MIN) val1_inc = SERVO1_MIN;
      }

      else if (servo1_pot-10 > val1) { // move right
        if (val1_inc < SERVO1_MAX) {
          servo1_mov = ON;
          digitalWrite(servo1_mosfet, servo1_mov);
          val1_inc = val1_inc + ((servo1_pot-val1)/100);
          servo1.write(val1_inc);
          delay(40);
        }

        else if (val1_inc > SERVO1_MAX) val1_inc = SERVO1_MAX;
      }

      else if ((servo1_pot+10 > val1) && (servo1_pot-10 < val1)) servo1_mov = OFF;
    }

    // Servo 2
    if ((mosfet_power == OFF) && (servo1_mov == OFF)) {
      if (servo2_pot+10 < val2) { // move left
        if (val2_inc > SERVO2_MIN) {
          servo2_mov = ON;
          digitalWrite(servo2_mosfet, servo2_mov);
          val2_inc = val2_inc + ((servo2_pot-val2)/150);
          servo2.write(val2_inc);
          delay(40);
        }

        else if (val2_inc < SERVO2_MIN) val2_inc = SERVO2_MIN;
      }

      else if (servo2_pot-10 > val2) { // move right
        if (val2_inc < SERVO2_MAX) {
          servo2_mov = ON;
          digitalWrite(servo2_mosfet, servo2_mov);
          val2_inc = val2_inc + ((servo2_pot-val2)/150);
          servo2.write(val2_inc);
          delay(40);
        }

        else if (val2_inc > SERVO2_MAX) val2_inc = SERVO2_MAX;
      }

      else if ((servo2_pot+10 > val2) && (servo2_pot-10 < val2)) servo2_mov = OFF;
    }

    // Servo 3
    if ((mosfet_power == OFF) && (servo1_mov == OFF)) {
      SERVO3_MIN = 40 - ((servo2.read() - SERVO2_MIN) / 2);
      SERVO3_MAX = 70 + ((servo2.read() - SERVO2_MIN) / 2);

      if (servo3_pot+10 < val3) { // move left
        if (val3_inc > SERVO3_MIN) {
          servo3_mov = ON;
          digitalWrite(servo3_mosfet, servo3_mov);
          val3_inc = val3_inc + ((servo3_pot-val3)/150);
          servo3.write(val3_inc);
          delay(40);
        }

        else if (val3_inc < SERVO3_MIN) val3_inc = SERVO3_MIN;
      }

      else if (servo3_pot-10 > val3) { // move right
        if (val3_inc < SERVO3_MAX) {
          servo3_mov = ON;
          digitalWrite(servo3_mosfet, servo3_mov);
          val3_inc = val3_inc + ((servo3_pot-val3)/150);
          servo3.write(val3_inc);
          delay(40);
        }

        else if (val3_inc > SERVO3_MAX) val3_inc = SERVO3_MAX;
      }

      else if ((servo3_pot+10 > val3) && (servo3_pot-10 < val3)) servo3_mov = OFF;
    }
  }

  /*
  AUTOMATIC MODE
  The robotic arm first positions itself in the default position. If we just started it up for the first time it releases current
  for 15 seconds throught EPM so that it heats up, otherwise it will not release the object immediately. After that it moves left
  to pick up the desired object. It then mirrors and moves to its position on the right side where it stops and releases current
  to drop the object.
  */
  else if (manual_mode == OFF){
    default_position(val1_def, val2_def, val3_def);

    if (!heat_up_magnet){
      mosfet_power = !mosfet_power;
      digitalWrite(mosfet_signal, mosfet_power);
      delay(15000);
      mosfet_power = !mosfet_power;
      digitalWrite(mosfet_signal, mosfet_power);
      heat_up_magnet = !heat_up_magnet;
    }

    servo1_mov = ON;
    digitalWrite(servo1_mosfet, servo1_mov);

    for (val1_inc; val1_inc <= val1_def+30; val1_inc++){
      servo1.write(val1_inc);
      delay(40);
    }

    servo1_mov = OFF;
    digitalWrite(servo1_mosfet, servo1_mov);
    delay(200);

    servo2_mov = ON;
    servo3_mov = ON;
    digitalWrite(servo2_mosfet, servo2_mov);
    digitalWrite(servo3_mosfet, servo3_mov);

    for (val2_inc, val3_inc; (val2_inc <= val2_def+20) && (val3_inc <= val3_def+20); val2_inc++, val3_inc++){
      servo2.write(val2_inc);
      servo3.write(val3_inc);
      delay(40);
    }
    delay(200);

    for (val2_inc, val3_inc; (val2_inc <= val2_def+35) && (val3_inc >= val3_def+5); val2_inc++, val3_inc--){
      servo2.write(val2_inc);
      servo3.write(val3_inc);
      delay(40);
    }
    delay(200);

    for (val2_inc, val3_inc; (val2_inc >= val2_def+20) && (val3_inc <= val3_def+20); val2_inc--, val3_inc++){
      servo2.write(val2_inc);
      servo3.write(val3_inc);
      delay(40);
    }
    delay(200);

    for (val2_inc, val3_inc; (val2_inc >= val2_def) && (val3_inc >= val3_def); val2_inc--, val3_inc--){
      servo2.write(val2_inc);
      servo3.write(val3_inc);
      delay(40);
    }

    servo2_mov = OFF;
    servo3_mov = OFF;
    digitalWrite(servo2_mosfet, servo2_mov);
    digitalWrite(servo3_mosfet, servo3_mov);
    delay(200);

    servo1_mov = ON;
    digitalWrite(servo1_mosfet, servo1_mov);

    for (val1_inc; val1_inc >= val1_def-30; val1_inc--){
      servo1.write(val1_inc);
      delay(40);
    }
    
    servo1_mov = OFF;
    digitalWrite(servo1_mosfet, servo1_mov);
    delay(200);

    servo2_mov = ON;
    servo3_mov = ON;
    digitalWrite(servo2_mosfet, servo2_mov);
    digitalWrite(servo3_mosfet, servo3_mov);

    for (val2_inc, val3_inc; (val2_inc <= val2_def+20) && (val3_inc <= val3_def+20); val2_inc++, val3_inc++){
      servo2.write(val2_inc);
      servo3.write(val3_inc);
      delay(40);
    }
    delay(200);

    servo2_mov = OFF;
    servo3_mov = OFF;
    digitalWrite(servo2_mosfet, servo2_mov);
    digitalWrite(servo3_mosfet, servo3_mov);
    delay(200);

    mosfet_power = !mosfet_power;
    digitalWrite(mosfet_signal, mosfet_power);
    delay(500);
    mosfet_power = !mosfet_power;
    digitalWrite(mosfet_signal, mosfet_power);
    delay(200);

    servo2_mov = ON;
    servo3_mov = ON;
    digitalWrite(servo2_mosfet, servo2_mov);
    digitalWrite(servo3_mosfet, servo3_mov);

    for (val2_inc, val3_inc; (val2_inc >= val2_def) && (val3_inc >= val3_def); val2_inc--, val3_inc--){
      servo2.write(val2_inc);
      servo3.write(val3_inc);
      delay(40);
    }

    servo2_mov = OFF;
    servo3_mov = OFF;
    digitalWrite(servo2_mosfet, servo2_mov);
    digitalWrite(servo3_mosfet, servo3_mov);
    delay(200);

    servo1_mov = ON;
    digitalWrite(servo1_mosfet, servo1_mov);

    for (val1_inc; val1_inc <= val1_def; val1_inc++){
      servo1.write(val1_inc);
      delay(40);
    }

    servo1_mov = OFF;
    digitalWrite(servo1_mosfet, servo1_mov);
    delay(200);
  }
}
