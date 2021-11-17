#include <ArduinoBLE.h>

const int MAX_BUFFER_SIZE = 256;
const bool BUFFER_FIXED_LENGTH = false;

const char* serviceUUID = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* input1UUID = "19b10001-e8f2-537e-4f6c-d104768a1214";
const char* input2UUID = "19b10002-e8f2-537e-4f6c-d104768a1214";

BLEService ledService(serviceUUID); 

BLEByteCharacteristic input1Char(input1UUID, BLERead | BLEWrite | BLENotify);
BLEByteCharacteristic input2Char(input2UUID, BLERead | BLEWrite | BLENotify);

const int ledPin = LED_BUILTIN; 
const int input1Pin = D9;
const int input2Pin = D10;

const int delayMS = 200;
const int blinkInterval = 500;

BLEDevice central;

enum CONNECTION_STATE { DISCONNECTED, SEARCHING, CONNECTED };
enum CONNECTION_STATE connectionState = DISCONNECTED;

void setup() {
  Serial.begin(9600);
//  while (!Serial);


  pinMode(ledPin, OUTPUT);
  pinMode(input1Pin, INPUT);
  pinMode(input2Pin, INPUT);

  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }


  BLE.setLocalName("Ring Comm");
  BLE.setAdvertisedService(ledService);


  ledService.addCharacteristic(input1Char);
  ledService.addCharacteristic(input2Char);


  BLE.addService(ledService);

  // set initial values
//  input1Char.writeValue(0);
  input2Char.writeValue(0);

  BLE.advertise();

  connectionState = SEARCHING;
  Serial.println("Peripheral advertising info: ");
  Serial.print("Name: ");
  Serial.println("LED Arduino");
  Serial.print("MAC: ");
  Serial.println(BLE.address());
  Serial.print("Service UUID: ");
  Serial.println(serviceUUID);

  Serial.println("Bluetooth device active, waiting for connections...");
}

String prevData = "";
unsigned long prevBlink = millis();
#define MAX_BUFFER_LEN 100
byte dataBuffer[MAX_BUFFER_LEN];

void loop() {
  unsigned long time = millis();

  if(connectionState == SEARCHING && millis() - prevBlink > blinkInterval){
    digitalWrite(ledPin, !digitalRead(ledPin));
    prevBlink = millis();
  }
  
  if(!central){
    central = BLE.central();
    if(central){
      Serial.print("Connected to central: ");
      Serial.println(central.address());
    }
  }
  
  if (central) {
    if(!central.connected()){
      digitalWrite(ledPin, LOW); 
      Serial.println("Central no longer connected!");
      connectionState = DISCONNECTED;
    } else {
      digitalWrite(ledPin, HIGH);
      connectionState = CONNECTED;
    }
    
    int inp1Val = digitalRead(input1Pin);
    int inp2Val = digitalRead(input2Pin);
//    int data = inp1Val | (inp2Val << 1);
    String data = prevData == "\n" ? "(0,1)\n" : "\n";  
    if(data != prevData){
//      input1Char.writeValue(data);
      Serial.print("Data: ");
      Serial.println(data);
      data.getBytes(dataBuffer, MAX_BUFFER_LEN);
      for(int i = 0; i < data.length(); i++){
        input1Char.writeValue(data[i]);
      }
      
      prevData = data;
    }
   
    
    while(millis() - time < delayMS){
      continue;
    }
  }
}
