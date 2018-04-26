/*********************************************************************************
* lcd.c	- Funções do modulo LCD					
* Microcontrolador LPC13xx/LPC17xx
* Programador: Lucas Antunes													    
* Data: 02/08/2012							
**********************************************************************************/

#include <LPC17xx.h>		
#include "timer.h"
#include "lcd.h"
#include "gpio.h"
#include "math.h"

//----------------------------------------------------------------------------//
//! Variavel contendo constantes de inicialicação   
uint8_t const INIC_LCD[4] = {0x28, Cursor, 1, CursorDesloc};   
uint8_t volatile lcdintp = 0;

//----------------------------------------------------------------------------//
/**
* @brief 	Função para envio de nibble ao display.
* @details 	Função para enviar 4 bits ao display.
* @param dado - Meio byte contido na parte baixa da variavel.
*/

void LcdEnviaNibble( uint8_t dado )
{													  
	// Seta os valores dos 4 bits nas linhas de dados.
	GPIO_set_value(DATA_4,(dado&1)?(1):(0));
	GPIO_set_value(DATA_5,(dado&2)?(1):(0));
	GPIO_set_value(DATA_6,(dado&4)?(1):(0));
	GPIO_set_value(DATA_7,(dado&8)?(1):(0));
  
    // Pulsa a linha enable.      
   	GPIO_set_value(ENABLE,1);
    GPIO_set_value(ENABLE,0);

#ifdef USEBUSY	
	LcdBusy();
#else
	delay_ms(2);
#endif
   	return;      
}

//----------------------------------------------------------------------------//
/** 
 *  @brief Função para envio de bytes.
 *  @details Envia um byte para o display. 
 *  @param RS - Sinal de controle: Dado enviado é caracter
 *  ou instrução.
 *  @param dado - Dado a ser enviado ao display 
 */
    
void LcdEnviaByte( uint8_t rs, uint8_t dado )
{ 	
    // Configura linha RS 
    GPIO_set_value(RS,(rs&1));	  
    LcdEnviaNibble(dado >> 4);          // Manda enviar o primeiro nibble.      
	
	// Configura linha RS 
	GPIO_set_value(RS,(rs&1));	  
    LcdEnviaNibble(dado & 0xf);         // Manda enviar o segundo nibble.      
	return;
}

//----------------------------------------------------------------------------//
/** 
 *  @brief     Função de inicialização do display.
 *  @details   Configura função e direção dos pinos utilizados no modulo LCD
 *  @pre       Inicialização do timer 0 de 32 bits e do GPIO. 
 */

void LcdInit(void)
{                
	uint8_t conta;                
		
	// Configura as linhas de dados e controle.		

	// Enable                                  
	GPIO_set_dir(ENABLE,1);
	GPIO_set_value(ENABLE,0);
    
	// RS
	GPIO_set_dir(RS,1);
	GPIO_set_value(RS,0);

#ifdef USEBSUSY	
	// RW
	GPIO_set_dir(RW,1);
	GPIO_set_value(RW,0);
#endif
		
	// Data 4                                  
	GPIO_set_dir(DATA_4,1);
	GPIO_set_value(DATA_4,0);

	// Data 5                                  
	GPIO_set_dir(DATA_5,1);
	GPIO_set_value(DATA_5,0);
	
	// Data 6                                  
	GPIO_set_dir(DATA_6,1);
	GPIO_set_value(DATA_6,0);
	
	// Data 7                                  
	GPIO_set_dir(DATA_7,1);
	GPIO_set_value(DATA_7,0);
	    
#ifdef USEBUSY
    LcdBusy();
#else
	delay_ms(20);
#endif
      
    // Envia 3 vezes o numero 3 e depois o n° 2 para configurar o barramento com 4 bits.      
    for(conta = 0; conta <= 3; ++ conta ){
       LcdEnviaNibble(3);
    }
      
    LcdEnviaNibble(2);
	  
    for(conta = 0; conta <= 3;++ conta ) LcdEnviaByte(0,INIC_LCD[conta]);                  
	return;
}  

//----------------------------------------------------------------------------//
#ifdef USEBUSY
void LcdBusy(void)
{
	short timeout = 3000;
	
	// Configura pino D7 como entrada.
	GPIOSetDir(DATA_7,0);
	
	// Pino de enable em nivel baixo.
	GPIO_set_value(ENABLE,0);
	
	// Seleciona para leitura.
	GPIO_set_value(RW,RW,1);
	
	// Seleciona como instrução.
	GPIO_set_value(RS,RS,0);
	
	// Pino de enable em nivel alto.
	GPIO_set_value(ENABLE,ENABLE,1);
	
	// Lê busy flag e espera.
	while(GPIO_get_value(DATA_7) && --timeout)
		delay_us(10);
	
	// Seleciona para gravação.
	GPIO_set_value(RW,0);
	
	// Configura pino D7 como saida.
	GPIO_set_dir(DATA_7,1);
	
	return;
}
#endif
//----------------------------------------------------------------------------//
/** 
 *  @brief     Função para posicionamento do cursor no display.
 *  @details   Posiciona o cursor nas posições X e Y no display. 
 *  @param  x - Posição do cursor no eixo X.
 *  @param  y - Posição do cursor no eixo Y.
 */

void LcdGotoXy( uint8_t x, uint8_t y )
{
	int endereco;

   	switch(y){
		case 1:	endereco = lcd_primeira_linha;
				break;
		
		case 2:	endereco = lcd_segunda_linha;
				break;
		
		case 3:	endereco = lcd_terceira_linha;
				break;
		
		case 4:	endereco = lcd_quarta_linha;
				break;

		default: endereco = lcd_primeira_linha;
				break;
	}

    endereco += --x;
    LcdEnviaByte(0,0x80 | endereco);
	return;
}

//----------------------------------------------------------------------------//
/** 
 *  @brief     Função para escrever strings.
 *  @details   Envia uma string para o display. 
 *  @param  String - String estatica ou vetor contendo uma string.
 */

void LcdPuts(char *String)
{
	// enquanto ponteiro da string for valido... 
	while(*String){
		LcdEnviaByte(1,*String++);	// envie o caracter e incremente o ponteiro.
	}
	return;
}


void LcdPrintNum(short Num, short Digits)
{
	char tmp, NegFlag = 0;
	
	if(Num<0)
	{
		Num*=(-1);
		NegFlag = 1;
		Digits--;
	}
	
	while(Digits>0)
	{
		tmp = Num/pow(10,--Digits);
		if(tmp || !Digits) 
		{
			while(Digits+1)
			{
				if(NegFlag)
				{
					LcdPuts("-");
					NegFlag = 0;
				}
				
				LcdEnviaByte(1,(tmp+0x30));
				Num -= tmp*pow(10,Digits);				
				tmp = Num/pow(10,--Digits);
			}
		}
		else
			LcdEnviaByte(1,' ');
	}
	return;
}

char LcdIntStat(char valor)
{
	if(valor)
		lcdintp = valor;
	
	return(lcdintp);	
}
//----------------------------------------------------------------------------//
// EOF
