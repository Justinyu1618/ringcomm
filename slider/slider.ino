#include <CapacitiveSensor.h>

uint32_t timer;

int sender = 25;
int touch_pins[6] = {15, 2, 4, 14, 12, 13};
int prev_buttons[6] = {0, 0, 0, 0, 0, 0};
int mins[6] = {30, 30, 30, 30, 30, 30}; // should recalibrate on new surface
int maxes[6] = {60, 58, 72, 73, 73, 76};// should recalibrate on new surface
double touchValues[6]= {0, 0, 0, 0, 0, 0};
int button[6] ={0, 0, 0, 0, 0, 0};

int samples_touch = 10;
int threshold = 50;
int debug = 1;

CapacitiveSensor sensor0 = CapacitiveSensor(sender, touch_pins[0]);
CapacitiveSensor sensor1 = CapacitiveSensor(sender, touch_pins[1]);
CapacitiveSensor sensor2 = CapacitiveSensor(sender, touch_pins[2]);
CapacitiveSensor sensor3 = CapacitiveSensor(sender, touch_pins[3]);
CapacitiveSensor sensor4 = CapacitiveSensor(sender, touch_pins[4]);
CapacitiveSensor sensor5 = CapacitiveSensor(sender, touch_pins[5]);
CapacitiveSensor sensors[6] = {sensor0, sensor1, sensor2, sensor3, sensor4, sensor5};

void setup() {
   Serial.begin(9600);
}

void loop() {
  if (millis() - timer > 200) {
    for (int i = 0; i < 1; i++) {
      touchValues[i] = sensors[i].capacitiveSensor(samples_touch);
      button[i] = int(touchValues[i] > mins[i]);
      if (debug) Serial.printf("%f, ", touchValues[i]);
    }
    if (debug) Serial.println("");

    int touched[2] = {-1, -1};
    int touch = 0;
    for (int i = 0; i < 6; i++) {
      if (button[i] == 1) {
        touched[touch] = i;
        touch++;
      }
    }

    if (touched[0] >= 0) {
        if (touched[1] < 0) {
          // single value pressed
          int index1 = touched[0];
          float frac1 = (touchValues[index1] - mins[index1]) / maxes[index1];
          
          if (!debug) Serial.printf("slider,%.2f;\n", index1 / 5.0);
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

          float formula = ((1-frac1)+frac2) * 0.2 + index1 / 5.0;
          if (formula > 1) formula = 1.00;
          if (!debug) Serial.printf("slider,%.2f;\n", formula);
        }        
      }

    timer = millis();
  }
}
