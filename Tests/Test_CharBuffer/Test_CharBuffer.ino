
#define DATA_MASK "<ID=001,B0=%04d,T1=%04d,T2=%04d,H1=%04d,H2=%04d>" // Formato de trama de este dispositivo
#define DATA_LENGTH 48
char localVariables[DATA_LENGTH] = "000000000000000000000000000000000000000000000000"; // Si no se inicializa no anda

void setup(){
    Serial.begin(9600);
    Serial.println("Setup");
}

void loop(){

	unsigned int B, T1, T2, H1, H2;  // Variables para sensores conectados

  B = 123;
  T1 = 1020;
  T2 = 2;
  H1 = 58;
  H2 = 745;
  
	sprintf(localVariables, DATA_MASK, B, T1, T2, H1, H2);

  Serial.println(localVariables);
  
  delay(1000);
}
