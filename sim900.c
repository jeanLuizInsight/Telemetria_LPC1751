/*************************************************************************************************
* Sistema de telemetria por rede GSM.
* Arquivo: sim900.c
* Programador: Jean Luiz Zanatta
* Data: 10/08/2014
*************************************************************************************************/
//includes
#include <LPC17xx.h>
#include "sim900.h"
#include <string.h>
#include "timer.h"
#include "uart.h" 
#include "gpio.h"
#include "lcd.h"

//variavel global
unsigned short _respPoint = 0;

//configura pinos de IO para interface com o modulo GSM.
void Sim900_io_conf(void) {
	//direção dos pinos do uC
	GPIO_set_dir(DCDPIN, 0);    //port 2 pino 3  entrada
	GPIO_set_dir(POWERPIN, 1);	 //port 0 pino 18 saida
	GPIO_set_dir(RESETPIN, 1);  //port 0 pino 17 saida
	//valor dos pinos
	GPIO_set_value(RESETPIN, 0); //port 0 pino 17 nivel baixo
	GPIO_set_value(POWERPIN, 0);	//port 0 pino 18 nivel baixo	
	return;
}

//liga modulo GSM.
int Sim900_power_on(void) {
	char n = 0;
	char timeOut;				
	for(n = 0; n < 5; n++) {				
		//se modulo estiver desligado, liga.
	  if(!GPIO_get_value(DCDPIN)) {
			GPIO_set_value(POWERPIN, 1);
	    delay_ms(1500);
	    GPIO_set_value(POWERPIN, 0);
	    delay_ms(2000);
	  } else {  //se modulo estiver ligado, reinicia.
	    GPIO_set_value(RESETPIN, 1);
	    delay_ms(1000);
	    GPIO_set_value(RESETPIN, 0);
	    delay_ms(2000);
	  }	
		//envia o comando AT duas vezes para reconhecer o baudrate.
		Sim900_send_cmd("AT", 2000);
		Sim900_send_cmd("AT", 2000);
		//módulo responde, procura pela resposta do comando AT
		if(!Resp_search("OK")) { //se resposta for =!OK						
			timeOut = 10;			     
			while(--timeOut) {		
				Sim900_send_cmd("ATE0", 2000);		    //envia o comando para desabilitar o eco dos comandos AT 	
				if(!Resp_search("ATE0\r\n\r\nOK")) {		
					if(Sim900_send_cmd("AT", 2000) == 6)
						return(0);	
				}						
			}				
		}								
	}	
	//sem resposta
	return (-1);						//se não conseguir resposta retorna falha.
}

//envia comandos para o modulo GSM.
int Sim900_send_cmd(const char *cmd, int timeOut) {
	int retNum = 0;	
	UARTGSMCOUNT = 0;									              //zera contagem do buffer da uart.
	memset((void *)UARTGSMBUFFER, '\0', BUFSIZE);	  //limpa Buffer.		
	UART_send(UARTGSM,(uint8_t *)cmd, strlen(cmd)); //envia string de comando.	
	UART_send(UARTGSM,(uint8_t *)"\r\n", 2);			  //envia CR(move p/ inicio da linha) LF(nova linha) apos o comando.	
	retNum = UART_receiving_wait(UARTGSM, timeOut);	//aguarda resposta da UART ou time out de 1s.		
	return(retNum);										              //retorna o numero de bytes recebido.
}

//inicializa modulo GSM.
int Sim900_warm_up(void) {
	int8_t timeOut = 0, ret = 0;	
	LcdGotoXy(1,4);
	LcdPuts("Ligando SIM900      ");
	//liga modulo GSM.
	if(Sim900_power_on())	
		return(-1);		
	//verifica nivel de sinal.		
	timeOut = 20;
	while(--timeOut) {			
		ret = Sim900_get_CSQ();		
		LcdGotoXy(1,4);
		LcdPuts("CSQ:                ");
		LcdGotoXy(6,4);
		LcdPrintNum(ret,2);		
		if(ret >= 10)  //se sinal > que 10
			break;		
		delay_ms(1000);
	}			
	if(!timeOut)
		return(-2);	
	//define o tipo de acesso a rede.
	timeOut = 5;
	while(--timeOut) {			
		if(Sim900_send_cmd("AT+CREG=0", 1000) == 6) //registro da rede desativado, código resultado não solicitado
			break;		
	}	
	if(!timeOut)
		return(-3);
	//aguarda o registro na rede.
	timeOut = 20;
	while(--timeOut) {		
		ret = Sim900_CREG_status();		
		LcdGotoXy(1,4);
		LcdPuts("CREG:               ");
		LcdGotoXy(7,4);
		LcdPrintNum(ret,2);				
		if(ret == 01)
			break;
		
		delay_ms(1000);
	}	
	if(!timeOut) {
		LcdGotoXy(1,4);
		LcdPuts("ERRO                ");
		return(-4);	
	}
	LcdGotoXy(1,4);
	LcdPuts("OK                  ");
	delay_ms(500);
	return(0);
}

//retorna nivel de sinal GSM.
int Sim900_get_CSQ(void) {
	uint8_t timeOut = 4, CSQ = 0;	
	while(--timeOut) {	
		if(Sim900_send_cmd("AT+CSQ", 1000) > 7) {  //envia comando, aguarda os 7 bytes de resposta da UART
			//se entre 0 e 60 é consideravel, se 99 não detectavel
			if(UARTGSMBUFFER[UARTGSMCOUNT - 6] < 48 || UARTGSMBUFFER[UARTGSMCOUNT - 6 ] > 57) { //se valor < 48 ou >57, sinal excelente ou péssimo
				CSQ = ((UARTGSMBUFFER[UARTGSMCOUNT - 5] - 48));
				return(CSQ);
			} else {																																								//se não sinal bom
				CSQ =  ((UARTGSMBUFFER[UARTGSMCOUNT - 6] - 48) * 10);
				CSQ += ((UARTGSMBUFFER[UARTGSMCOUNT - 5] - 48));
				return(CSQ);
			}
		}		
	}
	return(-1);
}

//retorna status de registro na rede GSM.
int Sim900_CREG_status(void) {
	uint8_t timeOut = 4, STS = 0;	
	while(--timeOut) {	
		if(Sim900_send_cmd("AT+CREG?", 1000) == 14) {		//aguarda os 14 bytes retornados pela UART	
			STS =  ((UARTGSMBUFFER[UARTGSMCOUNT - 5] - 48) * 10);
			STS += ((UARTGSMBUFFER[UARTGSMCOUNT - 3] - 48));						//armazena o status
			return(STS);
		}
	}
	return(-1);
}

//procura uma string na resposta do modulo.
//parametros: Resposta a ser esperada do modulo.
int Resp_search(const char *resp) {
	int respLen, n = 0, b = 0;
	char compFlag = 0;		
	respLen = strlen(resp);	
	while((n < UARTGSMCOUNT) && (respLen)) {
		if(resp[b] == UARTGSMBUFFER[n] && !compFlag)
			compFlag = 1;		
		if(resp[b] != UARTGSMBUFFER[n] && compFlag) {
			b = 0;
			compFlag = 0;			
			respLen = strlen(resp);			
		}		
		if(resp[b] == UARTGSMBUFFER[n] && compFlag)			
			respLen--,b++;			
		n++;
	}
	//pego apenas a resposta depois dos :
	_respPoint = n;	
	return(respLen);
}
