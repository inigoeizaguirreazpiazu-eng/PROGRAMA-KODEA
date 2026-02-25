#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ======== THINGSBOARD KONFIGURAZIOA ========
#define TOKEN "1267VXVAYkywiLEECJDA"   // <-- HEMEN zure Access Token-a
#define THINGSBOARD_SERVER "eu.thingsboard.cloud"
#define THINGSBOARD_PORT 1883

// ======== WIFI DATUAK ========
const char* ssid = "OLP";
const char* password = "oteitzaLP";

// ======== OBJEKTUAK ========
WiFiClient espClient;
PubSubClient client(espClient);

// ======== PIN DEFINIZIOAK ========
const int pinPotentziometroa = A0;  // Potentziometroa
const int ledGorria = D1;           // LED gorriak GPIO pin honetan

// ======== ALDAKORRAK ========
int balioGordina = 0;
int balioEskalatua = 0;

void setup() {
  Serial.begin(115200);

  pinMode(ledGorria, OUTPUT);
  digitalWrite(ledGorria, LOW);  // LED itzali hasieran

  // WiFi konektatzea
  WiFi.begin(ssid, password);
  Serial.print("WiFi konektatzen...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi konektatuta!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // ThingsBoard konfiguratu
  client.setServer(THINGSBOARD_SERVER, THINGSBOARD_PORT);
}

void reconnect() {
  // Berriro konektatu MQTT ThingsBoard-era
  while (!client.connected()) {
    Serial.print("ThingsBoard konektatzen...");
    if (client.connect("ESP8266_Client", TOKEN, NULL)) {
      Serial.println(" konektatuta!");
    } else {
      Serial.print("Errorea rc=");
      Serial.print(client.state());
      Serial.println(" berriro 2 segundutan...");
      delay(2000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 1️⃣ Potentziometroaren balioa irakurri (0-1023)
  balioGordina = analogRead(pinPotentziometroa);

  // 2️⃣ Eskalatu 0-100 tartera
  balioEskalatua = map(balioGordina, 0, 1023, 0, 100);
  balioEskalatua = constrain(balioEskalatua, 0, 100);

  // 3️⃣ LED gorri kontrola eta alarma
  bool ledState = false;   // LED egoera ThingsBoard-era bidaltzeko
  bool alarma = false;
  if(balioEskalatua > 80) {
    digitalWrite(ledGorria, HIGH);  // LED piztu
    alarma = true;
    ledState = true;
  } else {
    digitalWrite(ledGorria, LOW);   // LED itzali
  }

  // 4️⃣ JSON bidalketa ThingsBoard-era (potentziometroa + alarma + LED egoera)
  char payload[120];
  sprintf(payload, "{\"potentziometroa\":%d, \"alarma\":%s, \"led\":%s}", 
          balioEskalatua, 
          alarma ? "true" : "false",
          ledState ? "true" : "false");
  client.publish("v1/devices/me/telemetry", payload);

  // 5️⃣ Serial monitorrean bistaratzea
  Serial.println(payload);

  delay(1000); // 1 segundoko atzerapena
}
