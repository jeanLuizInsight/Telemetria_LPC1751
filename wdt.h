/*********************************************************************************
* Medidor de peso com celula de carga.
* wdt.h					
* Microcontrolador LPC1766
* Programador: Jean Luiz Zanatta
* Data: 06/08/2014				
**********************************************************************************/

//defines
#define		WDLOCK    0x80000000
#define		WDEN		  0x01
#define		WDRESET   0x02
#define		WDTOF		  0x04
#define   WDTC_1SEG 1000000

//protótipo de funções
void Wdt_config (short nseg);
void Wdt_feed (void);
