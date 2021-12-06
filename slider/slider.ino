#include <CapacitiveSensor.h>

uint32_t timer;

int sender = 20;
int len_pins = 3;
int touch_pins[6] = {14, 15, 17, 17, 18, 21};
int prev_buttons[6] = {0, 0, 0, 0, 0, 0};
int mins[6] = {33, 30, 20, 30, 30, 30}; // should recalibrate on new surface
int maxes[6] = {90, 60, 30, 73, 73, 76};// should recalibrate on new surface
double touchValues[6]= {0, 0, 0, 0, 0, 0};
int button[6] ={0, 0, 0, 0, 0, 0};

int samples_touch = 30;
int threshold = 50;
int debug = 1;

CapacitiveSensor sensor0 = CapacitiveSensor(sender, touch_pins[0]);
CapacitiveSensor sensor1 = CapacitiveSensor(sender, touch_pins[1]);
CapacitiveSensor sensor2 = CapacitiveSensor(sender, touch_pins[2]);
CapacitiveSensor sensor3 = CapacitiveSensor(sender, touch_pins[3]);
CapacitiveSensor sensor4 = CapacitiveSensor(sender, touch_pins[4]);
CapacitiveSensor sensor5 = CapacitiveSensor(sender, touch_pins[5]);
CapacitiveSensor sensors[6] = {sensor0, sensor1, sensor2, sensor3, sensor4, sensor5};

int state_NOT_TOUCHED = 0;
int state_TOUCHED = 1;
int state = 0;
int prev_state = 0;
int start_index = 0;
int end_index = 0;

void setup() {
   Serial.begin(9600);
   pinMode(sender, OUTPUT);
   for (int i = 0; i < len_pins; i++) {
     pinMode(touch_pins[i], INPUT); 
   }
}

void loop() {
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
      Serial.print("touched");
      Serial.println(start_index);
    } else if (prev_state == state_TOUCHED && state == state_NOT_TOUCHED) {
      Serial.println("released");
      if (start_index == end_index) {
        Serial.print("tab,");
        Serial.print(start_index);
        Serial.println(";");
      } else if (start_index < end_index) {
        Serial.print("slide,1;");
      } else {
        Serial.print("slide,-1;");
      }
    }

    /*
    int touched[2] = {-1, -1};
    int touch = 0;
    for (int i = 0; i < 2; i++) {
      if (debug) Serial.print(button[i]);
      if (button[i] == 1) {
        touched[touch] = i;
        touch++;
      }
    }
    if (debug) Serial.println(button[2]);
    // hack for button #3
    if (touched[0] < 0 && touched[1] < 0 && button[2] == 1) touched[0] = 2;

    if (touched[0] >= 0) {
        if (touched[1] < 0) {
          // single value pressed
          int index1 = touched[0];
          
          float frac1 = index1 / (len_pins - 1.0);

          if (!debug) {
             Serial.print("tab,");
             Serial.print(index1 + 1);
             Serial.println(";");
          }
        } else {
          int index1 = touched[0];
          int index2 = touched[1];
          float frac1 = (touchValues[index1] - mins[index1]) / maxes[index1];
          float frac2 = (touchValues[index2] - mins[index2]) / maxes[index2];
          if (frac1 < 0) frac1 = 0.0;
          if (frac2 < 0) frac2 = 0.0;
          if (frac1 > 1) frac1 = 1.0;
          if (frac2 > 1) frac2 = 1.0;
          // Serial.printf("\n%d: %f, %d: %f\n", touched[0], frac1, touched[1], frac2);

          float formula = (((1-frac1)+frac2) + index1) / (len_pins - 1);
          if (formula > 1) formula = 1.00;
          if (!debug) {
             Serial.print("slider,");
             Serial.print(formula, 2);
             Serial.println(";");
          }
        }        
      }
    */
    for (int i = 0; i < len_pins; i++) {
      prev_buttons[i] = button[i];
    }
    prev_state = state;
    
    timer = millis();
  }
}
