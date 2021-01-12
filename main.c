/*
 * I2C_MT.c
 *
 * Created: 10/01/2021 22:24:34
 * Author : USER
 */ 

/*Inclusión de librerías */
#define F_CPU	16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

/*Definiciones*/

//Estados para el master i2c transmisor
#define STA_START			0x08
#define STA_RE_START		0x10
#define STA_SLA_W_ACK		0x18
#define STA_SLA_W_NACK		0x20
#define STA_DATA_ACK		0x28
#define STA_DATA_NACK		0x30

//Bit Rate Generator
#define PRESCALER			1
#define SCL_FREQ_KHZ		100			

//Led
#define LED1				5

//I2C
#define DDR_TW				DDRC
#define PORT_TW				PORTC
#define SDA					4
#define SCL					5

/*Función principal*/
void main(void)
{	
		
	//Led solo de indicador
	DDRB |= (1<<LED1); //Salida
	PORTB &= ~(1<<LED1); //Low
	
	//Iniciamos pines I2C
	DDR_TW |= (1<<SCL) | (1<<SDA);
	PORT_TW &= ~((1<<SCL)|(1<<SDA));
	
	//Bit Rate Generator Formula 
	TWBR = (uint8_t)(((F_CPU/(SCL_FREQ_KHZ*1000))-16)/(2*PRESCALER));
	
	//TWSR se deja como está, prescaler 1 no necesita modificacion, lo demás son para los estados
		
    while (1) 
    {	
		/************************************************************************/
		/* BIT START */
		/************************************************************************/
		
		//Inicialización de la bandera, bit start y habilitamos el i2c
		TWCR = (1<<TWINT) | (1<<TWSTA) |(1<<TWEN);
		
		//Bandera para el START
		while(!(TWCR & (1<<TWINT))); 
		
		//Checkeamos el estado según la tabla del datasheet
		if((TWSR & 0xF8) != STA_START)
		{
			PORTB |= (1<<LED1);
		}	
		
		/************************************************************************/
		/* DIRECCION Y R/nW */
		/************************************************************************/
		
		//Cargamos el valor de la dirección(1) y write(0)
		TWDR = 0b00000010;
		//Según datasheet se limpia la bandera después de cargar el valor 
		TWCR = (1<<TWINT) | (1<<TWEN);
		
		while(!(TWCR & (1<<TWINT))); 
		
		if((TWSR & 0xF8) != STA_SLA_W_ACK)
		{
			PORTB |= (1<<LED1);
		}
	
		/************************************************************************/
		/* DATA (8bits) */
		/************************************************************************/
	
		//Cargamos el valor de la data, nuestro caso, letra 'A'
		TWDR = 0b01000001;
		//Limpiamos la bandera y seguimos el proceso
		TWCR = (1<<TWINT) | (1<<TWEN);
		
		//Esperamos la bander por enviar la data
		while(!(TWCR & (1<<TWINT))); 
			
		//Checkeamos si es la bandera del ACK del receptor
		if((TWSR & 0xF8) != STA_DATA_ACK)
		{
			PORTB |= (1<<LED1);
		}
		
		/************************************************************************/
		/* STOP */
		/************************************************************************/
		
		////Limpiamos la bandera y seguimos con el STOP condition	
		//TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
		//
		////Esperamos la bandera
		//while(!(TWCR & (1<<TWINT)));
		//
		//if((TWSR & 0xF8) != STA_DATA_NACK)
		//{
			//PORTB |= (1<<LED1);
		//}
		
		/************************************************************************/
		/* REPEATED START */
		/************************************************************************/
		
		//Repeated start
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA);	
		//Limpiamos la bandera
		while(!(TWCR & (1<<TWINT)));
		
		//Checkeamos si se env+ia el re-start
		if((TWSR & 0xF8) != STA_RE_START)
		{
			PORTB |= (1<<LED1);
		}
		
    }
}

