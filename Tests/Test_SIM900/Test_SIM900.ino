
// Formato de trama de este dispositivo (cambiar ID para cada uno)
#define DATA_MASK "ID=Mati&B0=%04d&T1=%04d&T2=%04d&H1=%04d&H2=%04d>" 
// Trama de caracteres para enviar datos medidos
#define DATA_LENGTH 48 // Longitud de las tramas de transmision (485 y sim900)
#define START_CHAR 'I' // Caracter indicador de inicio de la trama (La I de ID)
#define END_CHAR '>' // Caracter indicador de fin de la trama

#define SERVER_HTTP_CMD "AT+HTTPPARA=\"URL\",\"http://ascasubi.inta.gob.ar/redae/insertar.php?" // Direccion del server

// Puerto serie de Sim900
#define SIM900_ENABLE 9 // Habilitacion del modulo GSM-GPRS
#define SIM900_RX 5
#define SIM900_TX 6

#include <SoftwareSerial.h>



SoftwareSerial Sim900(5, 6); // RX, TX
SoftwareSerial Rs485(2, 4); 		// RX, TX

char localVariables[DATA_LENGTH] =  "000000000000000000000000000000000000000000000000";

void setup() {  
	pinMode(SIM900_ENABLE,OUTPUT);
  Serial.begin(9600);    
	Rs485.begin(9600);
	Rs485.listen();
	Serial.println("Datalogger listo.");
}

void sim900BufferDump(){ // Vaciar buffer para sim900 por serie nativo
	Serial.print("Vaciando buffer sim900... ");
	while(Sim900.available())
    Serial.write(Sim900.read());
	Serial.println("Listo.");
}

void sim900Config(){ // Comandos de configuracion del sim900 como modulo gsm
  Serial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
	Sim900.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
	delay(2000);
	Serial.println("AT+CSTT=\"internet.gprs.unifon.com.ar\",\"internet\",\"internet\"");
	Sim900.println("AT+CSTT=\"internet.gprs.unifon.com.ar\",\"internet\",\"internet\"");
	delay(2000);
  Serial.println("AT+SAPBR=1,1");
	Sim900.println("AT+SAPBR=1,1");
  Serial.println(" Listo.");
}

void logServerTest(){ // Prueba de enviar datos fijos	
	Sim900.println("AT+HTTPPARA=\"URL\",\"http://ascasubi.inta.gob.ar/redae/insertar.php?ID=Mati&B0=0001&T1=0001&T2=0001&H1=0001&H2=0001>\"");
  delay(500);
	Sim900.println("AT+HTTPACTION=2");
}

void httpInit(){ // Inicializar comunicacion modo http
	Sim900.println("AT+HTTPINIT");
	delay(1000);
	Sim900.println("AT+HTTPPARA=\"CID\",1");
}

void serialEvent(){ // Interrupcion de puerto serie
	switch((char) Serial.read()){  
	  case 'a': // Enviar comando al sim900
    {
	  	String arg = Serial.readStringUntil('\n'); // Comando
			Serial.println(arg);
			Sim900.println(arg);
			delay(1000);
			sim900BufferDump();
	  	break;
    }
		case 'b':
		{
			Serial.readStringUntil('\n');
			sim900BufferDump();
			break;
		}
    case 'c': // Configuracion inicial
    {
			Rs485.end();
			Sim900.begin(9600);
			Serial.readStringUntil('\n');
      sim900Config();
			Sim900.end();
			Rs485.begin(9600);
			Rs485.listen();
      break;
    }
		case 'd': // httpinit
		{
			Serial.readStringUntil('\n');
			Rs485.end();
			Sim900.begin(9600);
			httpInit();
			Sim900.end();
			Rs485.begin(9600);
			Rs485.listen();
			break;
		}
	  case 'e': // loggeo prueba
		{
			Serial.readStringUntil('\n');
			Rs485.end();
			Sim900.begin(9600);
			logServerTest();	
			Sim900.end();
			Rs485.begin(9600);
			Rs485.listen();		
			break;
    }
		case 'g': // Comandos de loggeo con datos
		{
			Rs485.end();
			Sim900.begin(9600);
			Serial.readStringUntil('\n');
			// Numeros aleatorios
			int B = millis() % 50;
			delay(2);
			int T1 = millis() % 50;
			delay(2);
			int T2 = millis() % 50;
			delay(2);
			int H1 = millis() % 100;
			delay(2);
			int H2 = millis() % 100;
			delay(2);
			sprintf(localVariables, DATA_MASK, B, T1, T2, H1, H2);
			Sim900.print("AT+HTTPPARA=\"URL\",\"http://ascasubi.inta.gob.ar/redae/insertar.php?");
			Sim900.print(localVariables);
			Sim900.println("\"");
			delay(500);
			Sim900.println("AT+HTTPACTION=2");
			delay(500);
			Sim900.end();
			Rs485.begin(9600);
			Rs485.listen();
			break;
		}
		case 'h': // Encender/apagar sim900
		{
			Serial.readStringUntil('\n');
			digitalWrite(SIM900_ENABLE,LOW);
			delay(500);
			digitalWrite(SIM900_ENABLE,HIGH);
			delay(500);
			digitalWrite(SIM900_ENABLE,LOW);
			break;
		}
	  default: 
			Serial.println("Comando desconocido");
	  	break;
	}
}


void loop() {
  
}
