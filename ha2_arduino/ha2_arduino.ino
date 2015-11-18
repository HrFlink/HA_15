
/*
 * Licence : CC -by -sa
 * Auteur : Matthieu Cargnelli
 * Author : Henrik Vestergaard
 */

/* Protocol elements:
 * I Send IR
 *    AC_Mode
 *    Temp (HEX)
 *    Fan Mode
 *    Vanne Mode
 *    Profile Mode //Panasonic only - otherwise not used
 *    ON/OFF (1/0)
 *    Model (MITSUBISHI = 0, PANASONIC = 1)
 * To test: I00160500000101 (HOT, 22, auto, anto, normal, on, Panasonic)
 *
 * i send ir code 8 bytes+19 bytes
 * 
 * T Set LCD-Text
 *   Row (byte 0-1)
 *   Col (byte 0-15)
 *   Text payload (always 16 bytes)

 * C Set LCD color
 *   R value (byte)
 *   G value (byte)
 *   B value (byte)

 * h Read and return humidity

 * t Read and return temperature

*/
 
 

#include "DHT.h"
#include <Wire.h>
#include "rgb_lcd.h"
#include "IRremote2.h"
/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 *
 * Version update Nov, 2014
 * Mitsubishi HVAC protocol added by Vincent Cruvellier.
 *  delaymicrosecond() modified to avoid limit.
 * Version update Dec, 2014
 * Panasonic HVAC protocol added by Mathieu Thomas. 
 *
 */

#define DHTPIN A0     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);




#define MAX_PAYLOAD 20

// COMMANDS CODES
#define SEND_IRCOMMAND 2
#define SEND_LCD_TEXT 3
#define SEND_LCD_COLOR 4
#define READ_TEMP 5
#define READ_HUM 6
#define SEND_IRCODE 7

// MAXIMUM SIZE OF THE PAYLOAD
#define MAX_PAYLOAD 27
#define IR_PAYLOAD_SIZE 7
#define IR_CODE_PAYLOAD_SIZE 27
#define LCD_TEXT_PAYLOAD_SIZE 18
#define LCD_COLOR_PAYLOAD_SIZE 3

// payload description -- IR commands timings and info

byte payload[MAX_PAYLOAD]; // no commands will be bigger than 27

boolean expectData = false;

byte command = 0;
boolean commandComplete = false;
long timeout;

long usbTimeoutMs = 1000;

// IR sender (from IRremote.h) - sets the pin and PWM properly

IRsend irsend;

rgb_lcd lcd;

void setup()
{	
  Serial.begin(9600);
  
  dht.begin();

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
    
  lcd.setRGB(100, 0, 0);
  // Initializing...
  byte lcd_introText[LCD_TEXT_PAYLOAD_SIZE] = {0x00, 0x00, 0x49, 0x6e, 0x69, 0x74, 0x69, 0x61, 0x6c, 0x69, 0x7a, 0x69, 0x6e, 0x67, 0x2e, 0x2e, 0x2e, 0x2e };
    
  send_LCDText(lcd_introText, LCD_TEXT_PAYLOAD_SIZE);
  

}

void loop()
{
  // when a command has been received from USB, execute it
  if (commandComplete) 
  {
    if (command == SEND_IRCOMMAND){
      emitIRCommand(payload, IR_PAYLOAD_SIZE);
      Serial.println("OK"); // response to the caller
    } 
    if (command == SEND_IRCODE){
      emitIRCode(payload, IR_CODE_PAYLOAD_SIZE);
      Serial.println("OK"); // response to the caller
    } 
    if (command == SEND_LCD_TEXT){
      send_LCDText(payload, LCD_TEXT_PAYLOAD_SIZE);
      Serial.println("OK"); // response to the caller
    } 
    if (command == SEND_LCD_COLOR){
      send_LCDColor(payload, LCD_COLOR_PAYLOAD_SIZE);
      Serial.println("OK"); // response to the caller
    }
    if (command == READ_TEMP){
      read_temperature();
    } 
    if (command == READ_HUM){
      read_humidity();
    } 
    
    command = 0;
    commandComplete = false;
  }

}

void send_LCDText(byte* payload, int payloadSize){
  lcd.setCursor(payload[1],payload[0]);
  // print the letter:
  for (int i = 2 ; i < payloadSize + 1 - payload[0]; i++) {
    lcd.write(char(payload[i]));
  }  
  delay(200);  
}

void send_LCDColor(byte* payload, int payloadSize){
  lcd.setRGB(payload[0], payload[1], payload[2]);
  delay(200);  
}

void read_temperature() {
  float t = dht.readTemperature();
  Serial.print(t);
  delay(200);  
}

void read_humidity() {
  float h = dht.readHumidity();
  Serial.print(h);
  delay(200);  
}

void emitIRCommand(byte* payload, int payloadSize){
  int temp, on_off;
  HvacMode mode;
  HvacFanMode fan_mode;
  HvacVanneMode vanne_mode;
  HvacProfileMode profile_mode;
    
   switch (payload[0]) {
    case 0: mode = HVAC_HOT; break;
    case 1: mode = HVAC_COLD; break;
    case 2: mode = HVAC_DRY; break;
    case 3: mode = HVAC_FAN; break;
    case 4: mode = HVAC_AUTO; break;
  }
  switch (payload[2]) {
    case 0: fan_mode = FAN_SPEED_1; break;
    case 1: fan_mode = FAN_SPEED_2; break;
    case 2: fan_mode = FAN_SPEED_3; break;
    case 3: fan_mode = FAN_SPEED_4; break;
    case 4: fan_mode = FAN_SPEED_5; break;
    case 5: fan_mode = FAN_SPEED_AUTO; break;
    case 6: fan_mode = FAN_SPEED_SILENT; break;
  }
  switch (payload[3]) {
    case 0: vanne_mode = VANNE_AUTO; break;
    case 1: vanne_mode = VANNE_H1; break;
    case 2: vanne_mode = VANNE_H2; break;
    case 3: vanne_mode = VANNE_H3; break;
    case 4: vanne_mode = VANNE_H4; break;
    case 5: vanne_mode = VANNE_H5; break;
    case 6: vanne_mode = VANNE_AUTO_MOVE; break;
  }
  switch (payload[4]) {
    case 0: profile_mode = NORMAL; break;
    case 1: profile_mode = QUIET; break;
    case 2: profile_mode = BOOST; break;
  }  
  switch (payload[6]) {
    case 0: irsend.sendHvacMitsubishi(mode, payload[1], fan_mode, vanne_mode, payload[5]); break;
    case 1: irsend.sendHAPanasonic(mode, payload[1], fan_mode, vanne_mode, profile_mode, payload[5]); break;
  }  
  
  
}

void emitIRCode(byte* payload, int payloadSize){

  
}

void serialEvent() {
  // this method is called whenever there is data coming from the serial link
  boolean timeoutOK;
  byte payload_size;
  while (Serial.available()) {
    delay(100);
    if (!expectData){
      // get the new byte:
      byte in = Serial.read();
      char inChar = (char)in; 
      switch(inChar){
      case 'I':
        //incomming IR message to send - the payload will follow
        commandComplete = false;
        command = SEND_IRCOMMAND;
        expectData = true;
        payload_size = IR_PAYLOAD_SIZE;
        break;
      case 'i':
        //incomming IR message to send - the payload will follow
        commandComplete = false;
        command = SEND_IRCODE;
        expectData = true;
        payload_size = IR_CODE_PAYLOAD_SIZE;
        break;
      case 'T':
        //incomming Text message to display - the payload will follow
        commandComplete = false;
        command = SEND_LCD_TEXT;
        expectData = true;
        payload_size = LCD_TEXT_PAYLOAD_SIZE;
        break;
      case 'C':
        //Set LCD colour - the payload will follow
        commandComplete = false;
        command = SEND_LCD_COLOR;
        expectData = true;
        payload_size = LCD_COLOR_PAYLOAD_SIZE;
        break;
      case 't':
        // Read temerature - no payload will follow
        command = READ_TEMP;
        expectData = false;
        commandComplete = true;
        break;
      case 'h':
        // Read humidity - no payload will follow
        command = READ_HUM;
        expectData = false;
        commandComplete = true;
        break;
      default:
        Serial.println("Unexpected parameter");
        return;
      }
    }else{ // data is expected - the order was received (SEND_IRCOMMAND), the payload is expected
      timeout = millis()+usbTimeoutMs;
      timeoutOK = true;
      // wait for the payload to be available. The payload is sent as characters representing hex value, 
      // so we'll have 2 chars per byte (from 00 to FF), hence the PAYLOAD_SIZE*2 value
      while (Serial.available() < payload_size * 2 && timeoutOK){
        timeoutOK = (timeout > millis());
      }
      if (timeoutOK){
        // hexConvert[] will be used to convert 2 chars from 00 to FF to an HEXA value through sscanf
        char hexConvert[5] = "0x";  
        // ASCII is received
        for (int i = 0 ; i < payload_size ; i++){
          hexConvert[2] = Serial.read();
          hexConvert[3] = Serial.read();
          hexConvert[4] = '\0';
          // convert the received chars to a numeric value in payload[i] 
          sscanf(hexConvert, "%x", &(payload[i]));
// Serial.println("Data " + (char) payload[i]);         

        }
        // in the end, the command is complete
        commandComplete = true;        
      }
      else{
        Serial.println("Error, timeout while waiting for data");
        command = 0;
        expectData = false;
        return;
      }
      expectData = false;
    }
  }
}
