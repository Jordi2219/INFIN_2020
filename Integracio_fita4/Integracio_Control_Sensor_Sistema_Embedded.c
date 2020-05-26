/* Integració Control Sensor - Sistema Embedded Grup F ☻ */

//Llibreries
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
#include <sys/ioctl.h>
#include <termios.h> 
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h> 
#include <sys/wait.h>

#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4

#define REQUEST_MSG_SIZE	1024
#define BAUDRATE B9600                                                
//#define MODEMDEVICE "/dev/ttyS0"         // Connexió IGEP - Arduino
#define MODEMDEVICE "/dev/ttyACM0"         // Connexió directa PC(Linux) - Arduino                                   
#define _POSIX_SOURCE 1 				   // POSIX comarchaliant source

//Definició de variables globals
char buffer[256];
char buffer2[256];
char missatge [256];
char barduino [256];
float medidas[100];
float medidast[10];
signed int j=-1;
signed int w=-1;
char marcha='a';
float menor=70; 
float mayor=0;
char mediachar='u';
char accion;
char caractertemps1;	
char caractertemps2;	
char caracternum;		
int fd, k = 0, res=0, ecl=0;
int bytes=0;

pthread_mutex_t varmutex;

struct termios oldtio,newtio;                                            


int	ConfigurarSerie(void)
{	// Configuració del port serie
	int fd;                                                           
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );                             
	if (fd <0) {perror(MODEMDEVICE); exit(-1); }                            

	tcgetattr(fd,&oldtio); // Save current port settings                 

	bzero(&newtio, sizeof(newtio));                                         
	// newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;             
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;             
	newtio.c_iflag = IGNPAR;                                                
	newtio.c_oflag = 0;                                                     

	// Set input mode (non-canonical, no echo,...)                       
	newtio.c_lflag = 0;                                                     

	newtio.c_cc[VTIME]    = 0;   // Inter-character timer unused        
	newtio.c_cc[VMIN]     = 1;   // Blocking read until 1 chars received

	tcflush(fd, TCIFLUSH);                                                  
	tcsetattr(fd,TCSANOW,&newtio);
	
		
 	sleep(2); 					 // Per donar temps a que l'arduino es recuperi del RESET
		
	return fd;
}               

void TancarSerie(int fd)
{
	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
}
// Declaració de funcions
void enviar_serie (void);
void recibir_serie (int j);
void floatACadena(float numero, char *bufer);
void enterACadena(int numero, char *bufer);
void* mainserver(void* parametre);
void* mainarduino(void* parametre);  
float map(int x, int in_min, int in_max, int out_min, int out_max);

// Programa
int main(int argc, char **argv){
	
	pthread_t threadserver;
	pthread_t threadarduino;
	
	pthread_create(&threadserver, NULL, mainserver, 0);   //Es crea el thread server
	pthread_create(&threadarduino, NULL, mainarduino, 0); //Es crea el thread arduino
	pthread_join(threadarduino, NULL);			
	pthread_join(threadserver, NULL);

return EXIT_SUCCESS;
}

// TcpServidor
void* mainserver(void* parametre)
{
	struct sockaddr_in	serverAddr;
	struct sockaddr_in	clientAddr;
	unsigned int			sockAddrSize;
	int			sFd;
	int			newFd;
	int 		result;
	char grande[5];
	char pequeno[5];
	char ultimo[5];
	char longitud[4];

	// Preparar l'adreça local
	sockAddrSize=sizeof(struct sockaddr_in);
	bzero ((char *)&serverAddr, sockAddrSize); // Posar l'estructura a zero
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT_NUM);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Crear un socket
	sFd=socket(AF_INET, SOCK_STREAM, 0);
	
	// Nominalitzar el socket
	result = bind(sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
	
	// Crear una cua per les peticions de connexió
	result = listen(sFd, SERVER_MAX_CONNECTIONS);
	
	// Bucle d'acceptació de connexions*/
	
	printf("El servidor esta conectado\n\n");
	while(1){ 
		printf("-----------------------------------------------"); 
		printf("\nServidor esperando connexiones\n");

		// Esperar conexió. sFd: socket pare, newFd: socket fill
		newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
		printf("Connexión aceptada del cliente: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		pthread_mutex_lock(&varmutex);		// Limitació de l'ús de les variables globals
		// Rebre
		memset( buffer, 0, 256 );
		result = read(newFd, buffer, 256);
		printf("Mensaje recibido del cliente(bytes %d): %s\n",	result, buffer);

		accion=buffer[1];  		// Variable de control de la estructura switch case, en funció de la lletra rebuda
		
								// Definim les variables per detectar els errors en la comunicació
		char caracter1;         // Llegeix el primer {
		char caracter2;			// Llegeix l'últim }
		switch (accion){		// Switch case que escull com actuar en funció de les peticions del client
			
			case 'm': 
			case 'M':    
				marcha=buffer[2];   				// Llegim la "v" del missatge enviat per el client			
				caracter1=buffer[0]; 				// Llegim el primer {
				caracter2=buffer[6];				// Llegim l'últim }
				caractertemps1=buffer[3]-48;		// Llegim les desenes del temps de mostreig i corregim el valor ascii
				caractertemps2=buffer[4]-48;		// Llegim les unitats del temps de mostreig i corregim el valor ascii
				caracternum=buffer[5]-48;			// Leemos el número de cifras para hacer la medias y corregimos el valor ascii
				if (caracter1=='{' && caracter2=='}') {		// Comprobem que l'inici i final de trama es correcte
					if (marcha=='0' && caractertemps1>=0 && caractertemps1<=2 && caractertemps2>=0 && caractertemps2<=9 && caracternum>=0 && caracternum<=9){      //Decidimos si la toma de muestras ha de detenerse a la vez que comprobamos que los parámetros están dentro de rango
						printf("La toma de muestras ha sido detenida\n\n"); 	
						strcpy(buffer2, "{M0}");      // Copiem el missatge en el buffer
						ecl=1;
					}
				
					else if (marcha=='1'  && caractertemps1>=0 && caractertemps1<=2 && caractertemps2>=0 && caractertemps2<=9 && caracternum>=0 && caracternum<=9){      //Decidimos si la toma de muestras ha de iniciarse a la vez que comprobamos que los parámetros están dentro de rango
						printf("La toma de muestras ah sido iniciada\n"); 	
						strcpy(buffer2, "{M0}");        // Copiem el missatge en el buffer
						printf("El tiempo de muestreo demandado es de:  %d%d s\n", caractertemps1,caractertemps2); 
						printf("El número de muestras demandado para hacer la media es de: %d medidas\n",caracternum);
						ecl=1;
					}
					
					else{
						printf("El valor de la ""v"" no es ni 0 ni 1 \n");
						strcpy(buffer2, "{M2}"); 
					}		
				}
				else {       // Si la trama es incorrecta entrem aqui
				 strcpy(buffer2, "{M1}");
				 printf("La trama recibida del cliente no es correcta"); 
				}
			break; 
		
			case 'x':					// Retornarem el máxim del array
			case 'X':
			
				caracter1=buffer[0]; 
				caracter2=buffer[2]; 
				printf("Nos han pedido el máximo\n");									
				floatACadena(mayor,grande);					// Funció que converteix un nombre float a lletres
				if (caracter1=='{' && caracter2=='}') {		// Escollim si el códi d'error ha de ser '0' o '1'
					if (mayor<10){							// Ajustem el missatge en funció de si el nombre a enviar es superior o inferior a 10
					strcpy(buffer2,"{X00");
					}
					else {
					strcpy(buffer2,"{X0");
					}
				}			
				else  {
					printf("El formato del pedido no es correcto \n");
					strcpy(buffer2, "{X1"); 
					}
								
				strcat(buffer2,grande);       // Acabem de formar el missatge
				strcat(buffer2, "}");
				printf("El valor de la medida máxima es: %.2f ºC\n", mayor); 
			break; 
			
			case 'y':                     // Retornarem el mínim del array
			case 'Y':
			
				caracter1=buffer[0]; 
				caracter2=buffer[2]; 
				printf("Nos han pedido el mínimo\n"); 
				floatACadena(menor,pequeno);
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
				strcat(buffer2,pequeno); 
				strcat(buffer2, "}"); 
				printf("El valor de la medida mínima es: %.2f ºC\n",menor); 
			break; 
			
			case 'u': 					// Retornarem l'últim nombre del array
			case 'U':
			
				caracter1=buffer[0]; 
				caracter2=buffer[2]; 
				printf("Nos han pedido el último\n"); 
				if (caracter1=='{' && caracter2=='}') {
					if (medidas[j]<10){
					strcpy(buffer2,"{U00"); 
					}
					else {
					strcpy(buffer2,"{U0"); 		
					}
				}
				else{
					if (medidas[j]<10){
					strcpy(buffer2,"{U10"); 
					}
					else {
					strcpy(buffer2,"{U1"); 		
					}
				}
				floatACadena(medidas[j],ultimo);	
				strcat(buffer2,ultimo); 
				strcat(buffer2, "}");
				j--;    					// Després de treure l'últim valor del array, eliminem aquest valor retallant en una unitat del array
			break; 
			
			case 'r':						// Reset dels registres
			case 'R':
			
				caracter1=buffer[0]; 
				caracter2=buffer[2]; 
				printf("Nos han pedido cargarnos el máximo y mínimo\n"); 
				mayor=0; 
				menor=70; 
				if (caracter1=='{' && caracter2=='}') {
					strcpy(buffer2, "{R0}"); 
					}
				else {
					strcpy(buffer2, "{R1}");
				}
				printf("Se han restablecido los valores máximo y mínimo\n"); 
			
			break; 
			
			
			case 'B':						// Direm el tamany del array
			case 'b':
			
				caracter1=buffer[0]; 
				caracter2=buffer[2];
				printf("Nos han pedido el número de muestras\n"); 
				if (caracter1=='{' && caracter2=='}') {
					if (j+1<10){
					strcpy(buffer2,"{B0000"); 
					}
					else if (j+1>=10&&j+1<100){
					strcpy(buffer2,"{B000"); 
					}
					else if (j+1>=100&&j+1<1000){
					strcpy(buffer2,"{B00"); 
					}
					else {
					strcpy(buffer2,"{B0"); 		
					}
				}
				else{
					if (j+1<10){
					strcpy(buffer2,"{B1000"); 
					}
					else if (j+1>=10&&j+1<100){
					strcpy(buffer2,"{B100"); 
					}
					else if (j+1>=100&&j+1<1000){
					strcpy(buffer2,"{B10"); 
					}
					else {
					strcpy(buffer2,"{B1"); 
					}
	
				}
				enterACadena(j+1,longitud);
				strcat(buffer2,longitud);
				strcat(buffer2,"}");
				printf("El tamaño del array de medidas es de %d posiciones\n",j+1); 
				
			break;
			
			default:						// En cas de que la lletra d'acció no s'entengui, saltarà un error per parámetre fora de rang
			strcpy(buffer2,"{E2}");			// Donat que la lletra rebuda en aquest cas mancarà de sentit, enviem per defecte la lletra 'E', de error
			printf("No se ha podido entender la letra de acción, devolvemos error 2\n"); 
			break; 
		}	
		// Enviar 
		result = write(newFd, buffer2, strlen(buffer2)+1); // +1 per enviar el 0 final de cadena
		printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer2);
		// Tancar el socket fill
		result = close(newFd);
		printf("Acción demandada por el cliente finalizada\n"); 
		printf("-----------------------------------------------\n\n\n\n");
		pthread_mutex_unlock(&varmutex);
	}
	pthread_exit(NULL);
    return NULL;
}

// Enviar/Rebre arduino
void* mainarduino(void* parametre){   
	// Definició de variables locals                                                                      			
	char tmuestra [3];
	int tmuestran=0;
	char gradochar[5];
	int grado=0;
	float temperatura=0;
	int contlecturas=0;
	float media=0;
	int median=0;
	
	while (1) {
		fd = ConfigurarSerie();
		pthread_mutex_lock(&varmutex); // Limitació de l'ús de les variables globals
		switch (accion){
			case 'M':
				// Posta en marxa
				if (ecl==1){	// Variable que autoritza a realitzar el case M un cop, per missatge M rebut del client 
					ecl=0;
					mediachar=caracternum+48;
					median=atoi (&mediachar);
					printf("Numero escogido para hacer la media: %d\n", median);
					sprintf(tmuestra, "%c%c", caractertemps1+48, caractertemps2+48);
					tmuestran=atoi (tmuestra);
					tmuestran=1+((tmuestran-1)/2);
					printf("El tiempo de muestreo para arduino es: %d\n", tmuestran);
					enterACadena(tmuestran,tmuestra); 
					if (tmuestran<10){
						sprintf(missatge, "%s%c%c%s%s", "AM", marcha,'0', tmuestra, "Z\n");
					}
					else{
						sprintf(missatge, "%s%c%s%s", "AM", marcha, tmuestra, "Z\n");
					}
					enviar_serie();
					recibir_serie(5);
				}
				break;
			default:
				break;
		}
		if (marcha=='1'){
			// ACZ periòdic
			sleep(tmuestran*2);
			sprintf(missatge,"ACZ");
			enviar_serie();
			recibir_serie(9);
			
			// Conversió a enter per despres convertir a graus
			sprintf(gradochar,"%c%c%c%c", barduino[3], barduino[4], barduino[5], barduino[6]);
			grado=atoi (gradochar);
			printf("Valor analogico: %d\n",grado);
			
			// Encendre/Parar led 13
			sprintf(missatge,"AS131Z");
			enviar_serie();
			recibir_serie(5);
			sleep(0.5);
			sprintf(missatge,"AS130Z");
			enviar_serie();
			recibir_serie(5);
			
			// Comptador de lectures realitzades
			contlecturas++;
			printf("Lecturas realizadas: %d\n",contlecturas);
			
			// Conversió a graus
			temperatura=map(grado, 0000, 1023, 0000, 7000);
			temperatura=temperatura/100;
			printf("La temperatura es: %.2fºC\n\n",temperatura);
			
			// Array circular mostres
			if (w>8){
				w=w;
			}
			else{
				w++;
			}
			for (int i=0; i<w+1; i++) {
				if (w>8 && i<1) {
					medidast[9]=0;  	// ES substitueix el valor més antic
				}
				medidast[w+1-i]=medidast[w-i]; 
			}
			medidast[0]=temperatura;
			
			// Cálcul mitjana demanada
			float suma=0;
			for (int i=0; i<median; i++){
				suma+=medidast[i];	
			}
			media=suma/median;
			printf("La media de los ultimos %d valores es: %.2fºC\n",median,media);
			
			// Array circular mitjanes
			if (j>98){
				j=j;
			}
			else{
				j++;
			}
			for (int i=0; i<j+1; i++) {
				if (j>98 && i<1) {
					medidas[99]=0;  	// És substitueix el valor més antic
				}
				medidas[j+1-i]=medidas[j-i]; 
			}
			medidas[0]=media;
			printf("Los valores del array son:\n"); 
			for (int i=0; i<j+1; i++) {
				printf(" %.2f;",medidas[i]);
			}
			printf("\n\nLa medida mas reciente es: %.2fºC\n", medidas[0]); 
			printf("La medida mas antigua es: %.2fºC\n", medidas[j]);


			// Registre màxim i mínim
			if (media>mayor){
				mayor=media;
				printf("Se ha actualizado el registro del maximo a: %.2fºC\n", mayor);
			}
			if (media<menor){
				menor=media;
				printf("Se ha actualizado el registro del minimo a: %.2fºC\n",menor );
			}
			printf("\n --------------------------\n");
		}
	pthread_mutex_unlock(&varmutex);	
	TancarSerie(fd);
	}		                                                           
	pthread_exit(NULL);
    return NULL;
}

float map(int x, int in_min, int in_max, int out_min, int out_max){
	// Canvi d'escala
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void enviar_serie (void){
	// S'envia
	res = write(fd,missatge,strlen(missatge));

	if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1);}

	printf("Enviats %d bytes: ",res);
	for (k = 0; k < res; k++){	
		printf("%c",missatge[k]);
	}
	printf("\n");
}
																			
void recibir_serie (int j){
	sleep(1);
	// Es mira quants bytes envia l'Arduino
	ioctl(fd, FIONREAD, &bytes);
	
	// Es rep
	res = read(fd,barduino,1);
	for(int k=1; k<j; k++){
		res = res+read(fd,barduino+k,1);
	}
								
	printf("Rebuts %d bytes: ",res);
	for (k = 0; k < res; k++){
		printf("%c",barduino[k]);
	}
	printf("\n");
	memset( buffer, 0, 256 );
}
void floatACadena(float numero, char *bufer){
    sprintf(bufer, "%.2f", numero);
}
void enterACadena(int numero, char *bufer){
    sprintf(bufer, "%d", numero);
}
