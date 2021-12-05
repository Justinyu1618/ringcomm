#define TWIST_START 0
#define TWIST_UNTWISTED 1
#define TWIST_TWISTED 2

const int STRAIN_INP_PIN = 9;
const int TOUCH_INP_PIN = 8;

const int delayMS = 10;

// twist variables
int TWIST_PWM_PIN = 12;
int TWIST_RECEIVER_PIN = 17;

// PWM
int twist_duty_cycle = int(0.1*255);

double untwisted_threshold = 700;
int twist_state = TWIST_START;

const int twist_numReadings = 10;     // number of twist readings to average
int twist_readings[twist_numReadings];      // the readings from the analog input
int twist_readIndex = 0;              // the index of the current reading
int twist_total = 0;                  // the running total
int twist_average = 0;                // the average

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial)
    continue;
  pinMode(TWIST_PWM_PIN, OUTPUT);
  pinMode(TWIST_RECEIVER_PIN, INPUT);  

  // bluetooth setup functions
  //setupBluetooth();
  //advertiseBluetooth();

  // initialize all the twist readings to 0:
  for (int thisReading = 0; thisReading < twist_numReadings; thisReading++) {
    twist_readings[thisReading] = 0;
  }
}

void loop() {
  unsigned long time = millis();
  //verifyConnection();

  //updateTwistAverage();
  //updateTwistFSM();

 
  //int strainVal = digitalRead(STRAIN_INP_PIN);
  //int touchVal = digitalRead(TOUCH_INP_PIN);

  /* send bluetooth using sendValue(inp, value)
      inp: TWIST, STRAIN, TOUCH
      value: int, int, float  
  */
  
  //sendValue(STRAIN, strainVal);
  //sendValue(TOUCH, touchVal);
  
  while(millis() - time < delayMS){
    continue;
  }
}

void updateTwistFSM() {
   switch(twist_state) {
    case TWIST_START: 
      twist_state = TWIST_UNTWISTED;
      break;
    case TWIST_UNTWISTED:
      if (twist_average < untwisted_threshold) {
        //  sendValue(TWIST, 0);
        twist_state = TWIST_TWISTED;
      }
      break;
    case TWIST_TWISTED:
      if (twist_average > untwisted_threshold) {
        //  sendValue(TWIST, 1);
        twist_state = TWIST_UNTWISTED;
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

   // keep scale constant
    Serial.print(0);
    Serial.print(",");
  // send it to the computer as ASCII digits
  Serial.println(twist_average);
}
