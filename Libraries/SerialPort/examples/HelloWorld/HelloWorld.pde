// Simple usage with buffering like Arduino 1.0
#include <SerialPort.h>

// use NewSerial for port 0
USE_NEW_SERIAL;

void setup() {
  NewSerial.begin(9600);
  NewSerial.println("Hello World!");
}
void loop() {}