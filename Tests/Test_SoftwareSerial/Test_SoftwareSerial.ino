#include <SoftwareSerial.h>

SoftwareSerial mySerial(5, 6); // RX, TX

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);

  Serial.println("Goodnight moon!");

  mySerial.begin(9600);
  mySerial.println("Hello, world?");
}

void loop() {
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
}