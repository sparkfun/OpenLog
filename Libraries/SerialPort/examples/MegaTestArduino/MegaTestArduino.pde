// Test all ports on the Mega
//
// place loopback jumpers, RX connected to TX,
// on ports 1, 2, and 3.
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
