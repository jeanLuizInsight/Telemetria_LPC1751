/*************************************************************************************************
* Sistema de telemetria por rede GSM.
* Arquivo: sim900.h
* Programador: Jean Luiz Zanatta
* Data: 06/08/2014
*************************************************************************************************/
#ifndef	SIM900_H
#define SIM900_H

//includes
#include <LPC17xx.h>
#include <stdint.h>
#include "uart.h"

//defines
#define POWERPIN	0,18
#define RESETPIN	0,17
#define DCDPIN		2,3
#define UARTGSM		1			  //seleção da UART para comunicação com modulo GSM.
#define BAUDGSM		57600		//baudrate para comunicação com modulo GSM.

#if UARTGSM
	#define UARTGSMBUFFER	_UART1Buffer
	#define UARTGSMCOUNT	_UART1Count
#else
	#define UARTGSMBUFFER	_UART0Buffer
	#define UARTGSMCOUNT	_UART0Count
#endif

//variaveis
extern unsigned short _respPoint;

//prototipo das funções do arquivo.
void Sim900_io_conf (void);
int Sim900_power_on	(void);
int Sim900_warm_up (void);
int	Sim900_get_CSQ (void);
int	Sim900_CREG_status (void);
int Resp_search	(const char *resp);
int	Sim900_send_cmd	(const char *cmd, int timeOut);

#endif
