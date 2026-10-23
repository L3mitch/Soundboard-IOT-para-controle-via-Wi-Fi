#include <Arduino.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

TFT_eSPI tft = TFT_eSPI();

// Chamada pelo decodificador JPG para desenhar cada bloco na tela
bool saida(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return false;
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}

void testeInicial() {
  // Teste de cores em tela cheia
  tft.fillScreen(TFT_RED);   delay(700);
  tft.fillScreen(TFT_GREEN); delay(700);
  tft.fillScreen(TFT_BLUE);  delay(700);
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

void mostrarImagem() {
  if (!LittleFS.begin(true)) {
    Serial.println("Falha ao montar LittleFS");
    return;
  }
  Serial.println("LittleFS ok");

  if (!LittleFS.exists("/foto.jpg")) {
    Serial.println("foto.jpg nao encontrada (rode o uploadfs)");
    return;
  }

  uint16_t jw, jh;
  TJpgDec.getFsJpgSize(&jw, &jh, "/foto.jpg", LittleFS);
  Serial.printf("jpg: %dx%d\n", jw, jh);

  tft.setSwapBytes(true);
  TJpgDec.setJpgScale(1);        // 1, 2, 4 ou 8 (reduz a imagem)
  TJpgDec.setCallback(saida);

  tft.fillScreen(TFT_BLACK);
  TJpgDec.drawFsJpg(0, 0, "/foto.jpg", LittleFS);
  Serial.println("imagem desenhada");
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);            // paisagem 320x240, igual a imagem
  Serial.printf("w=%d h=%d\n", tft.width(), tft.height());

  testeInicial();
  delay(3000);                   // tempo para ver o teste
  mostrarImagem();
}

void loop() {
  // A imagem fica parada na tela
}