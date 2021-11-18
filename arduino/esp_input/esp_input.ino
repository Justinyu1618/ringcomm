
const int TWIST_OUT_PIN = 16;
const int STRAIN_OUT_PIN = 17;
const int TOUCH_OUT_PIN = 5;

int delayMS = 500;

void setup() {
  // put your setup code here, to run once:
  pinMode(TWIST_OUT_PIN, OUTPUT);
  pinMode(STRAIN_OUT_PIN, OUTPUT);
  pinMode(TOUCH_OUT_PIN, OUTPUT);

  Serial.begin(9600);
}

int lastVal = 0;
void loop() {
  unsigned long time = millis();
  // put your main code here, to run repeatedly:

  digitalWrite(TWIST_OUT_PIN, lastVal);
  digitalWrite(STRAIN_OUT_PIN, !lastVal);
  digitalWrite(TOUCH_OUT_PIN, lastVal);
  Serial.printf("%d, %d, %d\n", lastVal, !lastVal, lastVal);

  lastVal = !lastVal;
  
  while(millis() - time < delayMS){
    continue;
  }
}
