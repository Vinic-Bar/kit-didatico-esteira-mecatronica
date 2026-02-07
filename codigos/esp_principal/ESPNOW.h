#include <esp_now.h>
#include <WiFi.h>
#include "globals.h"

// Função chamada após cada envio de dados (confirmação)
void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("Send Status: "); /////////////////////////
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void onDataRecv(const esp_now_recv_info_t *info,
                const uint8_t *incomingData,
                int len) {

    if (len != sizeof(struct_message)) {
        Serial.println("Frame recebido com tamanho inválido");
        return;
    }

    struct_message msg;
    memcpy(&msg, incomingData, sizeof(msg));

    // Salva o frame
    ultimoFrameRPM = msg.frame;
    novoFrameRPM = true;

    // Decodifica o frame recebido de outra ESP
    uint8_t tipo     = ((msg.frame >> 9) & 0x03); // bits 9-10
    uint8_t endereco = ((msg.frame >> 12) & 0x0F); // bits 12-15
    uint8_t valor    = msg.frame & 0xFF;          // bits 0-7

    if (tipo != 1) return; // só processa RPM

    Serial.printf("RPM recebido da ESP %d: %d\n", endereco, valor);
}

// Estrutura para mapear MAC por endereço lógico
struct Peer {
    uint8_t mac[6];
    uint8_t endereco;
};

// MAC das outras ESPs
Peer peers[] = { // MAC, endereço B ou 8
    {{0xCC, 0xDB, 0xA7, 0x13, 0xC8, 0xAC}, 1},  // ESP1
    {{0x94, 0xB5, 0x55, 0xF8, 0xB6, 0x14}, 2}   // ESP2 (exemplo)
};

const size_t numPeers = sizeof(peers) / sizeof(peers[0]);

// Setup
void setupESPNOW() {
  WiFi.mode(WIFI_STA);         // modo Station (necessário pro ESP-NOW)
  WiFi.setChannel(11);         // força canal 11 pois é o do roteador utilizado
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar ESP-NOW!");
    return;
  }

  esp_now_register_send_cb(onDataSent); // registra callback de envio
  esp_now_register_recv_cb(onDataRecv); // registro callbacl de recebimento

 // Adiciona cada peer
    for (size_t i = 0; i < numPeers; i++) {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, peers[i].mac, 6); // Esse 6 é pq o MAC tem 6 espaços
        peerInfo.channel = 0; // canal padrão
        peerInfo.encrypt = false; //criptografia desativada

        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.print("Erro ao adicionar peer "); Serial.println(i);
        } else {
            Serial.print("Peer ");
            Serial.print(i);
            Serial.println(" adicionado com sucesso.");
        }
    }

    Serial.print("MAC da placa transmissora: ");
    Serial.println(WiFi.macAddress());
}

// Envia os dados via ESP-NOW
void sendData(){
    // Extrai o endereço destino do frame do CLP (bits 15..12)
    uint8_t enderecoDestino = (ultimoFrameCLP >> 12) & 0x0F;

    // Não envia se for para esta ESP
    if (enderecoDestino == MEU_ENDERECO) return;

    // Procura o peer correspondente
    for (size_t i = 0; i < numPeers; i++) {
        if (peers[i].endereco == enderecoDestino) {
            esp_err_t result = esp_now_send(peers[i].mac, (uint8_t *)&ultimoFrameCLP, sizeof(ultimoFrameCLP));

            if (result != ESP_OK) {
                Serial.print("Falha ao enviar dados para ESP ");
                Serial.println(enderecoDestino);
            } else {
                Serial.print("Frame enviado para ESP ");
                Serial.println(enderecoDestino);
            }
            break; // já enviou
        }
    }
}
