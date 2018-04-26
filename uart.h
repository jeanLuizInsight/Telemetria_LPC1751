/****************************************************************************
 *   $Id:: uart.h 5751 2010-11-30 23:56:11Z usb00423                        $
 *   Project: NXP LPC17xx UART example
 *
 *   Description:
 *     This file contains UART code header definition.
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
#ifndef __UART_H 
#define __UART_H

// Parametros da uart.
#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04
#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01
#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80
//tamanho do buffer de recepção da uart.
#define BUFSIZE		512
//RS485 mode definition. 
#define RS485_NMMEN		(0x1<<0)
#define RS485_RXDIS		(0x1<<1)
#define RS485_AADEN		(0x1<<2)
#define RS485_SEL		  (0x1<<3)
#define RS485_DCTRL		(0x1<<4)
#define RS485_OINV		(0x1<<5)

//variaveis globais
extern volatile uint32_t _UART0Status;
extern volatile uint32_t _UART1Status;
extern volatile uint8_t _UART0TxEmpty;
extern volatile uint8_t _UART1TxEmpty;
extern volatile uint8_t _UART0Buffer[BUFSIZE];
extern volatile uint8_t _UART1Buffer[BUFSIZE];
extern volatile uint32_t _UART0Count;
extern volatile uint32_t _UART1Count;

//protótipo de funções.
uint32_t UART_init (uint32_t portNum, uint32_t baudrate);
void UART0_IRQHandler (void);
void UART1_IRQHandler (void);
void UART_send (uint32_t porta, uint8_t *buffer, uint32_t length );
int UART_receiving_wait (char uartPort, short msTimeOut);

#endif /* end __UART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
