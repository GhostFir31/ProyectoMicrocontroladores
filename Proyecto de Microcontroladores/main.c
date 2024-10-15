/*
 * Proyecto de Microcontroladores.c
 *
 * Created: 10/14/2024 5:09:27 PM
 */ 

#define F_CPU 16000000L
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "LCD.h"

bool kpf = false;

char PressedKey(void){
	
	char key = 0;
	
	PORTD = 0b11110111;
	
	_delay_ms(1);
	
	if(!(PIND & (1<<PIND7))) key='1';
	if(!(PIND & (1<<PIND6))) key='4';
	if(!(PIND & (1<<PIND5))) key='7';
	if(!(PIND & (1<<PIND4))) key='C';

	if(key!=0){
		
		PORTD = 0b11110000;
		return key;
		
	}
	
	PORTD = 0b11111011;
	
	_delay_ms(1);
	
	if(!(PIND & (1<<PIND7))) key='2';
	if(!(PIND & (1<<PIND6))) key='5';
	if(!(PIND & (1<<PIND5))) key='8';
	if(!(PIND & (1<<PIND4))) key='0';
	
	if(key!=0){
		
		PORTD = 0b11110000;
		return key;
		
	}
	
	PORTD = 0b11111101;
	
	_delay_ms(1);
	
	if(!(PIND & (1<<PIND7))) key='3';
	if(!(PIND & (1<<PIND6))) key='6';
	if(!(PIND & (1<<PIND5))) key='9';
	if(!(PIND & (1<<PIND4))) key='=';
	
	if(key!=0){
		
		PORTD = 0b11110000;
		return key;
		
	}
	
	PORTD = 0b11111110;
	
	_delay_ms(1);
	
	if(!(PIND & (1<<PIND7))) key='+';
	if(!(PIND & (1<<PIND6))) key='-';
	if(!(PIND & (1<<PIND5))) key='x';
	if(!(PIND & (1<<PIND4))) key='/';
	
	PORTD = 0b11110000;
	return key;
	
}

int main(void)
{
	// Configurar pines del teclado
	DDRD |= 0b00001111;
	DDRD &= 0b00001111;
	PORTD |= 0b11110000;
	PORTD &= 0b11110000;
	// Habilitar interrupciones
	PCICR |= 0b00000100;
	PCMSK2 |= 0b11110000;
	sei();
	
	// Resetear calculadora
	void calculator_reset(void) {
		lcd_clear();
		lcd_goto_xy(0, 0);
		lcd_write_word("CALCULADORA...");
		lcd_goto_xy(1, 0);
	}
	
	lcd_init();
	calculator_reset();
	
	
	//Declaracion de Variables
	char pressedkey = 0;
	int num1 = 0;
	int num2 = 0;
	int num_temp = 0;
	float resultado = 0;
	char operacion = 0;
	bool operacion_nueva = false;
	char resultado_str[16];
	
	while (1)
	{
		if (kpf)
		{
			// Capturar la tecla presionada
			pressedkey = PressedKey();
			
			if (pressedkey >= '0' && pressedkey <= '9')
			{
				num_temp = num_temp * 10 + (pressedkey - '0');
				lcd_write_character(pressedkey);
				operacion_nueva = false;
			}
			else if (pressedkey == '+' || pressedkey == '-' || pressedkey == 'x' || pressedkey == '/')
			{
				if (operacion_nueva) {
					num1 = resultado;
					} else {
					num1 = num_temp;
				}
				
				operacion = pressedkey;
				lcd_write_character(pressedkey);
				num_temp = 0;
				operacion_nueva = true;
			}
			// Cuando se presiona '=' se realiza la operación
			else if (pressedkey == '=')
			{
				num2 = num_temp;
				
				switch (operacion)
				{
					case '+':
					resultado = num1 + num2;
					break;
					case '-':
					resultado = num1 - num2;
					break;
					case 'x':
					resultado = num1 * num2;
					break;
					case '/':
					if (num2 != 0) {
						resultado = (float)num1 / num2;
						} else {
						lcd_clear();
						lcd_write_word("Error");
						continue;
					}
					break;
				}
				
				// Mostrar el resultado en el LCD
				lcd_clear();
				lcd_goto_xy(0, 0);
				lcd_write_word("Resultado:");
				
				sprintf(resultado_str, "%.2f", resultado);
				lcd_goto_xy(1, 0);
				lcd_write_word(resultado_str);
				
				// Guardar el resultado en num1 para la próxima operación
				num1 = resultado;
				num_temp = 0;
				operacion_nueva = true;
			}
			else if (pressedkey == 'C')
			{
				// Reiniciar la calculadora
				calculator_reset();
				num1 = 0;
				num2 = 0;
				resultado = 0;
				num_temp = 0;
				operacion = 0;
				// Volver al estado de nueva operación
				operacion_nueva = false;
			}

			_delay_ms(10);
			// Resetear la bandera de interrupción
			kpf = false;
		}
	}
}

ISR (PCINT2_vect)
{
	
	if(!kpf)
	{

		kpf=true;

	}
	
}
