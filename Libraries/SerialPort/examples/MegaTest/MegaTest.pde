// Test all ports on the Mega
//
// place loopback jumpers, RX connected to TX,
// on ports 1, 2, and 3.
//
#include <SerialPort.h>
// port 0 unbuffered
SerialPort<0, 0, 0>  port0;

// port 1 buffered RX
SerialPort<1, 32, 0> port1;

// port 2 buffered RX and TX
SerialPort<2, 32, 32> port2;

// port 3 buffered RX and TX
SerialPort<3, 32, 32> port3;

void transfer(Stream* in, Stream* out) {
  while(!in->available()) {}
  do {
    out->write(in->read());
    uint32_t m = millis();
    while (!in->available() && (millis() -m) < 3) {}
  } while (in->available());
}

void setup() {
  port0.begin(9600);
  port1.begin(9600);
  port2.begin(9600);
  port3.begin(9600);
}
void loop() {
  port0.write("type a string\r\n");
  transfer(&port0, &port1);
  transfer(&port1, &port2);
  transfer(&port2, &port3);
  transfer(&port3, &port0);
  port0.write("\r\n\r\n");
}
