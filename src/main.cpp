#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"

//
// ===== MQTT =====
//

const char* mqtt_server = MQTT_SERVER; 
const char* mqtt_topic_botoes = "satc/botoes";

WiFiClient espClient;
PubSubClient client(espClient);

//
// ===== LEDS =====
//

#define LED_Verde 3
#define LED_Vermelho 14

//
// ===== MATRIZ DE BOTÕES 2x5 =====
//

const unsigned long DEBOUNCE_MS = 50;

const uint8_t NUM_LINHAS  = 2;
const uint8_t NUM_COLUNAS = 5;

const uint8_t PINOS_LINHAS[NUM_LINHAS]   = {4, 15};
const uint8_t PINOS_COLUNAS[NUM_COLUNAS] = {23, 22, 21, 19, 18};

// Cor de cada botão: CORES[linha][coluna]. Ajuste conforme a sua montagem.
const char* CORES[NUM_LINHAS][NUM_COLUNAS] = {
  {"vermelho1", "verde1",  "azul1",  "amarelo1",  "preto1"},
  {"vermelho2", "verde2",  "azul2",  "amarelo2",  "preto2"},
};

struct EstadoTecla {
  int ultimoLido;
  int estavel;
  unsigned long ultimoTempo;
};

EstadoTecla teclas[NUM_LINHAS][NUM_COLUNAS];   // começa tudo zerado (LOW)

void enviarBotao(const char* cor) {
  if (client.publish(mqtt_topic_botoes, cor)) {
    Serial.print("Botão pressionado, enviado: ");
    Serial.println(cor);
  } else {
    Serial.println("Falha ao publicar no MQTT");
  }
}

void verificarMatriz() {
  for (uint8_t l = 0; l < NUM_LINHAS; l++) {

    digitalWrite(PINOS_LINHAS[l], HIGH);
    delayMicroseconds(10);                       // tempo para o sinal estabilizar

    for (uint8_t c = 0; c < NUM_COLUNAS; c++) {
      EstadoTecla &t = teclas[l][c];
      int leitura = digitalRead(PINOS_COLUNAS[c]);

      if (leitura != t.ultimoLido) {
        t.ultimoTempo = millis();
        t.ultimoLido = leitura;
      }

      if ((millis() - t.ultimoTempo) > DEBOUNCE_MS && leitura != t.estavel) {
        t.estavel = leitura;
        if (t.estavel == HIGH) {
          enviarBotao(CORES[l][c]);
        }
      }
    }

    digitalWrite(PINOS_LINHAS[l], LOW);
  }
}

//
// ======================================================
// FUNÇÕES
// ======================================================
//

void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Conectando em ");
  Serial.println(WIFI_SSID);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");

    if (client.connect("ESP32Client")) {
      Serial.println("conectado");
      digitalWrite(LED_Verde, HIGH);
      digitalWrite(LED_Vermelho, LOW);

    } else {
      digitalWrite(LED_Verde, LOW);
      digitalWrite(LED_Vermelho, HIGH);
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}




//
// ======================================================
// SETUP
// ======================================================
//

void setup() {
  Serial.begin(115200);

  // Pull-down já existe na placa, então INPUT simples
  for (uint8_t l = 0; l < NUM_LINHAS; l++) {
    pinMode(PINOS_LINHAS[l], OUTPUT);
    digitalWrite(PINOS_LINHAS[l], LOW);
  }
  for (uint8_t c = 0; c < NUM_COLUNAS; c++) {
    pinMode(PINOS_COLUNAS[c], INPUT);
  }

  pinMode(LED_Verde, OUTPUT);
  pinMode(LED_Vermelho, OUTPUT);
  digitalWrite(LED_Verde, LOW);
  digitalWrite(LED_Vermelho, HIGH);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
}

//
// ======================================================
// LOOP
// ======================================================
//

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  verificarMatriz();
}