#include "Arduino.h"
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
 
#define SEALEVELPRESSURE_HPA (1013.25)
#define rx 16                                          //LORA TX
#define tx 17                                          //LORA RX
#define RXD2 12                                       //GPS RX
#define TXD2 13                                       //GPD TX
#define mq02pin 34
#define firepin 4

int measurePin = 39;
int ledPower = 23;

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

HardwareSerial mySerial(1);

TinyGPSPlus gps;

Adafruit_BME280 bme;

#define uS_TO_S_FACTOR 1000000  
#define TIME_TO_SLEEP  10
RTC_DATA_ATTR int bootCount = 0;

float latitude, longitude;
String fire;
String mq02;
String gpslat,gpslng;  
String temp, humidity, pressure, al;
String sentString;

void setup() {
  Serial.begin(9600);
  delay(500);
  pinMode(ledPower,OUTPUT);
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  bme.begin(0x76);  
  
  mySerial.begin(9600,SERIAL_8N1,rx,tx);
  Serial2.begin(9600, SERIAL_8N1,RXD2,TXD2);
  delay(200);
  bme280();
  delay(200);
  mq02read();
  delay(200);
  Dust();
  firesensor();
  delay(200);
  while (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))
      GPSlocation();
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
  delay(500);
  sentString = fire + "," + mq02 + "," + dustDensity + "," + temp + "," + humidity + "," + pressure + "," + al + "," + gpslat + "," + gpslng;
  
  Serial.println(sentString);
  mySerial.println(sentString);
  
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}
 
void loop() {
}

void bme280() {
  temp = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  al = bme.readAltitude(SEALEVELPRESSURE_HPA);
  
}
void mq02read(){
  mq02=analogRead(mq02pin);
}

void firesensor(){
  fire=digitalRead(firepin);
}

void GPSlocation()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    latitude = gps.location.lat();
    gpslat = String(latitude , 6);
    longitude = gps.location.lng();
    gpslng = String(longitude , 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }
}

void Dust(){
  digitalWrite(ledPower,LOW); 
  delayMicroseconds(samplingTime);
  voMeasured = analogRead(measurePin); 
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH);
  delayMicroseconds(sleepTime);
  calcVoltage = voMeasured * (3.3 / 4096.0);
  dustDensity = 0.17 * calcVoltage - 0.1;
  Serial.println(dustDensity);
  delay(1000);
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
