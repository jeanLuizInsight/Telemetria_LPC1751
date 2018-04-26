/*************************************************************************************************
* Sistema de telemetria por rede GSM.
* Arquivo: rs485.h
* Programador: Jean Luiz Zanatta
* Data: 06/08/2014
*************************************************************************************************/

#ifndef	RS485_H
#define RS485_H

//includes
#include <stdint.h>
#include <LPC17xx.h>

//defines
#define MAXDEVNUM 20
#define RXENABLE	0x400
#define TXENABLE	0x800000
#define UART485		0			  	// Seleção da UART para interface RS485.
#define BAUD485		57600			// Baudrate da interface RS485.

#if UART485
	#define UART485BUFFER	_UART1Buffer
	#define UART485COUNT	_UART1Count
#else
	#define UART485BUFFER	_UART0Buffer
	#define UART485COUNT	_UART0Count
#endif

//variaveis globais
extern unsigned char  _devAdd[MAXDEVNUM];
extern unsigned char  _devCount;

//protótipo de funções
void Gera_CRC (void);
void DE_485 (void);
void REN_485 (void);
char Verifica_CRC (void);
void RS485_io_conf (void);
int  Device_search (void);
void Processa_CRC_offline (void);
void Send485_string (const char *buffer);
void Send485_block (uint8_t *buffer, uint16_t length);

#endif
