// Manter todas as variáveis globais aqui (fica mais fácil)
// Eu tava me complicando todo sem isso kkk

#ifndef GLOBALS_H // define Globals_H se não estiver definido ainda
#define GLOBALS_H

#define MEU_ENDERECO 0  // Endereço da ESP

// Estrutura de dados (O ESPNOW só envia em blocos, por isso o struct)
typedef struct struct_message {
    uint16_t frame; // WORD de controle do CLP
} struct_message;

// Variáveis globais
extern struct_message myData;    // Frame enviado via ESP-NOW

// Memórias do último frame recebido do CLP
extern uint16_t ultimoFrameCLP;  // Último frame recebido
extern bool novoFrameCLP;        // Flag novo frame

// Variáveis de controle do motor da ESP0
extern bool estadoMotor0;        // Motor ligado/desligado
extern uint8_t pwm0;             // PWM atual (0-255 ou 0-100%)
extern bool direcao0;            // Direção do motor (true/false)

// Variáveis de recebimento do RPM
extern uint16_t ultimoFrameRPM; // último frame de RPM recebido
extern bool novoFrameRPM;       // flag novo frame RPM

#endif // Essa linha é pra evitar sobreleitura