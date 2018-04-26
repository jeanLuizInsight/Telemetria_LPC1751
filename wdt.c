/*********************************************************************************
* wdt.c
* Microcontrolador LPC1766
* Programador: Jean Luiz Zanatta
* Data: 06/08/2014					
**********************************************************************************/
//includes
#include <LPC17xx.h>
#include "wdt.h"	

void Wdt_config(short nseg) {
	//configura fonte de clock e trava registrador
	LPC_WDT->WDCLKSEL  = 0x01;
	LPC_WDT->WDCLKSEL |= WDLOCK;	
	//configura constante do watchdog
	LPC_WDT->WDTC	= nseg * WDTC_1SEG;	
	//configura watchdog e reset
	LPC_WDT->WDMOD = WDEN | WDRESET;	
	//alimenta watchdog
	LPC_WDT->WDFEED = 0xAA;
	LPC_WDT->WDFEED = 0x55;		
	return;
}

void Wdt_feed(void) {
	//alimenta watchdog
	LPC_WDT->WDFEED = 0xAA;
	LPC_WDT->WDFEED = 0x55;		
	return;
}	
