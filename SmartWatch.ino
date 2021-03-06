// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

//Sorry for the messy code, this is still a work-in-progress :)
//Many of the Serial communications are disabled at this point to save on memory


#include <SPI.h>
#include <BLEPeripheral.h>

#include <SPI.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiSoftSpi.h"

// pin definitions for OLED
#define CS_PIN    12
#define RST_PIN   13
#define DC_PIN    11
#define MOSI_PIN 9
#define CLK_PIN  10

// Define proper RST_PIN if required.
//#define RST_PIN 4

SSD1306AsciiSoftSpi oled;

//#include <BLEUtil.h>

// define pins for BLE (varies per shield/board)
#define BLE_REQ   6
#define BLE_RDY   2
#define BLE_RST   5
//#define OLED_RESET 4
//Adafruit_SSD1306 display(OLED_RESET);

#define FREQ (8000000)
#define INT_FREQ (1)

// create peripheral instance, see pinouts above
BLEPeripheral                    blePeripheral                            = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEBondStore                     bleBondStore;

// remote services
BLERemoteService                 ancsService                              = BLERemoteService("7905f431b5ce4e99a40f4b1e122d00d0");

// remote characteristics
BLERemoteCharacteristic          ancsNotificationSourceCharacteristic     = BLERemoteCharacteristic("9fbf120d630142d98c5825e699a21dbd", BLENotify);
BLERemoteCharacteristic          ancsControlPointCharacteristic           = BLERemoteCharacteristic("69d1d8f345e149a898219bbdfdaad9d9", BLEWrite);
BLERemoteCharacteristic          ancsDataSourceCharacteristic             = BLERemoteCharacteristic("22eac6e924d64bb5be44b36ace7c7bfb", BLENotify);

//byte ANCS_COMMAND_GET_NOTIF_ATTRIBUTES = 0;
#define ANCS_NOTIFICATION_ATTRIBUTE_TITLE 1; //name of sender
#define ANCS_NOTIFICATION_ATTRIBUTE_SUBTITLE 2;
#define ANCS_NOTIFICATION_ATTRIBUTE_MESSAGE 3;
//byte ANCS_NOTIFICATION_ATTRIBUTE_MESSAGE_SIZE = 4;
#define ANCS_NOTIFICATION_ATTRIBUTE_DATE 5;
//byte ANCS_NOTIFICATION_ATTRIBUTE_DATA_SIZE = 16;
//long MESSAGE_SIZE = 100;
//byte ANCS_NOTIFICATION_ATTRIBUTE_APP_IDENTIFIER = 0;
boolean command_send_enable = true;
unsigned long last_command_send = 0;


int newNotif = false;
int newNotifs = 0;

char message[10] = "";
char sender[10] = "";
char title[10] = "";
char date[10] = "";
/*String message = "Hello World", sender = "", title = "", date = "";*/
byte day = 0, hour = 0, minute = 0, second = 0;

char reading = ' ';
byte attrLen = 0;

void setup() {
  Serial.begin(9600);
  
#if defined (__AVR_ATmega32U4__)
  while(!Serial);
#endif

  //init_timer();
  /*
  oled.setFont(System5x7);

  oled.clear();
  oled.println("Hello World");

  delay(10000);*/
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  
  // clears bond data on every boot
  bleBondStore.clearData();

  blePeripheral.setBondStore(bleBondStore);

  blePeripheral.setServiceSolicitationUuid(ancsService.uuid());
  blePeripheral.setLocalName("ANCS");

  // set device name and appearance
  blePeripheral.setDeviceName("sw");
  blePeripheral.setAppearance(0x0080); //192

  blePeripheral.addRemoteAttribute(ancsService);
  blePeripheral.addRemoteAttribute(ancsNotificationSourceCharacteristic);
  blePeripheral.addRemoteAttribute(ancsControlPointCharacteristic);
  blePeripheral.addRemoteAttribute(ancsDataSourceCharacteristic);

  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  blePeripheral.setEventHandler(BLEBonded, blePeripheralBondedHandler);
  blePeripheral.setEventHandler(BLERemoteServicesDiscovered, blePeripheralRemoteServicesDiscoveredHandler);

  // assign event handlers for characteristic
  ancsNotificationSourceCharacteristic.setEventHandler(BLEValueUpdated, ancsNotificationSourceCharacteristicValueUpdated);
  ancsDataSourceCharacteristic.setEventHandler(BLEValueUpdated, ancsDataSourceCharacteristicCharacteristicValueUpdated);

  // begin initialization
  blePeripheral.begin();

  //Uncommenting this line causes the program to stop working
  //oled.begin(&Adafruit128x64, CS_PIN, DC_PIN, CLK_PIN, MOSI_PIN, RST_PIN);
   
  //oled.clear();
  //oled.print("New Message");

  Serial.println(F("BLE Peripheral - ANCS Restart"));

}

void loop() {
  blePeripheral.poll();
  
}

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  //Serial.print(F("Connected, central: "));
  //Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  //Serial.print(F("Disconnected, central: "));
  //Serial.println(central.address());
}

void blePeripheralBondedHandler(BLECentral& central) {
  // central bonded event handler
  //Serial.print(F("Bonded, central: "));
  //Serial.println(central.address());

  if (ancsNotificationSourceCharacteristic.canSubscribe()) {
    ancsNotificationSourceCharacteristic.subscribe();
  }
}

void blePeripheralRemoteServicesDiscoveredHandler(BLECentral& central) {
  // central remote services discovered event handler
  //Serial.print(F("Discovered, central: "));
  //Serial.println(central.address());

  if (ancsNotificationSourceCharacteristic.canSubscribe()) {
    ancsNotificationSourceCharacteristic.subscribe();
  }
}
/*
enum AncsNotificationEventId {
  AncsNotificationEventIdAdded    = 0,
  AncsNotificationEventIdModified = 1,
  AncsNotificationEventIdRemoved  = 2
};

enum AncsNotificationEventFlags {
  AncsNotificationEventFlagsSilent         = 1,
  AncsNotificationEventFlagsImportant      = 2,
  AncsNotificationEventFlagsPositiveAction = 4,
  AncsNotificationEventFlagsNegativeAction = 8
};

enum AncsNotificationCategoryId {
  AncsNotificationCategoryIdOther              = 0,
  AncsNotificationCategoryIdIncomingCall       = 1,
  AncsNotificationCategoryIdMissedCall         = 2,
  AncsNotificationCategoryIdVoicemail          = 3,
  AncsNotificationCategoryIdSocial             = 4,
  AncsNotificationCategoryIdSchedule           = 5,
  AncsNotificationCategoryIdEmail              = 6,
  AncsNotificationCategoryIdNews               = 7,
  AncsNotificationCategoryIdHealthAndFitness   = 8,
  AncsNotificationCategoryIdBusinessAndFinance = 9,
  AncsNotificationCategoryIdLocation           = 10,
  AncsNotificationCategoryIdEntertainment      = 11
};*/

struct AncsNotification {
  unsigned char eventId;
  unsigned char eventFlags;
  unsigned char catergoryId;
  unsigned char catergoryCount;
  unsigned long notificationUid;
};

void ancsNotificationSourceCharacteristicValueUpdated(BLECentral& central, BLERemoteCharacteristic& characteristic) {
  Serial.println(F("ANCS Notification Source Value Updated:"));
  struct AncsNotification notification;
  
  memcpy(&notification, characteristic.value(), sizeof(notification));
  
  //This can all be ignored for the moment
  
  /*
  Serial.print(("\tEvent ID: "));
  Serial.println(notification.eventId);
  Serial.print(("\tEvent Flags: 0x"));
  Serial.println(notification.eventFlags, HEX);
  Serial.print(("\tCategory ID: "));
  Serial.println(notification.catergoryId);
  Serial.print(("\tCategory Count: "));
  Serial.println(notification.catergoryCount);
  Serial.print(("\tNotification UID: "));
  Serial.println(notification.notificationUid);*/
  /*
  Serial.print("\tEvent: ");
  switch(notification.eventId){
    case AncsNotificationEventId.AncsNotificationEventIdAdded:
      Serial.println("Message Added");
    break;
  }*/

  newNotifs = notification.catergoryCount;
  if (ancsDataSourceCharacteristic.canSubscribe()) {
  ancsDataSourceCharacteristic.subscribe();
  //Serial.println(("Subscribed to Data Source"));
  }
  unsigned long uid = notification.notificationUid;
  byte buffer[15];
  buffer[0] = 0;
  buffer[1] = uid & 0xFF;
  buffer[2] = (uid >> 8) & 0xFF;
  buffer[3] = (uid >> 16) & 0xFF;
  buffer[4] = (uid >> 24) & 0xFF;
  buffer[5] = ANCS_NOTIFICATION_ATTRIBUTE_MESSAGE;
  buffer[6] = 23;
  buffer[7] = 0;
  buffer[8] = ANCS_NOTIFICATION_ATTRIBUTE_SUBTITLE; //Message Title
  buffer[9] = 23;
  buffer[10] = 0;
  buffer[11] = ANCS_NOTIFICATION_ATTRIBUTE_TITLE; //Sender
  buffer[12] = 23;
  buffer[13] = 0;
  buffer[14] = ANCS_NOTIFICATION_ATTRIBUTE_DATE; //Date
  //BLEUtil::printBuffer(buffer, 12);
  //Serial.println("Sending notification attribute buffer ");
  //Serial.println();
  ancsControlPointCharacteristic.canWrite();//Serial.println(ancsControlPointCharacteristic.canWrite());
  ancsControlPointCharacteristic.write((const unsigned char*)buffer, 15);//Serial.println(ancsControlPointCharacteristic.write((const unsigned char*)buffer, 12));

  //BLEUtil::printBuffer(characteristic.value(), characteristic.valueLength());
}

void charsToStr(unsigned char buffer[], int len, char *chars){  
  for(int i = 0; i < len; i++){
    int buf = buffer[i];
    char val = char(buf);
    chars[i] = val;
  }
}

void ancsDataSourceCharacteristicCharacteristicValueUpdated(BLECentral& central, BLERemoteCharacteristic& characteristic) {
  //Serial.println(F("ANCS Data Source Value Updated: "));

  int len = characteristic.valueLength();
  //unsigned char val[len] = {};// = characteristic.value();
  unsigned char chars[len] = {};
  for(int i = 0; i < len; i++){
    chars[i] = characteristic.value()[i]; 
    //Serial.print(String(chars[i], HEX));
    //Serial.print(" ");
  }
  //Serial.println();
  //charsToStr(val, len, chars);

  //BLEUtil::printBuffer(val, len);
  
  int idx = 0;
  if(int(chars[0]) == 0 && reading == ' '){
    //Serial.println(F("New Notif"));
    strcpy(message, "");
    strcpy(title, "");
    strcpy(sender, "");
    strcpy(date, "");
    hour = 0;
    minute = 0;
    second = 0;
    reading = 'm';
    attrLen = chars[6];
    //Serial.println(attrLen);
    idx = 8;
  }
  
  for(int i = idx; i < len; i++){    
      if(reading == 'm'){
        //if(attrLen == 0){
        if(chars[i+1] == 0){
          reading = 's';
          attrLen = chars[i+1];
          //Serial.println(F("\nFinished Message"));
          i+=2;
        }  else {
          strcat(message, (char)chars[i]);
          attrLen--;
        }
      //}else if(reading == 's'){
      }else if(chars[i+1] == 0){
          if(attrLen == 0){
            reading = 't';
            attrLen = chars[i+1];
            i+=2;
            //Serial.println(F("\nFinished Subtitle (Message Title)"));
          } else {
            strcat(title, (char)chars[i]);
            attrLen--; 
          }
      //}else if(reading == 't'){
      }else if(chars[i+1] == 0){
          if(attrLen == 0){
            reading = 'd';
            attrLen = chars[i+1];
            i+=2;
            //Serial.println(F("\nFinished Title (Sender)"));
          } else {
            strcat(sender, (char)chars[i]);
            attrLen--;
          }
      }else{
        if(char(chars[i])>30){
          strcat(date, (char)chars[i]);
        }
      }
  }
  /*
  if(reading == 'd'){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10,0);
    display.println(F("Sender: "));
    display.println(sender);
    display.display();
  }*/
  //date.trim();
  //Serial.println(date);
  if(reading == 'd'){// && date.length() == 15){
    //Serial.println(F("Sender: "));
    //Serial.println(sender);
    //Serial.println(F("Title: "));
    //Serial.println(title);
    //Serial.println(F("Message: "));
    //Serial.println(message);
    //Serial.println(F"Date: ");
    char z = '0';
    hour = ((int(date[9]-z)*10+int(date[10]-z)-1)%12)+1;
    minute = int(date[11]-z)*10+int(date[12]-z);
    second = int(date[13]-z)*10+int(date[14]-z);
    //Serial.println(date[10]);
    /*Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(second);
    Serial.println(date);*/
    reading = ' ';
  }
}
/*
void init_timer() {
  cli();//stop interrupts
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = FREQ / 1024 / INT_FREQ; // (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
}

ISR(TIMER1_COMPA_vect) {
  if (second < 59) {
    second++;
    /*Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(second);*
    //oled.clear();
    //oled.print("New Message");
  
  } 
  else if (minute < 59) {
    second = 0;
    minute++;
  } 
  else if (hour < 23) {
    second = 0;
    minute = 0;
    hour++;
  } 
  else {
    second = 0;
    minute = 0;
    hour = 0;
    day++;
  }
}*/


