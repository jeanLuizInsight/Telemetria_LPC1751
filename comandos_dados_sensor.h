/*************************************************************************************************
* Sistema de telemetria por rede GSM.
* Arquivo: comandos_dados_sensor.h
* Programador: Jean Luiz Zanatta
* Data: 07/08/2014
*************************************************************************************************/
#ifndef COMANDOS_DADOS_SENSOR_H
#define COMANDOS_DADOS_SENSOR_H

//defines
//comando de status.
#define CMDHANDSHAKE        0x42  									//Retorno do disp: <ID, CMD, 0x6F, 0x6B, + 2bytes de CRC> 
//comandos para solicitar leitura atual de NH3.
#define CMDLERNH3						0x64										//Retorno do disp: <ID, CMD, 2bytes para NH3_atual, + 2bytes de CRC>

//variaveis globais.
extern short _NH3;

//prototipo de funções
int Send485_cmd (char cmd, char address);
#endif
