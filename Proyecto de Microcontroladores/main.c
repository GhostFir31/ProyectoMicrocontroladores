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
#include "buzzer.h"
#include "EEPROM.h"

uint8_t estado_anterior;
unsigned int seed = 1;
volatile unsigned int millis_count = 0;

//Variables del juego
uint8_t secuencia[30];
uint8_t count = 0;
uint8_t ronda = 1;
uint16_t bestScore = 0;
bool start = false;
bool scoreScreen = false;
char msgS[18] = "<-Play  Score:";


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
			buzzer_tone(NOTE_C2); // Toca Do (C3)
			_delay_ms(500);
			buzzer_off();
			_delay_ms(100);
			break;
		case 1:
			PORTB |= (1 << PORTB3);  // Encender PB3
			buzzer_tone(NOTE_D2); // Toca Re (D3)
			_delay_ms(500);
			buzzer_off();
			_delay_ms(100);
			break;
		case 2:
			PORTB |= (1 << PORTB2) | (1 << PORTB3);  // Encender PB2 y PB3
			buzzer_tone(NOTE_E2); // Toca Mi (E3)
			_delay_ms(500);
			buzzer_off();
			_delay_ms(100);
			break;
		case 3:
			PORTB |= (1 << PORTB0);  // Encender PB0
			buzzer_tone(NOTE_F2); // Toca Fa (F3)
			_delay_ms(500);
			buzzer_off();
			_delay_ms(100);
			break;
		case 4:
			PORTB |= (1 << PORTB2) | (1 << PORTB0);  // Encender PB2 y PB0
			buzzer_tone(NOTE_G2); // Toca Sol (G3)
			_delay_ms(500);
			buzzer_off();
			_delay_ms(100);
			break;
		case 5:
			PORTB |= (1 << PORTB3) | (1 << PORTB0);  // Encender PB3 y PB0
			buzzer_tone(NOTE_A2); // Toca La (A3)
			_delay_ms(500);
			buzzer_off();
			_delay_ms(100);
			break;
		case 6:
			PORTB |= (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB0);  // Encender PB2, PB3 y PB0
			buzzer_tone(NOTE_B2); // Toca Si (B3)
			_delay_ms(500);
			buzzer_off();
			_delay_ms(100);
			break;
		
	}
	if (!longer)
		_delay_ms(250);
	else
		_delay_ms(2000);
	PORTB &= ~(1 << PORTB2);  // Apagar PB2
	PORTB &= ~(1 << PORTB3);  // Apagar PB3
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
		_delay_ms(1);//estoy intentado arreglar un bug con la pantalla despues de completar la secuencia 
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
		printMainMenu();
	}
	
}

void perderJuego() {
	updateTopScores(ronda);
	bestScore = EEPROM_read_word(0);
	
	count = 0;
	ronda = 1;
	start = false;
	generarSecuencia();
	
	
}

void printMainMenu()
{
	snprintf(msgS, sizeof(msgS), "<-Play  Score:%02u", bestScore);
	lcd_clear();
	lcd_goto_xy(0, 0);
	lcd_write_word("   Simon Says");
	lcd_goto_xy(1, 0);
	lcd_write_word(msgS);
}


void updateTopScores(uint16_t newScore) {
	// Leer los puntajes actuales
	uint16_t score0 = EEPROM_read_word(0);   // Dirección 0
	uint16_t score1 = EEPROM_read_word(2);   // Dirección 2
	uint16_t score2 = EEPROM_read_word(4);   // Dirección 4
	uint16_t score3 = EEPROM_read_word(6);   // Dirección 6

	// Comparar con el puntaje más alto y reordenar si es necesario
	if (newScore > score0) {
		// Nuevo puntaje es el mejor, desplazar los demás
		EEPROM_write_word(6, score2);
		EEPROM_write_word(4, score1);
		EEPROM_write_word(2, score0);
		EEPROM_write_word(0, newScore);
		} else if (newScore > score1) {
		// Nuevo puntaje es el segundo mejor, desplazar los demás
		EEPROM_write_word(6, score2);
		EEPROM_write_word(4, score1);
		EEPROM_write_word(2, newScore);
		} else if (newScore > score2) {
		// Nuevo puntaje es el tercero mejor
		EEPROM_write_word(6, score2);
		EEPROM_write_word(4, newScore);
		} else if (newScore > score3) {
		// Nuevo puntaje es el cuarto mejor
		EEPROM_write_word(6, newScore);
	}
}


void printScores()
{
	

	// Valores para los números a imprimir
	uint16_t S1 = EEPROM_read_word(0), S2 = EEPROM_read_word(2);
	uint16_t S3 = EEPROM_read_word(4), S4 = EEPROM_read_word(6);

	// Crear cadenas para cada línea
	char line1[16]; // Primera línea del LCD
	char line2[16]; // Segunda línea del LCD
									
	snprintf(line1, sizeof(line1), " #1:%02u    #2:%02u", S1, S2);
	snprintf(line2, sizeof(line2), " #3:%02u    #4:%02u", S3, S4);

	// Mostrar cadenas en la pantalla LCD
	lcd_clear();
	lcd_goto_xy(0, 0); // Primera línea
	lcd_write_word(line1);
	lcd_goto_xy(1, 0); // Segunda línea
	lcd_write_word(line2);
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
	
	
	/*
	address 0 es best score
	address 1 es 2nd
	address 2 es 3nd
	address 3 es 4nd
	*/
	//Reiniciar scores
	/*EEPROM_write_word(0,0);
	EEPROM_write_word(1,0);
	EEPROM_write_word(2,0);
	EEPROM_write_word(3,0);*/
	
	bestScore = EEPROM_read_word(0);
	
	//16 caracteres por mensaje, 2 caracteres por num
	
	lcd_init();
	printMainMenu();
	
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
					if (start == false && !scoreScreen)
					{
						start = true;
						generarSecuencia();
						reproducirSecuencia();
					}
				}
				else if (start)
				{
					verificarBotonSecuencia(i);
				}
				else if(i == 0)
				{
					if (scoreScreen == false)
					{
						scoreScreen = true;
						printScores();
					} else
					{
						scoreScreen = false;
						printMainMenu();
					}
				}
				else if(i == 6)
				{
					EEPROM_write_word(0,0);
					EEPROM_write_word(2,0);
					EEPROM_write_word(4,0);
					EEPROM_write_word(6,0);
					bestScore = EEPROM_read_word(0);
					printMainMenu();
				}
			}
		}
	}
}

ISR(TIMER1_COMPA_vect) {
	millis_count++; // Incrementa el conteo de milisegundos
}