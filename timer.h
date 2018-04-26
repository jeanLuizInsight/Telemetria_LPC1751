/******************************************************************************
* timer.h				
* Microcontrolador LPC17xx
* Programador: Jean Luiz Zanatta
* Data: 06/08/2014							
*******************************************************************************/

//inicialização dos timers.
void Timer0_init (void);
void Timer1_init (void);
void Timer2_init (void);
void Timer3_init (void);

//funções de espera: Timer 0.
void delay_ms (uint32_t ms);
void delay_us (uint32_t us);

//funções de espera em interrupções: Timer 1.
void Int_delay_ms (uint32_t ms);
void Int_delay_us (uint32_t us);

//funções de interrupção: Timer 2 e 3.
void Timer2_ISR_set (uint32_t ms);
void Timer3_ISR_set (uint32_t ms);
