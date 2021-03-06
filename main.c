/*
 * I2C_MT.c
 *
 * Created: 10/01/2021 22:24:34
 * Author : USER
 */ 

/*Inclusi�n de librer�as */
#define F_CPU	16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

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

#define ADDRESS				1

//Estado I2C
#define TWI_OK				0
#define TWI_ERROR			1

/*Declaraci�n de funciones*/

//I2C
uint8_t TWI_MT_Start_Condition(void);
uint8_t TWI_MT_Address(uint8_t address, uint8_t r_nw);
uint8_t TWI_MT_Data_Upload(uint8_t data);
void TWI_MT_Stop_Condition(void);
uint8_t TWI_MT_Re_Start_Condition(void);
uint8_t TWI_MT_Write_Data(uint8_t address, uint8_t n_data, uint8_t Trama[]);

//UART
void UART_Config();
void UART_Caracter(uint8_t c);
void UART_Cadena(uint8_t *str);

//Variables

//I2C
uint8_t TWI_Buffer[20] = "Mi nombre es Jean";

//UART
uint8_t UART_Msg_1[20] = "Error en el TWI\n";

/*Funci�n principal*/
int main(void)
{	

	uint8_t Salida = TWI_OK;
	
	UART_Config();
			
	//Bit Rate Generator Formula 
	TWBR = (uint8_t)(((F_CPU/(SCL_FREQ_KHZ*1000))-16)/(2*PRESCALER));
	
	//Condiciones iniciales
		
    while(1) 
    {	
		Salida = TWI_MT_Write_Data(ADDRESS,20,TWI_Buffer);
		
		if(Salida == TWI_ERROR)
		{
			UART_Cadena(UART_Msg_1);
			_delay_ms(1000);	
		}
		
		_delay_ms(100);
    }
	
	return 0;
}


/************************************************************************/
/* START CONDITION : Inicia el TWI , y devuelve un valor de error si el 
	estado no es el correcto */
/************************************************************************/
uint8_t TWI_MT_Start_Condition(void)
{
	//Inicializamos el estado del TWI, si cambia, entonces ocurrir� un error
	uint8_t salida = TWI_OK;
	
	//Inicializaci�n de la bandera, bit start y habilitamos el i2c. 
	TWCR = (1<<TWINT) | (1<<TWSTA) |(1<<TWEN);
	
	//Checkeamos si la bandera se activa
	while(!(TWCR & (1<<TWINT)));
	
	//Checkeamos el estado seg�n la tabla del datasheet
	if((TWSR & 0xF8) != STA_START)
	{	
		//Cambiamos el estado del TWI
		salida = TWI_ERROR;
	}
	
	return salida;	
}


/************************************************************************/
/* DIRECCION Y R/nW: Carga la direcci�n(7bits) y si se quiere leer o escribir 
Nota: No se actualiza a�n si se quiere leer*/
/************************************************************************/
uint8_t TWI_MT_Address(uint8_t address, uint8_t r_nw)
{
	
	uint8_t salida = TWI_OK;
	
	//Cargamos el valor de la direcci�n(1er) y write(2do)
	if(r_nw)
	{
		salida = TWI_ERROR;
	}
	else
	{	
		TWDR = (address<<1) & 0xFE ;
	}
	
	//Seg�n datasheet se limpia la bandera despu�s de cargar el valor
	TWCR = (1<<TWINT) | (1<<TWEN);
	
	//Checkeamos si la bandera se activa
	while(!(TWCR & (1<<TWINT))); 
	
	if(r_nw)
	{
		salida = TWI_ERROR;
	}
	else
	{
		//Checkeamos si es el estado correcto
		if((TWSR & 0xF8) != STA_SLA_W_ACK)
		{
			salida = TWI_ERROR;
		}			
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
/* STOP CONDITION: Detener el perif�rico TWI*/
/************************************************************************/
void TWI_MT_Stop_Condition(void)
{
	
	//Limpiamos la bandera y seguimos con el STOP condition
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	
}


/************************************************************************/
/* TRANSMITING DATA */
/************************************************************************/
uint8_t TWI_MT_Write_Data(uint8_t address, uint8_t n_data, uint8_t *ptr_buffer)
{
	uint8_t i = 0;
	uint8_t salida = TWI_OK;
	
	//Asignamos la salida al resultado del START CONDITION
	salida = TWI_MT_Start_Condition();
	//Evaluamos el start condition
	if(salida == TWI_ERROR)
	{
		return salida;	
	}
	
	//Asignamos la salida al resultado de enviar el ADDRESS + R/nW
	salida = TWI_MT_Address(address,0);
	//Evaluamos el start condition
	if(salida == TWI_ERROR)
	{
		return salida;
	}
	
	//Transitiendo data
	for(i = 0; i<n_data; i++)
	{
		salida = TWI_MT_Data_Upload(*(ptr_buffer + i));
		if(salida == TWI_ERROR)
		{
			break;
		}
	}
	
	if(salida == TWI_ERROR)
	{
		return salida;
	}
	
	//Deteniendo el I2C
	TWI_MT_Stop_Condition();
	
	return salida;
	
}

void UART_Config()
{
	//Tx Salida
	DDRD |= (1<<1);
	PORTD &= ~(1<<0);
	
	//Habilitar Tx
	UCSR0B |= (1<<TXEN0);
	
	//Modo Asincrono
	UCSR0C &= ~((1<<UMSEL01) | (1<<UMSEL00));
	//No paridad
	UCSR0C &= ~((1<<UPM01) | (1<<UPM00));
	//8bits
	UCSR0B &= ~(1<<UCSZ02);
	UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);
	//BaudRate 9600
	UBRR0 = 103; //fosc = 16Mhz
}

void UART_Caracter(uint8_t c)
{
	//Verificar el buffer si esta vacio: 1 Vacio, 0 Lleno
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}

void UART_Cadena(uint8_t *str)
{
	while(*str != '\0')
	{
		UART_Caracter(*str);
		str++; //Corrimiento de la direccion de memoria
	}
}