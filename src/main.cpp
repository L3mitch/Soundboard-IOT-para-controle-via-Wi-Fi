#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <DHT.h>

//
// ===== WIFI =====
//

const char* ssid = "Aula_Satc";
const char* password = "123teste";

//
// ===== MQTT =====
//

const char* mqtt_server = "127.0.0.1"; // IP do Node-RED/MQTT Broker

WiFiClient espClient;
PubSubClient client(espClient);

//
// ===== HTTP SERVER =====
//

WebServer server(80);

//
// ===== DHT22 =====
//

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

//
// ===== LDR =====
//

#define LDR_PIN 34

//
// ===== HC-SR04 =====
//

#define TRIG_PIN 5
#define ECHO_PIN 18

//
// ===== LEDs =====
//

#define LED_AUTO 2
#define LED_MANUAL 15

//
// ===== Variáveis =====
//

unsigned long lastMsg = 0;

float temperatura = 0;
float umidade = 0;
int luminosidade = 0;
float distancia = 0;

//
// ======================================================
// FUNÇÕES
// ======================================================
//

void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Conectando em ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());
}

//
// ======================================================
// MQTT CALLBACK
// ======================================================
//

void callback(char* topic, byte* payload, unsigned int length) {

  String message;

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Mensagem recebida: ");
  Serial.println(message);

  //
  // Controle manual do LED
  //

  if (String(topic) == "casa/led/manual") {

    if (message == "ON") {
      digitalWrite(LED_MANUAL, HIGH);
    }

    else if (message == "OFF") {
      digitalWrite(LED_MANUAL, LOW);
    }
  }
}

//
// ======================================================
// RECONEXÃO MQTT
// ======================================================
//

void reconnect() {

  while (!client.connected()) {

    Serial.print("Tentando conexão MQTT...");

    if (client.connect("ESP32Client")) {

      Serial.println("conectado");

      //
      // Tópicos assinados
      //

      client.subscribe("casa/led/manual");

    } else {

      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");

      delay(5000);
    }
  }
}

//
// ======================================================
// LEITURA HC-SR04
// ======================================================
//

float lerDistancia() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);

  float distance = duration * 0.034 / 2;

  return distance;
}

//
// ======================================================
// HTTP ENDPOINT
// ======================================================
//

void handleRoot() {

  String json = "{";
  json += "\"temperatura\":" + String(temperatura) + ",";
  json += "\"umidade\":" + String(umidade) + ",";
  json += "\"luminosidade\":" + String(luminosidade) + ",";
  json += "\"distancia\":" + String(distancia);
  json += "}";

  server.send(200, "application/json", json);
}

//
// ======================================================
// SETUP
// ======================================================
//

void setup() {

  Serial.begin(115200);

  //
  // LEDs
  //

  pinMode(LED_AUTO, OUTPUT);
  pinMode(LED_MANUAL, OUTPUT);

  //
  // HC-SR04
  //

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  //
  // DHT
  //

  dht.begin();

  //
  // WIFI
  //

  setup_wifi();

  //
  // MQTT
  //

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //
  // HTTP
  //

  server.on("/", handleRoot);
  server.begin();

  Serial.println("Servidor HTTP iniciado");
}

//
// ======================================================
// LOOP
// ======================================================
//

void loop() {

  //
  // MQTT
  //

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  //
  // HTTP
  //

  server.handleClient();

  //
  // Publicação MQTT a cada 2 segundos
  //

  unsigned long now = millis();

  if (now - lastMsg > 2000) {

    lastMsg = now;

    //
    // Leituras
    //

    temperatura = dht.readTemperature();
    umidade = dht.readHumidity();

    luminosidade = analogRead(LDR_PIN);

    distancia = lerDistancia();

    //
    // Automação:
    // baixa luminosidade aciona LED
    //

    if (luminosidade < 1500) {
      digitalWrite(LED_AUTO, HIGH);
    } else {
      digitalWrite(LED_AUTO, LOW);
    }

    //
    // Publicações MQTT
    //

    client.publish(
      "casa/sensores/temperatura",
      String(temperatura).c_str()
    );

    client.publish(
      "casa/sensores/umidade",
      String(umidade).c_str()
    );

    client.publish(
      "casa/sensores/luminosidade",
      String(luminosidade).c_str()
    );

    client.publish(
      "casa/sensores/distancia",
      String(distancia).c_str()
    );

    //
    // Debug Serial
    //

    Serial.println("======== DADOS ========");

    Serial.print("Temperatura: ");
    Serial.println(temperatura);

    Serial.print("Umidade: ");
    Serial.println(umidade);

    Serial.print("Luminosidade: ");
    Serial.println(luminosidade);

    Serial.print("Distancia: ");
    Serial.println(distancia);

    Serial.println("=======================");
  }
}
