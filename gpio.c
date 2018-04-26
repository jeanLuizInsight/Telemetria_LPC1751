/******************************************************************************
* Monitoramento do gás amônia sensor tgs2444.
* gpio.c				
* Microcontrolador LPC1766
* Programador: Jean Luiz Zanatta
* Data: 07/08/2014							
*******************************************************************************/

//includes
#include <LPC17xx.h>

//variaveis constantes.
static LPC_GPIO_TypeDef (* const LPC_GPIO[5]) = { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3, LPC_GPIO4 };


//função para inicialização do GPIO. Habilita clock no PCONP para o GPIO.
void GPIO_init(void) {
	LPC_SC->PCONP  |= (1 << 15);
	return;
}

//função para ler o valor de um pino do GPIO. Retorna o estado logico do pino de GPIO especificado, 0 ou 1.
//uint8_t - bit 0.
uint8_t GPIO_get_value(uint8_t porta, uint8_t pino) {
	return((LPC_GPIO[porta]->FIOPIN & (1 << pino)) ? 1 : 0); //x ? verdadeiro : falso 
}

//função para escrita de valor logico em um pino do GPIO. Escreve o valor logico no pino indicado.
void GPIO_set_value(uint8_t porta, uint8_t pino, uint8_t value) {
	if(value) {  //value = 1
		LPC_GPIO[porta]->FIOSET = (1<<pino);  //saida nivel alto		
	} else {     //value = 0
		LPC_GPIO[porta]->FIOCLR = (1<<pino);	//saida nivel baixo
	}
	return;
}

//função para configurar a direção dos pinos do GPIO. Configura os pinos do GPIO como entrada ou saida.
//Dir = 1: Saida, Dir = 0: Entrada.
void GPIO_set_dir(uint8_t porta, uint8_t pino, uint8_t dir) {
	if(dir){
		LPC_GPIO[porta]->FIODIR |= (1<<pino); //pino setado como output
		return;
	}
	LPC_GPIO[porta]->FIODIR &= ~(1<<pino);  //pino setado como input
	return;
}
