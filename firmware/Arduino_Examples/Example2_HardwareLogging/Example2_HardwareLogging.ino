/*
  Recording to OpenLog example
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 8th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This is an example of basic recording to OpenLog using hardware serial. This example is best for users who
  are plugging the OpenLog directly onto the programming connector on an Arduino Pro Mini or Arduino Pro.

  We DON'T recommend this method for beginners. See Example 1 for an easier software serial example. 
  The reason is if you upload a sketch to an Arduino with OpenLog attached then OpenLog will log the 
  uploading of your sketch. This may cause the OpenLog to become reconfigured. No harm will be 
  caused but it can corrupt the log file.

  Connect the following OpenLog to Arduino:
  RXI of OpenLog to TX on Arduino
  VCC to 5V
  GND to GND

  This example records whatever the user Serial.prints(). This is the easiest but NOTE: You cannot
  upload sketches to your Arduino with the OpenLog attached (because of bus contention). Upload this
  sketch first, and then connect the OpenLog to the TX pin on the Arduino.
*/

int statLED = 13;

float dummyVoltage = 3.50; //This just shows to to write variables to OpenLog

void setup() {
  pinMode(statLED, OUTPUT);
  Serial.begin(9600);

  Serial.println("Example print to OpenLog");

  Serial.println("Anything printed to COM port gets logged!");

  //Write something to OpenLog
  Serial.println("Hi there! How are you today?");
  Serial.print("Voltage: ");
  Serial.println(dummyVoltage);
  dummyVoltage++;
  Serial.print("Voltage: ");
  Serial.println(dummyVoltage);
}

void loop() {
  digitalWrite(statLED, HIGH);
  delay(1000);
  digitalWrite(statLED, LOW);
  delay(1000);
}

