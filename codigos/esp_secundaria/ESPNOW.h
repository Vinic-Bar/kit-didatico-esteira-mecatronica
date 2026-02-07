#include <esp_now.h>
#include <WiFi.h>
#include "globals.h"


// Função chamada após cada envio de dados (confirmação)
void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("Send Status: "); 
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// função de recepção, sempre que um pacote chega ela é chamda
void onDataRecv(const esp_now_recv_info_t *info,
                const uint8_t *incomingData,
                int len) {
  
  if (len != sizeof(uint16_t)) {   // Verifica se recebeu o dadocom tamanho certo
    Serial.println("Tamanho de frame inválido recebido via ESP-NOW");
    return;
  }
  uint16_t frame;
  memcpy(&frame, incomingData, sizeof(frame)); // copia os dados no frame

  // memoria 
  ultimoFrameCLP = frame;
  novoFrameCLP = true;

  // Decodificação do frame
  uint8_t endereco;
  bool motorOn;
  bool direcao;
  uint8_t pwm;

  pwm      = frame & 0x00FF;             // Bits 0..7
  direcao  = (frame & 0x0100) != 0;      // Bit 8
  motorOn  = (frame & 0x0800) != 0;      // Bit 11
  endereco = (frame >> 12) & 0x0F;       // Bits 15..12

  Serial.printf("ESP-NOW → Frame=%u | ESP=%d | ON=%d | DIR=%d | PWM=%d\n",
                frame, endereco, motorOn, direcao, pwm);

  // Confere se o frame é para essa ESP
  if (endereco == MEU_ENDERECO) {
    if (!motorOn) {
      estadoMotor0 = false;
      pwm0 = 0;
    } else {
      estadoMotor0 = true;
      direcao0 = direcao;
      updatePWM(pwm);
    }
  } else {
    Serial.println("Frame recebido não é para esta ESP");
  }
}

// Setup
void setupESPNOW() {
  WiFi.mode(WIFI_STA); // modo STA (pro ESPNOW)
  WiFi.setChannel(11);  // força canal 11
  delay(100);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  // Adiciona Peer da ESP principal
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, macPrincipal, 6);

  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
}