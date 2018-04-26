/*************************************************************************************************
* Sistema de telemetria por rede GSM.
* Arquivo: application.h
* Programador: Jean Luiz Zanatta
* Data: 07/08/2014
*************************************************************************************************/
//defines
#define PRIMARI_DNS	 	"8.8.8.8"
#define SECONDARY_DNS	"8.8.4.4"

//prototipo de funções
int PDP_connect (void);
int Get_GSM_time (void);
int Solicita_credito (void);
int Close_conn (int numConn);
int Dns_res (char *dominio);
int NTP_get_time (char *ntpSvrAddress);
int Send_data (int numConn, char *data, int sendLen);
int Open_conn (int numConn, char modo, char *dominio, int porta);
int Send_SMS_Cel (char *data, int sendLen); 

//estrutura para armazenar data e hora
struct Datatime{
	int dia;
	int mes;
	int ano;	
	unsigned char hora;
	unsigned char minuto;
	unsigned char segundo;
};

//variveis globais
extern struct Datatime _rtc;
extern char _strCredito[250];
extern char _cmdS[250];
extern char _diasMes[12];
extern char _ipServer[16];
