#include "Bluetooth.h"

const int TWIST_INP_PIN = 10;
const int STRAIN_INP_PIN = 9;
const int TOUCH_INP_PIN = 8;

const int delayMS = 500;

void setup(){
  Serial.begin(9600);

  pinMode(TWIST_INP_PIN, INPUT_PULLUP);
  pinMode(STRAIN_INP_PIN, INPUT_PULLUP);
  pinMode(TOUCH_INP_PIN, INPUT_PULLUP);

  // bluetooth setup functions
  setupBluetooth();
//  advertiseBluetooth();
}

// test value
int val = 0;
void loop(){
  unsigned long time = millis();
//  verifyConnection();

  int twistVal = digitalRead(TWIST_INP_PIN);
  int strainVal = digitalRead(STRAIN_INP_PIN);
  int touchVal = digitalRead(TOUCH_INP_PIN);

  /* send bluetooth using sendValue(inp, value)
      inp: TWIST, STRAIN, TOUCH
      value: int, int, float 
  */
  
  sendValue(TWIST, twistVal);
  sendValue(STRAIN, strainVal);
  sendValue(TOUCH, touchVal);
  
  Serial.println(twistVal);
  Serial.println(strainVal);
  Serial.println(touchVal);
  Serial.println("\n\n");


  
  // bool sent = sendValue(TWIST, val);
  // Serial.print(sent);
  
  // val = val == 0 ? 3 : 0;
  
  while(millis() - time < delayMS){
    continue;
  }
}
