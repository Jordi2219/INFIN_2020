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

////////
void ImprimirMenu(void)
{
	printf("\n\nMenu:\n");
	printf("--------------------\n");
	printf("1: Posar en marxa o para l'adquisicio\n");
	printf("2: Demanar mostra mes antiga\n");
	printf("3: Demanar maxim\n");
	printf("4: Demanar minim\n");
	printf("5: Reset maxim i minim\n");
	printf("6: Demanar comptador del nombre de mostres guardades\n");
	printf("s: Sortir\n");
	printf("--------------------\n");
}

/////////


int main(int argc, char *argv[]){
	struct sockaddr_in	serverAddr;
	char	    serverName[] = "127.0.0.1"; //AdreÃ§a IP on està  el servidor
	int			sockAddrSize;
	int			sFd;
	int			mlen;
	int 		result;
	char		buffer[256];
	//char		missatge[256];


////////
	char input;

	ImprimirMenu();                             
	input = getchar();

	switch (input)
	{
		case '1':											//falta
			printf("Heu seleccionat l'opcio 1\n"); 			//printf("Escriure missatge a enviar\n");
			strcpy(buffer,"Opcio 1 seleccionada");			//scanf("%s", missatge);
															//strcpy(buffer, missatge); //Copiar missatge a buffer			
			break;
			
		case '2':
			printf("Heu seleccionat l'opcio 2\n");	 
            strcpy(buffer,"{U}");
			break;
			
		case '3':
			printf("Heu seleccionat l'opcio 3\n");	
            strcpy(buffer,"{X}");
			break;
			
		case '4':
			printf("Heu seleccionat l'opcio 4\n");	
            strcpy(buffer,"{Y}");
			break;
			
		case '5':
			printf("Heu seleccionat l'opcio 5\n");	
            strcpy(buffer,"{R}");
			break;
			
		case '6':
			printf("Heu seleccionat l'opcio 6\n");	
            strcpy(buffer,"{B}");
            //
			break;
		
		case 0x0a: //Això és per enviar els 0x0a (line feed) que s'envia quan li donem al Enter
			break;
		default:
			printf("Opció incorrecta\n");	
			printf("He llegit 0x%hhx \n",input);
			break;
	}

	

////////////


	/*Crear el socket*/
	sFd=socket(AF_INET,SOCK_STREAM,0);

	/*Construir l'adreÃ§a*/
	sockAddrSize = sizeof(struct sockaddr_in);
	bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons (SERVER_PORT_NUM);
	serverAddr.sin_addr.s_addr = inet_addr(serverName);

	/*ConexiÃ³*/
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
	printf("Missatge enviat a servidor(bytes %d): %s\n",	result, buffer);

	/*Rebre*/
	result = read(sFd, buffer, 256);
	printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);

	/*Tancar el socket*/
	close(sFd);

	return 0;
	}
