# 🎛️ Soundboard IoT para controle via Wi-Fi

![ESP32](https://img.shields.io/badge/ESP32-C%2FC%2B%2B-red)
![PlatformIO](https://img.shields.io/badge/PlatformIO-firmware-orange)
![MQTT](https://img.shields.io/badge/MQTT-Mosquitto-660066)
![Node-RED](https://img.shields.io/badge/Node--RED-fluxos-8F0000)
![OBS](https://img.shields.io/badge/OBS%20Studio-WebSocket-302E31)
![Linux](https://img.shields.io/badge/Linux-dev-yellow)

Soundboard física baseada em **ESP32** que envia comandos por **MQTT** para um computador, onde o **Node-RED** os traduz em ações no **OBS Studio** via **OBS WebSocket**. Cada botão dispara um efeito sonoro durante transmissões ou gravações.

> Projeto Final (Etapa N1C) · Trilha formativa: Internet das Coisas (IoT) · UniSATC
> Autor: **Michel Almeida da Rosa**

---

## 📑 Sumário

- [Visão geral](#-visão-geral)
- [Demonstração](#-demonstração)
- [Arquitetura](#-arquitetura)
- [Funcionalidades](#-funcionalidades)
- [Hardware](#-hardware)
- [Tecnologias](#-tecnologias)
- [Estrutura do repositório](#-estrutura-do-repositório)
- [Como executar](#-como-executar)
- [Protocolo de mensagens MQTT](#-protocolo-de-mensagens-mqtt)
- [Resultados](#-resultados)
- [Desafios e soluções](#-desafios-e-soluções)
- [Trabalhos futuros](#-trabalhos-futuros)
- [Documentação complementar](#-documentação-complementar)

---

## 🎯 Visão geral

Streamers e criadores de conteúdo costumam usar atalhos de teclado ou interfaces de software para disparar efeitos sonoros. Este projeto propõe uma alternativa **física e sem fio**: uma soundboard com dez botões, um display LCD e comunicação via rede Wi-Fi local, integrada diretamente ao OBS Studio.

**Fluxo resumido:** botão → ESP32 → MQTT (Mosquitto) → Node-RED → OBS WebSocket → som no OBS.

## 🎬 Demonstração

| Evidência | Link / Imagem |
|---|---|
| Vídeo do protótipo funcionando | https://drive.google.com/drive/folders/1qvtZmp16oAlQ45rb66_H22N4SkEroZFq?usp=sharing |
| Foto da montagem em protoboard | <img width="3072" height="2458" alt="Hardware" src="https://github.com/user-attachments/assets/744f9811-02c7-4265-bfd9-d6fe5af4bf97" /> |
| Display LCD exibindo o som acionado | <img width="4080" height="3072" alt="TelaLCDperto" src="https://github.com/user-attachments/assets/563b61fd-fcd7-4b4c-8013-b7a57172d8b0" /> |
| Mensagens chegando no tópico MQTT | <img width="533" height="258" alt="MQTT input" src="https://github.com/user-attachments/assets/dca32094-fb69-4bfc-bbf3-8f51f1e1e9b9" /> |
| Fluxo do Node-RED | <img width="913" height="400" alt="fluxo node-red" src="https://github.com/user-attachments/assets/c551c44b-dd0e-47de-8769-f9a4dcd5ba1f" /> |

## 🧩 Arquitetura

```mermaid
flowchart LR
    A["10 botões<br/>(matriz)"] --> B["ESP32<br/>+ display LCD"]
    B -- "Wi-Fi · MQTT<br/>tópico satc/botoes" --> C["Broker<br/>Mosquitto"]
    C --> D["Node-RED<br/>(assinante)"]
    D -- "OBS WebSocket" --> E["OBS Studio"]
    E --> F["🔊 Efeito sonoro"]
```

1. O ESP32 lê a matriz de botões e identifica qual foi pressionado.
2. O display LCD mostra qual som será acionado.
3. O ESP32 publica uma mensagem no tópico `satc/botoes` do broker Mosquitto (local).
4. O Node-RED, inscrito nesse tópico, recebe a mensagem e decide qual comando enviar.
5. O OBS Studio recebe o comando via WebSocket e dispara o som correspondente.

## ✨ Funcionalidades

- 10 botões, cada um associado a um efeito sonoro
- Leitura em **matriz de botões**, economizando pinos do ESP32
- Display LCD com teste visual na inicialização, confirmação de conexão com o broker MQTT e exibição do som acionado
- Comunicação assíncrona e de baixa latência sobre Wi-Fi local
- Integração com o OBS Studio sem depender de software proprietário

## 🔧 Hardware

| Componente | Quantidade | Observação |
|---|---|---|
| ESP32 (DevKit) | 1 | Núcleo da soundboard |
| Botões (push button) | 10 | Ligados em matriz |
| Display LCD | 1 | 2.4" TFT SPI 240X320 |
| Protoboard e jumpers | — | Montagem do protótipo |

### Ligações (pinagem)

| Função | Pino do ESP32 |
|---|---|
| Linhas da matriz | `26, 27` |
| Colunas da matriz | `34, 35, 32, 33, 25` |
| LCD (SPI) | `RX2(16)(DC), TX2(17)(RESET), 5(CS), 18(SCK), 19(SDO), 23(SDI<MOSI>)` |
<img width="720" height="480" alt="LCD" src="https://github.com/user-attachments/assets/b323ced8-7da1-4b7c-8c1d-236e66e1353a" />


## 🛠️ Tecnologias

| Tecnologia | Papel no projeto |
|---|---|
| **ESP32** (C/C++) | Microcontrolador com Wi-Fi; lê botões, controla o LCD e publica no MQTT |
| **PlatformIO** | Compilação e gravação do firmware |
| **MQTT / Mosquitto** | Protocolo e broker local que intermedeiam ESP32 e computador |
| **Node-RED** | Fluxos que recebem as mensagens MQTT e decidem os comandos |
| **OBS WebSocket** | Interface nativa do OBS Studio para controle externo em tempo real |
| **Linux** | Sistema operacional do notebook de desenvolvimento |

## 📂 Estrutura do repositório

```
.
├── src/                    # Código do ESP32
│   ├── main.cpp
│   └── telasom.h
├── nodered/
│   └── flows.json          # Fluxo exportado do Node-RED
├── Sons/                   # Alguns sons para teste!
├── include/
│   └── secrets_example.h   # Siga as instruções presentes no arquivo e insira aqui as informações de rede Wi-Fi e IP do server MQTT
├── docs/               
│   └── Relatório.pdf       # Relatório final
└── README.md
```

## 🚀 Como executar

### Pré-requisitos

- ESP32 montado conforme a seção [Hardware](#-hardware)
- Computador na **mesma rede Wi-Fi** do ESP32
- [PlatformIO](https://platformio.org/) (CLI ou extensão do VS Code)
- [Mosquitto](https://mosquitto.org/)
- [Node-RED](https://nodered.org/)
- [OBS Studio](https://obsproject.com/) 28 ou superior (OBS WebSocket já incluído)

### 1. Clonar o repositório

```bash
git clone https://github.com/L3mitch/Soundboard-IOT-para-controle-via-Wi-Fi.git
cd Soundboard-IOT-para-controle-via-Wi-Fi
```

### 2. Configurar o broker Mosquitto

Instale e habilite o serviço (exemplo em Debian/Ubuntu):

```bash
sudo apt install mosquitto mosquitto-clients
sudo systemctl enable --now mosquitto
```

Como o ESP32 se conecta pela rede (não por `localhost`), o Mosquitto 2.x precisa aceitar conexões externas. Crie `/etc/mosquitto/conf.d/local.conf`:

```conf
listener 1883
allow_anonymous true
```

```bash
sudo systemctl restart mosquitto
```

> 🔒 `allow_anonymous true` é adequado apenas para rede local de testes.

Teste se o broker está recebendo mensagens:

```bash
mosquitto_sub -h localhost -t "satc/botoes" -v
```

### 3. Gravar o firmware no ESP32

Configure suas credenciais de rede no arquivo "include/secrets_example.h":

```cpp
// Renomeie este arquivo para "secrets.h" e adicione suas credenciais de Wi-Fi e MQTT.

#pragma once

#define WIFI_SSID      "NOME_DA_REDE"
#define EAP_IDENTITY   "seu.usuario"
#define EAP_USERNAME   "seu.usuario"
#define EAP_PASSWORD   "sua_senha"

#define MQTT_SERVER    "IP_DO_SERVIDOR_MQTT"
```

Libere o acesso à porta serial no Linux (erro comum de *permission denied*) e grave:

```bash
sudo usermod -aG dialout $USER   # faça logout/login depois
cd firmware
pio run --target upload
pio device monitor               # acompanhar o log serial
```

> Em algumas distribuições (ex.: Arch) o grupo é `uucp` em vez de `dialout`.

### 4. Configurar o Node-RED

```bash
node-red
```

1. Abra `http://localhost:1880`.
2. Instale o nó de OBS WebSocket pelo *Manage palette*: `[nome do pacote utilizado]`.
3. Importe o fluxo: **Menu → Import → `nodered/flows.json`**.
4. Confirme que o nó `mqtt in` aponta para `localhost:1883` e para o tópico `satc/botoes`.
5. Configure a conexão com o OBS (host `localhost`, porta `4455` por padrão e a senha definida no OBS).
6. Clique em **Deploy**.

### 5. Configurar o OBS Studio

1. Em **Ferramentas → Configurações do servidor WebSocket**, ative o servidor e defina uma senha.
2. Crie/configure as fontes de áudio dos efeitos sonoros: `[descreva: fontes de mídia, nomes usados no fluxo...]`.

### 6. Testar

Pressione um botão na soundboard. Você deve ver:

1. O nome do som no display LCD
2. A mensagem chegando em `mosquitto_sub`
3. O fluxo do Node-RED sendo executado
4. O som sendo reproduzido no OBS

Também é possível simular um botão sem o hardware:

```bash
mosquitto_pub -h localhost -t "satc/botoes" -m "1"
```

> ⚠️ Ajuste o payload (`"1"`) para o formato que seu firmware realmente envia.

## 📡 Protocolo de mensagens MQTT

| Item | Valor |
|---|---|
| Broker | Mosquitto local, porta `1883` |
| Tópico | `satc/botoes` |
| Publicador | ESP32 |
| Assinante | Node-RED |
| Payload | `[descreva: ex. número do botão 1–10]` |

## ✅ Resultados

O protótipo operou com êxito no fluxo completo, de ponta a ponta:

- Na inicialização, o LCD executa uma sequência de testes visuais e, em seguida, confirma a conexão com o broker MQTT
- Ao acionar qualquer botão, o ESP32 identifica a entrada instantaneamente e exibe o nome do som no LCD
- A mensagem é publicada no tópico MQTT e chega ao Node-RED, que envia o comando ao OBS via WebSocket
- O OBS executa o efeito sonoro de forma imediata, sem atrasos perceptíveis
- O sistema apresentou alta estabilidade na troca de mensagens assíncronas

## 🧗 Desafios e soluções

| Desafio | Solução |
|---|---|
| **Permissão da porta USB no Linux** impedia gravar o ESP32 | Liberação de acesso à porta serial (grupo `dialout`) |
| **Dependências e bibliotecas** do PlatformIO, Mosquitto e Node-RED | Instalação e atualização dos pacotes necessários após pesquisa |
| **Protoboard**: conexões instáveis e sensíveis a ruído | Reforço das conexões; solução definitiva prevista com PCB própria |
| **Tela LCD**: duas das três telas separadas estavam com defeito, o que levou a um tempo gasto depurando software | Troca da tela; lição: isolar e testar o hardware antes de depurar o código |
| **Dez botões com poucos pinos** | Pesquisa e implementação de **matriz de botões** |

## 🔮 Trabalhos futuros

- [ ] **PCB dedicada** para eliminar mau contato e ruídos da protoboard
- [ ] **Status da rede Wi-Fi** no display, além do status do broker MQTT, para facilitar a depuração
- [ ] **Menus com encoder rotativo** para alternar entre bancos de efeitos e configurações
- [ ] **Gabinete impresso em 3D** e botões mais táteis e ergonômicos

## 📚 Documentação complementar

- 📄 [Relatório final (Etapa N1C)](docs/relatorio-final.pdf)
- 🎥 Vídeo de demonstração: `[link]`

## 👤 Autor

**Michel Almeida da Rosa**
Trilha formativa: Internet das Coisas (IoT) · Centro Universitário UniSATC
GitHub: [@L3mitch](https://github.com/L3mitch)
