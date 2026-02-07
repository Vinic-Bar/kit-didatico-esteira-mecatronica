#include <ESP32Encoder.h>

#define ENCODER_A 32          // Pino A do encoder
#define ENCODER_B 33          // Pino B do encoder
#define PULSOS_POR_REV 1133    // pulsos por volta 11 PPR * redução de 103

ESP32Encoder encoder;         // objeto do encoder

// Variáveis para cálculo da velocidade
long lastCount = 0;           // contagem anterior
unsigned long lastTime = 0;   // tempo da última medição
float currentRPM = 0;         // velocidade atual

// Configura o encoder
void setupEncoder() {
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  encoder.attachHalfQuad(ENCODER_A, ENCODER_B); // modo meia-quadratura
  encoder.clearCount();                         // zera contagem inicial
  lastTime = millis();                          // salva tempo atual

  Serial.println("Encoder iniciado");
}

// Calcula a velocidade em RPM
float getEncoderRPM() {
  unsigned long now = millis();
  float delta_t = (now - lastTime) / 1000.0; // tempo desde a última leitura (s)

  if (delta_t >= 0.2) { // atualiza a cada 200 ms
    long count = encoder.getCount();       // lê a contagem atual
    long delta = count - lastCount;        // diferença de pulsos
    float revolucoes = (float)delta / PULSOS_POR_REV; // voltas feitas
    currentRPM = revolucoes * (60.0 / delta_t);       // converte para RPM

    lastCount = count;    // atualiza a contagem
    lastTime = now;       // atualiza o tempo
  }

  return currentRPM; // retorna o valor calculado
}

// Zera a contagem do encoder
void resetEncoder() {
  encoder.clearCount();
  lastCount = 0;
  currentRPM = 0;
}