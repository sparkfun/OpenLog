/*
 Example of creating a file, reading a file, and reading the disk properties on OpenLog
 By: Nathan Seidle
 SparkFun Electronics
 Date: September 22nd, 2013
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This example reads the firmware version of the OpenLog without the need for a USB to serial connection.
 The firmware version of your OpenLog is very helpful if you need tech support.
 
 Connect the following OpenLog to Arduino:
 RXI of OpenLog to pin 2 on the Arduino
 TXO to 3
 GRN to 4
 VCC to 5V
 GND to GND
 
 This example code assumes the OpenLog is set to operate in default mode. This is 9600bps 
 in NewLog mode, meaning OpenLog should power up and output '12<'. This code then sends the 
 three escape characters and then sends the commands to bring up the help menu '?' and then
 looks for the "OpenLog v4.0" text at the top of the menu. It will then print the version
 # to the serial terminal.
 
 This code assume OpenLog is in the default state of 9600bps with ASCII-26 as the esacape character.
 If you're unsure, make sure the config.txt file contains the following: 9600,26,3,0
 
 Be careful when sending commands to OpenLog. println() sends extra newline characters that 
 cause problems with the command parser. v2.51 and above ignores \n commands so it should be easier to 
 talk to on the command prompt level. This example code works with all OpenLog v2 and higher.
 
 */

#include <SoftwareSerial.h>

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//Connect TXO of OpenLog to pin 3, RXI to pin 2
SoftwareSerial OpenLog(3, 2); //Soft RX on 3, Soft TX out on 2
//SoftwareSerial(rxPin, txPin)

int resetOpenLog = 4; //This pin resets OpenLog. Connect pin 4 to pin GRN on OpenLog.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int statLED = 13;

void setup() {                
  pinMode(statLED, OUTPUT);
  Serial.begin(9600);

  Serial.println("Find OpenLog Firmware Version");

  setupOpenLog(); //Resets logger and waits for the '<' I'm alive character
  Serial.println("OpenLog online");

  gotoCommandMode(); //Puts OpenLog in command mode
  OpenLog.println('?'); //Send a character to get help menu
  delay(100);

  byte counter = 200;
  while(OpenLog.available() && counter > 0)
  {
    byte incoming = OpenLog.read();
    if(incoming == 'v') counter = 4; //Get the next four characters

    Serial.write(incoming);
    counter--;
  }
  Serial.println();
}

void loop() {
  //Do nothing
}

//Setups up the software serial, resets OpenLog so we know what state it's in, and waits
//for OpenLog to come online and report '<' that it is ready to receive characters to record
void setupOpenLog(void) {
  pinMode(resetOpenLog, OUTPUT);
  OpenLog.begin(9600);

  //Reset OpenLog
  digitalWrite(resetOpenLog, LOW);
  delay(100);
  digitalWrite(resetOpenLog, HIGH);

  //Wait for OpenLog to respond with '<' to indicate it is alive and recording to a file
  while(1) {
    if(OpenLog.available())
      if(OpenLog.read() == '<') break;
  }
}

//This function pushes OpenLog into command mode
void gotoCommandMode(void) {
  //Send three control z to enter OpenLog command mode
  //Works with Arduino v1.0
  OpenLog.write(26);
  OpenLog.write(26);
  OpenLog.write(26);

  //Wait for OpenLog to respond with '>' to indicate we are in command mode
  while(1) {
    if(OpenLog.available())
      if(OpenLog.read() == '>') break;
  }
}
