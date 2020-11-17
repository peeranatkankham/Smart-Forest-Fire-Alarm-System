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
#define DEVICE_CREDENTIAL "peeranatkankham"

#define SSID "Mi Mix 2"
#define SSID_PASSWORD "88888888"
#define LINE_TOKEN  "7CItLr8vbGZYBPur6KrYtCjn58oUaP3254WBlBkxEVi"
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

TaskHandle_t Task1;
TaskHandle_t Task2;

String latitude, longitude;
String fire;
String mq02;
String gpslat,gpslng;  
String temp, humidity, pressure, al;
String dustDensity;
String incomingString;
String part2;
int buttonstatus;

void setup() {
  Serial.begin(9600);
  delay(500);
  pinMode(5, OUTPUT);
  pinMode(4, INPUT);
  digitalWrite(5,HIGH);
  fire = "1";
  Serial.println(LINE.getVersion());
  thing.add_wifi(SSID, SSID_PASSWORD);
  LINE.setToken(LINE_TOKEN);
  mySerial.begin(9600,SERIAL_8N1,rx,tx);
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500);
    
    thing["fire"] >> [](pson& out){
      out = fire;
    };
    thing["mq02"] >> [](pson& out){
      out = mq02;
    }; 
    thing["dustDensity"] >> [](pson& out){
      out = dustDensity;
    }; 
    thing["temp"] >> [](pson& out){
      out = temp;
    }; 
    thing["humidity"] >> [](pson& out){
      out = humidity;
    };
    thing["pressure"] >> [](pson& out){
      out = pressure;
    };
    thing["altitude"] >> [](pson& out){
      out = al;
    };
    thing["location"] >> [](pson& out){
      out["latitude"] = latitude;
      out["longitude"] = longitude;
    };
    buttonstatus = 0;
    
}

void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    if (mySerial.available()) {
    incomingString = mySerial.readString();
    Serial.println(incomingString);
    storesensor();
    checkfire();
    }
    delay(100);
  } 
}

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    thing.handle();
    
  }
}

void loop() {
  
}

void storesensor(){
  StringSplitter *splitter = new StringSplitter(incomingString, ',', 5); 
  fire = splitter->getItemAtIndex(0);
  mq02 = splitter->getItemAtIndex(1);    
  dustDensity = splitter->getItemAtIndex(2);
  temp = splitter->getItemAtIndex(3);    
  part2 = splitter->getItemAtIndex(4);
  StringSplitter *splitter1 = new StringSplitter(part2, ',', 5);
  humidity = splitter1->getItemAtIndex(0);     
  pressure = splitter1->getItemAtIndex(1);
  al = splitter1->getItemAtIndex(2);
  latitude = splitter1->getItemAtIndex(3);    
  longitude = splitter1->getItemAtIndex(4);                        
}

void checkfire(){
  int dust = dustDensity.toInt();
  int Temp = temp.toInt();
  int mq_02 = mq02.toInt();
  if (fire=="0" && dust > 150 && mq_02 > 3000 && Temp > 40){
    LINE.notify("เตือนภัยระดับ 4");
    a_larm();
  }
  else if (fire=="0" && dust > 150 && Temp > 40){
    LINE.notify("เตือนภัยระดับ 3");
    a_larm();
  }
  else if (fire=="0" && dust > 150){
    LINE.notify("เตือนภัยระดับ 2");
    a_larm();
  }
  else if (fire == "0"){
    LINE.notify("เตือนภัยระดับ 1");
    a_larm();
  }
  
  
  
  if (fire == "1"){
    buttonstatus = 0;
    digitalWrite(5,HIGH);
  }
  else{
    digitalWrite(5,HIGH);
  }
  
}

void a_larm(){
  if (buttonstatus == 0){
    while(digitalRead(4)==LOW){
      digitalWrite(5,LOW);
      delay(10);
    }
    buttonstatus = 1; 
  }
 
}
