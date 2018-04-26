/*************************************************************************************************
* Sistema de telemetria por rede GSM.
* Arquivo: rs485.c
* Programador: Jean Luiz Zanatta
* Data: 08/08/2014
*************************************************************************************************/
/*
A comunicação UART RS-485 entre uC GSM e uC Sensor NH3 é realizada através do protocolo Jean
em formato RTU(necessitando haver um tempo para inicio e fim da mensagem).
1º campo de endereço: (1 byte) endereço do dispositivo(mestre dirije-se ao escravo colocando o seu número no campo de endereço) 
                      e quando o escravo envia a resposta ele coloca seu endereço no campo endereço p/ o mestre saber qual escr
											avo está respondendo ou coloca o numero do mestre.
2º campo de função:   (8 bits) o escravo ao receber a mensagem, verifica qual ação a ser executada neste campo. Após executada 
										  retorna neste campo a função mesma da pergunta, ou um código entendido pelo mestre simbolizando a execuç
											ão correta. 
3º campo de dados:	  (conjunto de 2 digitos binários) serve para receber os dados do escravo. É inexistente quando a função p/
											solicitar a concentração NH3 é enviada do mestre para o escravo, pois o escravo já entende o que fazer ap
											enas através do campo função.
4º campo de erro:	    (16 bits) calculo de paridade utilizado p/ receber o conteúdo do cálculo de integridade e o valor que esse
											campo irá receber, é o resultado de um Cálculo de Redundância Cíclica (CRC). Após feito o cálculo, os cara
											cteres de CRC são concatenados à mensagem como sendo o último campo antes do caracter de finalização.
*/
//includes
#include <LPC17xx.h>
#include <string.h>
#include "rs485.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "application.h"
#include "comandos_dados_sensor.h"

//variaveis globais.
unsigned short _CRC16;
unsigned short _indx = 0;
unsigned short _jndx = 0;
unsigned short _rs485Count = 0;
unsigned char  _devAdd[MAXDEVNUM];
unsigned char  _devCount;

//função de configuração dos pinos da interafe RS485.
void RS485_io_conf(void) {
	GPIO_set_dir(1,14,1);
	GPIO_set_dir(1,15,1);	
	REN_485();	
	return;
}

//habilita pino de TX e desabilita pino RX no transceiver RS485.
void DE_485(void) {												
	GPIO_set_value(1,14,1);
	GPIO_set_value(1,15,1);														
}                                               

//habilita pino de RX e desabilita pino TX no transceiver RS485.
void REN_485(void) {	
#if	UART485
	while(!_UART1TxEmpty);
#else
	while(!_UART0TxEmpty){}
#endif	
 	GPIO_set_value(1,14,0);
	GPIO_set_value(1,15,0);
}	

//busca por dispositivos na rede RS485.
int Device_search(void) {
	unsigned char address = 0; 	
	_devCount = 0;	
	while(++address < MAXDEVNUM) {
		UART485COUNT = 0;	
		UART485BUFFER[UART485COUNT++] = address;
		UART485BUFFER[UART485COUNT++] = CMDHANDSHAKE;	
		Gera_CRC();	
		UART_send(UART485, (uint8_t *)UART485BUFFER, UART485COUNT);	
		UART485COUNT = 0;	
		if(UART_receiving_wait(UART485, 100) > 4)
			if(Verifica_CRC())			
				if(UART485BUFFER[2] == 0x6F && UART485BUFFER[3] == 0x6B)
					_devAdd[_devCount++] = address;				
		delay_ms(250);
	}	
	return(_devCount); //qtd de disps encontrados
}

//envia um pacote pela interface RS485.
void Send485_block(uint8_t *buffer, uint16_t length) {
	UART485COUNT = 0;									            // Zera contagem do buffer da uart.
	memset((void *)UART485BUFFER, '\0', BUFSIZE);		// Limpa Buffer.
	DE_485();
	UART_send(UART485, buffer, length);
	REN_485();
	return;
}

//envia strings pela interface RS485.
void Send485_string(const char *buffer) {	
	UART485COUNT = 0;									            // Zera contagem do buffer da uart.
	memset( (void *)UART485BUFFER, '\0', BUFSIZE );		// Limpa Buffer.
	DE_485();		
	UART_send(UART485, (uint8_t *)buffer, strlen(buffer));
	UART_send(UART485, (uint8_t *)"\r\n", 2);
	REN_485();	
	return;
}

//Calcula CRC baseado no polinômio CRC16 reverso, 0xA001.
//\n\n\b Modifica: 	CRC16, UART485BUFFER. 
void Processa_CRC_offline(void) {													
  _CRC16 = 0xFFFF;									
	for (_indx =0; _indx < UART485COUNT - 2; _indx++) {	
    	_CRC16 ^= UART485BUFFER[_indx];					
    	_jndx = 0;						
    	while (_jndx < 8) {								
        	if ((_CRC16 & 0x01) ) {							
            	_CRC16  = _CRC16>>1;	
            	_CRC16 ^= 0xA001;		
         	} else {							
            	_CRC16  = _CRC16>>1;
         	}		
			_jndx++;				
   		}											
	}										
}			
//verifica se o CRC está OK. Subtrai o valor recebido de CRC do calculado de CRC.
//\n\n\b Modifica: 	CRC16. 
char Verifica_CRC(void) {
	if(UART485COUNT > 0) {
		Processa_CRC_offline();						
		if ((_CRC16 - (((unsigned short)UART485BUFFER[UART485COUNT - 1])<<8) - ((unsigned short)UART485BUFFER[UART485COUNT - 2])) == 0 ) {												//
			return 1;			//CRC check OK
		}else {												
			return 0;			//CRC check not OK
		}    											
	} else {
		return 0;
	}
}													

//Gera código verificador de erro. Gera CRC de 16 bits.
//\n\n\b Modifica: 	CRC16. 
void Gera_CRC(void) {														//
	UART485COUNT +=  2;									// aloca 2 caracteres para o valor do CRC
 	Processa_CRC_offline();								// gera CRC
	UART485BUFFER[UART485COUNT - 2] = (char)(_CRC16  & 0x00FF);// separa byte menos significativo e posiciona parte do CRC no buffer
	UART485BUFFER[UART485COUNT - 1] = (char)(_CRC16 >> 8); 	// posiciona o byte mais significativo do CRC no buffer
}

