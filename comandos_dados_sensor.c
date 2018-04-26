/*************************************************************************************************
* Sistema de telemetria por rede GSM.
* Arquivo: comandos_dados_sensor.c
* Programador: Jean Luiz Zanatta
* Data: 08/08/2014
*************************************************************************************************/
#include <LPC17xx.h>
#include "comandos_dados_sensor.h"
#include "application.h"
#include "rs485.h"
#include "uart.h"
#include "timer.h"
#include <stdlib.h>

short _NH3 = 0;

//função envia comando pela 485
//AJUSTAR ESTA FUNÇÃO!!!!!
//verificar o pacote de dados para troca de informações com o dispositivo sensor
int Send485_cmd(char cmd, char address) {	
	char tryTimes = 3;	
	while(--tryTimes) {
		UART485COUNT = 0;	 
		UART485BUFFER[UART485COUNT++] = address; 
		UART485BUFFER[UART485COUNT++] = cmd;
		Gera_CRC();
		//envia para o dispositivo sensor 		
		UART_send(UART485, (uint8_t *)UART485BUFFER, UART485COUNT);
		UART485COUNT = 0;		
		//aguardo retorno OK
		if(UART_receiving_wait(UART485, 1000) > 4)  //verificar a contagem da uart p/ ver se dispositivo respondeu correto(bytes recebidos > 4)
			if(Verifica_CRC()) {	
				//trata retorno de handshake				
				if((cmd == CMDHANDSHAKE) && (UART485BUFFER[2] == 0x6F) && (UART485BUFFER[3] == 0x6B))
					return(0);			
				//trata retorno de ler NH³
				if(cmd == CMDLERNH3) {		
					_NH3 = ((UART485BUFFER[2]<<8)|UART485BUFFER[3]);					
					return(0);							
				}
			}			
  		delay_ms(1000);
	}	
	return(-1);
}



