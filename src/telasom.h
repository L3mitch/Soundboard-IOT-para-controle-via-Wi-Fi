// telasom.h - Tela "Tocando agora" para o soundboard (TFT_eSPI, 320x240)
// A moldura é desenhada UMA vez; a cada botão só a linha do texto é redesenhada.
//
// Uso: #include "telasom.h" e chame mostrarSom(nome) a cada clique.
// (Opcional) chame desenharTelaSom() no setup para já mostrar a moldura.

#pragma once
#include <TFT_eSPI.h>

extern TFT_eSPI tft;  // objeto tft já existente

// Paleta (RGB565)
#define COR_FUNDO    0x0841  // cinza quase preto
#define COR_CARD     0x10A2  // cinza escuro
#define COR_DESTAQUE 0xFD20  // laranja
#define COR_TEXTO    TFT_WHITE
#define COR_SUAVE    0x8410  // cinza médio

// Posição do card e da área do texto
#define CARD_X 10
#define CARD_Y 42
#define CARD_H 110
#define TEXTO_Y 105   // centro vertical do nome do som
#define TEXTO_AREA_Y 70
#define TEXTO_AREA_H 68

static bool telaSomDesenhada = false;
static bool mqttStatusAtual = false;  // último estado do MQTT mostrado

// Desenha o indicador de MQTT na barra superior (canto direito)
void desenharStatusMQTT(bool conectado)
{
  const int W = tft.width();
  const int px = W - 100;
  const int py = 5;

  // Limpa só a área do indicador dentro da barra
  tft.fillRect(px - 2, 0, 102, 30, COR_DESTAQUE);

  // "Pílula" escura com bolinha verde/vermelha e texto
  tft.fillRoundRect(px, py, 92, 20, 10, TFT_BLACK);
  tft.fillCircle(px + 12, py + 10, 5, conectado ? TFT_GREEN : TFT_RED);

  tft.setTextDatum(ML_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(conectado ? "CONECTADO" : "OFFLINE", px + 22, py + 10);
  tft.setTextDatum(TL_DATUM);
}

// Barras de "equalizador" decorativas
void desenharEqualizador(int x, int y, int w, int h, uint16_t cor)
{
  const int barras = 12;
  int larg = w / barras;

  // Apaga só a área do equalizador antes de redesenhar
  tft.fillRect(x, y, w, h, COR_FUNDO);
  for (int i = 0; i < barras; i++)
  {
    int alt = random(h / 4, h);
    tft.fillRoundRect(x + i * larg + 1, y + h - alt, larg - 3, alt, 2, cor);
  }
}

// Escolhe o maior tamanho de fonte que cabe na largura disponível
void ajustarTextoNoCard(const String &texto, int larguraMax)
{
  for (int tam = 5; tam >= 1; tam--)
  {
    tft.setTextSize(tam);
    if (tft.textWidth(texto) <= larguraMax) return;
  }
}

// Desenha a parte fixa da tela (só precisa rodar uma vez)
void desenharTelaSom()
{
  const int W = tft.width();
  const int H = tft.height();

  tft.fillScreen(COR_FUNDO);

  // Cabeçalho
  tft.fillRect(0, 0, W, 30, COR_DESTAQUE);
  tft.setTextColor(TFT_BLACK, COR_DESTAQUE);
  tft.setTextDatum(ML_DATUM);
  tft.setTextSize(2);
  tft.drawString("SOUNDBOARD", 10, 15);
  desenharStatusMQTT(mqttStatusAtual);
  tft.setTextDatum(MC_DATUM);  // o resto da tela usa texto centralizado

  // Card central com borda
  tft.fillRoundRect(CARD_X, CARD_Y, W - 2 * CARD_X, CARD_H, 12, COR_CARD);
  tft.drawRoundRect(CARD_X, CARD_Y, W - 2 * CARD_X, CARD_H, 12, COR_DESTAQUE);

  // Rótulo pequeno
  tft.setTextColor(COR_SUAVE, COR_CARD);
  tft.setTextSize(1);
  tft.drawString("TOCANDO AGORA", W / 2, 58);

  // Equalizador decorativo (fixo)
  desenharEqualizador(20, 165, W - 40, 50, COR_DESTAQUE);

  // Rodapé
  tft.setTextColor(COR_SUAVE, COR_FUNDO);
  tft.setTextSize(1);
  tft.drawString("Enviado via MQTT", W / 2, H - 10);

  tft.setTextDatum(TL_DATUM);
  telaSomDesenhada = true;
}

// Atualiza o indicador de MQTT na barra superior (só redesenha se mudou)
void atualizarStatusMQTT(bool conectado)
{
  if (conectado == mqttStatusAtual) return;
  mqttStatusAtual = conectado;
  if (telaSomDesenhada) desenharStatusMQTT(conectado);
}

// Atualiza APENAS a linha com o nome do som
void mostrarSom(const String &nome)
{
  if (!telaSomDesenhada) desenharTelaSom();  // primeira vez: monta a moldura

  const int W = tft.width();

  // Apaga só a área do texto (dentro do card, sem tocar na borda)
  tft.fillRect(CARD_X + 4, TEXTO_AREA_Y, W - 2 * (CARD_X + 4), TEXTO_AREA_H, COR_CARD);

  // Escreve o novo nome
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COR_TEXTO, COR_CARD);
  ajustarTextoNoCard(nome, W - 40);
  tft.drawString(nome, W / 2, TEXTO_Y);
  tft.setTextDatum(TL_DATUM);

  // Novo conjunto de barras a cada clique
  desenharEqualizador(20, 165, W - 40, 50, COR_DESTAQUE);
}