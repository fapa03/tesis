

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>


//CONFIGURAMOS ACCESO A WIFI//
const char* ssid     = "MonacoFamily";
const char* password = "Monaco12345";

const char *mqtt_server = "tesisiot.ga";
const int mqtt_port = 1883;
const char *mqtt_user = "web_client";
const char *mqtt_pass = "121212";


//CONFIGURAMOS DHT SENSOR//
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);



//CONFIGURAMOS MQTT LIBRERIA//
WiFiClient espClient;
PubSubClient client(espClient);


//ASIGNAMOS VARIABLES
long lastMsg = 0;
char msg[25];
//int temp1 = 0;
//int temp2 = 1;
//int volts = 2;

//*****************************
//*** DECLARACION FUNCIONES ***
//*****************************
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();


    void setup() {
    	pinMode(BUILTIN_LED, OUTPUT);
    	Serial.begin(115200);
    	randomSeed(micros());
    	setup_wifi();
      dht.begin();
    	client.setServer(mqtt_server, mqtt_port);
    	client.setCallback(callback);
    }

void loop() {
	if (!client.connected()) {
		reconnect();
	}

	client.loop();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.computeHeatIndex(t, h, false);
    // Check if any reads failed and exit early (to try again).
   if (isnan(h) || isnan(t) || isnan(f)) {
     Serial.println(F("Failed to read from DHT sensor!"));
     return;
 }

	long now = millis();
	if (now - lastMsg > 500){
		lastMsg = now;


		String to_send = String(h) + "," + String(t) + "," + String(f);
		to_send.toCharArray(msg, 25);
		Serial.print("Publicamos mensaje -> ");
		Serial.println(msg);
		client.publish("values", msg);
	}
}



//*****************************
//***    CONEXION WIFI      ***
//*****************************
void setup_wifi(){
	delay(10);
	// Nos conectamos a nuestra red Wifi
	Serial.println();
	Serial.print("Conectando a ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("Conectado a red WiFi!");
	Serial.println("Dirección IP: ");
	Serial.println(WiFi.localIP());
}



void callback(char* topic, byte* payload, unsigned int length){
	String incoming = "";
	Serial.print("Mensaje recibido desde -> ");
	Serial.print(topic);
	Serial.println("");
	for (int i = 0; i < length; i++) {
		incoming += (char)payload[i];
	}
	incoming.trim();
	Serial.println("Mensaje -> " + incoming);

	if ( incoming == "on") {
		digitalWrite(BUILTIN_LED, HIGH);
	} else {
		digitalWrite(BUILTIN_LED, LOW);
	}
}

void reconnect() {

	while (!client.connected()) {
		Serial.print("Intentando conexión Mqtt...");
		// Creamos un cliente ID
		String clientId = "esp32_";
		clientId += String(random(0xffff), HEX);
		// Intentamos conectar
		if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
			Serial.println("Conectado!");
			// Nos suscribimos
			client.subscribe("led1");
			client.subscribe("led2");
		} else {
			Serial.print("falló :( con error -> ");
			Serial.print(client.state());
			Serial.println(" Intentamos de nuevo en 5 segundos");

			delay(5000);
		}
	}
}
