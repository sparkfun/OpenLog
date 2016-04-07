/*
 4-14-2010
 SparkFun Electronics 2012
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 This is a simple test sketch for OpenLog. It sends a large batch of characters to the serial port at 9600bps.
 Original test was recomended by ScottH on issue #12:
 http://github.com/nseidle/OpenLog/issues#issue/12
 
 Arduino TX to OpenLog RXI
 Arduino 5V to OpenLog VCC
 Arduino GND to OpenLog GND
 
 To use this sketch, attach RXI pin of OpenLog to TX pin on Arduino. Power OpenLog from 5V (or 3.3V) and GND pins from Arduino.
 After power up, OpenLog will start flashing (indicating it's receiving characters). It takes about 1 minute for 
 the sketch to run to completion. This will create a file that looks like this:
 
 ...
 6:abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-!#
 7:abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-!#
 8:abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-!#
 9:abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-!#
 #:abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-!#
 1:abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-!#
 2:abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-!#
 ...
 
 The reason for creating these character blocks is it allows for a reader to very quickly scan the visible characters and 
 indentify any byte errors or glitches along the way. Every 9 lines we print a 10th line that has a leading character 
 such as # or !. These allow us to quickly see the different blocks of 10 lines.
 
 Note: Bootloading an Arduino will cause OpenLog to drop to command mode. This is because OpenLog is looking for the 
 escape character (ctrl+z). During a bootload, all sorts of characters get sent to the Arduino and therefore ctrl+z is likely
 to get sent to OpenLog. v1.51 of the OpenLog firmware fixes this potential error by requiring three escape characters.
 
 */

int ledPin =  13; //Status LED connected to digital pin 13

void setup() 
{ 
  pinMode(ledPin, OUTPUT);     

  //Serial.begin(9600); //9600bps is default for OpenLog
  //Serial.begin(57600); //Much faster serial, used for testing buffer overruns on OpenLog
  Serial.begin(115200); //Much faster serial, used for testing buffer overruns on OpenLog

  delay(1000); //Wait a second for OpenLog to init

  Serial.println(); 
  Serial.println("Run OpenLog Test"); 

  int testAmt = 30;
  //At 9600, testAmt of 4 takes about 1 minute, 10 takes about 3 minutes
  //At 57600, testAmt of 10 takes about 1 minute, 40 takes about 5 minutes
  //At 115200, testAmt of 30 takes about 1 minute
  //testAmt of 10 will push 111,000 characters/bytes. With header and footer strings, total is 111,052

  //Each test is 100 lines. 10 tests is 1000 lines (11,000 characters)
  for(int numofTests = 0 ; numofTests < testAmt ; numofTests++)
  {
    //This loop will print 100 lines of 110 characters each
    for(int k = 33; k < 43 ; k++)
    {
      //Print one line of 110 characters with marker in the front (markers go from '!' to '*')
      Serial.write(k); //Print the ASCII value directly: ! then " then #, etc
      Serial.println(":abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-!#");
      //delay(50);

      //Then print 9 lines of 110 characters with new line at the end of the line
      for(int i = 1 ; i < 10 ; i++)
      {
        Serial.print(i, DEC);
        Serial.println(":abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-!#");
        //delay(50);
      }  

      if(digitalRead(ledPin) == 0) //Turn the status LED on/off as we go
        digitalWrite(ledPin, HIGH);
      else
        digitalWrite(ledPin, LOW);
    }
  } //End numofTests loop

  unsigned long totalCharacters = (long)testAmt * 100 * 110;
  Serial.print("Characters pushed: ");
  Serial.println(totalCharacters);  
  Serial.print("Time taken (s): ");
  Serial.println(millis()/1000);
  Serial.println("Done!");
} 

void loop() 
{ 
  //Blink the Status LED because we're done!
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
  delay(1000);
} 


