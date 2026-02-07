#include <Arduino.h>
#include "globals.h"

#define PWM_PIN 25          // Pino do motor (saída PWM)
#define PWM_FREQ 5000       // Frequência do PWM (Hz)
#define PWM_RESOLUTION 8    // Resolução (8 bits = 0–255)

// Configura o PWM
void setupPWM() {
  ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION); 
  Serial.println("PWM inicializado!"); 
}

// Atualiza o valor do PWM (0 a 100%)
void updatePWM(int percent) {
  percent = constrain(percent, 0, 100); // limitar valor de 0 a 100

  // Converter % para duty cycle de 8 bits
  int dutyCycle = map(percent, 0, 100, 0, 255);
  ledcWrite(PWM_PIN, dutyCycle);  // Aplica o PWM no canal

  Serial.printf("PWM atualizado para: %d%% (%d/255)\n", percent, dutyCycle);
}