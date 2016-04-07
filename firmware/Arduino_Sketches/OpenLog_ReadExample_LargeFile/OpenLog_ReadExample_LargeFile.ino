/*
 Example of reading of a large file
 By: Nathan Seidle
 SparkFun Electronics
 Date: September 22nd, 2013
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This example connects to OpenLog over the hardware serial pins (pins 0 and 1) at 9600bps,
 then opens a file called 'bigtext.txt',
 then reports the contents of that file to a Bluetooth (or other serial) device over software serial on pins 9/10 at 14400bps.
 
 Note: We listen to OpenLog at 9600 but report to Bluetooth at 14400 in order to avoid dropping characters. 115200bps,
 57600bps, 19200 and 14400 works great. Reporting to the device hooked to softare serial at 9600 will not work.
 
 Note: You must disconnect OpenLog from the Arduino's TX and RX pins when flashing new firmware onto the Arduino.
 
 Connect the following OpenLog pins to Arduino:
 TXO of OpenLog to pin 0 on the Arduino
 RXI to 1
 GRN to 4
 VCC to 5V
 GND to GND
 
 Connect the following Bluetooth pins to Arduino:
 RXI of Bluetooth serial device to pin 9 on the Arduino
 TXO to 10 (not really used)
 
 This example code assumes the OpenLog is set to operate at 9600bps in NewLog mode, meaning OpenLog 
 should power up and output '12<'. This code then sends the three escape characters and then sends 
 the commands to create a new random file called log###.txt where ### is a random number from 0 to 999.
 The example code will then read back the random file and print it to the serial terminal.
 
 This code assume OpenLog is in the default state of 9600bps with ASCII-26 as the esacape character.
 If you're unsure, make sure the config.txt file contains the following: 9600,26,3,0
 
 Be careful when sending commands to OpenLog. println() sends extra newline characters that 
 cause problems with the command parser. The new v2.51 ignores \n commands so it should be easier to 
 talk to on the command prompt level. This example code works with all OpenLog v2 and higher.
 */

#include <SoftwareSerial.h>

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//Connect Bluetooth module via software serial
SoftwareSerial BluetoothOut(10, 9); //Soft Bluetooth RX on 10, soft bluetooth TX on 9

int resetOpenLog = 4; //This pin resets OpenLog. Connect pin 4 to pin GRN on OpenLog.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int statLED = 13;

void setup() {                
  pinMode(statLED, OUTPUT);
  Serial.begin(9600);
  
  setupOpenLog(); //Resets logger and waits for the '<' I'm alive character

  BluetoothOut.begin(14400); //Start connection with Bluetooth
  BluetoothOut.println("Bluetooth connection online");
}

void loop() {

  //Now let's read back
  gotoCommandMode(); //Puts OpenLog in command mode
  readFile("bigtext.txt"); //Count the number of characters in this file
  //readFile("smltext.txt"); //Small text is only a few dozen characters - used to locate white space chars

  BluetoothOut.println();
  BluetoothOut.println("File read complete");

  //Infinite loop
  while(1) {
    digitalWrite(13, HIGH);
    delay(250);
    digitalWrite(13, LOW);
    delay(250);
  }
}

//Reads the contents of a given file and dumps it to the serial terminal
//This function assumes the OpenLog is in command mode
void readFile(char *fileName) {

  //BluetoothOut.println("Counting characters, be right back");

  //Old way
  Serial.print("read ");
  Serial.print(fileName);
  Serial.write(13); //This is \r

  //New way
  //OpenLog.print("read ");
  //OpenLog.println(filename); //regular println works with OpenLog v2.51 and above

  //The OpenLog echos the commands we send it by default so we have 'read log823.txt\r' sitting 
  //in the RX buffer. Let's try to not print this.
  while(1) {
    if(Serial.available())
      if(Serial.read() == '\r') break;
  }  

  //This will listen for characters coming from OpenLog and print them to the terminal
  //This relies heavily on the SoftSerial buffer not overrunning. This will probably not work
  //above 38400bps.
  //This loop will stop listening after 1 second of no characters received

  int charactersRead = 0;

  for(int timeOut = 0 ; timeOut < 1000 ; timeOut++)
  {
    while(Serial.available())
    {
      BluetoothOut.print(Serial.read()); //Take the character from OpenLog and push it to software serial TX
      
      charactersRead++;//= charsToRead;
      
      timeOut = 0;
    }

    delay(1);
    
  }
  
  //BluetoothOut.print("Characters read: ");
  //BluetoothOut.println(charactersRead);

  //This is not perfect. The above loop will print the '.'s from the log file. These are the two escape characters
  //recorded before the third escape character is seen.
  //It will also print the '>' character. This is the OpenLog telling us it is done reading the file.  

  //This function leaves OpenLog in command mode
}

//This function pushes OpenLog into command mode
void gotoCommandMode(void) {
  //Send three control z to enter OpenLog command mode
  //Works with Arduino v1.0
  Serial.write(26);
  Serial.write(26);
  Serial.write(26);

  //Wait for OpenLog to respond with '>' to indicate we are in command mode
  while(1) {
    if(Serial.available())
      if(Serial.read() == '>') break;
  }
}

//Setups up the software serial, resets OpenLog so we know what state it's in, and waits
//for OpenLog to come online and report '<' that it is ready to receive characters to record
void setupOpenLog(void) {
  pinMode(resetOpenLog, OUTPUT);

  //Reset OpenLog
  digitalWrite(resetOpenLog, LOW);
  delay(500);
  digitalWrite(resetOpenLog, HIGH);

  //Wait for OpenLog to respond with '<' to indicate it is alive and recording to a file
  while(1) {
    if(Serial.available())
      if(Serial.read() == '<') break;
  }
}

