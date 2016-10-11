// Print free RAM for Arduino 1.0 style buffering.
//
#include <SerialPort.h>
#include "FreeRam.h"

SerialPort<0, 63, 63> NewSerial;

// for Arduino 0022 style buffering use this
//SerialPort<0, 127, 0> NewSerial;

void setup() {
  NewSerial.begin(9600);
  NewSerial.println(FreeRam());
}
void loop() {}