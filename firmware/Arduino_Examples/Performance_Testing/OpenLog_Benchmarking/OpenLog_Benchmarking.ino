/*
 11-22-2015
 SparkFun Electronics
 Nathan Seidle

 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This code sends a series of test strings to OpenLog to see if it can handle large amounts of data
 at different baud rates, file sizes, and delays between writes.

 For maximum speed the Arduino is connected to OpenLog via hardware UART during testing. All menus and control
 are transmitted over software serial. This means you'll need an external FTDI connected to pins 6/7 to be able
 to configure and control the test suite.

 Arduino TX to OpenLog RXI
 Arduino RX to OpenLog TX
 Arduino 5V to OpenLog VCC
 Arduino GND to OpenLog GND
 Arduino 8 to OpenLog GRN (aka DTR or reset)

 Arduino 6 to FTDI TX
 Arduino 7 to FTDI RX

 */

#include <SoftwareSerial.h>

byte ledPin =  13; //Status LED connected to digital pin 13
byte resetOpenLog = 8;

int settingLoops = 10;
long settingBaudRate = 115200;
int settingWriteDelay = 0; //Number of miliseconds between strings sent
int settingTestNumber = 5; //Run these settings across 5 logs, run 5 tests

float testTimeSeconds;
long testSizeCharacters;

const char testString[] = ":abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-!#";

//The actual test string has a unique char in the front and a \n\r at the end
//Remove 1 for the \0 at the end of the array
int testStringSize = (sizeof(testString) - 1) + 3; 

char testFile[] = "testfile.txt";

SoftwareSerial TestSuite(6, 7); // RX, TX

void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(resetOpenLog, OUTPUT);

  Serial.begin(settingBaudRate);

  TestSuite.begin(57600);

  //setupOpenLog(); //Resets logger and waits for the '<' I'm alive character
}

void loop()
{
  //Calc test run time based on current settings
  calcTestTime(false);

  TestSuite.println();
  TestSuite.println();
  TestSuite.println("OpenLog Benchmarking");

  TestSuite.print("Test time: ");

  if (testTimeSeconds / 60 > 1)
  {
    TestSuite.print(testTimeSeconds / 60, 2);
    TestSuite.println(" minutes");
  }
  else
  {
    TestSuite.print(testTimeSeconds, 2);
    TestSuite.println(" seconds");
  }

  TestSuite.print("Test size: ");
  TestSuite.print(testSizeCharacters);
  TestSuite.println(" characters");

  TestSuite.print("1) Baud to test at: ");
  TestSuite.println(settingBaudRate);

  TestSuite.print("2) Loops to run: ");
  TestSuite.println(settingLoops);

  TestSuite.print("3) Delay between sending each test string: ");
  TestSuite.println(settingWriteDelay);

  TestSuite.print("4) Number of tests: ");
  TestSuite.println(settingTestNumber);

  TestSuite.println("r) Run test with current settings");
  TestSuite.println("T) Test connection to OpenLog");
  TestSuite.println("e) Evaluate the test file");

  TestSuite.print(">");

  //Read command
  while (!TestSuite.available());
  char command = TestSuite.read();

  //Execute command
  if (command == '1')
  {
    baud_menu();
  }
  else if (command == '2')
  {
    loop_menu();
  }
  else if (command == '3')
  {
    delay_menu();
  }
  else if (command == '4')
  {
    numberOfTests_menu();
  }
  else if (command == 'r')
  {
    calcTestTime(true);
  
    TestSuite.print("This test will take approximately ");
  
    if (testTimeSeconds < 60)
    {
      TestSuite.print(testTimeSeconds, 2);
      TestSuite.println(" seconds");
    }
    else
    {
      TestSuite.print(testTimeSeconds / 60, 2);
      TestSuite.println(" minutes");
    }
  
    TestSuite.println();
    TestSuite.print("Are you sure?");
  
    while (!TestSuite.available());
    char incoming = TestSuite.read();
  
    if (incoming == 'y' || incoming == 'Y')
    {
      TestSuite.println();
      TestSuite.println("Beginning test...");
  
      for(int x = 0 ; x < settingTestNumber ; x++)
      {
        TestSuite.print("Running test ");
        TestSuite.print(x);
        TestSuite.print(" out of ");
        TestSuite.println(settingTestNumber);
  
        runTest(); //Each time we call this it resets OpenLog and creates new log

        delay(1000); //Give OpenLog a chance to record the remaining bits in buffer before we reset it
      }

      TestSuite.println("Test complete");
    }
    else
    {
      TestSuite.println();
      TestSuite.print(F("Bailing from test"));
    }
  }
  else if (command == 't')
  {
    if(gotoCommandMode())
      TestSuite.println("OpenLog responded!");
    else
      TestSuite.println("OpenLog failed to go into command mode.");
  }
  else if (command == 'e')
  {
    TestSuite.println("Evaluating file...");
    
    long charactersRead = readCharacterCount(testFile);

    TestSuite.print("Expected: ");
    TestSuite.println(testSizeCharacters);
    TestSuite.print("Found: ");
    TestSuite.println(charactersRead);

    long characterDelta = testSizeCharacters - charactersRead;

    TestSuite.print("Delta: ");
    TestSuite.println(characterDelta);
  }

}

void runTest()
{
  setupOpenLog(); //Resets module and waits for '<' ready character

  long startTime = millis();

  //Each test is 100 lines. 10 tests is 1000 lines (11,000 characters)
  for (int numofTests = 0 ; numofTests < settingLoops ; numofTests++)
  {
    //This loop will print 100 lines of 110 characters each
    for (int k = 33; k < 43 ; k++)
    {
      //Print one line of 110 characters with marker in the front (markers go from '!' to '*')
      Serial.write(k); //Print the ASCII value directly: ! then " then #, etc
      Serial.println(testString);

      if (settingWriteDelay > 0) delay(settingWriteDelay); //There is a small amount of time to do the if test

      //Then print 9 lines of 110 characters with new line at the end of the line
      for (int i = 1 ; i < 10 ; i++)
      {
        Serial.print(i, DEC);
        Serial.println(testString);

        if (settingWriteDelay > 0) delay(settingWriteDelay); //There is a small amount of time to do the if test
      }

      if (digitalRead(ledPin) == 0) //Turn the status LED on/off as we go
        digitalWrite(ledPin, HIGH);
      else
        digitalWrite(ledPin, LOW);
    }
  } //End numofTests loop

  unsigned long totalCharacters = (long)settingLoops * 100 * testStringSize;

  TestSuite.print("Characters pushed: ");
  TestSuite.println(totalCharacters);

  TestSuite.print("Time taken (s): ");
  TestSuite.println( (millis() - startTime) / (float)1000, 3);

  digitalWrite(ledPin, LOW);
}

//Reads the number of characters in a given file
//This function assumes the OpenLog is in command mode
long readCharacterCount(char *fileName) {

  if(gotoCommandMode() == false) //Puts OpenLog in command mode
  {
    TestSuite.println("OpenLog failed to go into command mode");
    return(0);
  }

  //Clear buffer
  while(Serial.available()) Serial.read();

  Serial.print("read ");
  Serial.print(fileName);
  Serial.write(13); //This is \r

  //The OpenLog echos the commands we send it by default so we have 'read log823.txt\r' sitting
  //in the RX buffer. Let's try to not print this.

  int timeOut;

  for (timeOut = 0 ; timeOut < 1000 ; timeOut++)
  {
    if (Serial.available())
    {
      if (Serial.read() == '\r') break;

      timeOut = 0;
    }

    delay(1);
  }
  if (timeOut == 1000)
  {
    TestSuite.println("OpenLog did not echo file name.");
  }

  //This will listen for characters coming from OpenLog
  //This loop will stop listening after 1 second of no characters received

  long charactersRead = 0;

  for (timeOut = 0 ; timeOut < 1000 ; timeOut++)
  {
    while (Serial.available())
    {
      Serial.read(); //Read the incoming char
      charactersRead++;

      timeOut = 0;
    }

    delay(1);
  }

  //TestSuite.print("Characters read: ");
  //TestSuite.println(charactersRead);

  return (charactersRead);

  //This is not perfect. The above loop will print the '.'s from the log file. These are the two escape characters
  //recorded before the third escape character is seen.
  //It will also print the '>' character. This is the OpenLog telling us it is done reading the file.

  //This function leaves OpenLog in command mode
}

void calcTestTime(boolean printIt)
{
  float microSecondsPerByte = (float)10000000 / settingBaudRate; //10 bits per byte

  /*if (printIt)
  {
    TestSuite.println();
    TestSuite.print("Each byte will take ");
    TestSuite.print(microSecondsPerByte, 2);
    TestSuite.println(" microseconds");
  }*/

  testSizeCharacters = (long)settingLoops * testStringSize * 100; //Each test is 100 lines

  if (printIt)
  {
    TestSuite.print("Test size: ");
    TestSuite.print(testSizeCharacters, DEC);
    TestSuite.println(" characters");
  }

  float loopTimeSeconds = testSizeCharacters * microSecondsPerByte / (float)1000000; //145,000 * 173 = 25,085,000 / 1,000,000 = 25 seconds

  float totalDelaySeconds = (100 * settingWriteDelay) * settingLoops / 1000; //Each time we run a 100 line test we will have 100 times the writeDelay

  testTimeSeconds = loopTimeSeconds + totalDelaySeconds;

  testTimeSeconds *= settingTestNumber; //Across the multiple tests
}

void numberOfTests_menu()
{
  long testNumber = settingTestNumber;

  TestSuite.print(F("\n\rCurrent number of tests: "));
  TestSuite.println(testNumber, DEC);

  TestSuite.println(F("Enter new number of tests ('x' to exit):"));

  TestSuite.print(F(">"));

  //Read user input
  char userInput[8]; //Max at 1000000
  readLine(userInput, sizeof(userInput));

  if (userInput[0] == 'x')
  {
    TestSuite.println(F("Exiting"));
    return; //Look for escape character
  }

  long newTests = strtolong(userInput); //Convert this string to a long

  if (newTests < 1 || newTests > 1000)
  {
    TestSuite.println(F("Out of bounds"));
  }
  else
  {
    TestSuite.print(F("Tests set to "));
    TestSuite.println(newTests);

    settingTestNumber = newTests;
  }
  
}

void baud_menu()
{
  long uartSpeed = settingBaudRate;

  TestSuite.print(F("\n\rCurrent rate: "));
  TestSuite.print(uartSpeed, DEC);
  TestSuite.println(F(" bps"));

  TestSuite.println(F("Enter new baud rate ('x' to exit):"));

  TestSuite.print(F(">"));

  //Read user input
  char newBaud[8]; //Max at 1000000
  readLine(newBaud, sizeof(newBaud));

  if (newBaud[0] == 'x')
  {
    TestSuite.println(F("Exiting"));
    return; //Look for escape character
  }

  long newRate = strtolong(newBaud); //Convert this string to a long

  if (newRate < 300 || newRate > 1000000)
  {
    TestSuite.println(F("Out of bounds"));
  }
  else
  {
    TestSuite.print(F("Baud set to "));
    TestSuite.print(newRate);
    TestSuite.println(F("bps"));

    settingBaudRate = newRate;
    Serial.begin(settingBaudRate);
  }

}

void delay_menu()
{
  long writeDelay = settingWriteDelay;

  TestSuite.print(F("\n\rCurrent delay in miliseconds between writes: "));
  TestSuite.println(writeDelay, DEC);

  TestSuite.println(F("Enter new delay ('x' to exit):"));

  TestSuite.print(F(">"));

  //Read user input
  char newDelay[8]; //Max at 1000000
  readLine(newDelay, sizeof(newDelay));

  if (newDelay[0] == 'x')
  {
    TestSuite.println(F("Exiting"));
    return; //Look for escape character
  }

  long newWriteDelay = strtolong(newDelay); //Convert this string to a long

  if (newWriteDelay > 1000) //1,000 miliseconds = 1s - This is crazy long between strings
  {
    TestSuite.println(F("Out of bounds"));
  }
  else
  {
    settingWriteDelay = newWriteDelay;

    TestSuite.print(F("Write delay set to "));
    TestSuite.println(settingWriteDelay);
  }

}

void loop_menu()
{
  TestSuite.print("Number of loops: ");
  TestSuite.println(settingLoops, DEC);

  TestSuite.println(F("Enter new number of loops ('x' to exit):"));

  TestSuite.print(F(">"));

  //Read user input
  char newLoops[8]; //Max at 1000000
  readLine(newLoops, sizeof(newLoops));

  if (newLoops[0] == 'x')
  {
    TestSuite.println(F("Exiting"));
    return; //Look for escape character
  }

  settingLoops = strtolong(newLoops); //Convert this string to a long

  TestSuite.println();
  TestSuite.print(F("New number of loops: "));
  TestSuite.println(settingLoops);
}


//Reads a line until the \n enter character is found
byte readLine(char* buffer, byte buffer_length)
{
  memset(buffer, 0, buffer_length); //Clear buffer

  byte read_length = 0;

  while (read_length < buffer_length - 1)
  {
    while (!TestSuite.available());
    byte c = TestSuite.read();

    if (c == 0x08 || c == 0x7f) { //Backspace characters
      if (read_length < 1)
        continue;

      --read_length;
      buffer[read_length] = '\0'; //Put a terminator on the string in case we are finished

      TestSuite.print((char)0x08); //Move back one space
      TestSuite.print(F(" ")); //Put a blank there to erase the letter from the terminal
      TestSuite.print((char)0x08); //Move back again

      continue;
    }

    TestSuite.print((char)c); //Echo back characters

    if (c == '\r') {
      TestSuite.println();
      buffer[read_length] = '\0';
      break;
    }
    else if (c == '\n') {
      //Do nothing - ignore newlines
    }
    else {
      buffer[read_length] = c;
      ++read_length;
    }
  }

  //Split the command line into arguments
  //split_cmd_line_args(buffer, buffer_length);

  return read_length;
}

//A rudimentary way to convert a string to a long 32 bit integer
//Used by the read command, in command shell and baud from the system menu
uint32_t strtolong(const char* str)
{
  uint32_t l = 0;
  while (*str >= '0' && *str <= '9')
    l = l * 10 + (*str++ - '0');

  return l;
}

//This function pushes OpenLog into command mode
boolean gotoCommandMode(void)
{
  int counter = 0;
  int maxWaitTime = 500;

  //Clear buffer
  while(Serial.available()) Serial.read();
  
  //We are unsure which mode the module might be in. Assume it is in command mode, let's make sure:
  Serial.write(13); //This is \r  
  
  //Wait for OpenLog to respond with '>' to indicate we are in command mode
  while (counter < maxWaitTime)
  {
    if (Serial.available())
      if (Serial.read() == '>') break;

    delay(1);
    counter++;
  }

  if (counter != maxWaitTime)
  {
    //TestSuite.println("OpenLog is in command mode");
    return(true);
  }

  //If we're this far, then maybe we are in logging mode.
  //Send three control z to enter OpenLog command mode
  Serial.write(26);
  Serial.write(26);
  Serial.write(26);

  //Wait for OpenLog to respond with '>' to indicate we are in command mode
  counter = 0;
  while (counter < maxWaitTime)
  {
    if (Serial.available())
      if (Serial.read() == '>') break;

    delay(1);
    counter++;
  }

  if (counter == maxWaitTime)
  {
    //TestSuite.println("OpenLog failed to go into command mode");
    return(false);
  }
  else
  {
    //TestSuite.println("OpenLog is now in command mode");
    return(true);
  }
}

//Setups up the software serial, resets OpenLog so we know what state it's in, and waits
//for OpenLog to come online and report '<' that it is ready to receive characters to record
void setupOpenLog(void) {
  pinMode(resetOpenLog, OUTPUT);

  //Clear buffer
  while(Serial.available()) Serial.read();

  //Reset OpenLog
  digitalWrite(resetOpenLog, LOW);
  delay(500);
  digitalWrite(resetOpenLog, HIGH);
  
  //Wait for OpenLog to respond with '<' to indicate it is alive and recording to a file
  int counter = 0;
  int maxTime = 5000; //It takes OpenLog awhile to boot up
  while (counter < maxTime)
  {
    if (Serial.available())
      if (Serial.read() == '<') break;

    delay(1);
    counter++;
  }

  if (counter == maxTime)
  {
    TestSuite.println("OpenLog failed to respond to reset");
  }
}


