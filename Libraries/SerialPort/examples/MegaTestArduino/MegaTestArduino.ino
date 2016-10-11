// Test all ports on the Mega.
//
//
// A string read on port zero will be sent to port one,
// it will then be read from port one and sent to port
// two, next it will be read from port two and sent to
// port three, and finally it will be read from port
// three and sent to port zero.
//
// Place a loopback jumper connecting
// RX1 to TX1,  RX2 to TX2,  RX3 to TX3 on ports 1, 2, and 3.
// Do not place a jumper on port zero.
//
void transfer(Stream* in, Stream* out) {
  while(!in->available()) {}
  do {
    out->write(in->read());
    uint32_t m = millis();
    while (!in->available() && (millis() - m) < 3) {}
  } while (in->available());
}
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
}
void loop() {
  Serial.write("type a string\r\n");
  transfer(&Serial, &Serial1);
  transfer(&Serial1, &Serial2);
  transfer(&Serial2, &Serial3);
  transfer(&Serial3, &Serial);
  Serial.write("\r\n\r\n");
}
