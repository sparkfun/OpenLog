// Print free RAM for Arduino HardwareSerial
//
#include "FreeRam.h"

void setup() {
  Serial.begin(9600);
  Serial.println(FreeRam());
}
void loop() {
}
