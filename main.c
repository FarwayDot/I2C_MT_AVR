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

//Estado I2C
#define TWI_OK				0
#define TWI_ERROR			1

/*Declaración de funciones*/
uint8_t TWI_MT_Start_Condition(void);
uint8_t TWI_MT_Address(uint8_t address, uint8_t r_nw);
uint8_t TWI_MT_Data_Upload(uint8_t data);
void TWI_MT_Stop_Condition(void);
uint8_t TWI_MT_Re_Start_Condition(void);
uint8_t TWI_MT_Write_Data(uint8_t address, uint8_t n_data, uint8_t Trama[]);

/*Función principal*/
int main(void)
{	
		
	//Bit Rate Generator Formula 
	//TWBR = (uint8_t)(((F_CPU/(SCL_FREQ_KHZ*1000))-16)/(2*PRESCALER));
	
    while(1) 
    {	
		
    }
	
	return 0;
}

/************************************************************************/
/* START CONDITION : Inicia el TWI , y devuelve un valor de error si el 
	estado no es el correcto */
/************************************************************************/

uint8_t TWI_MT_Start_Condition(void)
{
	//Inicializamos el estado del TWI, si cambia, entonces ocurrirá un error
	uint8_t salida = TWI_OK;
	
	//Inicialización de la bandera, bit start y habilitamos el i2c. 
	TWCR = (1<<TWINT) | (1<<TWSTA) |(1<<TWEN);
	
	//Checkeamos si la bandera se activa
	while(!(TWCR & (1<<TWINT)));
	
	//Checkeamos el estado según la tabla del datasheet
	if((TWSR & 0xF8) != STA_START)
	{	
		//Cambiamos el estado del TWI
		salida = TWI_ERROR;
	}
	
	return salida;	
}


/************************************************************************/
/* DIRECCION Y R/nW: Carga la dirección(7bits) y si se quiere leer o escribir 
Nota: No se actualiza aún si se quiere leer*/
/************************************************************************/

uint8_t TWI_MT_Address(uint8_t address, uint8_t r_nw)
{
	
	uint8_t salida = TWI_OK;
	
	//Cargamos el valor de la dirección(1er) y write(2do)
	if(r_nw)
	{
		salida = TWI_ERROR;
	}
	else
	{	
		TWDR = (address<<1) & 0xFE ;
	}
	
	//Según datasheet se limpia la bandera después de cargar el valor
	TWCR = (1<<TWINT) | (1<<TWEN);
	
	//Checkeamos si la bandera se activa
	while(!(TWCR & (1<<TWINT))); 
		
	//Checkeamos si es el estado correcto
	if((TWSR & 0xF8) != STA_SLA_W_ACK)
	{
		salida = TWI_ERROR;
	}
	
	return salida;
}

/************************************************************************/
/* DATA (8bits): Cargar data de 8 bits*/
/************************************************************************/


uint8_t TWI_MT_Data_Upload(uint8_t data)
{
	uint8_t salida = TWI_OK;
	
	//Cargamos el valor de la data, nuestro caso, letra 'A'
	TWDR = data;
	//Limpiamos la bandera y seguimos el proceso
	TWCR = (1<<TWINT) | (1<<TWEN);
	
	//Esperamos la bander por enviar la data
	while(!(TWCR & (1<<TWINT)));
	
	 //Checkeamos si es la bandera del ACK del receptor
	if((TWSR & 0xF8) != STA_DATA_ACK)
	{
		salida = TWI_ERROR;
	}
	
	return salida;
}

/************************************************************************/
/* REPEATED START: En el casi que queremos seguir enviando*/
/************************************************************************/

uint8_t TWI_MT_Re_Start_Condition(void)
{
	uint8_t salida = TWI_OK;
	
	//Repeated start
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
	//Limpiamos la bandera
	while(!(TWCR & (1<<TWINT)));
	
	//Checkeamos si se env+ia el re-start
	if((TWSR & 0xF8) != STA_RE_START)
	{
		salida = TWI_ERROR;
	}
	
	return salida;
}

/************************************************************************/
/* STOP CONDITION: Detener el periférico TWI*/
/************************************************************************/

void TWI_MT_Stop_Condition(void)
{
	
	//Limpiamos la bandera y seguimos con el STOP condition
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	
}

uint8_t TWI_MT_Write_Data(uint8_t address, uint8_t n_data, uint8_t Trama[])
{
	uint8_t i = 0;
	uint8_t salida = TWI_OK;
	
	//Asignamos la salida al resultado del START CONDITION
	salida = TWI_MT_Start_Condition();
	//Evaluamos el start condition
	if(salida == TWI_ERROR)
	{
		salida = TWI_ERROR;
	}
	
	//Asignamos la salida al resultado de enviar el ADDRESS + R/nW
	salida = TWI_MT_Address(address,0);
	//Evaluamos el start condition
	if(salida == TWI_ERROR)
	{
		salida = TWI_ERROR;
	}
	
	//Transitiendo data
	for(i = 0; i<n_data; i++)
	{
		salida = TWI_MT_Data_Upload(Trama[i]);
		if(salida == TWI_ERROR)
		{
			break;
		}
	}
	
	if(salida == TWI_ERROR)
	{
		salida = TWI_ERROR;
	}
	
	//Deteniendo el I2C
	TWI_MT_Stop_Condition();
	
	return salida;
	
}