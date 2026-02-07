#include <WiFi.h>
#include "globals.h" 

// Configurações de rede
const char* ssid = "MaxLink-300-2A"; 
const char* password = "Senha123";      

IPAddress local_IP(192, 168, 0, 123);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// IP do CLP
const char* plc_ip = "192.168.0.6";

// PORTAS
#define PORT_TX 2024 // ESP -> CLP
#define PORT_RX 2023 // CLP -> ESP

WiFiClient clientTX;  
WiFiClient clientRX;

unsigned long ultimoEnvio = 0; // Memória do último tempo do envio

unsigned long start = millis(); // pra contar depois

// Setup
void setupTCP() {
  Serial.println("Inicializando TCP/IP para CLP");

  // Configurar IP estático
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Falha ao configurar IP estático!");
  }

  // Conectar no Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("IP da ESP32: ");
  Serial.println(WiFi.localIP());
  Serial.print("Canal WiFi: ");
  Serial.println(WiFi.channel());

  // Conecta o TX (ESP -> CLP) 
  Serial.println("Conectando TX (ESP->CLP)...");
  while (!clientTX.connect(plc_ip, PORT_TX)) {
    Serial.println("Falha TX, tentando novamente 1s...");
    if (millis() - start > 10000) break; // tenta 10 segundos
    delay(1000);
  }
  Serial.println("TX conectado!");

  start = millis();
  // Conecta o RX (CLP -> ESP)
  Serial.println("Conectando RX (CLP->ESP)...");
  while (!clientRX.connect(plc_ip, PORT_RX)) {
    Serial.println("Falha RX, tentando novamente 1s...");
    if (millis() - start > 10000) break; // tenta 10 segundos
    delay(1000);
  }
  Serial.println("RX conectado!");
}

// Função para decodificar o frame recebido do CLP
void decodeFrame(uint16_t frame,
                 uint8_t &endereco,
                 bool &motorOn,
                 bool &direcao,
                 uint8_t &pwm)
{
  pwm      = frame & 0x00FF;             // Bits 0..7: PWM
  direcao  = (frame & 0x0100) != 0;      // Bit 8: Direção
  motorOn  = (frame & 0x0800) != 0;      // Bit 11: Start
  endereco = (frame >> 12) & 0x0F;       // Bits 15..12: Endereço da ESP
}

// Loop
void loopTCP() {
  // Reconectar TX (quando cair)
  if (!clientTX.connected()) {
    Serial.println("Reconectando TX...");
    clientTX.stop();
    clientTX.connect(plc_ip, PORT_TX);
  }

  // Reconectar RX
  if (!clientRX.connected()) {
    Serial.println("Reconectando RX...");
    clientRX.stop();
    clientRX.connect(plc_ip, PORT_RX);
  }

  // Enviar dados para o CLP a cada 1s
  if (millis() - ultimoEnvio >= 1000) {
    ultimoEnvio = millis();

    // Lê o RPM atual
    float rpmAtual = ultimoFrameRPM; //getEncoderRPM(); // Função do Encoder.ino

    // Converte para int16_t (arredondando)
    int16_t valorEnvio = (int16_t)rpmAtual;

    // Divide em dois bytes para envio
    uint8_t bytes[2] = { valorEnvio >> 8, valorEnvio & 0xFF };

    if (clientTX.connected()) {
      clientTX.write(bytes, 2);
      Serial.print("Enviado ao CLP (RPM): ");
      Serial.println(valorEnvio);
    }
  }

  // Receber dados do CLP
  while (clientRX.available() >= 2) {
    uint8_t buffer[2];
    clientRX.read(buffer, 2);

    uint16_t frame = (buffer[0] << 8) | buffer[1]; //Big endian 
    
    // Declarando as variáveis
    uint8_t endereco;
    bool motorOn;
    bool direcao;
    uint8_t pwm;

    decodeFrame(frame, endereco, motorOn, direcao, pwm); //Função da tradução

    Serial.printf("CLP → ESP | ESP=%d | ON=%d | DIR=%d | PWM=%d\n",
              endereco, motorOn, direcao, pwm);

    
    // Guarda o último frame recebido
    ultimoFrameCLP = frame;
    novoFrameCLP = true;

    // Verifica o endereço da ESP
    if (endereco == MEU_ENDERECO) {
      if (!motorOn) {
        estadoMotor0 = false;
        pwm0 = 0;
      } else {
        estadoMotor0 = true;
        direcao0 = direcao;
        pwm0 = pwm;
      }
    } else {
      Serial.println("Frame não é da ESP0 Encaminhando para ESP destinada");
    }
  }
}