/*
 * Proyecto de Microcontroladores.c
 *
 * Created: 10/14/2024 5:09:27 PM
 */ 

#define F_CPU 16000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "LCD.h"

uint8_t estado_anterior;

void configurar_pin_change_interrupt() {
	PCICR |= (1 << PCIE2);  // Habilitar interrupción para PCINT[23:16] (Puerto D)
	PCMSK2 |= 0xFF;         // Habilitar interrupciones por cambio de pin en todos los pines PD0-PD7
	estado_anterior = PIND;  // Guardar el estado inicial de los pines
}

void prenderLed(uint8_t pin){
	
	switch(pin){
		case 0:	PORTB |= (1 << PORTB2);  // Encender PB2
		_delay_ms(1000);
		PORTB &= ~(1 << PORTB2);  // Apagar PB2
		break;
		case 1:	PORTB |= (1 << PORTB1);  // Encender PB1
		_delay_ms(1000);
		PORTB &= ~(1 << PORTB1);  // Apagar PB1
		break;
		case 2:	PORTB |= (1 << PORTB2) | (1 << PORTB1);  // Encender PB2 y PB1
		_delay_ms(1000);
		PORTB &= ~(1 << PORTB2);  // Apagar PB2
		PORTB &= ~(1 << PORTB1);  // Apagar PB1
		break;
		case 3:	PORTB |= (1 << PORTB0);  // Encender PB0
		_delay_ms(1000);
		PORTB &= ~(1 << PORTB0);  // Apagar PB0
		break;
		case 4:	PORTB |= (1 << PORTB2) | (1 << PORTB0);  // Encender PB2 y PB0
		_delay_ms(1000);
		PORTB &= ~(1 << PORTB2);  // Apagar PB2
		PORTB &= ~(1 << PORTB0);  // Apagar PB0
		break;
		case 5:	PORTB |= (1 << PORTB1) | (1 << PORTB0);  // Encender PB1 y PB0
		_delay_ms(1000);
		PORTB &= ~(1 << PORTB1);  // Apagar PB1
		PORTB &= ~(1 << PORTB0);  // Apagar PB0
		break;
		case 6:	PORTB |= (1 << PORTB2) | (1 << PORTB1) | (1 << PORTB0);  // Encender PB2, PB1 y PB0
		_delay_ms(1000);
		PORTB &= ~(1 << PORTB2);  // Apagar PB2
		PORTB &= ~(1 << PORTB1);  // Apagar PB1
		PORTB &= ~(1 << PORTB0);  // Apagar PB0
		break;
	}
}


int main(void)
{
	DDRB = 0b00001111;    // Configurar PB0-PB3 como salidas
	PORTB = 0b00000000;   // Apagar salidas en PB0-PB3

	DDRD = 0b00000000;    // Configurar PD0-PD7 como entradas
	PORTD = 0xFF;         // Activar pull-ups internos para PD0-PD7

	sei();                // Habilitar interrupciones globales
	configurar_pin_change_interrupt();  // Configurar interrupciones por cambio de pin

	lcd_init();
	lcd_clear();
	lcd_goto_xy(0, 0);
	lcd_write_word("Simon Says");
	lcd_goto_xy(1, 0);
	lcd_write_word("Press Start..");

	while (1)
	{
		// El microcontrolador esperará las interrupciones y responderá a ellas
	}
}

// ISR para "Pin Change Interrupt" del puerto D (PCINT[23:16])
ISR(PCINT2_vect) {
	uint8_t estado_actual = PIND;  // Leer el estado actual de los pines PD0-PD7
	uint8_t cambio = estado_anterior ^ estado_actual;  // Detectar qué pines cambiaron
	estado_anterior = estado_actual;  // Actualizar el estado anterior

	for (uint8_t i = 0; i < 8; i++) {
		if (cambio & (1 << i)) {  // Si el pin PDx ha cambiado de estado
			if (!(estado_actual & (1 << i))) {  // Si el pin PDx está en nivel bajo (botón presionado)
				prenderLed(i);
			}
		}
	}
}
