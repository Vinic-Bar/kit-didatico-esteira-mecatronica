// Importando arquivos secundários
#include "globals.h" // Variáveis globais
#include "PWM.h"     // Gerar PWM
#include "Encoder.h" // Leitura do Encoder
#include "CLP_TCP.h" // Comunicação com o CLP via TCP/IP
#include "ESPNOW.h"  // Comunicação entre ESPs via ESPNOW

// Delcarando variáveis a serem utilizadas
struct_message myData;
uint16_t ultimoFrameCLP = 0;
bool novoFrameCLP = false;
bool estadoMotor0 = false;
uint8_t pwm0 = 0;
bool direcao0 = false;

uint16_t ultimoFrameRPM = 0;
bool novoFrameRPM = false;

// Setup (roda 1 vez)
void setup() {
  Serial.begin(115200);          // Inicia a comunicação serial

  setupPWM();                    // Configura o PWM
  setupEncoder();                // Inicializa o encoder
  setupTCP();                    //Es chama setup do TCP e conecta no WIFI
  setupESPNOW();                 // Configura a comunicação ESP-NOW

  pwm0 = 100; // Para iniciar a esteira com DC do PWM de 100%
  estadoMotor0 = true;
  direcao0 = false;

  Serial.println("Sistema de Controle de Motor DC com Encoder + ESP-NOW");
  updatePWM(pwm0);
  Serial.println("PWM iniciado em 100%");
}

void loop() {

  loopTCP(); // Loop da comunicação TCP com o CLP
  
  updatePWM(pwm0); // Atualizar o valor do PWM

    // Se o último frame recebido não era para esta ESP, encaminha via ESP-NOW
  if (novoFrameCLP && ultimoFrameCLP >> 12 != MEU_ENDERECO) {
      myData.frame = ultimoFrameCLP;
      sendData();
      Serial.println("Frame encaminhado via ESP-NOW para outra ESP.");
  }
  
  delay(100);                    
}