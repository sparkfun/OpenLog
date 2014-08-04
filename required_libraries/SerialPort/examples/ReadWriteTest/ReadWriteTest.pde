// test that ring buffer overrun can be detected
#include <SerialPort.h>
// port 0, 16 byte RX and TX buffers
SerialPort<0, 16, 16>  port0;

void setup() {
  port0.begin(9600);
  port0.write("SerialPort version: ");
  port0.println(SERIAL_PORT_VERSION);
}
void loop() {
  uint8_t buffer[10];
  port0.writeln("Enter a string. Overrun error for more than 16 bytes.");
  while (!port0.available()) {}
  // delay so an ring buffer overrun will occur for long strings
  delay(50);
  uint32_t m = millis();
  do {
    size_t n = port0.read(buffer, sizeof (buffer));
    if (n) {
      m = millis();
      port0.write(buffer, n);
    }
  } while ((millis() - m) < 4);
  port0.writeln();
  uint8_t e = port0.getRxError();
  if (e) {
    port0.write("Error: ");
    port0.println(e, HEX);
    port0.clearRxError();
  }
}