#include "Bluetooth.h"
#include <CapacitiveSensor.h>

#define TWIST_START 0
#define TWIST_UNTWISTED 1
#define TWIST_TWISTED 2
#define TWIST_BASELINE_UNTWISTED 3
#define TWIST_BASELINE_TWISTED 4
#define TWIST_BASELINE_SETTLE 5

const int STRAIN_INP_PIN = 21; //change this
const int TOUCH_INP_PIN = 8;

const int delayMS = 10;

int strain_start = 0;
int strain_calibration_duration = 5*1000;

// twist variables
int TWIST_PWM_PIN = 12;
int TWIST_RECEIVER_PIN = 17;

// PWM
int twist_duty_cycle = int(0.1*255);

int twist_baseline_untwisted_duration = 10*1000;
int twist_baseline_twisted_duration = 10*1000;
int twist_baseline_untwisted_start;
int twist_baseline_twisted_start;
int twist_baseline_readings = 0;
double twist_baseline = 0.0;
double untwisted_threshold;
int twist_state = TWIST_START;

const int twist_numReadings = 75;     // number of twist readings to average
int twist_readings[twist_numReadings];      // the readings from the analog input
int twist_readIndex = 0;              // the index of the current reading
int twist_total = 0;                  // the running total
int twist_average = 0;                // the average


int v_out; float VOut; float oldVOut; //analog read and calibration floats
int size_arr = 40; //size of running average array
float raw_volts [40]; //raw readings array
int iterator; //pointer to store in array
int delay_iter;
float sum_volts;


int twist_window_start;
int twist_window_duration = 200;
int twist_count_above_threshold = 0;

int sender = 20;
int len_pins = 3;
int touch_pins[3] = {14, 15, 16};
int mins[3] = {35, 35, 35}; // should recalibrate on new surface
double touchValues[3]= {0, 0, 0};
int button[3] ={0, 0, 0};

CapacitiveSensor sensor0 = CapacitiveSensor(sender, touch_pins[0]);
CapacitiveSensor sensor1 = CapacitiveSensor(sender, touch_pins[1]);
CapacitiveSensor sensor2 = CapacitiveSensor(sender, touch_pins[2]);
CapacitiveSensor sensors[3] = {sensor0, sensor1, sensor2};

int samples_touch = 30;
int debug = 1;

int state_NOT_TOUCHED = 0;
int state_TOUCHED = 1;
int state = 0;
int prev_state = 0;
int start_index = 0;
int end_index = 0;
uint32_t timer;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
//  while(!Serial)
//    continue;
  pinMode(TWIST_PWM_PIN, OUTPUT);
  pinMode(TWIST_RECEIVER_PIN, INPUT);
  pinMode(STRAIN_INP_PIN, OUTPUT);
  pinMode(sender, OUTPUT);
  for (int i = 0; i < len_pins; i++) {
    pinMode(touch_pins[i], INPUT);
  }

  // bluetooth setup functions
  setupBluetooth();
  advertiseBluetooth();

  // initialize all the twist readings to 0:
  for (int thisReading = 0; thisReading < twist_numReadings; thisReading++) {
    twist_readings[thisReading] = 0;
  }

  //initialize all the force variables to zero
  iterator = 0;
  sum_volts = 0;
  delay_iter = 0;
  oldVOut = 0;

  strain_start = millis();
}

void loop() {
  unsigned long time = millis();
  verifyConnection();

  updateTwistAverage();
  updateTwistFSM();


  //code for force sensor
  v_out = analogRead(STRAIN_INP_PIN); //v_out from wheatstone_bridge circuit
  raw_volts[iterator] = v_out;
  for (int i = 0; i < size_arr; i++) {
    sum_volts += raw_volts[i];
  }

   VOut = sum_volts / size_arr -  oldVOut;
   oldVOut = VOut;
   sum_volts = 0;
  iterator = iterator + 1;
  delay_iter = delay_iter + 1;
  if (iterator == size_arr) {
    iterator = 0;
  }


if (delay_iter > 100) {
  sendValue(STRAIN, VOut); //send value to Heroku app
  //Serial.print("strain:"); Serial.println(VOut);
  delay_iter = 0;
}

  //int touchVal = digitalRead(TOUCH_INP_PIN);

  /* send bluetooth using sendValue(inp, value)
      inp: TWIST, STRAIN, TOUCH
      value: int, int, float
  */
  if (millis() - timer > 200) {
    for (int i = 0; i < len_pins; i++) {
      touchValues[i] = sensors[i].capacitiveSensor(samples_touch);
      button[i] = int(touchValues[i] > mins[i]);
      if (debug) {
         Serial.print(touchValues[i]);
         Serial.print(", ");
      }
    }
    if (debug) Serial.println("");

    if (!button[0] && !button[1] && !button[2]) {
      state = state_NOT_TOUCHED;
    } else {
      state = state_TOUCHED;
      end_index = button[0] ? 0 : (button[1] ? 1 : 2);
    }

    if (prev_state == state_NOT_TOUCHED && state == state_TOUCHED) {
      start_index = button[0] ? 0 : (button[1] ? 1 : 2);
      if (debug) Serial.print("touched");
    } else if (prev_state == state_TOUCHED && state == state_NOT_TOUCHED) {
      if (debug) Serial.println("released");
      if (start_index == end_index) {
        Serial.print("tab,");
        Serial.print(start_index);
        Serial.println(";");
        sendValue(TOUCH, start_index);
      } else if (start_index < end_index) {
        Serial.print("slide,1;");
        sendValue(TOUCH, 3);
      } else {
        Serial.print("slide,-1;");
        sendValue(TOUCH, 4);
      }
    }

    prev_state = state;
    timer = millis();
    }
  //sendValue(TOUCH, touchVal);

  while(millis() - time < delayMS){
    continue;
  }
}

void updateTwistFSM() {
   switch(twist_state) {
    case TWIST_START: 
      if (millis() > strain_start + strain_calibration_duration) {
        Serial.println("beginning untwisted baseline");
        twist_state = TWIST_BASELINE_UNTWISTED;
        twist_baseline_untwisted_start = millis();
        twist_baseline_readings = 0;
      }
      break;
    case TWIST_BASELINE_UNTWISTED:
      if (millis() < twist_baseline_untwisted_start + twist_baseline_untwisted_duration) {
        twist_baseline += twist_average;
        twist_baseline_readings ++;
      } else {
        twist_baseline /= twist_baseline_readings;
        Serial.print("untwisted baseline:"); Serial.println(twist_baseline);
        Serial.println("beginning twisted baseline");
        twist_state = TWIST_BASELINE_TWISTED;
        twist_baseline_twisted_start = millis();
        untwisted_threshold = twist_baseline;
      }
      break;
    case TWIST_BASELINE_TWISTED:
      if (millis() < twist_baseline_twisted_start + twist_baseline_twisted_duration) {
        untwisted_threshold = min(twist_average, untwisted_threshold);
      } else {
        double twist_delta = twist_baseline - untwisted_threshold;
        untwisted_threshold = twist_baseline - twist_delta * 0.3;
        Serial.print("untwisted threshold:"); Serial.println(untwisted_threshold);
        twist_state = TWIST_BASELINE_SETTLE;
      }
      break;
    case TWIST_BASELINE_SETTLE: // let average adjust after baselining
      if (millis() > twist_baseline_twisted_start + twist_baseline_twisted_duration + 5000) {
        twist_state = TWIST_UNTWISTED;
      }
      break;
    case TWIST_UNTWISTED:
      if (twist_average < untwisted_threshold) {
        sendValue(TWIST, 0);
        twist_state = TWIST_TWISTED;
        //Serial.println("twisted");
        twist_window_start = millis();
        twist_count_above_threshold = 0;
      }
      break;
    case TWIST_TWISTED:
      if (millis() > twist_window_start + twist_window_duration) {
        if (twist_count_above_threshold == 0) {
           sendValue(TWIST, 1);
        //Serial.println("untwisted");
        twist_state = TWIST_UNTWISTED;
        } else { // new window
           twist_window_start = millis();
            twist_count_above_threshold = 0;
        }
      } else {
        if (twist_average < untwisted_threshold) {
          twist_count_above_threshold ++;
        }
      }
      break;
    default:
      break;
  }
}

void updateTwistAverage() {
  // write pwm
  digitalWrite(TWIST_PWM_PIN, twist_duty_cycle);

  // subtract the last reading:
  twist_total -= twist_readings[twist_readIndex];

  // read from the sensor:
  twist_readings[twist_readIndex] = analogRead(TWIST_RECEIVER_PIN);

  // add the reading to the total:
  twist_total += twist_readings[twist_readIndex];

  // advance to the next position in the array:
  twist_readIndex ++;

  // if we're at the end of the array...
  if (twist_readIndex >= twist_numReadings) {
    // ...wrap around to the beginning:
    twist_readIndex = 0;
  }

  // calculate the average:
  twist_average = twist_total / twist_numReadings;


 // keep scale constant
 /*
  Serial.print(twist_baseline);
  Serial.print(",");
  Serial.print(untwisted_threshold);
  Serial.print(",");
  Serial.print(500);
  Serial.print(",");
  // send it to the computer as ASCII digits
  Serial.println(twist_average);
  */
 
 
}
