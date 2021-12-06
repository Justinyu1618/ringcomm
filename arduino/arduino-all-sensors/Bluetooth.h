#ifndef Bluetooth_h
#define Bluetooth_h

#include <ArduinoBLE.h>

const int MAX_BUFFER_SIZE = 256;
const bool BUFFER_FIXED_LENGTH = false;

const char* serviceUUID = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* twistInputUUID = "19b10001-e8f2-537e-4f6c-d104768a1214";
const char* strainInputUUID = "19b10002-e8f2-537e-4f6c-d104768a1214";
const char* touchInputUUID = "19b10003-e8f2-537e-4f6c-d104768a1214";

BLEService ledService(serviceUUID); 

BLEByteCharacteristic twistInputCharacteristic(twistInputUUID, BLERead | BLEWrite | BLENotify);
BLEFloatCharacteristic strainInputCharacteristic(strainInputUUID, BLERead | BLEWrite | BLENotify);
BLEFloatCharacteristic touchInputCharacteristic(touchInputUUID, BLERead | BLEWrite | BLENotify);

int lastTwistInputValue = 0;
float lastStrainInputValue = 0;
float lastTouchInputValue = 0;

enum SENSOR_INPUTS { TWIST, STRAIN, TOUCH };


enum CONNECTION_STATE { DISCONNECTED, SEARCHING, CONNECTED };
enum CONNECTION_STATE connectionState = DISCONNECTED;

BLEDevice central;

const int ledPin = LED_BUILTIN; 
const int blinkInterval = 500;


void setupBluetooth() { 
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("Ring Comm");
  BLE.setAdvertisedService(ledService);

  ledService.addCharacteristic(twistInputCharacteristic);
  ledService.addCharacteristic(strainInputCharacteristic);
  ledService.addCharacteristic(touchInputCharacteristic);

  BLE.addService(ledService);

  twistInputCharacteristic.writeValue(0);
  strainInputCharacteristic.writeValue(0);
  touchInputCharacteristic.writeValue(0);
}


// hangs until bluetooth is connected
void advertiseBluetooth() {
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

  pinMode(ledPin, OUTPUT);
  unsigned long prevBlink = millis();
  while(connectionState == SEARCHING){
    if(millis() - prevBlink > blinkInterval){
      digitalWrite(ledPin, !digitalRead(ledPin));
      prevBlink = millis();
    }

    if(!central){
      central = BLE.central();
      if(central && central.connected()){
        Serial.print("Connected to central: ");
        Serial.println(central.address());
        digitalWrite(ledPin, HIGH);
        connectionState = CONNECTED;
        
      }
    }
  }
}

void verifyConnection() {
  if(!central.connected()){
    digitalWrite(ledPin, LOW); 
    Serial.println("Central no longer connected!");
    connectionState = DISCONNECTED;
  }
}



template <class T>
bool sendValue(SENSOR_INPUTS inp, T val ) {
  switch(inp){
    case TWIST:
      if(val != lastTwistInputValue){
        Serial.print("Twist sensor sending: ");
        Serial.println(val);
        twistInputCharacteristic.writeValue(val);
        lastTwistInputValue = val;
        return true;
      }
      
    case STRAIN:
      if(val != lastStrainInputValue){
        strainInputCharacteristic.writeValue(val);
        lastStrainInputValue = val;
        return true;
      }
    case TOUCH:
      if(val != lastTouchInputValue){
        Serial.print("Touch sensor sending: ");
        Serial.println(val);
        touchInputCharacteristic.writeValue(val);
        lastTouchInputValue = val;
        return true;
      }
  }
  return false;
}

#endif
