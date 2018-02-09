/*
  Example of creating a file, reading a file, and reading the disk properties on OpenLog
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 8th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example drops to command mode and begins writing to a file with an offset

  Connect the following OpenLog to Arduino:
  RXI of OpenLog to pin 2 on the Arduino
  TXO to 3
  GRN to 4
  VCC to 5V
  GND to GND

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
//Connect TXO of OpenLog to pin 3, RXI to pin 2
SoftwareSerial OpenLog(3, 2); //Soft RX on 3, Soft TX out on 2
//SoftwareSerial(rxPin, txPin)

int resetOpenLog = 4; //This pin resets OpenLog. Connect pin 4 to pin GRN on OpenLog.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int statLED = 13;

void setup() {
  pinMode(statLED, OUTPUT);
  Serial.begin(9600);

  setupOpenLog(); //Resets logger and waits for the '<' I'm alive character
  Serial.println("OpenLog online");

  //Create a file with random name
  randomSeed(analogRead(A0)); //Use the analog pins for a good seed value
  int fileNumber = random(999); //Select a random file #, 0 to 999
  char fileName[12]; //Max file name length is "12345678.123" (12 characters)
  sprintf(fileName, "log%03d.txt", fileNumber);

  gotoCommandMode(); //Puts OpenLog in command mode
  createFile(fileName); //Creates a new file called log###.txt where ### is random

  Serial.print("Random file created: ");
  Serial.println(fileName);

  //Write 1000 random characters to this file
  for (int x = 0 ; x < 1000 ; x++)
  {
    char randomCharacter = random('a', 'z'); //Pick a character, a to z
    OpenLog.write(randomCharacter);
  }

  //Now write to the same file but at a specific location
  gotoCommandMode(); //Puts OpenLog in command mode
  if (writeFile(fileName, 10) == true) //Write to fileName starting at location 10
  {
    //Write something to this file at location 10
    OpenLog.println("Hello world!");

    //The write command will continue to draw in characters until a blank line is received
    OpenLog.print("\r");

    //OpenLog is now returned to the command prompt
  }

  //Now let's read back the file to see what it looks like
  readFile(fileName); //This dumps the contents of a given file to the serial terminal

  Serial.println();
  Serial.println("File read complete");
}

void loop() {
  digitalWrite(statLED, HIGH);
  delay(1000);
  digitalWrite(statLED, LOW);
  delay(1000);
}

//Setups up the software serial, resets OpenLog so we know what state it's in, and waits
//for OpenLog to come online and report '<' that it is ready to receive characters to record
void setupOpenLog(void) {
  pinMode(resetOpenLog, OUTPUT);
  OpenLog.begin(9600);

  //Reset OpenLog
  digitalWrite(resetOpenLog, LOW);
  delay(10);
  digitalWrite(resetOpenLog, HIGH);

  //Wait for OpenLog to respond with '<' to indicate it is alive and recording to a file
  while (1) {
    if (OpenLog.available())
      if (OpenLog.read() == '<') break;
  }
}

//Open a file for writing. Begin writing at an offset
boolean writeFile(char *fileName, int offset) {

  OpenLog.print("write ");
  OpenLog.print(fileName);
  OpenLog.print(" ");
  OpenLog.print(offset);
  OpenLog.write(13); //This is \r

  //The OpenLog echos the commands we send it by default so we have 'write log254.txt 10\r' sitting
  //in the RX buffer. Let's try to ignore this.
  while (1) {
    if (OpenLog.available())
      if (OpenLog.read() == '\r') break;
  }

  //OpenLog should respond with a < letting us know it's receiving characters
  int counter = 0;
  while (counter++ < 1000) {
    if (OpenLog.available())
      if (OpenLog.read() == '<') return (true);
    delay(1);
  }

  Serial.println("Write offset failed: Does the file exist?");
  return (false);
}

//This function creates a given file and then opens it in append mode (ready to record characters to the file)
//Then returns to listening mode
void createFile(char *fileName) {

  //Old way
  OpenLog.print("new ");
  OpenLog.print(fileName);
  OpenLog.write(13); //This is \r

  //New way
  //OpenLog.print("new ");
  //OpenLog.println(filename); //regular println works with OpenLog v2.51 and above

  //Wait for OpenLog to return to waiting for a command
  while (1) {
    if (OpenLog.available())
      if (OpenLog.read() == '>') break;
  }

  OpenLog.print("append ");
  OpenLog.print(fileName);
  OpenLog.write(13); //This is \r

  //Wait for OpenLog to indicate file is open and ready for writing
  while (1) {
    if (OpenLog.available())
      if (OpenLog.read() == '<') break;
  }

  //OpenLog is now waiting for characters and will record them to the new file
}

//Reads the contents of a given file and dumps it to the serial terminal
//This function assumes the OpenLog is in command mode
void readFile(char *fileName) {

  while(OpenLog.available()) OpenLog.read(); //Clear incoming buffer

  OpenLog.print("read ");
  OpenLog.print(fileName);
  OpenLog.write(13); //This is \r

  //The OpenLog echos the commands we send it by default so we have 'read log823.txt\r' sitting
  //in the RX buffer. Let's try to not print this.
  while (1) {
    if (OpenLog.available())
      if (OpenLog.read() == '\r') break;
  }

  Serial.println("Reading from file:");

  //This will listen for characters coming from OpenLog and print them to the terminal
  //This relies heavily on the SoftSerial buffer not overrunning. This will probably not work
  //above 38400bps.
  //This loop will stop listening after 1 second of no characters received
  for (int timeOut = 0 ; timeOut < 1000 ; timeOut++) {
    while (OpenLog.available()) {
      char tempString[100];

      int spot = 0;
      while (OpenLog.available()) {
        tempString[spot++] = OpenLog.read();
        if (spot > 98) break;
      }
      tempString[spot] = '\0';
      Serial.write(tempString); //Take the string from OpenLog and push it to the Arduino terminal
      timeOut = 0;
    }

    delay(1);
  }

  //This is not perfect. The above loop will print the '.'s from the log file. These are the two escape characters
  //recorded before the third escape character is seen.
  //It will also print the '>' character. This is the OpenLog telling us it is done reading the file.

  //This function leaves OpenLog in command mode
}

//Check the stats of the SD card via 'disk' command
//This function assumes the OpenLog is in command mode
void readDisk() {

  //Old way
  OpenLog.print("disk");
  OpenLog.write(13); //This is \r

  //New way
  //OpenLog.print("read ");
  //OpenLog.println(filename); //regular println works with OpenLog v2.51 and above

  //The OpenLog echos the commands we send it by default so we have 'disk\r' sitting
  //in the RX buffer. Let's try to not print this.
  while (1) {
    if (OpenLog.available())
      if (OpenLog.read() == '\r') break;
  }

  //This will listen for characters coming from OpenLog and print them to the terminal
  //This relies heavily on the SoftSerial buffer not overrunning. This will probably not work
  //above 38400bps.
  //This loop will stop listening after 1 second of no characters received
  for (int timeOut = 0 ; timeOut < 1000 ; timeOut++) {
    while (OpenLog.available()) {
      char tempString[100];

      int spot = 0;
      while (OpenLog.available()) {
        tempString[spot++] = OpenLog.read();
        if (spot > 98) break;
      }
      tempString[spot] = '\0';
      Serial.write(tempString); //Take the string from OpenLog and push it to the Arduino terminal
      timeOut = 0;
    }

    delay(1);
  }

  //This is not perfect. The above loop will print the '.'s from the log file. These are the two escape characters
  //recorded before the third escape character is seen.
  //It will also print the '>' character. This is the OpenLog telling us it is done reading the file.

  //This function leaves OpenLog in command mode
}

//This function pushes OpenLog into command mode
void gotoCommandMode(void) {
  //Send three control z to enter OpenLog command mode
  //Works with Arduino v1.0
  OpenLog.write(26);
  OpenLog.write(26);
  OpenLog.write(26);

  //Wait for OpenLog to respond with '>' to indicate we are in command mode
  while (1) {
    if (OpenLog.available())
      if (OpenLog.read() == '>') break;
  }
}
