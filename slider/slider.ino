#include <CapacitiveSensor.h>

uint32_t timer;

int sender = 20;
int len_pins = 3;
int touch_pins[6] = {14, 15, 16, 17, 18, 21};
int prev_buttons[6] = {0, 0, 0, 0, 0, 0};
int mins[6] = {35, 35, 35, 30, 30, 30}; // should recalibrate on new surface
int maxes[6] = {90, 60, 80, 73, 73, 76};// should recalibrate on new surface
double touchValues[6]= {0, 0, 0, 0, 0, 0};
int button[6] ={0, 0, 0, 0, 0, 0};

int samples_touch = 30;
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
      if (debug) Serial.print("touched");
    } else if (prev_state == state_TOUCHED && state == state_NOT_TOUCHED) {
      if (debug) Serial.println("released");
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

    prev_state = state;

    timer = millis();
  }
}
