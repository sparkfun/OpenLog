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

int statLED =  13; //Status LED connected to digital pin 13

void setup() 
{ 
  pinMode(statLED, OUTPUT);     

  //Serial.begin(9600); //9600bps is default for OpenLog
  //Serial.begin(57600); //Much faster serial, used for testing buffer overruns on OpenLog
  Serial.begin(115200); //Much faster serial, used for testing buffer overruns on OpenLog

  delay(1000); //Wait a second for OpenLog to init
  
  randomSeed(analogRead(A0)); //Feed the random number generator

  //const char* fileHeader = "OpenLong Binary Test";
  //Serial.print(fileHeader);
  
  long lastBlink = millis();
  
  long bytesToSend = 200000;
  for(long x = 0 ; x < bytesToSend ; x++)
  {
    Serial.write(random(0, 256)); //Write a binary number 0 to 255 to OpenLog
    
    if(millis() - lastBlink > 2000)
    {
      lastBlink = millis();
      if(digitalRead(statLED) == LOW)
        digitalWrite(statLED, HIGH);
      else
        digitalWrite(statLED, LOW);
    }
  }

  /*unsigned long totalCharacters = (long)testAmt * 100 * 110;
  Serial.print("Characters pushed: ");
  Serial.println(totalCharacters);  
  Serial.print("Time taken (s): ");
  Serial.println(millis()/1000);
  Serial.println("Done!");*/
} 

void loop() 
{ 
  //Blink the Status LED because we're done!
  digitalWrite(statLED, HIGH);
  delay(200);
  digitalWrite(statLED, LOW);
  delay(200);
} 


