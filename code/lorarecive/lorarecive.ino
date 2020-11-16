#include "Arduino.h"
#include <HardwareSerial.h>
#include <ThingerESP32.h>
#include "StringSplitter.h"
#include <TridentTD_LineNotify.h>

#define rx 16                                          //LORA TX
#define tx 17                                          //LORA RX 
HardwareSerial mySerial(1);

#define USERNAME "peeranat"
#define DEVICE_ID "esp32"
#define DEVICE_CREDENTIAL "zCslHQ4AM8ZaiG"

#define SSID "Mi Mix 2"
#define SSID_PASSWORD "88888888"
#define LINE_TOKEN  "7CItLr8vbGZYBPur6KrYtCjn58oUaP3254WBlBkxEVi"
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

String latitude, longitude;
String fire;
String mq02;
String gpslat,gpslng;  
String temp, humidity, pressure, al;
String dustDensity;
String incomingString;
 
void setup() {
  Serial.begin(9600);
  delay(500);
  pinMode(5, OUTPUT);
  digitalWrite(5,HIGH);
  fire = "1";
  Serial.println(LINE.getVersion());
  thing.add_wifi(SSID, SSID_PASSWORD);
  LINE.setToken(LINE_TOKEN);
  mySerial.begin(9600,SERIAL_8N1,rx,tx);
}
 
void loop() {
  if (mySerial.available()) {
    incomingString = mySerial.readString();
    Serial.println(incomingString);
    storesensor();
    checkfire();
  }
  delay(500);
}

void storesensor(){
  StringSplitter *splitter = new StringSplitter(incomingString, ',', 9); 
  fire = splitter->getItemAtIndex(0);
  mq02 = splitter->getItemAtIndex(1);    
  dustDensity = splitter->getItemAtIndex(2);
  temp = splitter->getItemAtIndex(3);    
  humidity = splitter->getItemAtIndex(4);    
  pressure = splitter->getItemAtIndex(5);
  al = splitter->getItemAtIndex(6);
  latitude = splitter->getItemAtIndex(7);    
  longitude = splitter->getItemAtIndex(8);                        
}

void checkfire(){
  Serial.println(fire);
  if (fire == "0"){
    LINE.notify("เตือนภัยระดับ 1");
    digitalWrite(5,LOW);
    delay(5000);
  }
  if (fire=="0" && dustDensity > "1"){
    LINE.notify("เตือนภัยระดับ 2");
  }
  if (fire=="0" && dustDensity > "1" && temp > "40"){
    LINE.notify("เตือนภัยระดับ 3");
  }
  if (fire=="0" && dustDensity > "1" && mq02 > "4000" && temp > "40"){
    LINE.notify("เตือนภัยระดับ 4");
  }
  else{
    digitalWrite(5,HIGH);
  }
  
}
