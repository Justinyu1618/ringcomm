#include "Bluetooth.h"

#define TWIST_START 0
#define TWIST_UNTWISTED 1
#define TWIST_TWISTED 2

const int STRAIN_INP_PIN = 14; //change this
const int TOUCH_INP_PIN = 8;

const int delayMS = 10;

// twist variables
int TWIST_PWM_PIN = 12;
int TWIST_RECEIVER_PIN = 15;

// PWM
int twist_duty_cycle = int(0.1*255);

double untwisted_threshold = 950;
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial)
    continue;
  pinMode(TWIST_PWM_PIN, OUTPUT);
  pinMode(TWIST_RECEIVER_PIN, INPUT);  
  pinMode(STRAIN_INP_PIN, OUTPUT);

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


if (delay_iter > 50) {
  sendValue(STRAIN, VOut); //send value to Heroku app
  //Serial.print("strain:"); Serial.println(VOut);
  delay_iter = 0;
}

  //int touchVal = digitalRead(TOUCH_INP_PIN);

  /* send bluetooth using sendValue(inp, value)
      inp: TWIST, STRAIN, TOUCH
      value: int, int, float  
  */
  

  //sendValue(TOUCH, touchVal);
  
  while(millis() - time < delayMS){
    continue;
  }
}

void updateTwistFSM() {
 //   Serial.println(twist_state);
   switch(twist_state) {
    case TWIST_START: 
      twist_state = TWIST_UNTWISTED;
      break;
    case TWIST_UNTWISTED:
      if (twist_average > untwisted_threshold) {
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
        if (twist_average > untwisted_threshold) {
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
  digitalWrite(TWIST_PWM_PIN, twist_duty_cycle);//pwm();
    
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
/*
 // keep scale constant
  Serial.print(500);
  Serial.print(",");
  // send it to the computer as ASCII digits
  Serial.println(twist_average);
  */
}
