#include "Bluetooth.h"

const int delayMS = 200;

void setup(){
  Serial.begin(9600);

  // bluetooth setup functions
  setupBluetooth();
  advertiseBluetooth();
}

// test value
int val = 0;
void loop(){
  unsigned long time = millis();
  verifyConnection();

  /* send bluetooth using sendValue(inp, value)
      inp: TWIST, STRAIN, TOUCH
      value: int, int, float 
  */
  bool sent = sendValue(TWIST, val);
  Serial.print(sent);
  
  val = val == 0 ? 3 : 0;
  
  while(millis() - time < delayMS){
    continue;
  }
}
