/*
  Recording to OpenLog example
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 8th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This is an example of basic recording to OpenLog using software serial. This example is best for users who
  want to use OpenLog with an Uno or other platform capable of softwareSerial.

  Connect the following OpenLog to Arduino:
  RXI of OpenLog to pin 5 on Arduino
  VCC to 5V
  GND to GND

  This example records whatever the user OpenLog.prints(). This uses software serial on pin 5 instead
  of hardware serial (TX/RX pins). Nearly any pin can be used for software serial.
*/

#include <SoftwareSerial.h>

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//Connect RXI of OpenLog to pin 5 on Arduino
SoftwareSerial OpenLog(0, 5); // 0 = Soft RX pin (not used), 5 = Soft TX pin
//5 can be changed to any pin. See limitation section on https://www.arduino.cc/en/Reference/SoftwareSerial
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int statLED = 13;

float dummyVoltage = 3.50; //This just shows to to write variables to OpenLog

void setup() {
  pinMode(statLED, OUTPUT);
  Serial.begin(9600);

  OpenLog.begin(9600); //Open software serial port at 9600bps

  Serial.println("This serial prints to the COM port");
  OpenLog.println("This serial records to the OpenLog text file");

  //Write something to OpenLog
  OpenLog.println("Hi there! How are you today?");
  OpenLog.print("Voltage: ");
  OpenLog.println(dummyVoltage);
  dummyVoltage++;
  OpenLog.print("Voltage: ");
  OpenLog.println(dummyVoltage);

  Serial.println("Text written to file. Go look!");
}

void loop() {
  digitalWrite(statLED, HIGH);
  delay(1000);
  digitalWrite(statLED, LOW);
  delay(1000);
}


