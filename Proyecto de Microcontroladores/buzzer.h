// buzzer.h
#ifndef BUZZER_H
#define BUZZER_H

#include <avr/io.h> // Incluir la biblioteca de AVR para definiciones de registros

// Define el pin del buzzer
#define BUZZER_PIN PB1

// Definir las frecuencias de las notas musicales

#define NOTE_C2 65
#define NOTE_D2 73
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_G2 98
#define NOTE_A2 110
#define NOTE_B2 123


// Prototipos de funciones
void buzzer_init();
void buzzer_tone(uint16_t frequency);
void buzzer_off();

#endif // BUZZER_H
