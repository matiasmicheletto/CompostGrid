#define AN_1V A3 // Pin de entrada para lectura del sensor de temperatura 1
#define AN_5V A0 // Pin de entrada para lectura del sensor de temperatura 1

int T1, T2; // Valores de temperatura (0 a 1023)

void setup(){
	Serial.begin(9600);
	
	pinMode(AN_1V,INPUT); // Creo que no hace falta para analogicos, pero por las dudas
	pinMode(AN_5V,INPUT);
	
	Serial.println("Listo.");
}

void toggleReference(boolean internal){
  if(internal){
    analogReference(INTERNAL); // Referencia de 1.1V  
    analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
    analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
    analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
    analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
    analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
    analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
  }else{
    analogReference(DEFAULT); // Referencia de 5V
  }
}

void loop(){
  toggleReference(true);
  T1 = analogRead(AN_1V);
  toggleReference(false);
  T2 = analogRead(AN_5V);

  // Monitor serie
	//Serial.print("T1 = ");Serial.print((float)T1/1023.0*1.1);Serial.print("V (");Serial.print(T1);Serial.println(")");
  //Serial.print("T2 = ");Serial.print((float)T2/1023.0*4.49);Serial.print("V (");Serial.print(T2);Serial.println(")");
  //Serial.println(" ");

  // Graficador serie
  Serial.print((float)T1/1023.0*1.1);Serial.print(",");Serial.println((float)T2/1023.0*4.49);
  
  delay(500);
}
