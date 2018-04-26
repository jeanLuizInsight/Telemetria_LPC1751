/****************************************************************************
 *   $Id:: uart.c 5751 2010-11-30 23:56:11Z usb00423                        $
 *   Project: NXP LPC17xx UART example
 *
 *   Description:
 *     This file contains UART code example which include UART initialization, 
 *     UART interrupt handler, and APIs for UART access.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#include <lpc17xx.h>
#include "timer.h"
#include "type.h"
#include "uart.h"
#include "rs485.h"

//variaveis globais
volatile uint32_t _UART0Status;
volatile uint32_t _UART1Status;
volatile uint8_t _UART0TxEmpty = 1;
volatile uint8_t _UART1TxEmpty = 1;
volatile uint8_t _UART0Buffer[BUFSIZE];
volatile uint8_t _UART1Buffer[BUFSIZE];
volatile uint32_t _UART0Count = 0;
volatile uint32_t _UART1Count = 0;

/*****************************************************************************
** Function name:		UART0_IRQHandler
**
** Descriptions:		UART0 interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void UART0_IRQHandler (void) 
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;
	
  IIRValue = LPC_UART0->IIR;
    
  IIRValue >>= 1;			/* skip pending bit in IIR */
  IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  if ( IIRValue == IIR_RLS )		/* Receive Line Status */
  {
	LSRValue = LPC_UART0->LSR;
	/* Receive Line Status */
	if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
	{
	  /* There are errors or break interrupt */
	  /* Read LSR will clear the interrupt */
	  _UART0Status = LSRValue;
	  Dummy = LPC_UART0->RBR;		/* Dummy read on RX to clear 
							interrupt, then bail out */
	  return;
	}
	if ( LSRValue & LSR_RDR )	/* Receive Data Ready */			
	{
	  /* If no error on RLS, normal ready, save into the data buffer. */
	  /* Note: read RBR will clear the interrupt */
	  _UART0Buffer[_UART0Count] = LPC_UART0->RBR;
	  _UART0Count++;
	  if ( _UART0Count == BUFSIZE )
	  {
		_UART0Count = 0;		/* buffer overflow */
	  }	
	}
  }
  else if ( IIRValue == IIR_RDA )	/* Receive Data Available */
  {
	/* Receive Data Available */
	_UART0Buffer[_UART0Count] = LPC_UART0->RBR;
	_UART0Count++;
	if ( _UART0Count == BUFSIZE )
	{
	  _UART0Count = 0;		/* buffer overflow */
	}
  }
  else if ( IIRValue == IIR_CTI )	/* Character timeout indicator */
  {
	/* Character Time-out indicator */
	_UART0Status |= 0x100;		/* Bit 9 as the CTI error */
  }
  else if ( IIRValue == IIR_THRE )	/* THRE, transmit holding register empty */
  {
	/* THRE interrupt */
	LSRValue = LPC_UART0->LSR;		/* Check status in the LSR to see if
									valid data in U0THR or not */
	if ( LSRValue & LSR_THRE )
	{
	  _UART0TxEmpty = 1;
	}
	else
	{
	  _UART0TxEmpty = 0;
	}
  }
  
//#if	!UART485
//  	// Se o primeiro byte não for igual ao endereço, zera contagem do buffer.
//	if((UART0Buffer[0] != RS485_ADDRESS) && (UART0Buffer[0] != RS485_BROADCAST) || (UART0Count > 10))
//		UART0Count = 0;
//#endif

}

/*****************************************************************************
** Function name:		UART1_IRQHandler
**
** Descriptions:		UART1 interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void UART1_IRQHandler (void) 
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;
	
  IIRValue = LPC_UART1->IIR;
    
  IIRValue >>= 1;			/* skip pending bit in IIR */
  IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  if ( IIRValue == IIR_RLS )		/* Receive Line Status */
  {
	LSRValue = LPC_UART1->LSR;
	/* Receive Line Status */
	if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
	{
	  /* There are errors or break interrupt */
	  /* Read LSR will clear the interrupt */
	  _UART1Status = LSRValue;
	  Dummy = LPC_UART1->RBR;		/* Dummy read on RX to clear 
								interrupt, then bail out */
	  return;
	}
	if ( LSRValue & LSR_RDR )	/* Receive Data Ready */			
	{
	  /* If no error on RLS, normal ready, save into the data buffer. */
	  /* Note: read RBR will clear the interrupt */
	  _UART1Buffer[_UART1Count] = LPC_UART1->RBR;		
	  _UART1Count++;
	  if ( _UART1Count == BUFSIZE )
	  {
		_UART1Count = 0;		/* buffer overflow */
	  }	
	}
  }
  else if ( IIRValue == IIR_RDA )	/* Receive Data Available */
  {
    /* Receive Data Available */
	_UART1Buffer[_UART1Count] = LPC_UART1->RBR;	
	_UART1Count++;
	if ( _UART1Count == BUFSIZE )
	{
	  _UART1Count = 0;		/* buffer overflow */
	}
  }
  else if ( IIRValue == IIR_CTI )	/* Character timeout indicator */
  {
	/* Character Time-out indicator */
	_UART1Status |= 0x100;		/* Bit 9 as the CTI error */
  }
  else if ( IIRValue == IIR_THRE )	/* THRE, transmit holding register empty */
  {
	/* THRE interrupt */
	LSRValue = LPC_UART1->LSR;		/* Check status in the LSR to see if
								valid data in U0THR or not */
	if ( LSRValue & LSR_THRE )
	{
	  _UART1TxEmpty = 1;
	}
	else
	{
	  _UART1TxEmpty = 0;
	}
  }
 
//#if	UART485
//  	// Se o primeiro byte não for igual ao endereço, zera contagem do buffer.
//	if((UART1Buffer[0] != RS485_ADDRESS) && (UART1Buffer[0] != RS485_BROADCAST) || (UART1Count > 10))
//		UART1Count = 0;
//#endif
//		
}

/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART port, setup pin select,
**						clock, parity, stop bits, FIFO, etc.
**
** parameters:			portNum(0 or 1) and UART baudrate
** Returned value:		true or false, return false only if the 
**						interrupt handler can't be installed to the 
**						VIC table
** 
*****************************************************************************/
uint32_t UART_init( uint32_t portNum, uint32_t baudrate )
{
  uint32_t Fdiv;
  uint32_t pclkdiv, pclk;

  if ( portNum == 0 )
  {
	LPC_PINCON->PINSEL0 &= ~0x000000F0;
	LPC_PINCON->PINSEL0 |= 0x00000050;  /* RxD0 is P0.3 and TxD0 is P0.2 */
	/* By default, the PCLKSELx value is zero, thus, the PCLK for
	all the peripherals is 1/4 of the SystemFrequency. */
	/* Bit 6~7 is for UART0 */
	pclkdiv = (LPC_SC->PCLKSEL0 >> 6) & 0x03;
	switch ( pclkdiv )
	{
	  case 0x00:
	  default:
		pclk = SystemCoreClock/4;
		break;
	  case 0x01:
		pclk = SystemCoreClock;
		break; 
	  case 0x02:
		pclk = SystemCoreClock/2;
		break; 
	  case 0x03:
		pclk = SystemCoreClock/8;
		break;
	}

    LPC_UART0->LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
	Fdiv = ( pclk / 16 ) / baudrate ;	/*baud rate */	

    LPC_UART0->DLM = Fdiv / 256;							
    LPC_UART0->DLL = Fdiv % 256;
	LPC_UART0->LCR = 0x03;		/* DLAB = 0 */
    LPC_UART0->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
	
//	LPC_UART0->LCR |= 1<<3;		/* Enable parity generation and checking. */
//	LPC_UART0->LCR |= (1<<4);	/* Sets parity */

   	NVIC_EnableIRQ(UART0_IRQn);

    LPC_UART0->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART0 interrupt */
    return (TRUE);
  }
  else if ( portNum == 1 )
  {
	LPC_PINCON->PINSEL4 &= ~0x0000C00F;
	LPC_PINCON->PINSEL4 |= 0x0000800A;	/* Enable RxD1 P2.1, TxD1 P2.0 and RTS1 p2.7 */
	  
	/* By default, the PCLKSELx value is zero, thus, the PCLK for
	all the peripherals is 1/4 of the SystemFrequency. */
	/* Bit 8,9 are for UART1 */
	pclkdiv = (LPC_SC->PCLKSEL0 >> 8) & 0x03;
	switch ( pclkdiv )
	{
	  case 0x00:
	  default:
		pclk = SystemCoreClock/4;
		break;
	  case 0x01:
		pclk = SystemCoreClock;
		break; 
	  case 0x02:
		pclk = SystemCoreClock/2;
		break; 
	  case 0x03:
		pclk = SystemCoreClock/8;
		break;
	}

    LPC_UART1->LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
	Fdiv = ( pclk / 16 ) / baudrate ;	/*baud rate */
    LPC_UART1->DLM = Fdiv / 256;							
    LPC_UART1->DLL = Fdiv % 256;
	LPC_UART1->LCR = 0x03;		/* DLAB = 0 */
    LPC_UART1->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
	
	//Controle automatico do driver 485.
	//LPC_UART1->RS485CTRL = RS485_DCTRL|RS485_OINV;

   	NVIC_EnableIRQ(UART1_IRQn);

    LPC_UART1->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART1 interrupt */
    return (TRUE);
  }
  return( FALSE ); 
}

/*****************************************************************************
** Function name:		UARTSend
**
** Descriptions:		Send a block of data to the UART 0 port based
**						on the data length
**
** parameters:			portNum, buffer pointer, and data length
** Returned value:		None
** 
*****************************************************************************/
void UART_send( uint32_t porta, uint8_t *buffer, uint32_t length )
{
  if (porta == 0) {
		DE_485();
    while ( length != 0 ) {
			/* THRE status, contain valid data */
			while ( !(_UART0TxEmpty & 0x01) );	
			LPC_UART0->THR = *buffer;
			_UART0TxEmpty = 0;	/* not empty in the THR until it shifts out */
			buffer++;
			length--;
		}
		REN_485();
  }
  else
  {
	while ( length != 0 )
    {
	  /* THRE status, contain valid data */
	  while ( !(_UART1TxEmpty & 0x01) );	
	  LPC_UART1->THR = *buffer;
	  _UART1TxEmpty = 0;	/* not empty in the THR until it shifts out */
	  buffer++;
	  length--;
    }
  }
  return;
}

//aguarda o recebimento de dados pela uart.
int UART_receiving_wait(char uartPort, short msTimeOut) {
	short tmpUartCount;		
	//trata se for uart 1.
	if(uartPort) {
		//aguarda o recebimento do primeiro byte ou time out.
		while(!_UART1Count && --msTimeOut)
			delay_ms(1);		
		//se der timeout retorne erro.
		if(msTimeOut <= 0)
			return(-1);		
		//enquanto recebe os outros...
		while(1) {
			tmpUartCount = _UART1Count;			//salva contador da uart num temporário.			
			delay_us(800);						      //aguarda o tempo do proximo byte.			
			if(tmpUartCount == _UART1Count)	//se contador da uart não mudou, cai fora.
				break;
		}
		return(_UART1Count);		
	}	else {  // Trata se for uart 0.
		//aguarda o recebimento do primeiro byte ou time out.
		while(!_UART0Count && --msTimeOut)
			delay_ms(1);		
		//se der timeout retorne erro.
		if(msTimeOut <= 0)
			return(-1);		
		//enquanto recebe os outros...
		while(1) {
			tmpUartCount = _UART0Count;			//salva contador da uart num temporário.			
			delay_us(800);						      //aguarda o tempo do proximo byte.			
			if(tmpUartCount == _UART0Count)	//se contador da uart não mudou, cai fora.
				break;
		}		
		return(_UART0Count);
	}
}
