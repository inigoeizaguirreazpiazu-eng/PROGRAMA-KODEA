#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi
const char* ssid = "OLP";
const char* password = "oteitzaLP";

// ThingsBoard Demo
const char* mqtt_server = "demo.thingsboard.io";
const int mqtt_port = 1883;
const char* access_token = "2ZjHyrN6e3CWWu5R1oSG";  // <-- CAMBIAR

WiFiClient espClient;
PubSubClient client(espClient);

// Pines
const int flashButton = 0;       // GPIO0 (FLASH)
const int led = LED_BUILTIN;     // LED interno (activo en LOW)

// Estado anterior del botón (para no enviar datos repetidos)
bool lastButtonState = HIGH;

// WiFi conexión
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("WiFi konektatzen: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi konektatua!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// Reconexión MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("ThingsBoard konektatzen...");
    if (client.connect("ESP8266", access_token, NULL)) {
      Serial.println(" konektatua!");
    } else {
      Serial.print("Errorea rc=");
      Serial.print(client.state());
      Serial.println(" berriro 5 segundotan...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(flashButton, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);  // LED apagado (activo LOW)

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  bool buttonState = digitalRead(flashButton);

  // Detectar cambio de estado
  if (buttonState != lastButtonState) {

    if (buttonState == LOW) {
      // Botón pulsado
      digitalWrite(led, LOW);  // Encender LED

      client.publish("v1/devices/me/telemetry",
                     "{\"button\":true,\"led\":true}");

      Serial.println("Botón pulsado - LED ON");
    } else {
      // Botón liberado
      digitalWrite(led, HIGH); // Apagar LED

      client.publish("v1/devices/me/telemetry",
                     "{\"button\":false,\"led\":false}");

      Serial.println("Botón liberado - LED OFF");
    }

    lastButtonState = buttonState;
    delay(50);  // pequeño debounce
  }
}
