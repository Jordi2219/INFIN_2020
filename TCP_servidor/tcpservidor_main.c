/*Programa del servidor del grupo F
 * 
 * Programa basado en el ejemplo publicado en EUSSternet por parte de los profesores de la asignatura 
 * 
 * 
 * */










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
	printf("El servidor está conectado\n\n"); 
	while(1){
		printf("-----------------------------------------------"); 
		printf("\nServidor esperant connexions\n");

		/*Esperar conexió. sFd: socket pare, newFd: socket fill*/
		newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
		printf("Connexión acceptada del client: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		/*Rebre*/
		memset( buffer, 0, 256 );
		result = read(newFd, buffer, 256);
		printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);


		
		char accion=buffer[1];  //Variable de control de la estructura switch case, en función la letra recibida
		float medidas [20]; 	//Array de medidas simuladas
		char marcha;			//Variable para determinar si arrancar o apagar la toma de medidas
		
		//Definimos las variables para detectar los errores en la comunicación
		char caracter1;         //Lee el primer {
		char caracter2;			//Lee el último }
		char caractertemps1;	//Lee el bit de las decenas del tiempo de muestreo
		char caractertemps2;	//Lee el bit de las unidades del tiempo de muestreo
		char caracternum;		//Lee el número de carácteres para hacer la media
		
		switch (accion){	//Swich case que escoge como actuar en función de las peticiones del cliente
			case 'm': 
			case 'M':    
			marcha=buffer[2];   			//Leemos la "v" del mensaje enviado por el cliente
						
			caracter1=buffer[0]; 			//Leemos el primer {
			caracter2=buffer[6];			//Leemos el último }
			caractertemps1=buffer[3]-48;		//Leemos las decenas del tiempo de muestreo y corregimos el valor asci
			caractertemps2=buffer[4]-48;		//Leemos las unidades del tiempo de muestreo y corregimos el valor asci
			caracternum=buffer[5]-48;			//Leemos el número de cifras para hacer la medias y corregimos el valor asci
			 
			
			 
				if (caracter1=='{' && caracter2=='}') {		//Comprobamos que el inicio y final de trama es correcto
					if (marcha=='0' && caractertemps1>=0 && caractertemps1<=2 && caractertemps2>=0 && caractertemps2<=9 && caracternum>=0 && caracternum<=9){      //Decidimos si la toma de muestras ha de detenerse a la vez que comprobamos que los parámetros están dentro de rango
						printf("La toma de muestras ha sido detenida\n\n"); 	
						strcpy(buffer2, "{M0}");      //Copiamos el mensaje en el buffer
					}
				
					else if (marcha=='1' /*&& temps1>=0 && temps1<=2 && temps2>=0 && temps2<=9 && num>=0 && num<=9*/){      //Decidimos si la toma de muestras ha de iniciarse a la vez que comprobamos que los parámetros están dentro de rango
						printf("La toma de muestras ah sido iniciada\n"); 
						for (int i=0; i<20; i++) {     //Simulamos que tomamos muestras, ignoramos el tiempo de toma de muestras
							medidas[i]=i; 
						}
						strcpy(buffer2, "{M0}");        //Copiamos el mensaje en el buffer
						printf("El timepo de muestreo demandado es de:  %d%d ms\n", caractertemps1,caractertemps2); 
						printf("El número de muestras demandado para hacer la media es de: %d medidas\n",caracternum); 
					}
					
					else{
						printf("El valor de la ""v"" no es ni 0 ni 1 \n");
						strcpy(buffer2, "{M2}"); 
					}
					//Por el momento no tenemos en cuenta el tiempo de muestreo	ni el número de muestras para hacer la media
				
				}
				else {       //Si la trama es incorrecta entramos aqui
				 strcpy(buffer2, "{M1}");
				 printf("La trama recibida del cliente no es correcta"); 
				}
			break; 
		
			case 'x':					//Devolveremos el máximo del array
			case 'X':
			
				caracter1=buffer[0]; 
				caracter2=buffer[2]; 

				printf("Nos han pedido el máximo\n");						
				for (int i=0; i<20; i++){		//Calculamos le máximo dentro del array
					if (medidas[i]> mayor){
						mayor=medidas[i];
					} 
				}
				enterACadena(mayor,grande);		//Función que convierte un número float a letras
				if (caracter1=='{' && caracter2=='}') {		//Escogemos si el código de error ha de ser '0' o '1'
					if (mayor<10){							//Ajustamos el mensaje en función de si el número a enviar es superior o inferior a 10
					strcpy(buffer2,"{X00");
					}
					else {
					strcpy(buffer2,"{X0");
					}
				}			
				else  {
					printf("El fomrato del pedido no es correcto \n");
					strcpy(buffer2, "{X1"); 
					}
								
				strcat(buffer2,grande);       //Acabamos de formar el mensaje
				strcat(buffer2, "}");
				printf("El valor de la medida máxima es; %.2f ºC\n", mayor); 
			break; 
			
			case 'y':                     //Devolveremos el mínimo del array
			case 'Y':
			
				caracter1=buffer[0]; 
				caracter2=buffer[2]; 
				printf("Nos han pedido el mínimo\n"); 
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
				printf("El valor de la medida mínima es: %.2f ºC\n",menor); 
			break; 
			
			case 'u': 					//Devolveremos el último número del array
			case 'U':
			
				caracter1=buffer[0]; 
				caracter2=buffer[2]; 
			
				printf("Nos han pedido el último\n"); 
				enterACadena(medidas[19],ultimo);		//Puesto que conocemos el tamaño del array de medidas simualda, nos limitamos a devolver el valor guardado en la última posicion
			
				if (caracter1=='{' && caracter2=='}') {
					strcpy(buffer2,"{U0"); 
				}
				
				else{
					strcpy(buffer2,"{U1");
				}
				printf("El valor de la última medida del array es: %.2f\n", medidas[19]); 
				strcat(buffer2,ultimo); 
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
				printf("Se han restablecido los valores máximo y mínimo\n"); 
			
			break; 
			
			
			case 'B':						//diremos el tamaño del array
			case 'b':
			
				caracter1=buffer[0]; 
				caracter2=buffer[2];
				printf("Nos han pedido el número de muestras\n"); 
				if (caracter1=='{' && caracter2=='}') {
					strcpy(buffer2,"{B020.0}"); 		//Puestos que el array simualdo tiene un tamaño constante, podemos escribir el tamaño por texto de forma directa
				}
				else{
					strcpy(buffer2,"{B120.0}"); 
				}
				printf("El tamaño del array de medidas es de 20 posiciones\n"); 
			break;
			
			default:						//En caso de que la letra de accion no se entienda, saltará un error por parámetro fuera de rango
			strcpy(buffer2,"{E2}");			//Dado que la letra recibida en este caso carecerá de sentido, enviamos por defecto la letra 'E', de error
			printf("No se ha podido entender la letra de acción, devolvemos error 2\n"); 
			break; 
		}
			
		/*Enviar*/
		result = write(newFd, buffer2, strlen(buffer2)+1); //+1 per enviar el 0 final de cadena
		printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer2);
		/*Tancar el socket fill*/
		result = close(newFd);
		printf("Acción demandada por el cliente finalizada\n"); 
		printf("-----------------------------------------------\n\n\n\n"); 
	}
}
void enterACadena(float numero, char *bufer){
    sprintf(bufer, "%.2f", numero);
}
