#include <SerialPort.h>

// serial port zero with no RX or TX buffering
SerialPort<0, 0, 0> NewSerial;

void setup() {
  NewSerial.begin(9600);
  uint32_t t = micros();
  NewSerial.write("This string is used to measure the time to buffer data.\r\n");
  t = micros() - t;
  NewSerial.write("Time: ");
  NewSerial.print(t);
  NewSerial.write(" us\r\n");
}
void loop() {
  NewSerial.write("\r\nenter a string\r\n");
  while (!NewSerial.available()) {}
  do {
    NewSerial.write(NewSerial.read());
    uint32_t m = millis();
    while (!NewSerial.available() && (millis() - m) < 3) {}
  } while(NewSerial.available());
  NewSerial.write("\r\n");
}