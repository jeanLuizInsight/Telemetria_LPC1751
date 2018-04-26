/******************************************************************************
* Monitoramento do gás amônia sensor TGS2444.
* gpio.h				
* Microcontrolador LPC17xx
* Programador: Jean Luiz Zanatta
* Data: 06/08/2014							
*******************************************************************************/

//includes
#include <lpc17xx.h>

//prototipo de funções
void GPIO_init (void);
uint8_t GPIO_get_value (uint8_t porta, uint8_t pino);
void GPIO_set_dir (uint8_t porta, uint8_t pino, uint8_t dir);
void GPIO_set_value (uint8_t porta, uint8_t pino, uint8_t value);
