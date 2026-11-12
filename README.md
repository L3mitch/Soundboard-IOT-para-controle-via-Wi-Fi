# 🎛️ Soundboard IoT para controle via Wi-Fi

![ESP32](https://img.shields.io/badge/ESP32-C%2FC%2B%2B-red)
![PlatformIO](https://img.shields.io/badge/PlatformIO-firmware-orange)
![MQTT](https://img.shields.io/badge/MQTT-Mosquitto-660066)
![Node-RED](https://img.shields.io/badge/Node--RED-fluxos-8F0000)
![OBS](https://img.shields.io/badge/OBS%20Studio-WebSocket-302E31)
![Linux](https://img.shields.io/badge/Linux-dev-yellow)
![Windows](https://img.shields.io/badge/Windows-suportado-0078D6)

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
- [Estrutura do repositório](#-estrutura-do-repositório)
- [Como executar](#-como-executar)
- [Protocolo de mensagens MQTT](#-protocolo-de-mensagens-mqtt)
- [Resultados](#-funcionamento-esperado)

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
| Protoboard e jumpers | ∞ | Montagem do protótipo |

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
| **Linux / Windows** | Sistemas operacionais suportados para o computador (desenvolvido em Linux) |

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

O projeto roda em **Linux** e em **Windows**. Os passos são os mesmos; só mudam os comandos de instalação e algumas configurações do sistema. Onde houver diferença, as instruções aparecem separadas por sistema.

### Pré-requisitos

- ESP32 montado conforme a seção [Hardware](#-hardware)
- Computador (Linux ou Windows) na **mesma rede Wi-Fi** do ESP32
- [PlatformIO](https://platformio.org/) (CLI ou extensão do VS Code)
- [Mosquitto](https://mosquitto.org/)
- [Node-RED](https://nodered.org/) (requer [Node.js](https://nodejs.org/))
- [OBS Studio](https://obsproject.com/) 28 ou superior (OBS WebSocket já incluído)

### 1. Clonar o repositório

```bash
git clone https://github.com/L3mitch/Soundboard-IOT-para-controle-via-Wi-Fi.git
cd Soundboard-IOT-para-controle-via-Wi-Fi
```

> No Windows, use o terminal do VS Code, o PowerShell ou o Git Bash. É necessário ter o [Git](https://git-scm.com/download/win) instalado.

### 2. Configurar o broker Mosquitto

Como o ESP32 se conecta pela rede (não por `localhost`), o Mosquitto 2.x precisa aceitar conexões externas.

#### 🐧 Linux (Debian/Ubuntu)

```bash
sudo apt install mosquitto mosquitto-clients
sudo systemctl enable --now mosquitto
```

Crie o arquivo `/etc/mosquitto/conf.d/local.conf`:

```conf
listener 1883
allow_anonymous true
```

```bash
sudo systemctl restart mosquitto
```

Descubra o IP do computador (será usado no `MQTT_SERVER` do firmware):

```bash
hostname -I
```

Teste se o broker está recebendo mensagens:

```bash
mosquitto_sub -h localhost -t "satc/botoes" -v
```

#### 🪟 Windows

1. Baixe e instale o Mosquitto em [mosquitto.org/download](https://mosquitto.org/download/) (instalador para Windows). Na instalação, mantenha marcada a opção de instalar como **serviço**.
2. Abra o **Bloco de Notas como administrador** e edite o arquivo `C:\Program Files\mosquitto\mosquitto.conf`, adicionando ao final:

   ```conf
   listener 1883
   allow_anonymous true
   ```

3. Abra o **PowerShell como administrador** e reinicie o serviço:

   ```powershell
   net stop mosquitto
   net start mosquitto
   ```

4. **Libere a porta 1883 no Firewall do Windows** (sem isso o ESP32 não consegue conectar), ainda no PowerShell como administrador:

   ```powershell
   netsh advfirewall firewall add rule name="Mosquitto MQTT" dir=in action=allow protocol=TCP localport=1883
   ```

5. Descubra o IP do computador (campo *Endereço IPv4* do adaptador Wi-Fi), que será usado no `MQTT_SERVER` do firmware:

   ```powershell
   ipconfig
   ```

6. Teste o broker. Os executáveis ficam em `C:\Program Files\mosquitto`; abra o PowerShell nessa pasta (ou adicione-a ao `PATH`):

   ```powershell
   & "C:\Program Files\mosquitto\mosquitto_sub.exe" -h localhost -t "satc/botoes" -v
   ```

> 🔒 `allow_anonymous true` é adequado apenas para rede local de testes.

### 3. Gravar o firmware no ESP32

Configure suas credenciais de rede no arquivo `include/secrets_example.h` e **renomeie-o para `secrets.h`**:

```cpp
// Renomeie este arquivo para "secrets.h" e adicione suas credenciais de Wi-Fi e MQTT.

#pragma once

#define WIFI_SSID      "NOME_DA_REDE"
#define EAP_IDENTITY   "seu.usuario"
#define EAP_USERNAME   "seu.usuario"
#define EAP_PASSWORD   "sua_senha"

#define MQTT_SERVER    "IP_DO_SERVIDOR_MQTT"
```

> Use no `MQTT_SERVER` o IP do computador obtido no passo anterior.

#### 🐧 Linux

Libere o acesso à porta serial (erro comum de *permission denied*) e grave:

```bash
sudo usermod -aG dialout $USER   # faça logout/login depois
pio run --target upload
pio device monitor               # acompanhar o log serial
```

> Em algumas distribuições (ex.: Arch) o grupo é `uucp` em vez de `dialout`.

#### 🪟 Windows

1. Instale o [VS Code](https://code.visualstudio.com/) e a extensão **PlatformIO IDE**.
2. Instale o driver USB-serial da sua placa, se o Windows não reconhecer o ESP32 como uma porta **COM** (veja em *Gerenciador de Dispositivos → Portas (COM e LPT)*):
   - Chips **CP210x**: [driver da Silicon Labs](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
   - Chips **CH340**: driver do fabricante WCH
3. Abra a pasta do projeto no VS Code. O PlatformIO instala as dependências automaticamente.
4. Clique em **Upload** (ícone →) na barra inferior, ou use o terminal do PlatformIO:

   ```powershell
   pio run --target upload
   pio device monitor               # acompanhar o log serial
   ```

> Se o upload falhar com *"Failed to connect"*, segure o botão **BOOT** do ESP32 durante o início da gravação. Não é necessário configurar permissões de porta serial no Windows.

### 4. Configurar o Node-RED

#### 🐧 Linux

```bash
node-red
```

#### 🪟 Windows

1. Instale o [Node.js](https://nodejs.org/) (versão LTS).
2. No PowerShell, instale e execute o Node-RED:

   ```powershell
   npm install -g --unsafe-perm node-red
   node-red
   ```

#### Nos dois sistemas

1. Abra `http://localhost:1880`.
2. Instale o nó de OBS WebSocket pelo *Manage palette*: `[nome do pacote utilizado]`.
3. Importe o fluxo: **Menu → Import → `nodered/flows.json`**.
4. Confirme que o nó `mqtt in` aponta para `localhost:1883` e para o tópico `satc/botoes`.
5. Configure a conexão com o OBS (host `localhost`, porta `4455` por padrão e a senha definida no OBS).
6. Clique em **Deploy**.

### 5. Configurar o OBS Studio

1. Em **Ferramentas → Configurações do servidor WebSocket**, ative o servidor e defina uma senha.
2. Crie/configure as fontes de áudio dos efeitos sonoros (há sons de exemplo na pasta `Sons/`): `[descreva: fontes de mídia, nomes usados no fluxo...]`.

### 6. Testar

Pressione um botão na soundboard. Você deve ver:

1. O nome do som no display LCD
2. A mensagem chegando em `mosquitto_sub`
3. O fluxo do Node-RED sendo executado
4. O som sendo reproduzido no OBS

Também é possível simular um botão sem o hardware:

```bash
# Linux
mosquitto_pub -h localhost -t "satc/botoes" -m "1"
```

```powershell
# Windows (PowerShell)
& "C:\Program Files\mosquitto\mosquitto_pub.exe" -h localhost -t "satc/botoes" -m "1"
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

## ✅ Funcionamento esperado

- Na inicialização, o LCD executa uma sequência de testes visuais e, em seguida, confirma a conexão com o broker MQTT
- Ao acionar qualquer botão, o ESP32 identifica a entrada instantaneamente e exibe o nome do som no LCD
- A mensagem é publicada no tópico MQTT e chega ao Node-RED, que envia o comando ao OBS via WebSocket
- O OBS executa o efeito sonoro de forma imediata, sem atrasos perceptíveis
- O sistema apresentou alta estabilidade na troca de mensagens assíncronas
