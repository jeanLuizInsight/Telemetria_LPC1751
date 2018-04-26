/*********************************************************************************
* lcd.h	- Funções do modulo LCD					
* Microcontrolador LPCxxxx
* Programador: Lucas Antunes
* Data: 02/08/2012							
**********************************************************************************/

#ifndef __LCD_H_
#define __LCD_H_

//definições do display
#define ENABLE		0,10
#define RS			  1,31
#define DATA_4		0,11
#define DATA_5		0,25	
#define DATA_6		0,26
#define DATA_7		1,4
//endereços das linhas
#define     lcd_primeira_linha      0x80
#define     lcd_segunda_linha       0xC0
#define     lcd_terceira_linha      0x94
#define     lcd_quarta_linha  	    0xD4
//modo de Funcionamento do display
//cursor do Display
#define Cursor 0x0C           // 0x0C Desliga o Cursor / 0x0D Liga o Cursor 
//sentido de deslocamento do cursor com a entrada de caractere
#define CursorDesloc 0x06     // 0x06 Deslocamento do cursor p/ a direita
                              // usar 0x04 p/ esquerda.   
   
//prototipo de funções.
void LcdInit(void);
void LcdBusy(void);
void LcdPuts(char *String);
char LcdIntStat(char valor);
void LcdEnviaNibble( uint8_t dado );
void LcdGotoXy( uint8_t x, uint8_t y );
void LcdPrintNum(short Num, short Digits);
void LcdEnviaByte( uint8_t rs, uint8_t dado );

#endif


