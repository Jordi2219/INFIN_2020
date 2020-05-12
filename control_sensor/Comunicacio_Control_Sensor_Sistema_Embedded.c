/*Fita 3: Comunicación Control Sensor - Sistema Embedded - Grupo F ☻ */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>                                                        
#include <termios.h>       
#include <sys/ioctl.h>
    
#define BAUDRATE B9600                                                
//#define MODEMDEVICE "/dev/ttyS0"        //Conexión IGEP - Arduino
#define MODEMDEVICE "/dev/ttyACM1"         //Conexión directa PC(Linux) - Arduino                                   
#define _POSIX_SOURCE 1 /* POSIX comarchaliant source */ 

//Definición de variables globales                      
int fd, i = 0, res=0;                                                          
char buffer[255];
int bytes=0;
char barduino[256];
char missatge [256];
float menor=70; 
float mayor=0;
float medidas[100];
signed int j=-1;
float media=0;
char mediachar='u';
int median=0;
  
	                                                        
struct termios oldtio,newtio;                                            


int	ConfigurarSerie(void)
{	//Configuración del puerto serie
	int fd;                                                           
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );                             
	if (fd <0) {perror(MODEMDEVICE); exit(-1); }                            

	tcgetattr(fd,&oldtio); /* save current port settings */                 

	bzero(&newtio, sizeof(newtio));                                         
	//newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;             
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;             
	newtio.c_iflag = IGNPAR;                                                
	newtio.c_oflag = 0;                                                     

	/* set input mode (non-canonical, no echo,...) */                       
	newtio.c_lflag = 0;                                                     

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */         
	newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */ 

	tcflush(fd, TCIFLUSH);                                                  
	tcsetattr(fd,TCSANOW,&newtio);
	
		
 	sleep(2); //Para dar tiempo a que el Arduino se recupere del RESET
		
	return fd;
}               

void TancarSerie(int fd)
{
	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
}
void enterACadena(int numero, char *bufer);
void enviar_serie (void);
void recibir_serie (int j);
float map(int x, int in_min, int in_max, int out_min, int out_max);                                                                             
int main(int argc, char **argv)                                                               
{   //Definición de variables locales                                                                      			
	char tmuestra [3];
	int tmuestran=0;
	char marcha='a';				//Ya esta creada en el server(duplicada)
	char gradochar[5];
	int grado=0;
	float temperatura=0;
	int contlecturas=0;

	fd = ConfigurarSerie();

	// Enviar el mensaje 1
	scanf("%s",buffer);
	while(1){
		char accion2=buffer[1];
		switch (accion2){
			case 'M':
				//Puesta en marcha
				marcha=buffer[2];
				mediachar=buffer[5];
				median=atoi (&mediachar);
				printf("Numero escogido para hacer la media: %d\n", median);
				sprintf(tmuestra, "%c%c", buffer[3], buffer[4]);
				tmuestran=atoi (tmuestra);
				tmuestran=1+((tmuestran-1)/2);
				printf("El tiempo de muestreo para arduino es: %d\n", tmuestran);
				enterACadena(tmuestran,tmuestra); //Tener en cuenta tiempo tmuestra que es la misma
				if (tmuestran<10){
					sprintf(missatge, "%s%c%c%s%s", "AM", marcha,'0', tmuestra, "Z\n");
				}
				else{
					sprintf(missatge, "%s%c%s%s", "AM", marcha, tmuestra, "Z\n");
				}
				enviar_serie();
				recibir_serie(5);
				break;
			default:
				break;
		}
		if (marcha=='1'){
			//ACZ periodico
			sleep(tmuestran*2);
			sprintf(missatge,"ACZ");
			enviar_serie();
			recibir_serie(9);
			
			//Conversión a entero para luego convertir a grados
			sprintf(gradochar,"%c%c%c%c", barduino[3], barduino[4], barduino[5], barduino[6]);
			grado=atoi (gradochar);
			printf("Valor analogico: %d\n",grado);
			
			//Encender/Parar led 13
			sprintf(missatge,"AS131Z");
			enviar_serie();
			recibir_serie(5);
			sleep(0.5);
			sprintf(missatge,"AS130Z");
			enviar_serie();
			recibir_serie(5);
			
			//Contador de lecturas realizadas
			contlecturas++;
			printf("Lecturas realizadas: %d\n",contlecturas);
			
			//Conversión a grados
			temperatura=map(grado, 0000, 1023, 0000, 7000);
			temperatura=temperatura/100;
			printf("La temperatura es: %.2fºC\n\n",temperatura);
			
			//Array circular
			if (j>98){
				j=j;
			}
			else{
				j++;
			}
			for (int i=0; i<j+1; i++) {
				if (j>98 && i<1) {
					medidas[99]=0;  	//Se substituye el valor más antiguo
				}
				medidas[j+1-i]=medidas[j-i]; 
			}
			medidas[0]=temperatura;
			printf("Los valores del array son:\n"); 
			for (int i=0; i<j+1; i++) {
				printf(" %.2f;",medidas[i]);
			}
			printf("\n\nLa medida mas reciente es: %.2fºC\n", medidas[0]); 
			printf("La medida mas antigua es: %.2fºC\n", medidas[j]);


			//Registro maximo y minimo
			if (temperatura>mayor){
				mayor=temperatura;
				printf("Se ha actualizado el registro del maximo a: %.2fºC\n", mayor);
			}
			if (temperatura<menor){
				menor=temperatura;
				printf("Se ha actualizado el registro del minimo a: %.2fºC\n",menor );
			}
			
			//Calculo media demandada
			float suma=0;
			for (int i=0; i<median; i++){
				suma+=medidas[i];	
			}
			media=suma/median;
			printf("La media de los ultimos %d valores es: %.2fºC\n",median,media);
			printf("\n --------------------------\n");
		}		
	}                                                             
	TancarSerie(fd);
	return 0;
}
void enterACadena(int numero, char *bufer){
	//Convierte de nº entero a char
    sprintf(bufer, "%d", numero);
}

float map(int x, int in_min, int in_max, int out_min, int out_max){
	//Cambio de escala
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void enviar_serie (void){
	//Se envia
	res = write(fd,missatge,strlen(missatge));

	if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1);}

	printf("Enviats %d bytes: ",res);
	for (i = 0; i < res; i++){	
		printf("%c",missatge[i]);
	}
	printf("\n");
}
																			
void recibir_serie (int j){
	sleep(1);
	//Se mira cuantos bytes nos envia el Arduino
	ioctl(fd, FIONREAD, &bytes);
	//printf("Anem a rebre %d byte(s)\n",bytes);

	//Se recibe
	res = read(fd,barduino,1);
	for(int i=1; i<j; i++){
		res = res+read(fd,barduino+i,1);
	}
								
	printf("Rebuts %d bytes: ",res);
	for (i = 0; i < res; i++){
		printf("%c",barduino[i]);
	}
	printf("\n");
	memset( buffer, 0, 256 );
}
