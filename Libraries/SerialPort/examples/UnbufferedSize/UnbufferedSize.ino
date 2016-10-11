// Print free RAM for unbuffered mode.
//
// You can reduce flash and RAM use more by setting
// BUFFERED_TX and BUFFERED_RX zero in SerialPort.h
// to always disable buffering.
//
#include <SerialPort.h>
#include "FreeRam.h"

// no buffers
SerialPort<0, 0, 0> NewSerial;

void setup() {
  NewSerial.begin(9600);
  NewSerial.println(FreeRam());
}
void loop() {}