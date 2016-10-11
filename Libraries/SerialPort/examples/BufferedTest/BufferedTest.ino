// Check time to buffer data and function of available().
#include <SerialPort.h>

// port zero, 63 character RX and TX buffers
SerialPort<0, 63, 63> NewSerial;

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