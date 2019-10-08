
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 4); // RX, TX

unsigned long int lastMs = 0;

void setup() {  
  Serial.begin(9600);    
  mySerial.begin(9600);  

  pinMode(3,OUTPUT);
  
  Serial.println("Setup");  
  lastMs = millis();
}


void loop() {
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  if (Serial.available()) {
    digitalWrite(3,HIGH);
    mySerial.write(Serial.read());
    digitalWrite(3,LOW);
  }


  if(millis()-lastMs > 1000){
    digitalWrite(3,HIGH);
    mySerial.print("abcdefghijklmnopqrstuvwxyz0123456789 ");
    mySerial.println(lastMs);
    digitalWrite(3,LOW);
    lastMs = millis();
  }
}
