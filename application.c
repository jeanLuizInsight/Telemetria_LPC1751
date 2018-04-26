/*************************************************************************************************
* Sistema de telemetria por rede GSM.
* Arquivo: application.c
* Programador: Jean Luiz Zanatta
* Data: 08/08/2014
*************************************************************************************************/
//Includes
#include <LPC17xx.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timer.h"
#include "rs485.h"
#include "uart.h" 
#include "sim900.h"
#include "application.h"
#include "lcd.h"
#include "comandos_dados_sensor.h"
#include "telemetria.h"
#include "wdt.h"

//variaveis 
char _cmdS[250];
struct Datatime _rtc;
char _strCredito[250];
char _ipServer[16];
char _diasMes[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
char numCelCliente[13] = "554991589588";

//configura conexão PDP - Packet Data Protocol (conexão a rede IP)
//depois de ativado, o usuário pode iniciar a transmissão de dados
//informações necessárias: APN (acess point name, endereço do nó correspondente), IP dinamico ou fixo e parametros de QoS
int PDP_connect(void) {
	int8_t timeOut;
	//----------ativa multiplas conexões---------------	
	LcdGotoXy(1,4);
	LcdPuts("CIPMUX=1...         ");		
	timeOut = 10;
	while(--timeOut) {		
		delay_ms(1000);
		//habilita multi connection
		if(Sim900_send_cmd("AT+CIPMUX=1", 1000) == 6) //envia comando, aguarda retorno de 6 bytes
			break;				
	}	
	if(!timeOut)
		return(-1);
  LcdGotoXy(1,4);	
	LcdPuts("OK                  ");
	delay_ms(500);	
	//----------------verifica APN----------------	
	LcdGotoXy(1,4);
	LcdPuts("APN Check...        ");
	timeOut = 5;
	while(--timeOut) {		
		delay_ms(1000);
		//verifica APN, se resposta módulo CMNET OK
		Sim900_send_cmd("AT+CSTT?", 1000); 	
		if(!Resp_search("CMNET"))					 
				break;						
	}	
	if(!timeOut)
		return(-2);	
	LcdGotoXy(1,4);
	LcdPuts("OK                  ");
	delay_ms(500);	
	//---------configura provedor APN-------------
	LcdGotoXy(1,4);
	LcdPuts("APN Config...       ");	
	timeOut = 10;
	while(--timeOut) {		
		delay_ms(1000);
		//configura provedor APN (chip CLARO)
		Sim900_send_cmd("AT+CSTT=\"claro.com.br\",\"claro\",\"claro\"", 1000);	//envia comando
		if(!Resp_search("OK"))																									//se resposta OK ok
			break;		
	}	
	if(!timeOut)
		return(-3);	
	LcdGotoXy(1,4);
	LcdPuts("OK                  ");
	delay_ms(500);	
	//------------verifica APN----------------------	
	LcdGotoXy(1,4);
	LcdPuts("APN Check...        ");		
	timeOut = 10;
	while(--timeOut) {		
		delay_ms(1000);
		//verifica APN, retorno deve ser o dominio da operadora
		Sim900_send_cmd("AT+CSTT?", 1000);	//envia comando	
		if(!Resp_search("claro.com.br"))		
				break;				
	}	
	if(!timeOut)
		return(-3);	
	LcdGotoXy(1,4);
	LcdPuts("OK                  ");
	delay_ms(500);
	//-------------solicita conexão----------------------	
	LcdGotoXy(1,4);
	LcdPuts("CIICR...            ");
	timeOut = 5;
	while(--timeOut) {		
		delay_ms(5000);
		if(Sim900_send_cmd("AT+CIICR", 20000) == 6)  //envia comando, aguarda retorno de 6 bytes
			break;				
	}	
	if(!timeOut)
		return(-4);
	LcdGotoXy(1,4);
	LcdPuts("OK                  ");
	delay_ms(500);		
	LcdGotoXy(1,4);
	LcdPuts("CIFSR...            ");
	timeOut = 5;
	while(--timeOut) {		
		delay_ms(1000);
		//PQ?
		if(Sim900_send_cmd("AT+CIFSR", 1000) != 37) //envia comando para obter endereço IP local, aguardo retorno diferente de 37 bytes
			break;		
	}	
	if(!timeOut)
		return(-1);	
	LcdGotoXy(1,4);
	LcdPuts("OK                  ");
	delay_ms(500);			
	return(0);
}

//abre conexão TCP/UDP:
// Parametros: NumConn - Numero da conexão.
//			       Mode	   - Modo: 0 = TCP, 1 = UDP.
//			       Port	   - Porta para conexão na aplicação.
int Open_conn(int numConn, char modo, char *dominio, int porta) {	
	char cmd[45];
	int tryTimes = 5, tryTimes2 = 20;
	//envia comando para conexão	
	while(--tryTimes) {
		//resolve o dominio em IP, apenas para a.ntp.br, servidor já existe IP direto
		if(strlen(dominio) == 12) { //se o dominio for IP do servidor apenas armazena
			char i = 0;
			do {
				_ipServer[i] = dominio[i];
				i++;
			}	while(i < 12);				
		} else {
			Dns_res(dominio);  //busca IP do dominio especificado
		}
		//armazena string da conexão no buffer
		sprintf(cmd, "AT+CIPSTART=%d,\"%s\",\"%s\",\"%d\"", numConn, (modo) ? ("UDP"):("TCP"), _ipServer, porta);		
		//envia para o módulo GSM
		if(Sim900_send_cmd(cmd, 10000) == 6) {
			while(--tryTimes2) {
				delay_ms(1000);						
				if(!Resp_search("CONNECT OK"))  //conexão estabelecida
					return(0);
			}
		}		
		if(!Resp_search("ALREADY CONNECT")) { //falha na conexão
			if(Close_conn(numConn))
				return(-1);
		}
	}	
	return(-2);
}

//fecha conexão TCP/UDP:
int Close_conn(int numConn) {
	char cmd[45];
	int tryTimes = 5;
	sprintf(cmd, "AT+CIPCLOSE=%d", numConn);	
	while(--tryTimes) {
		Sim900_send_cmd(cmd, 10000);
		//conexão encerrada
		if(!Resp_search("CLOSE OK"))	
			return(0);											
	}	
	return(-1);
}

//envia dados pela conexão TCP/UDP.
int Send_data(int numConn, char *data, int sendLen) {
	char cmd[25];
	int tryTimes = 5;	
	sprintf(cmd,"AT+CIPSEND=%d", numConn);	
	while(--tryTimes) {
		if(Sim900_send_cmd(cmd, 10000) == 4)	
			break;	
	}	
	if(!tryTimes)
		return(-1);	
	delay_ms(100);	
	//envia dados pela serial para o GSM
	UART_send(UARTGSM, (uint8_t*)data, sendLen);	
	//envia caracter especial para fechar conexão e não enviar mais nada após os dados
	cmd[0] = 26;	//tabela ASCII 26 = CTRL Z, indica que o corpo da mensagem terminou
	delay_ms(100);
	UART_send(UARTGSM, (uint8_t*)cmd, 1);
	tryTimes = 200;
	while(--tryTimes) {
		delay_ms(100);
		//envio OK
		if(!Resp_search("SEND OK")) {
			UARTGSMCOUNT = 0;
			return(0);
		}
	}		
	return(-1);
}

//envia SMS para número especificado
int Send_SMS_Cel (char *data, int sendLen) {	
	char cmd[25];
	int tryTimes = 200;		
	//se resposta OK
	while(--tryTimes) {
		sprintf(cmd,"AT+CMGF=1");
		if(Sim900_send_cmd(cmd, 1000) == 6) {
			//delay_ms(1000);
			sprintf(cmd,"AT+CMGS=\"+%s\"", numCelCliente);
			Sim900_send_cmd(cmd, 1000);	
			if(!Resp_search(">")) {
				delay_ms(100);	
				//envia dados pela serial para o GSM
				UART_send(UARTGSM, (uint8_t*)data, sendLen);	
				cmd[0] = 26;	//tabela ASCII 26 = CTRL Z, indica que o corpo da mensagem terminou
				delay_ms(100);
				UART_send(UARTGSM, (uint8_t*)cmd, 1);
				tryTimes = 200;
				while(--tryTimes) {
					delay_ms(100);
					//se envio OK
					if(!Resp_search("+CMGS")) {
						UARTGSMCOUNT = 0;
						return(0);
					}
				}
			}
		}	
	}		
	return(-1);
}

//resolve dominio
int Dns_res(char *dominio) {
	int timeOut = 5, n;	
	memset(_ipServer, '\0', 16); //limpa memória		
	//envia o comando de configuração do DNS.
	while(--timeOut) {
		sprintf(_cmdS, "AT+CDNSCFG=\"%s\",\"%s\"", PRIMARI_DNS, SECONDARY_DNS); //envia comando
		if(Sim900_send_cmd(_cmdS, 1000) == 6)																		//aguarda resposta em 6 bytes
			break;
	}	
	if(timeOut <= 0)
		return(-1);
	//pergunta o IP do target para o servidor DNS.
	while(--timeOut) {
		sprintf(_cmdS, "AT+CDNSGIP=\"%s\"", dominio);
		if(Sim900_send_cmd(_cmdS, 1000) == 6) { //envia comando aguarda os 6 bytes de OK \r\nOK\r\n
			timeOut = 10;			
			while(--timeOut) {
				if(!Resp_search("+CDNSGIP: 1,")) {  //se resposta igual "+CDNSGIP: 1," + dominio	
					sprintf(_cmdS, "\"%s\",\"", dominio);					
					Send485_string(_cmdS);
					if(!Resp_search(_cmdS)) {
						//percorre todos os pontos  (não bytes) retornados pelo motor GSM pela UART
						for(n = 0; n < (UARTGSMCOUNT - _respPoint - 3); n++){
							if(n >= 15) {
								return(-2);
							}							
							_ipServer[n] = UARTGSMBUFFER[_respPoint + n]; //armazena o endereço IP retornado do servidor (pelo buffer da uart)
						}
						return(0);
					}
				}				
				delay_ms(1000);
			}
			return(-3);
		}
	}	
	return(-4);
}

//função para solicitar valor em creditos pré pago.
int Solicita_credito(void) {
	int n = 200;		
	while(n--) {	
		Sim900_send_cmd("AT+CUSD=1,\"*544#\",15", 2000);	//envia comando solicitando			
		delay_ms(1000);
		UART_receiving_wait(UARTGSM, 2000);
		if(!Resp_search("+CUSD: 0,\"")) {		              //aguarda resposta, se igual recebeu dados
			for(n = 0; n < (UARTGSMCOUNT - _respPoint - 5); n++) {
				if(n >= 255)
					break;
				_strCredito[n] = UARTGSMBUFFER[_respPoint + n]; //armazena string no vetor
			}			
			return(0);
		}
		delay_ms(1000);
	}	
	return(-1);
}

//busca data e hora de um servidor NTP.
int NTP_get_time(char *ntpSvrAddress) {
	int timeOut;
	char NTPBuffer[48];
	unsigned long timeStamp;
  register long dia, min, seg, ano, salto; 
	//limpa os 48 bytes do pacote.
	memset(NTPBuffer, 0, 48);	
	///monta a requisição NTP.
	NTPBuffer[0]  = 0xE3; //
	NTPBuffer[1]  = 0x00; //
	NTPBuffer[2]  = 0x06; //
	NTPBuffer[3]  = 0xEC;	//
	NTPBuffer[12] = 0x31; //
	NTPBuffer[13] = 0x4E; //
	NTPBuffer[14] = 0x31; //
	NTPBuffer[15] = 0x34;	//
	//abre a conexão com o servidor.
	if(Open_conn(3, 1, ntpSvrAddress, 123)) //num com 3, UDP, dominio, porta 123
		return(-1);	
	//envia requisição para o servidor
	if(Send_data(3, NTPBuffer, 48))
		return(-2);			
	//configura timeout e aguarda recepção dos 48 bytes.
	timeOut = 200;
	while(timeOut) {
		timeOut--;		
		if(!Resp_search("+RECEIVE,3,48:\r\n")) //se resposta igual "+RECEIVE,3,48:\r\n" ok recebeu os dados de volta
			break;		
		delay_ms(100);		
	}	
	//se time out estourar, retorne erro.
	if(!timeOut)
		return(-3);	
	//converte o timestamp para unsigned long. (armazena data e hora na variavel)
	timeStamp  = UARTGSMBUFFER[_respPoint + 40]<<24;
	timeStamp |= UARTGSMBUFFER[_respPoint + 41]<<16;
	timeStamp |= UARTGSMBUFFER[_respPoint + 42]<<8;
	timeStamp |= UARTGSMBUFFER[_respPoint + 43];
	
	//converte de timestamp para dia, mes, ano...
	//OBS: fazer uma lógica para atualizar horário de verão ???
  dia = timeStamp / (24L*60*60); 
  seg = timeStamp % (24L*60*60); 
  _rtc.segundo = seg % 60;   
  min = seg / 60; 
  _rtc.hora = ((min / 60) - 2); //OBS: alterar aqui aqui (- 2: verão, -3 inverno) <<<
  _rtc.minuto = min % 60; 
  ano = (((dia * 4) + 2)/1461); 
  _rtc.ano = ano + 1900; 
  salto = !(_rtc.ano & 3); 
  dia -= ((ano * 1461) + 1) / 4; 
  dia += (dia > 58 + salto) ? ((salto) ? 1 : 2) : 0; 
  _rtc.mes = (((dia * 12) + 6)/367); 
  _rtc.dia = dia + 1 - ((_rtc.mes++ * 367) + 5)/12; 			
	//fecha conexão.
	Close_conn(3);	
	//teste de validação dos dados.
	if(_rtc.ano < 2014 || _rtc.ano > 2100 || _rtc.mes > 12 || _rtc.mes < 1 || _rtc.dia > 31 || _rtc.dia < 1 || _rtc.hora > 23 || _rtc. minuto > 59 || _rtc.segundo > 59)
		return(-4);	
	_rtc.ano -= 2000;	
	//Retorne OK!
	return(0);
}




