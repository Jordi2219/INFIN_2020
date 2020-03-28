

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

#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4

#define REQUEST_MSG_SIZE	1024


/************************
*
*
* tcpServidor
*
*
*/
void enterACadena(float numero, char *bufer);
int main(int argc, char *argv[])
{
	struct sockaddr_in	serverAddr;
	struct sockaddr_in	clientAddr;
	unsigned int			sockAddrSize;
	int			sFd;
	int			newFd;
	int 		result;
	char		buffer[256];
	char		buffer2[256];
	float menor=0; 
	float mayor=0;
	char grande[5];
	char pequeno[5];
	char ultimo[5];


	
	
	/*Preparar l'adreça local*/
	sockAddrSize=sizeof(struct sockaddr_in);
	bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT_NUM);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*Crear un socket*/
	sFd=socket(AF_INET, SOCK_STREAM, 0);
	
	/*Nominalitzar el socket*/
	result = bind(sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
	
	/*Crear una cua per les peticions de connexió*/
	result = listen(sFd, SERVER_MAX_CONNECTIONS);
	
	/*Bucle s'acceptació de connexions*/
	while(1){
		printf("\nServidor esperant connexions\n");

		/*Esperar conexió. sFd: socket pare, newFd: socket fill*/
		newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
		printf("Connexión acceptada del client: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		/*Rebre*/
		memset( buffer, 0, 256 );
		result = read(newFd, buffer, 256);
		printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);


//iniciamos el tratamiento de la señal que nos ha pedido el cliente
		char accion=buffer[1]; 
		float medidas [20]; 
		char marcha;
		
		char caracter1; 
		char caracter2;
		
		
		char caractertemps1;
		char caractertemps2;
		char caracternum;
		int temps1;
		int temps2;
		int num;
		//char caracter3 [1];
		
		switch (accion)
			{
			case 'M':      //Case principal de control
			marcha=buffer[2];
						
			caracter1=buffer[0]; 
			caracter2=buffer[6];
			caractertemps1=buffer[3];
			caractertemps2=buffer[4];
			caracternum=buffer[5];
			
			temps1= atoi(&caractertemps1);
			temps2= atoi(&caractertemps2);
			num= atoi(&caracternum);
						
			
				if (caracter1=='{' && caracter2=='}') {
					
					if (marcha=='0' && temps1>=0 && temps1<=2 && temps2>=0 && temps2<=9 && num>=0 && num<=9){      //Apagamos el muestreo
					printf("We are off boys\n"); 
					    			
						for (int i=0; i<20; i++) {      //Ponemos a 0 todas las muestras
							medidas[i]=0; 
						}
					
					strcpy(buffer2, "{M0}");        //Copiamos el mensaje en el buffer
					}
				
					else if (marcha=='1' && temps1>=0 && temps1<=2 && temps2>=0 && temps2<=9 && num>=0 && num<=9){                               //Encendemos el muestro
					printf("We are back bbys\n"); 
						for (int i=0; i<20; i++) {     //Simulamos que tomamos muestras, ignoramos el tiempo
							medidas[i]=i; 
						}
					strcpy(buffer2, "{M0}");        //Copiamos el mensaje en el buffer
					}
					
					else{
					printf("El valor no es ni 0 ni 1\n");
					strcpy(buffer2, "{M2}"); 
					}
					//Por el momento no tenemos en cuenta el tiempo de muestreo	
				
				}
				else {
				 strcpy(buffer2, "{M1}");
				}
				break; 
			
		
		
			case 'x':						//Haremos el máximo
			case 'X':
			
			caracter1=buffer[0]; 
			caracter2=buffer[2]; 

			printf("Nos han pedido el máximo\n");
			
								
				for (int i=0; i<20; i++){
					if (medidas[i]> mayor){
							mayor=medidas[i];
					} 
				}
				
				enterACadena(mayor,grande);
					
				
				if (caracter1=='{' && caracter2=='}') {
					strcpy(buffer2,"{X0"); 
					}			
				else{
					printf("El valor no es ni 0 ni 1\n");
					strcpy(buffer2, "{X1"); 
					}
								
				strcat(buffer2,grande); //Copiar missatge a buffer
				strcat(buffer2, "}");

			break; 
			
			case 'y':                     //Haremos el mínimo
			case 'Y':
			
				caracter1=buffer[0]; 
				caracter2=buffer[2]; 
				
				printf("Nos han pedido el menor\n"); 
				for (int i=0; i<20; i++){
					if (medidas[i]<menor){
						menor=medidas[i];
					}
				}
				enterACadena(menor,pequeno);
				
				if (caracter1=='{' && caracter2=='}') {
				
					if (menor<10){
					strcpy(buffer2,"{Y00");
					}
					else {
					strcpy(buffer2,"{Y0");
					}
				}
				
				else {
					if (menor<10){
					strcpy(buffer2,"{Y10");
					}
					else {
					strcpy(buffer2,"{Y1");
					}
				}
						
				strcat(buffer2,pequeno); //Copiar missatge a buffer
				strcat(buffer2, "}"); 
			break; 
			
			case 'u': 					//Haremos el último
			case 'U':
			
			caracter1=buffer[0]; 
			caracter2=buffer[2]; 
			
			printf("Nos han pedido el último\n"); 
			//Concatenamos las diferentes partes
			enterACadena(medidas[20],ultimo);
			
				if (caracter1=='{' && caracter2=='}') {
				
					strcpy(buffer2,"{U0"); 
				}
				else{
					strcpy(buffer2,"{U1");
				}
				
			strcat(buffer2,ultimo); //Copiar missatge a buffer
			strcat(buffer2, "}");


			break; 
			
			case 'r':						//Resetearemos los registros
			case 'R':
			
			caracter1=buffer[0]; 
			caracter2=buffer[2]; 
			
			printf("Nos han pedido cargarnos el máximo y mínimo\n"); 
			mayor=0; 
			menor=0; 
			
				if (caracter1=='{' && caracter2=='}') {
					strcpy(buffer2, "{R0}"); 
					}
				else {
					strcpy(buffer2, "{R1}");
				}
			
			break; 
			
			
			case 'B':						//diremos el tamaño del array
			case 'b':
			
			caracter1=buffer[0]; 
			caracter2=buffer[2]; 
			
			
			printf("Nos han pedido el número de muestras\n"); 
			
				if (caracter1=='{' && caracter2=='}') {
					strcpy(buffer2,"{B020.0}"); 
				}
				else{
					strcpy(buffer2,"{B120.0}"); 
				}
			break;
			
			/*default:
			caracter3[0]=accion;
			printf("Cadena caracter3: %s \n", caracter3);
			strcpy(buffer2,"{");
			strcat(buffer2,caracter3);
			strcat(buffer2,"2}");
			*/
			
			}
			
			/*Enviar*/
			result = write(newFd, buffer2, strlen(buffer2)+1); //+1 per enviar el 0 final de cadena
			printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer2);
			
			/*Tancar el socket fill*/
			result = close(newFd);
			
		}
}
void enterACadena(float numero, char *bufer){
    sprintf(bufer, "%.2f", numero);
}
