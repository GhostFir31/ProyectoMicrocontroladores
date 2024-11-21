#ifndef EEPROM_H_
#define EEPROM_H_

#include <avr/io.h>

unsigned char EEPROM_read(unsigned int);
void EEPROM_write(unsigned int, unsigned char);

unsigned char EEPROM_read(unsigned int uiAddress)
{
	//Wait for completion of previous write
	while(EECR & (1<<EEPE));
	//Set up address register
	EEAR = uiAddress;
	//Start EEPROM read by writing EERE
	EECR |= (1<<EERE);
	//Return data from Data Register
	return EEDR;
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	//Wait for completion of previous write
	while (EECR & (1<<EEPE));
	//Set up address and Data Registers
	EEAR = uiAddress;
	EEDR = ucData;
	//Write logical one to EEMPE
	EECR |= (1<<EEMPE);
	//Start EEPROM write by setting EEPE
	EECR |= (1<<EEPE);
}

void EEPROM_write_byte(unsigned int uiAddress, uint16_t word)
{
	EEPROM_write(uiAddress, (uint8_t)(word & 0xFF));          // Byte menos significativo
	//EEPROM_write(uiAddress + 1, (uint8_t)((word >> 8) & 0xFF)); // Byte más significativo
}

uint8_t EEPROM_read_byte(unsigned int uiAddress)
{
	uint8_t low_byte = EEPROM_read(uiAddress);
	//uint8_t high_byte = EEPROM_read(uiAddress + 1);
	return low_byte;
}


#endif //EEPROM_H_