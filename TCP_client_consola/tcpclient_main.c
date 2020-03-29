
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>


#define REQUEST_MSG_SIZE	1024
#define REPLY_MSG_SIZE		500
#define SERVER_PORT_NUM		5001



 /**
* tcpClient
*/

void enterACadena(unsigned int numero, char *bufer);

void ImprimirMenu(void)
{
	printf("\n\nMenu:\n");
	printf("--------------------\n");
	printf("Opcio 1: Posar en marxa o parar l'adquisicio\n");
	printf("Opcio 2: Demanar mostra mes antiga\n");
	printf("Opcio 3: Demanar maxim\n");
	printf("Opcio 4: Demanar minim\n");
	printf("Opcio 5: Reset maxim i minim\n");
	printf("Opcio 6: Demanar comptador del nombre de mostres guardades\n");
	printf("Opcio 7: Sortir\n");
	printf("--------------------\n");
}



int main(int argc, char *argv[])
{
	struct 			sockaddr_in	serverAddr;						//Declaració de variables
	char	   		serverName[] = "127.0.0.1"; 				//Adreça IP on està  el servidor
	int				sockAddrSize;
	int				sFd;
	int 			result;
	char			buffer[256];
	char			buffer2[256];
	
	int 			marxa = 0;									//Variables utilitzades abans d'enviar missatge
    char 			marxaC [] = "0";
    unsigned int 	temps = 01;
    char 			tempsC [] = "01";
    unsigned int 	numeromitjana = 1;
    char 			numeromitjanaC [] = "1";
	char			missatge1 [] = "{M";
	char			caracterfitrama [] = "}";
	int				input;
	
	char			comandaMarxa [3] = "{M}";					//Variables utilitzades després de rebre missatge retorn
	char			comandaAntiga [3] = "{U}";
	char			comandaMaxim [3] = "{X}";
	char			comandaMinim [3] = "{Y}";
	char			comandaReset [3] = "{R}";
	char			comandaComptador [3] = "{B}";
	
	
	while (input != 7)											//creacio ejecució continua
	{
		input = 0;
		strcpy(missatge1, "{M");								//"reiniciar" variable
			
		ImprimirMenu();
		printf("Selecciona una opcio: \n");
		scanf("%d", &input);
		                            
	
		switch (input)											//segons l'opció seleccionada per l'usuari, es realitzen accions i es configura el missatge
			{
				case 1:
					
					printf("Heu seleccionat l'opcio 1\n");	 
					
					do 
					{
						printf("Determinar posar en marxa o parar l'adquisicio (0 o 1): ");			//demanar v (parada o marxa):
						scanf("%d", &marxa);
					}
					while (marxa <0 || marxa >1);
						
					if (marxa == 0)
					{
						strcpy(marxaC,"0");
					}
					else if (marxa == 1)
					{
						strcpy(marxaC,"1");
					}
					else
					{
					}
					printf("Cadena (caracter) de marxa: %s \n", marxaC);							//no caldria mostrar-ho per pantalla
									
					do
					{
						printf("Determinar temps en segons de mostreig (1 a 20): ");				// demanar temps:
						scanf("%u", &temps);
					}
					while(temps <1 || temps>20);
					
					enterACadena(temps, tempsC);
					printf("Cadena (caracter) temps en segons: %s \n", tempsC);						//no caldria mostrar-ho per pantalla
					
					do
					{
						printf("Determinar numero de mostres fer la mitjana (1 a 9): ");			// demanar mostres mitjana:
						scanf("%u", &numeromitjana);
					}
					while(numeromitjana <1 || numeromitjana >9);
					
					enterACadena(numeromitjana, numeromitjanaC);
					printf("Cadena (caracter) numero de mosres mitjana: %s \n", numeromitjanaC); 	//no caldria mostrar-ho per pantalla
					
					strcat(missatge1, marxaC);														//concatenació cadenes per crear el missatge
					
					if (temps<10)
					{
						strcat(missatge1, "0");
						strcat(missatge1, tempsC);
					}
					else if (temps>9)
					{
						strcat(missatge1, tempsC);
					}
					else
					{
					}
								
					strcat(missatge1, numeromitjanaC);
					strcat(missatge1, caracterfitrama);
					
					printf("Cadena missatge marxa: %s \n", missatge1);
					strcpy(buffer, missatge1);
					
					break;
					
				case 2:
					printf("Heu seleccionat l'opcio 2\n");	 
					strcpy(buffer,"{U}");
					break;
					
				case 3:
					printf("Heu seleccionat l'opcio 3\n");	
					strcpy(buffer,"{X}");
					break;
					
				case 4:
					printf("Heu seleccionat l'opcio 4\n");	
					strcpy(buffer,"{Y}");
					break;
					
				case 5:
					printf("Heu seleccionat l'opcio 5\n");	
					strcpy(buffer,"{R}");
					break;
					
				case 6:
					printf("Heu seleccionat l'opcio 6\n");	
					strcpy(buffer,"{B}");
					break;
				
				case 7:
					return 0;
					break;
				
				case 0x0a: //Això és per enviar els 0x0a (line feed) que s'envia quan li donem al Enter
					break;
				default:
					printf("Opció incorrecta\n");	
					printf("He llegit 0x%hhx \n",input);
					break;
			}

	
	
		/*Crear el socket*/
		sFd=socket(AF_INET,SOCK_STREAM,0);

		/*Construir l'adreça*/
		sockAddrSize = sizeof(struct sockaddr_in);
		bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
		serverAddr.sin_family=AF_INET;
		serverAddr.sin_port=htons (SERVER_PORT_NUM);
		serverAddr.sin_addr.s_addr = inet_addr(serverName);

		/*Conexió*/
		result = connect (sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
		if (result < 0)
		{
			printf("Error en establir la connexiÃ³\n");
			exit(-1);
		}
		printf("\nConnexiÃ³ establerta amb el servidor: adreÃ§a %s, port %d\n",	inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

		/*Enviar*/
		//strcpy(buffer,missatge); //Copiar missatge a buffer
		result = write(sFd, buffer, strlen(buffer));
		printf("Missatge enviat a servidor(bytes %d): %s\n",	result, buffer);					//S'envia al servidor el missatge configurat prèviament

		/*Rebre*/
		result = read(sFd, buffer2, 256);
		printf("Missatge rebut del servidor(bytes %d): %s\n\n",	result, buffer2);					//Es rep un nou missatge del servidor
		
		
			
		if (strncmp(buffer2, comandaMarxa, 2) == 0)													//Segons el caracter de comanda, es realitzen accions per mostrar el codi de retorn i els valors correctes
		{
			printf("Codi retorn: %c\n", buffer2[2]);	
		}
		else if (strncmp(buffer2,comandaAntiga, 2) == 0)
		{
			printf("Codi retorn: %c\n", buffer2[2]);	
			printf("Valor mostra mes antiga: %c%c%c%c%c\n", buffer2[3], buffer2[4], buffer2[5], buffer2[6], buffer2[7]);
		}
		else if (strncmp(buffer2,comandaMaxim, 2) == 0)
		{
			printf("Codi retorn: %c\n", buffer2[2]);	
			printf("Valor maxim: %c%c%c%c%c\n", buffer2[3], buffer2[4], buffer2[5], buffer2[6], buffer2[7]);
		}
		else if (strncmp(buffer2,comandaMinim, 2) == 0)
		{
			printf("Codi retorn: %c\n", buffer2[2]);	
			printf("Valor minim: %c%c%c%c%c\n", buffer2[3], buffer2[4], buffer2[5], buffer2[6], buffer2[7]);
		}
		else if (strncmp(buffer2,comandaReset, 2) == 0)
		{
			printf("Codi retorn: %c\n", buffer2[2]);
		}
		else if (strncmp(buffer2,comandaComptador, 2) == 0)
		{
			printf("Codi retorn: %c\n", buffer2[2]);	
			printf("Valor del comptador: %c%c%c%c\n", buffer2[3], buffer2[4], buffer2[5], buffer2[6]);
		}
		else 
		{
		}
		
		/*Tancar el socket*/
		close(sFd);

	}
		
	return 0;

}




/*Funcions*/

void enterACadena(unsigned int numero, char *bufer){
    sprintf(bufer, "%u", numero);
}


