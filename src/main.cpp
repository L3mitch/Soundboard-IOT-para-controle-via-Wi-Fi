#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

//
// ===== WIFI =====
//

const char* ssid = "Aula_Satc";
const char* password = "123teste";

//
// ===== MQTT =====
//

const char* mqtt_server = "10.85.237.252"; // Troque pelo IP real do broker/Node-RED
const char* mqtt_topic_botoes = "satc/botoes";

WiFiClient espClient;
PubSubClient client(espClient);

//
// ===== BOTÕES =====
//

const unsigned long DEBOUNCE_MS = 50;

struct Botao {
  uint8_t pino;
  const char* cor;
  int ultimoEstadoLido;
  int estadoEstavel;
  unsigned long ultimoTempoDebounce;
};

// Para adicionar um novo botão, inclua uma linha aqui
Botao botoes[] = {
  {23,  "amarelo",  LOW, LOW, 0},
  {22,  "azul",     LOW, LOW, 0},
  {4,   "verde",    LOW, LOW, 0},
  {2,   "vermelho", LOW, LOW, 0},
};

const size_t NUM_BOTOES = sizeof(botoes) / sizeof(botoes[0]);

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

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");

    if (client.connect("ESP32Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

//
// Envia a cor do botão pressionado via MQTT
//

void enviarBotao(const char* cor) {
  if (client.publish(mqtt_topic_botoes, cor)) {
    Serial.print("Botão pressionado, enviado: ");
    Serial.println(cor);
  } else {
    Serial.println("Falha ao publicar no MQTT");
  }
}

//
// Lê um botão com debounce e envia na borda de subida
//

void verificarBotao(Botao &b) {
  int leitura = digitalRead(b.pino);

  if (leitura != b.ultimoEstadoLido) {
    b.ultimoTempoDebounce = millis();
    b.ultimoEstadoLido = leitura;
  }

  if ((millis() - b.ultimoTempoDebounce) > DEBOUNCE_MS) {
    if (leitura != b.estadoEstavel) {
      b.estadoEstavel = leitura;

      // HIGH = botão pressionado
      if (b.estadoEstavel == HIGH) {
        enviarBotao(b.cor);
      }
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
  for (size_t i = 0; i < NUM_BOTOES; i++) {
    pinMode(botoes[i].pino, INPUT);
  }

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

  for (size_t i = 0; i < NUM_BOTOES; i++) {
    verificarBotao(botoes[i]);
  }
}