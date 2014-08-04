
void setup() {
  Serial.begin(9600);
  uint32_t t = micros();
  Serial.write("This string is used to measure the time to buffer data.\r\n");
  t = micros() - t;
  Serial.write("Time: ");
  Serial.print(t);
  Serial.write(" us\r\n");
}
void loop() {
  Serial.write("\r\nenter a string\r\n");
  while (!Serial.available()) {}
  do {
    Serial.write(Serial.read());
    uint32_t m = millis();
    while (!Serial.available() && (millis() - m) < 3) {}
  } while(Serial.available());
  Serial.write("\r\n");
}