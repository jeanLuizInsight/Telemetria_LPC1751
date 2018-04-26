/******************************************************************************
* timer16.c					
* Microcontrolador LPC1766
* Programador: Jean Luiz Zanatta
* Data: 08/08/2014							
*******************************************************************************/

//includes
#include <LPC17xx.h>
#include <stdio.h>
#include "timer.h"

//função de inicialização do timer 0.	Configura clock para o timer 0. 
void Timer0_init(void) {		
	LPC_TIM0->TCR = 0x02;                /* reset timer */	
	LPC_TIM0->MCR = 0x04;                /* stop timer on match */	
	return;
}

//função de inicialização do timer 1.	Configura clock para o timer 1. 
void Timer1_init(void) {	
	LPC_TIM1->TCR = 0x02;                /* reset timer */	
	LPC_TIM1->MCR = 0x04;                /* stop timer on match */	
	return;
}

//função de inicialização do timer 2.	Configura clock para o timer 2. 
void Timer2_init(void) {	
	LPC_SC->PCONP |= (1<<22);	
	return;
}

//função de inicialização do timer 2.	Configura clock para o timer 2. 
void Timer3_init(void) {	
	LPC_SC->PCONP |= (1<<23);
	return;
}

//função DelayUs - Função de espera.	Espera o valor em microsegundos 
void delay_us(uint32_t us) {	
	LPC_TIM0->PR  = ((SystemCoreClock / 4) / 1000000) - 1; /* Prescaler para contador de microssegundos */
	LPC_TIM0->TCR = 0x02;                /* reset timer */
  LPC_TIM0->MR0 = us;
  LPC_TIM0->IR  = 0xff;                /* reset all interrrupts */
  LPC_TIM0->TCR = 0x01;                /* start timer */
  /* wait until delay time has elapsed */
  while (LPC_TIM0->TCR & 0x01);
  return;
}

//função DelayMs - Função de espera.	Espera o valor em milisegundos
void delay_ms(uint32_t ms) {	
	LPC_TIM0->PR  = ((SystemCoreClock / 4) / 1000) - 1; // Prescaler par ms.
	LPC_TIM0->TCR = 0x02;                /* reset timer */
  LPC_TIM0->MR0 = ms;
  LPC_TIM0->IR  = 0xff;                /* reset all interrrupts */
  LPC_TIM0->TCR = 0x01;                /* start timer */
  /* wait until delay time has elapsed */
  while (LPC_TIM0->TCR & 0x01);
  return;
}

//função IntDelayUs - Função de espera em interrupções. Espera o valor em microsegundos pelo timer 1.
void Int_delay_us(uint32_t us) {	
	LPC_TIM1->PR  = ((SystemCoreClock / 4) / 1000000) - 1; /* Prescaler para contador de microssegundos */
	LPC_TIM1->TCR = 0x02;                /* reset timer */
	LPC_TIM1->MR0 = us;
	LPC_TIM1->IR  = 0xff;                /* reset all interrrupts */
	LPC_TIM1->TCR = 0x01;                /* start timer */
	/* wait until delay time has elapsed */
  while (LPC_TIM1->TCR & 0x01);
  return;
}

//função IntDelayms - Função de espera em interrupções. Espera o valor em milisegundos pelo timer 1.
void Int_delay_ms(uint32_t ms) {	
	LPC_TIM1->PR  = ((SystemCoreClock / 4) / 1000) - 1; /* Prescaler para contador de microssegundos */
	LPC_TIM1->TCR = 0x02;                /* reset timer */
	LPC_TIM1->MR0 = ms;
	LPC_TIM1->IR  = 0xff;                /* reset all interrrupts */
	LPC_TIM1->TCR = 0x01;                /* start timer */
  /* wait until delay time has elapsed */
  while (LPC_TIM1->TCR & 0x01);
  return;
}

//função Timer2ISRSet - Função de Interrupção. Instala interrupção de tempo do timer 2
void Timer2_ISR_set(uint32_t ms) {
	LPC_TIM2->TCR = 0x02;                // Reseta timer 
	LPC_TIM2->PR  = ((SystemCoreClock / 4) / 1000) - 1; // Configura prescaler para um clock de 1ms.
  LPC_TIM2->MR0 = ms;			 		 // Configura tempo em microsegundos.  
  LPC_TIM2->IR  = 0xff;                // Limpa todas as interrupções.
  LPC_TIM2->MCR = 0x03;                // Configura para resetar e gerar interrupção do Match0
  LPC_TIM2->TCR = 0x01;                // Inicia timer.
	NVIC_EnableIRQ(TIMER2_IRQn);		 // Instala interrupção.
	return;
 }
  
//função Timer3ISRSet - Função de Interrupção. Instala interrupção de tempo do timer 3
void Timer3_ISR_set(uint32_t ms) {
	LPC_TIM3->TCR = 0x02;                // Reseta timer   
	LPC_TIM3->PR  = ((SystemCoreClock/4)/1000) - 1; // Configura prescaler para um clock de 1ms.
  LPC_TIM3->MR0 = ms;					 // Configura tempo em microsegundos.  
  LPC_TIM3->IR  = 0xff;                // Limpa todas as interrupções.
  LPC_TIM3->MCR = 0x03;                // Configura para resetar e gerar interrupção do Match0
  LPC_TIM3->TCR = 0x01;                // Inicia timer.
	NVIC_EnableIRQ(TIMER3_IRQn);		 // Instala interrupção.
	return;
 }

// Handler da IRQ do Timer 2.	Handler da interrupção do timer 2.
 void TIMER2_IRQHandler(void) {
 	// Testa interrupção do macth 0.
 	if(LPC_TIM2->IR & 1) {		
 		// Desabilita interrupção.
 		LPC_TIM2->MCR &= ~(1);	
 		// Fim do tratamento da interrupção.
 		LPC_TIM2->IR |= 1;		// Limpa flag de interrupção.
 		LPC_TIM2->MCR |= 1;		// Habilita interrupção. 		
 	}
 	return;
 }

//Handler da IRQ do Timer 3.Handler da interrupção do timer 3.
void TIMER3_IRQHandler(void) {
	// Testa interrupção do macth 0.
	if(LPC_TIM3->IR & 1) {		
		// Desabilita interrupção.
		LPC_TIM3->MCR &= ~(1); 		
		// Fim do tratamento da interrupção.
		LPC_TIM3->IR |= 1;		// Limpa flag de interrupção.
		LPC_TIM3->MCR |= 1;		// Habilita interrupção.		
	}
	return;
}
