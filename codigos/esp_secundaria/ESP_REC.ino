// Importando arquivos secundários
#include "globals.h" // Variáveis globais
#include "PWM.h"     // Gerar PWM
#include "Encoder.h" // Leitura do Encoder
#include "ESPNOW.h"  // Comunicação entre ESPs via ESPNOW

// Delcarando variáveis a serem utilizadas
struct_message myData;
uint16_t ultimoFrameCLP = 0;
bool novoFrameCLP = false;
bool estadoMotor0 = false;
uint8_t pwm0 = 0;
bool direcao0 = false;

unsigned long lastRPMsend = 0;

uint8_t macPrincipal[6] = {0x94, 0xB5, 0x55, 0xF8, 0xB6, 0x14};

// Setup (roda 1 vez)
void setup() {
  Serial.begin(115200);

  setupPWM();       // inicializa o PWM do motor
  setupEncoder();   // inicializa o encoder
  setupESPNOW();    // inicializa ESP-NOW

  // iniciar com o DC do PWM em 100%
  pwm0 = 100;
  estadoMotor0 = true;
  direcao0 = false;
  updatePWM(pwm0);
  
  Serial.println("ESP Receptora 1");
}

void loop() {
  
  //updatePWM(pwm0); //Atualiza valor do PWM

  if (novoFrameCLP) {
    Serial.print("Novo frame processado: ");
    Serial.println(ultimoFrameCLP);

    // Reseta a flag para evitar mensagens repetidas
    novoFrameCLP = false;
  }
  enviaRPM();

  delay(100); 
}

uint16_t montaFrameRPM(float rpm) {
  uint16_t frame = 0;

  uint8_t rpm8 = constrain((uint8_t)rpm, 0, 60);

  frame |= rpm8;                 // Bits 0..7  -> RPM
  frame |= (1 << 9);             // tipo = 01 (RPM)
  frame |= (1 << 11);            // Motor ON (opcional)
  frame |= (MEU_ENDERECO << 12); // Bits 12..15 -> endereço

  return frame;
}

void enviaRPM() {
  static unsigned long lastSend = 0;
  unsigned long now = millis();

  if (now - lastSend >= 5000) {
    lastSend = now;

    float rpm = getEncoderRPM();
    uint16_t frame = montaFrameRPM(rpm);

    esp_now_send(macPrincipal,
                 (uint8_t*)&frame,
                 sizeof(frame));

    Serial.print("RPM enviado: ");
    Serial.println(rpm);
  }
}
