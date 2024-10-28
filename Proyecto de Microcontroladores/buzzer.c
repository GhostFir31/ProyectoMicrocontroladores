// buzzer.c
#include "buzzer.h"
#include <util/delay.h>  // Para _delay_ms

void buzzer_init() {
	// Configurar el pin PB1 como salida
	DDRB |= (1 << BUZZER_PIN);

	// Configurar Timer1 en modo CTC para generar frecuencias específicas
	TCCR1A = 0;                             // Modo normal
	TCCR1B = (1 << WGM12) | (1 << CS11);    // CTC con preescaler de 8
}

void buzzer_tone(uint16_t frequency) {
	// Calcular el valor de OCR1A para la frecuencia deseada
	OCR1A = (F_CPU / (2 * 8 * frequency)) - 1;
	TCCR1A |= (1 << COM1A0); // Toggle OC1A on Compare Match para generar PWM en PB1
}

void buzzer_off() {
	TCCR1A &= ~(1 << COM1A0); // Apagar el sonido
}
