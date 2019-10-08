# Sistema de monitoreo remoto para proceso de compostaje

## Resumen
El monitoreo remoto de la temperatura y humedad de una pila de compost permite optimizar el tiempo en las prácticas de remoción, aporta información sobre la calidad del proceso y evita el hecho de estar pendiente de la adquisición manual de dichas variables.  

El sistema debe efectuar la medición de temperatura y humedad de la pila de compost al menos tomando una muestra diaria y en puntos separados cada 10 metros y registrar dichos valores en un medio accesible de manera remota en zonas donde únicamente se garantiza cobertura de telefonía móvil.  


## Esquema general

Para la comunicación inalámbrica se utilizará un módulo gprs que transmita los datos de adquisición de una serie de nodos conectados con enlace cableado. El arreglo de nodos consiste en un dispositivo central que opera como maestro y se encarga del reporte de datos al servidor y una serie de nodos esclavos que se conectan por medio de un bus que permite transmitir información entre los nodos mediante protocolo RS485. El protocolo RS485 admite una extensión cableada de hasta 30 metros dependiendo de la calidad de la línea y una tasa de transferencia de datos que puede llegar a los 10Mbps.  

El requerimiento de cablear los distintos nodos implica una reducción significativa en términos de costo de implementación debido a que puede compartirse una misma fuente de energía y una sóla línea telefónica entre varios registradores. Cada nodo posee una fuente conmutada que adapta y regula el voltaje de la línea de alimentación y una interfase de comunicación para conectarse al bus de datos. La conexión a los sensores se realiza de la misma manera en todos los nodos. Esto se detalla en las siguientes secciones.  


## Hardware

El hardware está basado en los módulos Arduino UNO y el shield GSM/GPRS Sim900. Esta plataforma permite un prototipado rápido y una implementación veloz del proyecto lo cual resulta apropiado para la producción de pocas unidades. El escalado del proyecto para una producción mayor se puede realizar en plazos relativamente cortos dada la disponibilidad de documentación que aporta el proyecto Arduino. Para la adaptación y acondicionamiento de señales se diseñó un “shield” o “hat” para Arduino UNO y será el mismo tantos para nodos maestros como esclavos. Se detallan los principales componentes a continuación.  


### Fuente de alimentación

Tanto el módulo microcontrolador Arduino UNO como el shield GSM/GPRS operan con 5V de corriente continua. Como el sistema debe operar a la intemperie lejos de la red de energía eléctrica y las baterías más apropiadas y accesibles del mercado para esta aplicación son de 12V, se requiere de una fuente de voltaje regulado. La opción más económica es el integrado uA7805 pero tiene la desventaja de disipar mucha potencia aún con poco consumo lo cual resulta inconveniente para un dispositivo que debe operar con fuente de energía limitada. La fuente seleccionada es el módulo basado en el integrado LM2576 que posee salida regulada variable de 1.5 a 35Vcc y 3A máximo.


### Medición de temperatura

Se utilizaron sensores LM35 ya que existe buena disponibilidad y poseen una buena relación entre costo y precisión. Estos sensores son de muy bajo consumo y poseen una curva de relación voltaje/temperatura constante de 10mV/°C lo cual evita tener que usar tablas o fórmulas complejas y un rango de trabajo de -55 a 150°C, que supera lo establecido por los requerimientos. Los sensores no requieren de fuente de alimentación regulada y soportan un máximo de 35V por lo que se conectan directamente a la batería.   

Para el acondicionamiento de la señal, que puede captar ruido electromagnético del ambiente, se agrega un capacitor electrolítico de 1uF en paralelo con un capacitor cerámico de 100pF.  


### Medición de humedad

La humedad se mide indirectamente mediante conductividad eléctrica del sustrato. Como los electrodos son propensos al desgaste por electrólisis, se utiliza un circuito de energización para alimentar los sensores sólo durante la adquisición de los datos y el resto del tiempo permanecen apagados.  

Por otro lado, y para estabilizar la señal analógica, se utiliza un filtro capacitivo y un potenciómetro tipo trimmer que debe ajustarse dependiendo de los valores de resistividad del sustrato.  

### Medición de voltaje de alimentación

El monitoreo del voltaje de alimentación es importante para conocer el estado de las baterías, verificar la efectividad de los paneles solares y evitar que el sistema se detenga por falta de energía. Para esto se emplea un divisor resistivo con resistencias de 8k2Ω y 2k7Ω para reducir el voltaje de alimentación, que puede rondar de los 10 a 25 volts, un 25% aproximadamente, de manera que sea compatible con el rango de operación del conversor A/D del microcontrolador.  

### Medición de condiciones ambientales

Mediante un sensor de temperatura y humedad DHT22 se puede conocer los valores de estas variables para el ambiente donde se encuentra instalado el registrador o bien si de deja el sensor en el interior del gabinete permite monitorear las condiciones de trabajo de la electrónica para extender la vida útil de los componentes. El sensor DHT22 requiere alimentación simple de 5V o de 3.3V, posee una interfase OneWire, con lo cual ocupa un sólo pin digital y emplea una resistencia de pull-up en la línea de comunicación.  

### Comunicación con servidor

El registro de datos al servidor se realiza por medio de un módulo basado en el chip Sim900 que permite, entre muchas otras funciones, conectarse a la red por GPRS. El módulo posee una interfase de comunicación serie (RS232) que funciona mediante comandos AT. Además dispone un array de siete pines que pueden ser configurados como Tx o Rx indistintamente y emplea los pines número 8 y 9 de Arduino UNO para reinicio y encendido/apagado respectivamente.  

### Comunicación inter nodos

Para la transmisión de datos de medición entre los diferentes nodos que no disponen de acceso a la red, se utiliza una conexión cableada que opera con protocolo RS485. Se utilizó el transceiver SN7517 que permite implementar el bus de comunicaciones utilizando mínima circuitería adicional. El bus requiere terminaciones con impedancias de 120Ω por lo que se agrega un jumper para desconectar las mismas en los módulos que sean intermedios.  


## Firmware

El programa del microcontrolador es relativamente sencillo debido a que no hay restricciones temporales fuertes y la adquisición de los datos se realiza mediante lecturas del conversor analógico digital. La mayor complejidad radica en la correcta configuración del módulo GPRS y el procesamiento de las respuestas del servidor. Los siguientes ítems detallan los procedimientos principales.  

### Inicialización

Algunos de los parámetros de configucación del datalogger se establecen como directivas de precompilador, como por ejemplo la asignación de pines, la velocidad de los puertos serie, el modo de debuggeo, el período de muestreo y de adquisición de datos, el grado de filtros promediadores, la máscara para el envío de parámetros por método post y el identificador del nodo, entre otros.  

El puerto serie nativo del microcontrolador se utiliza para el debuggeo del sistema cuando se encuentra conectado a una pc por puerto USB. Se emplean otros dos puertos serie emulados por software mediante la librería SoftwareSerial para la comunicación con el módulo GPRS y con los demás nodos por medio del bus RS485.  

En la primera fase de ejecución del firmware se inicializan los puertos digitales y analógicos y se cargan los parámetros que son configurados por código. A continuación se intenta detectar la disponibilidad del módulo GPRS para determinar si el microcontrolador va a operar como maestro o como esclavo. La diferencia es que si opera como maestro debe escuchar constantemente los mensajes que lleguen por bus RS485 y reportar al servidor estas mediciones como también las realizadas localmente.  

Como la librería SoftwareSerial admite el funcionamiento de un puerto serie por vez, el puerto de comunicación con el bus RS485 permanece conectado y se hace la conmutación sólo durante la transmisión de datos al servidor por medio del módulo GPRS.  

### Adquisición de datos

El conversor A/D del microcontrolador dispone de una referencia de voltaje por defecto de 5V, pero debido a que el fondo de escala del conversor supera ampliamente el máximo voltaje que entregan los sensores de temperatura LM35 (5V corresponde a una temperatura de 500°C), se conmuta a la referencia de voltaje interna que dispone el microcontrolador que es de 1.1V. De esta manera se amplía la resolución del conversor y es posible realizar una medición de temperatura mucho más precisa.   

Para la medición de conductividad se utiliza la referencia de voltaje por defecto que es de 5V, ya que a mayor voltaje, la medición contiene menos relación entre señal y ruido eléctrico.  

El inconveniente de conmutar el valor de referencia de voltaje radica en que la entrada de referencia del conversor no posee una descarga eléctrica y por lo tanto la conmutación lleva algo de tiempo y es necesario aplicar sucesivas lecturas analógicas hasta que la referencia se estabilice y la lectura sea correcta.  

Tanto para la adquisición de los valores de temperatura como de humedad se aplica un filtro promediador de grado 3 que puede configurarse por uno de mayor grado en caso de ser necesario. La activación de los sensores de humedad se realiza con 250 milisegundos de anticipación a la lectura para evitar cualquier tipo de transitorio de señal. Los valores de temperatura y humedad ambiente se adquieren del sensor DHT22 por medio de la librería que provee Adafruit y está disponible libremente en la web. Una vez finalizada la adquisición de los datos de todas las variables, se compila toda la información en un string mediante la función “sprintf()” donde se indica el formato mediante una máscara que debe ser compatible con la que espera el servidor. Este string permanece con el mismo valor hasta la próxima lectura de sensores.

### Comunicación con server

El reporte de los datos se realiza enviando un identificador de cuatro caracteres del nodo datalogger y una seguidilla de argumentos que contienen los valores de las variables de temperatura, humedad y nivel de carga de la batería y se realiza agregando un ​ query string al enlace de la página en la cual se aloja el sistema de registro de datos. Por lo tanto, luego de la adquisición de los datos y compilación de este ​ query string , ​ se intenta conectar a la página enviando ese argumento.  

La inicialización del Sim900 consiste en la ejecución de los siguientes comandos (puede cambiar según la operadora del chip utilizado) e intercalando demoras luego de cada comando para permitir la respuesta de la red:  

```
AT+SAPBR=3,1,"Contype","GPRS"  
AT+CSTT="internet.gprs.unifon.com.ar","internet","internet”  
AT+SAPBR=1,1  
AT+HTTPINIT  
AT+HTTPPARA="CID",1  
AT+SAPBR=2,1  
```

Si el módulo responde que se le ha asignado una dirección ip válida, entonces ya es posible realizar la publicación de las variables medidas. Como el proceso de inicialización lleva tiempo y en general es difícil de que la conexión se efectúe correctamente, se realiza por única vez al encender el datalogger maestro y permanece constantemente conectado hasta que caduque la sesión, momento en el cual es necesario repetir las instrucciones de incio.

Para el envío de datos se ejecutan los siguientes comandos:  

```
AT+HTTPPARA="URL","http://www...”
AT+HTTPACTION=2
```
Donde deben reemplazarse los puntos por la dirección del servidor y agregarse la máscara con las variables como ​ query string.  

### Comunicación entre nodos

Cuando el nodo no detecta el módulo GPRS pasa a funcionar en modo esclavo, con lo cual debe reportar las variables medidas por medio del puerto RS485 en lugar de hacerlo por internet. El proceso es sencillo del lado del transmisor ya que solamente se envía la máscara con los datos al bus pero para el procesamiento del string completo recibido por el maestro es más difícil de programar ya que hace falta un control para la lectura del buffer del puerto. La trama se finaliza con un caracter especial y se utiliza una función que se ejecuta no bien se detecta la presencia de datos en buffer y carga toda la trama en una variable global tipo string que se envía inmediatamente al servidor.  

```cpp
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
```

### Comandos de debuggeo

El puerto serie nativo del microcontrolador se utiliza para comunicación con PC mediante puerto serie emulado por USB que implementa el módulo Arduino UNO. Para el debuggeo se utiliza un protocolo sencillo de comandos identificados por caracteres aprovechando la interrupción de eventos de puerto serie que dispone el microcontrolador. La siguiente tabla enuncia los comandos implementados:  

'a': Realiza la adquisición de datos y contesta los valores medidos separados por coma.  
'b': Envía los datos de la última adquisición programada sin actualizar.  
'c': Realiza la adquisición de datos y los envía al servidor.  
'd': Realiza la adquisición de datos y los envía por bus RS485.  
'e': Ejecutar comandos de inicialización del SIM900.  
'f'​: Chequea el estado de la conectividad (​ query bearer ).  
'g'​+arg: Envía el comando arg al módulo GPRS y contesta con la respuesta recibida.  
'h'​: Conmuta encendido/apagado del módulo GPRS.  
'i'​: Realiza una transmisión de datos de prueba al servidor.  


## Implementación

Se diseñó un pcb tipo shield que sirve tanto para conectar directamente a la placa Arduino UNO como también para conectar al módulo GPRS que a la vez se conecta al Arduino UNO con lo cual tanto el firmware como hardware es el mismo independientemente de si el nodo operará como maestro o como esclavo, logrando de esta manera un diseño muy flexible y modular.

### PCB

El diseño del PCB se realizó con un programa CAD intentando lograr una plaqueta de tamaño reducido que no supere las dimensiones de los módulos utilizados. El shield implementado tiene unas dimensiones de 65x70 mm, lo cual cumple perfectamente con el requisito impuesto.

### Gabinete

Todos los componentes se ubican en el interior de una caja estanca de PVC con sellado impermeable. En la base se coloca un interruptor de encendido, conectores para las sondas de conductividad y bus de datos y en el exterior se puede atornillar un panel solar para la recarga de la batería.  

El gabinete que contiene batería y panel solar debe ser de mayores dimensiones para alojar el espacio suficiente y no necesariamente debe contener el nodo maestro, que de hecho al tener un módulo de mayores dimensiones y una antena puede resultar conveniente ubicar en otra caja separada sin la batería de manera de optimizar espacio.  

La base de la caja estanca posee una perforación para atravesar un caño metálico que se emplea como estaca para los dos sensores de temperatura y sirve tanto de soporte para la caja y panel solar en caso de estar presente este último.  

El sensor de temperatura y humedad ambiente es apto para entemperie con lo cual puede instalarse en el exterior de la caja o bien en su interior, soldado al shield para monitorear las condiciones de trabajo de los componentes electrónicos.