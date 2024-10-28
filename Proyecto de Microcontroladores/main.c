/* 
 * Proyecto de Microcontroladores.c 
 *	
 * Created: 10/14/2024 5:09:27 PM
 * Hora actual: 4:42pm
 */ 

#define F_CPU 16000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include "LCD.h"

uint8_t estado_anterior;
unsigned int seed = 1;
volatile unsigned int millis_count = 0;

//Variables del juego
uint8_t secuencia[30];
uint8_t count = 0;
uint8_t ronda = 1;
bool start = false;

void setup_timer1() {
	// Configura el Timer1 en modo CTC (Clear Timer on Compare Match)
	TCCR1A = 0;               // Normal mode
	TCCR1B = (1 << WGM12) | (1 << CS11); // Prescaler 8, modo CTC
	OCR1A = 1999;             // Generará una interrupción cada 1 ms con un prescaler de 8 y F_CPU a 16MHz

	// Habilita la interrupción por comparación
	TIMSK1 = (1 << OCIE1A);
}

unsigned int millis() {
	unsigned int ms;

	// Deshabilita las interrupciones para leer millis_count de manera segura
	cli();
	ms = millis_count;
	sei();

	return ms;
}

void configurar_pin_change_interrupt() {
	PCICR |= (1 << PCIE2);  // Habilitar interrupción para PCINT[23:16] (Puerto D)
	PCMSK2 |= 0xFF;         // Habilitar interrupciones por cambio de pin en todos los pines PD0-PD7
	estado_anterior = PIND;  // Guardar el estado inicial de los pines
}

void prenderLed(uint8_t pin, bool longer){
	switch(pin){
		case 0:
			PORTB |= (1 << PORTB2);  // Encender PB2
			break;
		case 1:
			PORTB |= (1 << PORTB1);  // Encender PB1
			break;
		case 2:
			PORTB |= (1 << PORTB2) | (1 << PORTB1);  // Encender PB2 y PB1
			break;
		case 3:
			PORTB |= (1 << PORTB0);  // Encender PB0
			break;
		case 4:
			PORTB |= (1 << PORTB2) | (1 << PORTB0);  // Encender PB2 y PB0
			break;
		case 5:
			PORTB |= (1 << PORTB1) | (1 << PORTB0);  // Encender PB1 y PB0
			break;
		case 6:
			PORTB |= (1 << PORTB2) | (1 << PORTB1) | (1 << PORTB0);  // Encender PB2, PB1 y PB0
			break;
		
	}
	if (!longer)
		_delay_ms(250);
	else
		_delay_ms(2000);
	PORTB &= ~(1 << PORTB2);  // Apagar PB2
	PORTB &= ~(1 << PORTB1);  // Apagar PB1
	PORTB &= ~(1 << PORTB0);  // Apagar PB0
}


void generarSecuencia() {
	seed = millis();
	
	srand(seed);  // Inicializar la semilla

	for (int i = 0; i < sizeof(secuencia); i++) {
		secuencia[i] = (rand() % 7) + 1;  // Generar un número aleatorio entre 1 y 7
	}
}

//Reproduce en orden la secuencia según el limite superior (ronda)
void reproducirSecuencia() {
	char rondaS[] = "    Ronda ";
	sprintf(rondaS, "%s%d", rondaS, ronda);
	lcd_clear();
	lcd_goto_xy(0, 0);
	lcd_write_word("Presta atencion!");
	lcd_goto_xy(1, 0);
	lcd_write_word(rondaS);
	_delay_ms(500);
	
	for(int i = 0; i < ronda+2;i++) {
		if (secuencia[i] != 0) {
			prenderLed(secuencia[i]-1, false);
			_delay_ms(350);
		}
		else
			break;
	}
	lcd_goto_xy(0, 0);
	lcd_write_word("       Go!      ");
}

void verificarBotonSecuencia(uint8_t pin)
{
	//lcd_clear();
	lcd_goto_xy(0, 0);
	if (pin == secuencia[count]-1)
	{
		lcd_write_word("     Bien!      ");
		prenderLed(pin, false);
		count++;
		if (count == ronda+2) {
			ronda ++;
			count = 0;
			reproducirSecuencia();
		}
	} else
	{
		lcd_write_word("   GAME OVER!   ");
		prenderLed(secuencia[count]-1, true);
		_delay_ms(1000);
		perderJuego();
		
		lcd_clear();
		lcd_goto_xy(0, 0);
		lcd_write_word("   Simon Dice");
		lcd_goto_xy(1, 0);
		lcd_write_word("<--- Press Start");
	}
	
}

void perderJuego() {
	count = 0;
	ronda = 1;
	start = false;
	generarSecuencia();
}



int main(void)
{
	DDRB = 0b00001111;    // Configurar PB0-PB3 como salidas
	PORTB = 0b00000000;   // Apagar salidas en PB0-PB3

	DDRD = 0b00000000;    // Configurar PD0-PD7 como entradas
	PORTD = 0xFF;         // Activar pull-ups internos para PD0-PD7

	setup_timer1();
	sei();                // Habilitar interrupciones globales
	configurar_pin_change_interrupt();  // Configurar interrupciones por cambio de pin
	

	lcd_init();
	lcd_clear();
	lcd_goto_xy(0, 0);
	lcd_write_word("   Simon Says");
	lcd_goto_xy(1, 0);
	lcd_write_word("<--- Press Start");
	
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
				if(i == 7)
				{
					if (start == false)
					{
						start = true;
						generarSecuencia();
						reproducirSecuencia();
					}
				}
				else if (start)
					verificarBotonSecuencia(i);
			}
		}
	}
}

ISR(TIMER1_COMPA_vect) {
	millis_count++; // Incrementa el conteo de milisegundos
}