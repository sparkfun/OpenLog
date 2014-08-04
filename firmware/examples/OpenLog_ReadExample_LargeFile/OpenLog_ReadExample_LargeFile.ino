/*
 Example of reading of a large file
 By: Nathan Seidle
 SparkFun Electronics
 Date: September 22nd, 2013
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This example opens a file called 'bigtext.txt' and reports the number of characters in the file.
 
 Note: In this example we increase the reporting baud to 57600 but the serial com with OpenLog
 remains at 9600bps.
 
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
  Serial.begin(57600);

  setupOpenLog(); //Resets logger and waits for the '<' I'm alive character
  Serial.println("OpenLog online");
}

void loop() {

  //Now let's read back
  gotoCommandMode(); //Puts OpenLog in command mode
  //readFile("bigtext.txt"); //Count the number of characters in this file
  readFile("smltext.txt"); //Small text is only a few dozen characters - used to locate white space chars

  Serial.println();
  Serial.println("File read complete");

  //Infinite loop
  while(1) {
    digitalWrite(13, HIGH);
    delay(250);
    digitalWrite(13, LOW);
    delay(250);
  }
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

//Reads the contents of a given file and dumps it to the serial terminal
//This function assumes the OpenLog is in command mode
void readFile(char *fileName) {

  Serial.println("Counting characters, be right back");

  //Old way
  OpenLog.print("read ");
  OpenLog.print(fileName);
  OpenLog.write(13); //This is \r

  //New way
  //OpenLog.print("read ");
  //OpenLog.println(filename); //regular println works with OpenLog v2.51 and above

  //The OpenLog echos the commands we send it by default so we have 'read log823.txt\r' sitting 
  //in the RX buffer. Let's try to not print this.
  while(1) {
    if(OpenLog.available())
      if(OpenLog.read() == '\r') break;
  }  

  //This will listen for characters coming from OpenLog and print them to the terminal
  //This relies heavily on the SoftSerial buffer not overrunning. This will probably not work
  //above 38400bps.
  //This loop will stop listening after 1 second of no characters received

  int charactersRead = 0;

  for(int timeOut = 0 ; timeOut < 1000 ; timeOut++) {
    while(OpenLog.available()) {
      char tempString[100];
      
      int spot = 0;
      while(OpenLog.available()) {
        charactersRead++;
        tempString[spot++] = OpenLog.read();
        if(spot > 98) break;
      }
      tempString[spot] = '\0';
      
      //Don't print these characters to the terminal for this example.
      //It takes too much time to try to both read and print the chars.
      //Serial.write(tempString); //Take the string from OpenLog and push it to the Arduino terminal

      timeOut = 0;
    }

    delay(1);
  }
  
  Serial.print("Characters read: ");
  Serial.println(charactersRead);

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
  while(1) {
    if(OpenLog.available())
      if(OpenLog.read() == '>') break;
  }
}
