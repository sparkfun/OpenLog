/*
  6-1-2011
  SparkFun Electronics 2011
  Nathan Seidle
  
  Controlling OpenLog command line from an Arduino
  
  This example code assumes the OpenLog is set to operate in NewLog mode, meaning OpenLog should power
  up and output '12<'. This code then sends the three escape characters and then sends the commands
  to create a new random file called nate###.txt where ### is a random number from 0 to 999.
  
  Be careful when sending commands to OpenLog. println() sends extra newline characters that 
  cause problems with the command parser. The new v2.51 ignores \n commands so it should be easier to 
  talk to on the command prompt level. This example code works with all OpenLog v2 and higher.
  
 */

char buff[50];
int fileNumber;

int statLED = 13;
int resetOpenLog = 2;

void setup() {                
  pinMode(statLED, OUTPUT);
  pinMode(resetOpenLog, OUTPUT);
  
  randomSeed(analogRead(0));
  Serial.begin(115200);

  //Reset OpenLog
  digitalWrite(resetOpenLog, LOW);
  delay(100);
  digitalWrite(resetOpenLog, HIGH);

  //Wait for OpenLog to respond with '<' to indicate it is alive and recording to a file
  while(1) {
    if(Serial.available())
      if(Serial.read() == '<') break;
  }
  
  //Send three control z to enter OpenLog command mode
  Serial.print(byte(26));
  Serial.print(byte(26));
  Serial.print(byte(26));
  
  //Wait for OpenLog to respond with '>' to indicate we are in command mode
  while(1) {
    if(Serial.available())
      if(Serial.read() == '>') break;
  }
  
  fileNumber = random(999); //Select a random file #, 0 to 999
  
  //Send new (random from 0 to 999) file name
  
  //Old way
  sprintf(buff, "new nate%03d.txt\r", fileNumber);
  Serial.print(buff); //\r in string + regular print works with older v2.5 Openlogs

  //New way
  //sprintf(buff, "new nate%03d.txt", fileNumber);
  //Serial.println(buff); //regular println works with v2.51 and above

  
  //Wait for OpenLog to return to waiting for a command
  while(1) {
    if(Serial.available())
      if(Serial.read() == '>') break;
  }
  
  sprintf(buff, "append nate%03d.txt\r", fileNumber);
  Serial.print(buff);
  
  //Wait for OpenLog to indicate file is open and ready for writing
  while(1) {
    if(Serial.available())
      if(Serial.read() == '<') break;
  }
}

void loop() {
  Serial.println("Yay!");
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
}


