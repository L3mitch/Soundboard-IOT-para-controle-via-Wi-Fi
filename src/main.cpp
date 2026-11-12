#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include "secrets.h"
#include "telasom.h"

// ======================================================
// CONFIGURAÇÃO
// ======================================================

// --- MQTT ---
const char *mqtt_server = MQTT_SERVER;
const char *mqtt_topic_botoes = "satc/botoes";
const char *MQTT_CLIENT_ID = "ESP32Client";
const uint16_t MQTT_PORTA = 1883;
const uint32_t RECONEXAO_MS = 5000;

// --- Matriz de botões 2x5 ---
const uint8_t NUM_LINHAS = 2;
const uint8_t NUM_COLUNAS = 5;
const uint8_t PINOS_LINHAS[NUM_LINHAS] = {26, 27};
const uint8_t PINOS_COLUNAS[NUM_COLUNAS] = {34, 35, 32, 33, 25};
const unsigned long DEBOUNCE_MS = 50;
const uint32_t VARREDURA_MS = 5;

// Nome de cada som (é o que vai pro MQTT e pra tela)
const char *SONS[NUM_LINHAS][NUM_COLUNAS] = {
    {"ERROU!", "ACERTOU", "IIIIRRRRRAAAA", "RATTINNHHOO", "Rapaaaazz"},
    {"ESSE EH O MEU PATRAO", "Zap", "Trombone", "Ba dum tss", "Grilo"},
};

// --- FreeRTOS ---
#define FILA_TAMANHO 20

// ======================================================
// VARIÁVEIS GLOBAIS
// ======================================================

TFT_eSPI tft = TFT_eSPI();
WiFiClient espClient;
PubSubClient client(espClient);

QueueHandle_t filaBotoes; // guarda ponteiros para as strings de SONS

struct EstadoTecla
{
  int ultimoLido;
  int estavel;
  unsigned long ultimoTempo;
};

EstadoTecla teclas[NUM_LINHAS][NUM_COLUNAS];

// ======================================================
// DISPLAY
// ======================================================

// Chamada pelo decodificador JPG para desenhar cada bloco na tela
bool saida(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
  if (y >= tft.height())
    return false;
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}

void testeInicial()
{
  // Teste de cores em tela cheia
  tft.fillScreen(TFT_RED);
  delay(700);
  tft.fillScreen(TFT_GREEN);
  delay(700);
  tft.fillScreen(TFT_BLUE);
  delay(700);
  tft.fillScreen(TFT_BLACK);

  // Barras de cor
  int w = tft.width() / 4;
  tft.fillRect(0 * w, 0, w, 60, TFT_RED);
  tft.fillRect(1 * w, 0, w, 60, TFT_GREEN);
  tft.fillRect(2 * w, 0, w, 60, TFT_BLUE);
  tft.fillRect(3 * w, 0, w, 60, TFT_WHITE);

  // Texto
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(10, 90);
  tft.println("ESP32 + ILI9341");

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 130);
  tft.printf("%dx%d OK", tft.width(), tft.height());

  tft.setCursor(10, 170);
  tft.println("Carregando imagem...");

  tft.drawRect(2, 2, tft.width() - 4, tft.height() - 4, TFT_MAGENTA);
  Serial.println("teste inicial feito");
}

void mostrarImagem()
{
  if (!LittleFS.begin(true))
  {
    Serial.println("Falha ao montar LittleFS");
    return;
  }
  Serial.println("LittleFS ok");

  if (!LittleFS.exists("/foto.jpg"))
  {
    Serial.println("foto.jpg nao encontrada (rode o uploadfs)");
    return;
  }

  uint16_t jw, jh;
  TJpgDec.getFsJpgSize(&jw, &jh, "/foto.jpg", LittleFS);
  Serial.printf("jpg: %dx%d\n", jw, jh);

  tft.setSwapBytes(true);
  TJpgDec.setJpgScale(1); // 1, 2, 4 ou 8 (reduz a imagem)
  TJpgDec.setCallback(saida);

  tft.fillScreen(TFT_BLACK);
  TJpgDec.drawFsJpg(0, 0, "/foto.jpg", LittleFS);
  Serial.println("imagem desenhada");
}

void iniciarDisplay()
{
  tft.init();
  tft.setRotation(1); // paisagem 320x240, igual a imagem
  Serial.printf("w=%d h=%d\n", tft.width(), tft.height());

  testeInicial();
  delay(3000); // tempo para ver o teste
  mostrarImagem();
  desenharTelaSom();
}

// ======================================================
// WIFI
// ======================================================

void setup_wifi()
{
  Serial.println();
  Serial.print("Conectando em ");
  Serial.println(WIFI_SSID);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, EAP_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());
}

// ======================================================
// BOTÕES (varredura da matriz + debounce)
// ======================================================

void iniciarBotoes()
{
  for (uint8_t l = 0; l < NUM_LINHAS; l++)
  {
    pinMode(PINOS_LINHAS[l], OUTPUT);
    digitalWrite(PINOS_LINHAS[l], LOW);
  }
  for (uint8_t c = 0; c < NUM_COLUNAS; c++)
  {
    pinMode(PINOS_COLUNAS[c], INPUT); // pull-down externo de 10k
  }
}

// Coloca o som na fila sem bloquear (se estiver cheia, descarta)
void enviarParaFila(const char *som)
{
  if (xQueueSend(filaBotoes, &som, 0) != pdTRUE)
  {
    Serial.println("Fila cheia, evento descartado");
  }
}

void verificarMatriz()
{
  for (uint8_t l = 0; l < NUM_LINHAS; l++)
  {
    digitalWrite(PINOS_LINHAS[l], HIGH);
    delayMicroseconds(10);

    for (uint8_t c = 0; c < NUM_COLUNAS; c++)
    {
      EstadoTecla &t = teclas[l][c];
      int leitura = digitalRead(PINOS_COLUNAS[c]);

      if (leitura != t.ultimoLido)
      {
        t.ultimoTempo = millis();
        t.ultimoLido = leitura;
      }

      if ((millis() - t.ultimoTempo) > DEBOUNCE_MS && leitura != t.estavel)
      {
        t.estavel = leitura;

        if (t.estavel == HIGH)
        {
          enviarParaFila(SONS[l][c]);
        }
      }
    }

    digitalWrite(PINOS_LINHAS[l], LOW);
  }
}

void taskBotoes(void *param)
{
  for (;;)
  {
    verificarMatriz();
    vTaskDelay(pdMS_TO_TICKS(VARREDURA_MS));
  }
}

// ======================================================
// MQTT (conexão, client.loop e publicação)
// ======================================================

// Retorna true se o Wi-Fi está conectado; senão tenta reconectar
bool garantirWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
    return true;

  atualizarStatusMQTT(false);
  Serial.println("WiFi perdido, reconectando...");
  WiFi.reconnect();
  vTaskDelay(pdMS_TO_TICKS(RECONEXAO_MS));
  return false;
}

// Retorna true se o MQTT está conectado; senão tenta reconectar
bool garantirMQTT()
{
  if (client.connected())
    return true;

  atualizarStatusMQTT(false);
  Serial.print("Tentando conexão MQTT...");

  if (client.connect(MQTT_CLIENT_ID))
  {
    Serial.println("conectado");
    xQueueReset(filaBotoes); // descarta cliques antigos da época offline
    atualizarStatusMQTT(true);
    return true;
  }

  Serial.print("falhou, rc=");
  Serial.print(client.state());
  Serial.println(" tentando novamente em 5 segundos");
  vTaskDelay(pdMS_TO_TICKS(RECONEXAO_MS));
  return false;
}

// Espera até 10 ms por um clique; publica no MQTT e mostra na tela
void processarFila()
{
  const char *som;

  if (xQueueReceive(filaBotoes, &som, pdMS_TO_TICKS(10)) != pdTRUE)
    return;

  if (client.publish(mqtt_topic_botoes, som))
  {
    mostrarSom(som);
    Serial.print("Botão pressionado, enviado: ");
    Serial.println(som);
  }
  else
  {
    Serial.println("Falha ao publicar no MQTT");
  }
}

void taskMQTT(void *param)
{
  for (;;)
  {
    if (!garantirWiFi())
      continue;
    if (!garantirMQTT())
      continue;

    client.loop();
    processarFila();
  }
}

// ======================================================
// SETUP / LOOP
// ======================================================

void setup()
{
  Serial.begin(115200);

  iniciarDisplay();
  iniciarBotoes();

  setup_wifi();
  client.setServer(mqtt_server, MQTT_PORTA);

  filaBotoes = xQueueCreate(FILA_TAMANHO, sizeof(const char *));

  // (função, nome, stack em bytes, parâmetro, prioridade, handle, core)
  xTaskCreatePinnedToCore(taskMQTT, "MQTT", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(taskBotoes, "Botoes", 2048, NULL, 3, NULL, 1);
}

void loop()
{
  vTaskDelete(NULL); // o loop() não é mais necessário
}