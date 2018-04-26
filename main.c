/*****************************************************************************************************
* Telemetria GSM LPC1751
* Arquivo: main.c
* Programador: Jean Luiz Zanatta
* Data: ../08/2014
*****************************************************************************************************/
//includes
#include <LPC17xx.h>
#include <stdlib.h>
#include <stdio.h>
#include "application.h"
#include "sim900.h"
#include "timer.h"
#include "gpio.h"
#include "rs485.h"
#include "string.h"
#include "wdt.h"
#include "comandos_dados_sensor.h"
#include "telemetria.h"
#include "lcd.h"

/**
* @brief 	  Função Main - função principal do programa	
* @details  chama todas as funções do programa 
*/
int main(void) {
	int n;
	uint8_t csq;
	uint16_t times = 0;
	char timeOut; //tempo pelo qual o sistema fica em estado de espera para receber dados, porém os dados não são enviados durante esse intervalo. 
	char text[250];	
	char textSMS[250];
	char flagSMS = 1; //aviso se Nh3 >= NH3Alto 	
	char flagLoop = 0;
	char NH3Alto = 50; //ppm
	
	//----inicializa watch dog timer, configurando estouro em seg----
	Wdt_config(600);	
	
	//----inicializa timer 0 usado para delays----
	Timer0_init();	
	
	//Inicialização do display LCD
	LcdInit();
	
	//----inicializa GPIOs----
	GPIO_init();	
	
	//----inicializa UARTs----
	UART_init(UART485, BAUD485); //485 = 0
	UART_init(UARTGSM, BAUDGSM); //GSM = 1
	
	//----configura pinos da interface RS485----
	RS485_io_conf();		
	
	//----configura pinos do modulo SIM900 
	Sim900_io_conf();		
		
	//----modo Sniffer para posicionamento da antena (analisador de rede)----	
	GPIO_set_dir(0, 0, 0);			
	if(!GPIO_get_value(0, 0)) { //se estado lógico do pino seja 0	
		LcdPuts("Modo Sniffer        ");
		LcdGotoXy(1,2);		
		LcdPuts("Iniciando SIM900    ");
		if(Sim900_power_on())
			NVIC_SystemReset();		//função disponibilizada pelo padrão CMSIS(Cortex Microcontroller Software Interface Standard): reinicia o sistema	
		LcdGotoXy(1,2);		
		LcdPuts("Iniciado            ");
		while(1) { //enquanto não retornar uma intensidade, permanece procurando			
			LcdGotoXy(1,3);
			LcdPuts("CSQ:                ");
			LcdGotoXy(6,3);
			LcdPrintNum(Sim900_get_CSQ(),2); //busca a intensidade do sinal GSM
			delay_ms(1000);			
		}			
	}	
	
	//Tela inicio Sistema
	LcdGotoXy(1,1);
  LcdPuts("UNOESC -- Eng. Comp.");
	LcdGotoXy(1,2);
  LcdPuts("  ALTEM Tecnologia  ");
	LcdGotoXy(1,3);
  LcdPuts("     J. SISMAAG     ");	
	LcdGotoXy(1,4);
  LcdPuts("By Jean Luiz Zanatta");
	delay_ms(5000);		
	
	//----busca dispositivos na rede RS485----
	LcdPuts("Inicializando RS-485");
	LcdGotoXy(1,2);
  LcdPuts("                    ");
	LcdGotoXy(1,3);
  LcdPuts("                    ");
	LcdGotoXy(1,4);
  LcdPuts("                    ");
	
	//delay_ms(5000);
	Device_search(); //salva no vetor _devAdd[] e retorna a posição do vetor em que está armazenado(qtdade disp)
	
	// Imprime quantos dispositivos foram encontrados.
	sprintf(text,"Encontrado %02d Disp. ",_devCount);
	LcdGotoXy(1,1);
	LcdPuts(text);
	// Imprime os dispositivos encontrados.
	LcdGotoXy(1,2);
	for(n = 0; n < _devCount; n++)
	{
		sprintf(text,"ID: %03d             ",_devAdd[n]);
		LcdPuts(text);
	}	
	//----se não encontrar nenhum dispositivo reinicie o modulo----	
	if(!_devCount) { //se _devCount = 0, 1 entra na condição
		delay_ms(2000);
		NVIC_SystemReset();	
	}	
	
	
	//----inicializa modulo GSM----	
	LcdGotoXy(1,3);
	LcdPuts("Inicializando SIM900");	
	if(Sim900_warm_up() < 0) //se modulo não ligar, não buscar CSQ, não buscar Status
		NVIC_SystemReset();    //reinicia o sistema		
	
	LcdGotoXy(1,4);
	LcdPuts("                    ");
	//----solicita credito----
	LcdGotoXy(1,3);
	LcdPuts("Solicitando Creditos");	
	if(Solicita_credito())
		NVIC_SystemReset();
	
	//----inicializa conexão PDP----
	LcdGotoXy(1,3);
	LcdPuts("Inicializando PDP   ");	
	if(PDP_connect() < 0)
		NVIC_SystemReset();	
	
	LcdGotoXy(1,4);
	LcdPuts("                    ");
	//----abre conexão com o servidor----
	LcdGotoXy(1,3);
	LcdPuts("Conectando Servidor ");	
	if(Open_conn(1, 0, "189.8.207.51", 4550))		//se não abriu conexão
		NVIC_SystemReset();
	//----armazena em text o código do GSM e o valor dos créditos do chip----
	//string: (id=%04d&credito=%s)=(id do módulo GSM & creditos disponiveis)
	sprintf(text,"%04d&%s\r\n",
	ID_MODULO, _strCredito
	);	
	
	//----envia créditos para o servidor----
	LcdGotoXy(1,3);
	LcdPuts("Enviando Creditos   ");	
	if(Send_data(1, text, strlen(text)))
		NVIC_SystemReset();	
	
	LcdGotoXy(1,1);
  LcdPuts("...                 ");
	LcdGotoXy(1,2);
  LcdPuts("                    ");
	LcdGotoXy(1,3);
  LcdPuts("                    ");
	LcdGotoXy(1,4);
  LcdPuts("                    ");
	////////////////////////////////
	// Loop principal do programa //
	////////////////////////////////
	while(1) {			
		//----atualiza data e hora----				
		NTP_get_time("a.ntp.br");	
	  
		// Imprime data e hora.
		sprintf(text,"%02d/%02d/20%02d -- %02d:%02d ",
		_rtc.dia, _rtc.mes, _rtc.ano, _rtc.hora, _rtc.minuto);		
		LcdGotoXy(1,1);
		LcdPuts(text);
		
		//----a cada 10 minutos verifica os dispositivos do barramento----
		if(times++ >= 10) {
			times = 0;
			Device_search();
		}
		
		//----solicita CSQ----	
		csq = Sim900_get_CSQ();	
    
		//Imprime CSQ
		LcdGotoXy(1,2);
		LcdPuts("CSQ:                ");
		LcdGotoXy(6,2);
		LcdPrintNum(csq, 2);
		delay_ms(1000);		
		
		delay_ms(1000);  			
		for(n = 0; n < _devCount; n++) {			
			//----conecta no servidor----	
			LcdGotoXy(1,4);
			LcdPuts("Conectando Servidor ");
			if(!Open_conn(0, 0, "189.8.207.51", 4550)) { //se abriu conexão	
				
				LcdGotoXy(1,4);
				LcdPuts("Conectado           ");
				
				LcdGotoXy(1,3);					
				sprintf(text,"Env Dispositivo %04d",_devAdd[n]);
				LcdPuts(text);

				//----solicita concetração de NH3 atual----				
				if(!Send485_cmd(CMDLERNH3, _devAdd[n])) {
					
					LcdGotoXy(1,3);
					sprintf(text,"NH3: %03d ppm        ",_NH3);
					LcdPuts(text);
					
					//armazena em text a string com os dados para enviar ao servidor 					
					//String: (id=%04d&nh3=%03d&data=20%02d-%02d-%02d&hora=%02d:%02d:%02d&csq=%02d)=(id dispositivo, nh3, data, hora, csq)				
					sprintf(text, "%02d&%04d&%03d&20%02d-%02d-%02d&%02d:%02d:%02d&%02d\r\n",
					ID_MODULO, _devAdd[n], _NH3, _rtc.ano, _rtc.mes, _rtc.dia, _rtc.hora, _rtc.minuto, _rtc.segundo, csq
					);
					
					//----se concentração for maior ou igual a 25ppm envia uma mensagem para o celular do cliente----
					if(_NH3 >= NH3Alto && flagSMS == 1) {
						LcdGotoXy(1,4);
						LcdPuts("Enviando SMS        ");
						flagSMS = 0;
						flagLoop = 1;
						sprintf(textSMS, "AVISO! Concentracao de amonia excedendo o limite. Dispositivo %04d medindo %03d ppm", _devAdd[n], _NH3);
						timeOut = 5;
						while(--timeOut) {
							if(!Send_SMS_Cel(textSMS, strlen(textSMS))) {
								LcdGotoXy(1,4);
								LcdPuts("SMS Enviado         ");
								break;
							}
						}
					}
					if(_NH3 < NH3Alto && flagLoop == 1) {
						flagSMS = 1;
						flagLoop = 0;
					}
				} else {  //se não conseguir ler a concentração NH3 solicitada 			
					
					LcdGotoXy(1,3);
					sprintf(text,"Erro ao ler NH3 %04d",_devAdd[n]);	
					LcdPuts(text);					
					LcdGotoXy(1,4);
					LcdPuts("                    ");
					
					//String: (id=%04d&error=1)=(id do dispositivo, erro ao solicitar NH3)
					sprintf(text,"%04d&1\r\n",
					_devAdd[n]);
					delay_ms(1000);					
					
					//----se der algum erro na leitura, atualiza os dispositivos no barramento----
					times = 10;
				}
				
				//----envia dados para o servidor----
				if(!Send_data(0, text, strlen(text))) {
					LcdGotoXy(1,4);
					sprintf(text,"Dados Enviados: %04d",_devAdd[n]);	
					LcdPuts(text);
					Wdt_feed();        //só alimenta wdt se os dados forem enviados para o servidor, caso contrario reinicia o sistema
				}
				
				//----verifica se fechou conexão----
				timeOut = 200;				
				while(--timeOut) {
					delay_ms(100);
					if(!Resp_search("0, CLOSED"))	
						break;
				}		
				if(timeOut <= 0)
					NVIC_SystemReset();						
			}				
		}						
		//----aguarda 10 segundos.
		delay_ms(10000);		
	}
}
