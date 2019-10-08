// Formato de trama de este dispositivo (cambiar ID para cada uno)
#define DATA_MASK "ID=Mati&B0=%04d&T1=%04d&T2=%04d&H1=%04d&H2=%04d>" 
// Trama de caracteres para enviar datos medidos
#define DATA_LENGTH 48 	// Longitud de las tramas de transmision (485 y sim900)
#define START_CHAR 'I' 	// Caracter indicador de inicio de la trama (La I de ID)
#define END_CHAR '>' 		// Caracter indicador de fin de la trama

#define SERVER_HTTP_CMD "AT+HTTPPARA=\"URL\",\"http://........................." // Direccion del server

// Entradas analogicas
#define BAT_PIN A0 		// Pin de voltaje de bateria
#define ZH_PIN_1 A1 	// Pin de entrada para lectura del sensor de humedad 1
#define ZH_PIN_2 A2 	// Pin de entrada para lectura del sensor de humedad 2 (opcional)
#define LM35_PIN_1 A3 // Pin de entrada para lectura del sensor de temperatura 1
#define LM35_PIN_2 A4 // Pin de entrada para lectura del sensor de temperatura 2

// Salidas digitales
#define PWR_ZH 7 			// Pin de encendido de sensores de humedad de suelo

// Puerto serie de bus 485
#define RS485_ENABLE 3 			// Habilitacion del puerto 485
#define RS485_RX 2
#define RS485_TX 4
#define RS485_TIMEOUT 1000 	// Maximo tiempo de espera a completar transmision

// Puerto serie de Sim900
#define SIM900_ENABLE 9 		// Habilitacion del modulo GSM-GPRS
#define SIM900_RX 5
#define SIM900_TX 6

#define DHT22_PIN 11 				// Pin para sensor de humedad y temperatura interno
 
// Disponibles: (8 reinicia sim900, 9 encendido sim900), 10, 12, 13, A5

// Parametros de medicion
#define N_FILTER 3 						// Grado de filtro promediador
#define SAMPLE_TIMEOUT 300000 	// Periodo de muestreo 
/* 	 	 ms -- minutos
	 300000 -- 5
	 600000 -- 10
	 900000 -- 15
	1800000 -- 30
	3600000 -- 60
*/

#define DEBUG true // Modo debuggeo por serial USB (no poner en false, comentar o descomentar esta linea)

// Librerias
#include <SoftwareSerial.h>
//#include "DHT.h"


unsigned long lastSampleTime = 0; // Instante de la ultima muestra 
// Buffers (deben inicializarse para que ande el micro)
char rs485RcvdChars[DATA_LENGTH] =  "000000000000000000000000000000000000000000000000"; 
char localVariables[DATA_LENGTH] =  "000000000000000000000000000000000000000000000000";
boolean isMaster; 					// Indicador de precencia del sim900
boolean new485Data = false; // Indicador de buffer actualizado

SoftwareSerial Rs485(RS485_RX, RS485_TX); 		// RX, TX
SoftwareSerial Sim900(SIM900_RX, SIM900_TX); 	// RX, TX

// Prototipos
void setup();									// Inicializacion del datalogger
void loop();									// Temporizado - escucha de puerto serie bus 485
void acquire();								// Leer pines analogicos
void sim900Init();						// Inicializar modulo GSM
void logToServer(bool local); // Mandar datos al server (locales o recibidas)
void logToServerTest();				// Hacer pueba de logeo de datos
void logTo485();		// Mandar datos por bus 485
void read485Buffer();					// Leer datos que llegan del bus 485
void serialEvent();						// Callback de llegada de datos al puerto serie nativo


void setup(){
  Serial.begin(9600);
  #ifdef DEBUG
    Serial.println("Iniciando datalogger... ");
  #endif

	// Entradas. Creo que no hace falta para analogicos, pero por las dudas
	pinMode(BAT_PIN, INPUT);
	pinMode(ZH_PIN_1, INPUT);
	pinMode(ZH_PIN_2, INPUT);
	pinMode(LM35_PIN_1, INPUT);
	pinMode(LM35_PIN_2, INPUT);
	
	// Salidas
	pinMode(PWR_ZH, OUTPUT);
	pinMode(RS485_ENABLE, OUTPUT);
	pinMode(SIM900_ENABLE, OUTPUT);
  #ifdef DEBUG
   Serial.println("Pines digitales configurados.");
  #endif

  digitalWrite(SIM900_ENABLE,LOW);
  delay(100);
  digitalWrite(SIM900_ENABLE,HIGH);
  delay(500);
  digitalWrite(SIM900_ENABLE,LOW);

	// Checkear sim900
  #ifdef DEBUG
    Serial.print("Detectando SIM900... ");
  #endif

	isMaster = false;
  Sim900.begin(9600); // Inicializar puerto serie del sim900
	delay(500);
	Sim900.println("AT");
	delay(200);
	if(Sim900.available()){ // Si hay caracteres en buffer significa que el sim900 esta conectado
		while(Sim900.available())
			Sim900.read(); // Vaciar buffer del softwareserial
    #ifdef DEBUG
      Serial.println(" Sim900 detectado. Modo maestro.");
    #endif
		isMaster = true;
		delay(5000);
		sim900Init(); // Comandos de inicializacion del sim900
	}else{
    #ifdef DEBUG
      Serial.println(" Sim900 no detectado. Modo esclavo.");
    #endif
		isMaster = false; // No hace falta repetir, pero para que no quede el else vacio
		delay(120000);
	}
	Sim900.end(); // Hay que cortar la comunicacion para iniciar el otro SoftwareSerial

	// Iniciar el SoftwareSerial para el bus 485 (tiene que estar siempre encendido)
	#ifdef DEBUG
    Serial.print("Iniciando bus 485... ");
  #endif
	Rs485.begin(9600);
	Rs485.listen();
	#ifdef DEBUG
    Serial.println("Listo.");
  #endif

  lastSampleTime = millis(); // Inicializar timer

	#ifdef DEBUG
		Serial.println("Datalogger listo.");
	#endif
}

void acquire(){ // Realizar lectura de sensores y guarda en variables locales
	#ifdef DEBUG
    Serial.print("Adquiriendo datos...");
  #endif

	unsigned int B, T1, T2, H1, H2;  // Variables para sensores conectados

	// Configurar referencia de 1.1V para sensores LM35
	analogReference(INTERNAL); // Referencia de 1.1V  
	// Lecturas para estabilizacion de voltaje
	analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
	analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
	analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
	analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
	analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);
	analogRead(A0);analogRead(A1);analogRead(A2);analogRead(A3);analogRead(A4);analogRead(A5);

  
	// Realizar lectura de T1 y T2 (promedio N)
	T1 = 0;
	for(short k = 0; k < N_FILTER; k++){
		T1 += analogRead(LM35_PIN_1); 
		delay(100);
	}
	T1 /= N_FILTER;

	T2 = 0;
	for(short k = 0; k < N_FILTER; k++){
		T2 += analogRead(LM35_PIN_2); 
		delay(100);
	}
	T2 /= N_FILTER;

	// Para pasar de entero al valor de temperatura hay que multiplicar T1 y T2 por 110/1023
	
	analogReference(DEFAULT); // Volver a configurar la referencia de 5V (no requiere espera)
	analogRead(A0); // Primera lectura puede ser erronea


	digitalWrite(PWR_ZH,HIGH); // Encender sensores de humedad
	delay(250); // Demora de estabilizacion de senial
	
	// Realizar lectura de H1 y H2 (promedio N)
	H1 = 0;
	for(short k = 0; k < N_FILTER; k++){
		H1 += analogRead(ZH_PIN_1); 
		delay(100);
	}
	H1 /= N_FILTER;

	H2 = 0;
	for(short k = 0; k < N_FILTER; k++){
		H2 += analogRead(ZH_PIN_2); 
		delay(100);
	}
	H2 /= N_FILTER;

	digitalWrite(PWR_ZH,LOW); // Apagar sensores de humedad

	B = analogRead(BAT_PIN); // Medir voltaje de bateria

  #ifdef DEBUG
    Serial.print("Datos (B,T1,T2,H1,H2: ");
    Serial.print(B);
    Serial.print(" ");
    Serial.print(T1);
    Serial.print(" ");
    Serial.print(T2);
    Serial.print(" ");
    Serial.print(H1);
    Serial.print(" ");
    Serial.println(H2);    
  #endif
  

	// Guardar datos en buffer de 48 caracteres
	sprintf(localVariables, DATA_MASK, B, T1, T2, H1, H2);

  #ifdef DEBUG
    Serial.println(" Listo.");
  #endif
}

void logTo485(){ // Enviar datos a los demas nodos por bus 485
  #ifdef DEBUG
    Serial.print("Enviando datos por bus 485: ");
		Serial.println(localVariables);
  #endif
	digitalWrite(RS485_ENABLE,HIGH); // Habilitar canal de transmision
	Rs485.print(localVariables);
	digitalWrite(RS485_ENABLE,LOW); // Deshabilitar canal de transimision para quedar en escucha
  #ifdef DEBUG
    Serial.println("Fin comunicacion por bus 485.");
  #endif
}


void sim900Init(){ // Comandos de configuracion del sim900 como modulo gsm
	Sim900.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
	delay(2000);
	Sim900.println("AT+CSTT=\"internet.gprs.unifon.com.ar\",\"internet\",\"internet\"");
	delay(2000);
	Sim900.println("AT+SAPBR=1,1");
	delay(15000);
	#ifdef DEBUG
		while(Sim900.available())
			Serial.write(Sim900.read());
  #endif
	Sim900.println("AT+HTTPINIT");
	delay(1000);
	Sim900.println("AT+HTTPPARA=\"CID\",1");
	delay(1000);
	Sim900.println("AT+SAPBR=2,1"); // Query bearer (debe responder IP)
 	#ifdef DEBUG
		while(Sim900.available())
			Serial.write(Sim900.read());
  #endif
}

void logToServer(bool local){ // Reportar datos en server
  #ifdef DEBUG
    Serial.print("Enviando datos al server: ");
		if(local) Serial.println(localVariables); // Mostrar lo que se envia
		else Serial.println(rs485RcvdChars);
  #endif
	
	Rs485.end(); // Detener la escucha del bus para escribir al sim900
	Sim900.begin(9600);
	delay(200);
	
	//sprintf(localVariables, DATA_MASK, B, T1, T2, H1, H2);
	Sim900.print("AT+HTTPPARA=\"URL\",\"http://ascasubi.inta.gob.ar/redae/insertar.php?");
	if(local) Sim900.print(localVariables);
	else Sim900.print(rs485RcvdChars);
	Sim900.println("\"");
	delay(500);
	Sim900.println("AT+HTTPACTION=2");
	delay(750);
  Sim900.println("AT+HTTPACTION=2");
  delay(750);
  Sim900.println("AT+HTTPACTION=2");
  delay(750);
 	#ifdef DEBUG
    Serial.println("Fin comunicacion con server.");
		while(Sim900.available())
			Serial.write(Sim900.read());
  #endif
	Sim900.end(); // Detener la comunicacion con el sim900 para escuchar el bus485
	Rs485.begin(9600);
	Rs485.listen();
}

void logToServerTest(){ // Prueba de enviar datos fijos	
	Rs485.end(); // Detener la escucha del bus para escribir al sim900
	Sim900.begin(9600);
	delay(200);
	Sim900.println("AT+HTTPPARA=\"URL\",\"http://ascasubi.inta.gob.ar/redae/insertar.php?ID=Mati&B0=0001&T1=0001&T2=0001&H1=0001&H2=0001>\"");
  delay(500);
	Sim900.println("AT+HTTPACTION=2");
	delay(1000);
	#ifdef DEBUG
    while(Sim900.available())
			Serial.write(Sim900.read());
  #endif
	Sim900.end();
	Rs485.begin(9600);
	Rs485.listen();
}

void serialEvent(){ // Interrupcion de puerto serie
	switch((char) Serial.read()){  
	  case 'a': // Medir y responder datos
    {
	  	Serial.readStringUntil('\n'); // Limpiar buffer
	  	acquire(); // Medir
			Serial.println(localVariables);
	  	break;
    }
    case 'b': // Enviar datos de la ultima medicion (sin actualizar)
    {
      Serial.readStringUntil('\n'); // Limpiar buffer
      Serial.println(localVariables);
      break;
    }
	  case 'c': // Medir y transmitir datos al server
		{
			Serial.readStringUntil('\n'); // Limpiar buffer
			acquire(); // Medir
			logToServer(true); // Transmitir
      break;
    }
		case 'd': // Medir y transmitir datos al bus 485
		{
			Serial.readStringUntil('\n'); // Limpiar buffer
			acquire(); // Medir
			logTo485();
      break;
    }
		case 'e': // Comandos de inicializacion del sim900
		{
			Serial.readStringUntil('\n'); // Limpiar buffer
			Rs485.end();
			Sim900.begin(9600);
			delay(200);
			sim900Init();
			delay(1000);
			Sim900.end();
			Rs485.begin(9600);
			Rs485.listen();
      break;
		}
		case 'f': // Query bearer - Checkear configuracion del server
		{
			Serial.readStringUntil('\n'); // Limpiar buffer
			Rs485.end();
			Sim900.begin(9600);
			Sim900.listen();
			delay(200);
			Sim900.println("AT+SAPBR=2,1");
			delay(500);
			while(Sim900.available())
				Serial.write(Sim900.read());
			Sim900.end();
			Rs485.begin(9600);
			Rs485.listen();
      break;
		}
		case 'g': // Enviar comando al sim900
    {
	  	String arg = Serial.readStringUntil('\n'); // Comando
			Rs485.end();
			Sim900.begin(9600);
			delay(200);
			Sim900.println(arg);
			delay(1000);
		 	while(Sim900.available())
				Serial.write(Sim900.read());
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
		case 'i':
		{
			Serial.readStringUntil('\n'); // Limpiar buffer
			logToServerTest(); // Transmitir
			break;
		}
	  default:
		{
		  Serial.readStringUntil('\n'); // Limpiar buffer 
      #ifdef DEBUG
	  	  Serial.println("Comando desconocido");
      #endif
	  	break;
		}
	}
}

void read485Buffer() { // Leer todo el buffer del puerto 485 hasta timeout o hasta caracter de fin de trama
	if(Rs485.available() && !new485Data){ // Si hay caracteres en buffer y la trama anterior fue procesada
		// Inicializar variables
		boolean recvInProgress = false; // Indicador de estado de recepcion de trama
		byte ndx = 0; // Indice de caracter recibido		
		char rc; // Caracter leido
		unsigned long rcvTimer = millis(); // Iniciar timer
		while (millis() - rcvTimer < RS485_TIMEOUT && !new485Data) { // Repetir hasta completar la lectura de la trama o que ocurra timeout
			rc = Rs485.read(); // Leer un caracter del buffer
			if(rc != -1){ // Si hay un caracter
				if(recvInProgress) { // Si ya esta recibiendo caracteres
					if(rc != END_CHAR) { // Si el caracter recibido no es el indicador de fin de trama
						rs485RcvdChars[ndx] = rc; // Guardar nuevo caracter
						ndx++; // Incrementar indice del arreglo de datos
						if (ndx >= DATA_LENGTH) { // Overflow
								#ifdef DEBUG
									Serial.println("Overflow de buffer de datos");
								#endif
								ndx = 0; // Si sigue recibiendo caracteres, empezar por el principio otra vez
						}
					} else { // Si el caracter recibido es el indicador de fin de trama                
							rs485RcvdChars[ndx] = rc; // Guardar el caracter de fin de trama tambien
							recvInProgress = false; // Recepcion de datos terminada
							new485Data = true;
							#ifdef DEBUG
								Serial.print("Datos recibidos por bus 485: ");
								Serial.println(rs485RcvdChars);
							#endif
					}
				} else if (rc == START_CHAR) { // Caracter indicador de inicio de trama
						rs485RcvdChars[ndx] = rc; // Guardar el caracter de inicio de trama
						ndx++; // Incrementar contador
						recvInProgress = true;
						rcvTimer = millis();
				}
			}
		}
	}
}


void loop(){
	if(millis() < lastSampleTime) // Overflow de millis (cada 49 días)
		lastSampleTime = millis();
	
	if(millis() - lastSampleTime > SAMPLE_TIMEOUT){ // Muestreo periodico

		acquire(); // Actualizar las medidas de las variables locales		

		if(isMaster)
			logToServer(true); // Enviar datos al server (Mientras transmite no puede leer el 485)
		else
			logTo485(); // Enviar datos a los otros nodos

		lastSampleTime = millis();
	}

	if(isMaster){ // Si es maestro, revisar buffer del 485 y enviar al server si hay datos
		read485Buffer(); // Esta funcion retorna luego de terminar la lectura de todos los caracteres
		if(new485Data){ // Al completar recepcion de datos, transmitir y borrar buffer
			logToServer(false);
			new485Data = false;
		}
	}
}

