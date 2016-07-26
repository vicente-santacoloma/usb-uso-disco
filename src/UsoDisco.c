/* Autores: Argenis Chang, Vicente Santacoloma
 * Carnet: 08-10220, 08-11044
 * Fecha: 16-03-2011
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>     
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include "Lista.c"
#include "Lista.h"

#define TRUE  1
#define FALSE 0
#define WRITE 1
#define READ 0
#define MAXBUFFER 1000

/*Variable Globales */
int numProcesos = 1;
int numHijo = 0;
int imprimirPantalla = 1;
char * directorio = ".";
char * salida = NULL;
int fd [2];
int sig = FALSE;
int blocksize = 0;
int totalsize = 0;
int numarchivos = 0;
int numdir = 0;
char readbuffer [MAXBUFFER];
char writebuffer[MAXBUFFER];
struct Lista *procesoslibre;
struct Lista *dirdisponibles;
struct Lista *archivosprocesados;
DIR * dp;

/* Funcion que se encarga de mostrar por pantalla las opciones validas para correr el programa.
 * Parametros de entrada: Ninguno.
 * Parametro de retorno: Ninguno.
 */ 
void Help () {
 
  FILE *fp;
  char c;
  fp = fopen("help","r");
  
  while((fscanf(fp,"%c",&c))!=EOF)
    printf("%c",c); 
    
  fclose(fp);
  exit(1);
}

/* Funcion que se encarga de verificar las entradas introducidas por el usuario.
 * Parametros de entrada: Opciones introducidas por pantalla.
 * Parametro de retorno: Ninguno.
 */ 
void Parametros (int argc, char *argv[]) {
  
  int k;
  int Opcion1 = TRUE;
  int Opcion2 = TRUE;
  int Opcion3 = TRUE;

  for (k = 1; k<argc; k++) {
    
    if(strcmp(argv[k],"-h")==0) {
    
      if (argc == 2) {
	Help();
      }
      else {
	perror("UsoDisco: Invalid Option\nTry ./UsoDisco -h for more information."); 
	exit(1);
      }
    }	
    else if ((strcmp(argv[k],"-n")==0)) {
      
      if (!Opcion1) {
	perror("UsoDisco: Invalid Option\nTry ./UsoDisco -h for more information."); 
	exit(1);
      }
	
      Opcion1 = FALSE;
      k++;
      if (atoi(argv[k]) >= 1) {     
	numProcesos = atoi(argv[k]);
      }
      else {
	perror("UsoDisco: Invalid Option\nTry ./UsoDisco -h for more information."); 
	exit(1);
      }
    }
    else if ((strcmp(argv[k],"-d")==0)) {
     
      if (!Opcion2) {
	perror("UsoDisco: Invalid Option\nTry ./UsoDisco -h for more information."); 
	exit(1);
      }
      
      Opcion2 = FALSE;
      k++;
      directorio = argv[k];
      dp = opendir(directorio);
      if (dp == NULL) {
	perror("opendir");
	exit(1);
      }
      closedir(dp); 
    }
    else {
      imprimirPantalla = 0;
      salida = argv[k];
    }
  }
  
}

/* Funcion que se encarga de crear los pipe.
 * Parametros de entrada: Pipes: to_Parent y to_Child .
 * Parametro de retorno: Ninguno.
 */ 
void create_Pipe (int to_Parent [numProcesos][2], int to_Child  [numProcesos][2]) {
  
   int i; 
  
   for (i = 0; i<numProcesos; i++) {
    
    pipe(to_Parent[i]);
    pipe(to_Child[i]);

  }
  
  pipe(fd);
  
}

/* Funcion que se encarga de cerrar los pipes de los procesos esclavo.
 * Parametros de entrada: Pipes: to_Parent y to_Child. Entradas del pipe a cerrar y numero del hijo.
 * Parametro de retorno: Ninguno.
 */ 
void close_Pipe_Esclavo (int to_Parent [numProcesos][2], int to_Child  [numProcesos][2], int i, int x, int y) {
  
  close(to_Parent[i][x]);
  close(to_Child[i][y]);
  close(fd[x]);
  
}

/* Funcion que se encarga de cerrar los pipes del proceso maestro.
 * Parametros de entrada: Pipes: to_Parent y to_Child. Entradas del pipe a cerrar.
 * Parametro de retorno: Ninguno.
 */ 
void close_Pipe_Maestro (int to_Parent [numProcesos][2], int to_Child  [numProcesos][2], int x, int y) {
  
  int i;  
  
  for (i = 0; i<numProcesos; i++) {
    
    close(to_Parent[i][y]);
    close(to_Child[i][x]);
      
  }
  
  close(fd[y]);
  
}

/* Funcion que se encarga de juntar los nombres de los directirios obtenido por cada proceso.
 * Parametros de entrada: Nombre del directorio actual, Directorio anterior.
 * Parametro de retorno: Nombre completo del directorio.
 * Ejemplo: Proceso 1: etc
 *          Proceso 2: /hola
 *          Al concatenar etc/hola.
 */ 
char * concatenar (char * name, char * dir) {

  char* c = (char*)malloc(1000);
  strcpy(c,dir);
  strcat(c,name);
  
  return c;
  
}

/* Funcion que se encarga de verificar si el directorio existe o no.
 * Parametros de entrada: Nombre del directorio.
 * Parametro de retorno: Caracter que indica si existe o no.
 */ 
char * checkDir(char * name) {

  char* c = (char*)malloc(1000);
  strcpy(c,name);
  int compare = strcmp((c+strlen(c)-1), "/");
  
  if (compare != 0) 
    strcat(c,"/");
  
  return c;
  
}

/* Funcion que se encarga de chequear si algo es un archivo o directorio y en caso de ser la primera, calcula su tamano en bloques.
 * Parametros de entrada: Nombre del directorio, Buffer.
 * Parametro de retorno: Ninguno.
 */ 
void report(char * name, struct stat * buffer) {
  
  int isDir;
  int isRF;
  ushort mode = buffer -> st_mode;
  
  isDir = S_ISDIR(buffer -> st_mode);
  isRF = S_ISREG(buffer -> st_mode);
  
  if (isDir) {
    Lista_Add(&dirdisponibles,name);
  }
  else if (isRF) {
    numarchivos++;
    blocksize = blocksize+(buffer -> st_blocks);
    free(name);
  }
   
}

/* Funcion que se encarga de explorar todos los directorios.
 * Parametros de entrada: Ninguno.
 * Parametro de retorno: Ninguno.
 */ 
void explorarDir () {
  
  struct dirent *entry;
  struct stat status_buf;
  char * name;
  
  dp = opendir(directorio);
  
  if (dp != NULL) {
    
    directorio = checkDir(directorio);
    
    while((entry = readdir(dp))) {
      
      if ((strcmp(entry->d_name,".")!=0) && (strcmp(entry->d_name,"..")!=0) ) {
	name = concatenar(entry->d_name,directorio);
	
	if (stat(name, &status_buf) < 0) {
	  perror(name);
	}
	else {
	  report(name, &status_buf);
	}
	
      }
  
    }
    closedir(dp);
    char* c = (char*)malloc(1000);
    sprintf(c, "%d\t%s", blocksize,directorio);
    Lista_Add(&archivosprocesados,c);
      
  }
   
}

/* Funcion que se encarga de manejar la senal envia por el padre indicando que el proceso esclavo debera posteriormente
 * cerrar los pipes y finalizar .
 * Parametros de entrada: Senal enviada por el padre
 * Parametro de retorno: Ninguno.
 */ 
void catch_usr(int sig_num) {
  
    sig = TRUE;
    
}

/* Funcion que se encarga de pasarle al padre la informacion obtenida despues del recorrido en las direcciones, esto
 * se hace mediante pipes.
 * Parametros de entrada: Pipes: to_ Parent, to_Child.
 * Parametro de retorno: Ninguno.
 */ 
void esclavo (int to_Parent [numProcesos][2], int to_Child  [numProcesos][2]) {
  
  int nbytes, tam, i;
  char* aux;
  
  while(1) {
    
    nbytes = read(to_Child[numHijo][READ], readbuffer, sizeof(readbuffer));
    
    if (sig) {
      close_Pipe_Esclavo(to_Parent, to_Child, numHijo, WRITE, READ);
      exit(EXIT_SUCCESS);
    }
    
    blocksize = 0;
    numarchivos = 0;
    directorio = readbuffer;
    
    explorarDir();
    
    nbytes = write(to_Parent[numHijo][WRITE], &numarchivos, sizeof(int));
    nbytes = write(to_Parent[numHijo][WRITE], &blocksize, sizeof(int));
    tam = Lista_Tam(archivosprocesados);
    nbytes = write(fd[WRITE], &numHijo, sizeof(int));
    nbytes = write(to_Parent[numHijo][WRITE], &tam, sizeof(int));

    if(tam!=0) {
      aux = Lista_Remove(&archivosprocesados);
      strcpy(writebuffer, aux);
      nbytes = write(to_Parent[numHijo][WRITE], writebuffer, sizeof(writebuffer));
      free(aux);
    }
    
    tam = Lista_Tam(dirdisponibles);
    nbytes = write(to_Parent[numHijo][WRITE], &tam, sizeof(int));
    
    for (i = 0; i<tam; i++) {
      aux = Lista_Remove(&dirdisponibles); 
      strcpy(writebuffer, aux);
      nbytes = write(to_Parent[numHijo][WRITE], writebuffer, sizeof(writebuffer));
      free(aux);
    }
    
    Lista_Clean(archivosprocesados);
    Lista_Clean(dirdisponibles);
    free(directorio);
    
  }
  
}

/* Funcion que se encarga de asignarle directorios a cada uno de los hijos que se encuentren en status no ocupado,
 * luego de que un hijo culmina su recorrido le pasa la informacion por medio del pipe al padre y se vuelve a meter
 * en la lista de no ocupado.
 * Parametros de entrada: Pipes: to_ Parent, to_Child
 * Parametro de retorno: Ninguno.
 */ 
void maestro (int to_Parent [numProcesos][2], int to_Child  [numProcesos][2]) {
  
  int i, j, tam, hijo, nbytes;
  int numProcesosOcupados = 0;
  char* dir;
  char* string;
  int* k = malloc(sizeof(int));
  int* x = malloc(sizeof(int)); 
  
  while(Lista_Tam(dirdisponibles) != 0) {
    
    tam = Lista_Tam(procesoslibre);
    
    for (i = 0; i<tam; i++) {

      dir = Lista_Remove(&dirdisponibles);

      if (Lista_Tam(dirdisponibles) == 0)
	i = tam;
	
      string = Lista_Remove(&procesoslibre);
      hijo = atoi(string);
      nbytes = write(to_Child[hijo][WRITE],dir,(strlen(dir)+1));
      free(string);
      free(dir);
      numProcesosOcupados++;
	
    }
       
    for (i = 0; i<numProcesosOcupados; i++) {
      
      nbytes = read(fd[READ], k, sizeof(int));
      nbytes = read(to_Parent[*k][READ], x, sizeof(int));
      numarchivos = numarchivos+*x;
      nbytes = read(to_Parent[*k][READ], x, sizeof(int));
      totalsize = totalsize+*x;
      nbytes = read(to_Parent[*k][READ], x, sizeof(int));
      
      if(*x!=0) {
	
	nbytes = read(to_Parent[*k][READ], readbuffer, sizeof(readbuffer));
	string = (char*)malloc(1000);
	strcpy(string,readbuffer);
	Lista_Add(&archivosprocesados, string);
	
      }
      
      nbytes = read(to_Parent[*k][READ], x, sizeof(int));
       
      for (j = 0; j<*x; j++) {
	
	nbytes = read(to_Parent[*k][READ], readbuffer, sizeof(readbuffer));
	string = (char*)malloc(1000);
	strcpy(string,readbuffer);
	Lista_Add(&dirdisponibles, string);

      }
      string = (char*)malloc(1000);
      sprintf(string, "%d", *k);
      Lista_Add(&procesoslibre,string);
     
    }
    
    numProcesosOcupados = 0;
    
  }
  free(x);
  free(k);

}

/* Funcion que se encarga de imprimir los resultados obtenidos por pantalla o por un arhivo de acuerdo al parametro 
 * introducido por pantalla por el usuario.
 * Parametros de entrada: Ninguno.
 * Parametro de retorno: Ninguno.
 */ 
void Imprimir () {
  
  char* aux;
  
  if (imprimirPantalla) {
    Lista_Print(archivosprocesados);
    printf("REPORTE:\n");
    printf("Tamano Total: %d\n",totalsize);
    printf("Numero de Procesos: %d\n",numProcesos);
    printf("Numero de Archivos: %d\n",numarchivos);
    printf("Numero de Directorios: %d\n",numdir);
  }
  else {
    
    FILE *fp;
    fp = fopen(salida,"w");

    while (archivosprocesados != NULL) {
      aux = Lista_Remove(&archivosprocesados);
      fprintf(fp,"%s\n", aux);
      free(aux);
    }
    fprintf(fp,"\nREPORTE:\n");
    fprintf(fp,"Tamano Total: %d\n",totalsize);
    fprintf(fp,"Numero de Procesos: %d\n",numProcesos);
    fprintf(fp,"Numero de Archivos: %d\n",numarchivos);
    fprintf(fp,"Numero de Directorios: %d\n",numdir);
    
    fclose(fp);
	
  } 
}

/* Programa principal*/ 
int main(int argc, char * argv[]) {
  
  Parametros(argc,argv);
  
  int to_Parent [numProcesos][2]; 
  int to_Child  [numProcesos][2];
  int pid_hijo [numProcesos];
  pid_t pid [numProcesos];
  int i, nbytes;
  char* string;
  
  create_Pipe(to_Parent, to_Child);
  
  for (i = 0; i<numProcesos; i++) {
    
    pid[i] = fork();
    
    if (pid[i] == 0) {
      
      close_Pipe_Esclavo(to_Parent, to_Child, i, READ, WRITE);
      signal(SIGUSR1, catch_usr);
      sprintf(writebuffer, "%d", getpid());
      nbytes = write(fd[WRITE], &i, sizeof(int));
      nbytes = write(to_Parent[i][WRITE], writebuffer, sizeof(writebuffer));
      numHijo = i;
      esclavo(to_Parent, to_Child);
    }
    else if (pid[i]<0) {   
      perror("Error fork");
    }
  }
  
  int *k = malloc(sizeof(int));
  
  close_Pipe_Maestro(to_Parent, to_Child, READ, WRITE);
  
  while(Lista_Tam(procesoslibre) != numProcesos) {
    
    string = (char*)malloc(sizeof(int));
    nbytes = read(fd[READ], k, sizeof(int));
    nbytes = read(to_Parent[*k][READ], readbuffer, sizeof(readbuffer));
    pid_hijo[*k] = atoi(readbuffer);
    sprintf(string, "%d", *k);
    Lista_Add(&procesoslibre,string);
  
  }
  
  explorarDir();
  
  totalsize = totalsize+blocksize;
  blocksize = 0;
  
  maestro(to_Parent, to_Child);
 
  numdir = Lista_Tam(archivosprocesados);
  
  Imprimir (numProcesos);
  
  Lista_Clean(archivosprocesados);
  Lista_Clean(dirdisponibles);
  Lista_Clean(procesoslibre);
  
  for (i = 0; i<numProcesos; i++) {
    kill(pid_hijo[i], SIGUSR1);
    nbytes = write(to_Child[i][WRITE], writebuffer, sizeof(writebuffer));

  }  
  
  close_Pipe_Maestro(to_Parent, to_Child, WRITE, READ);
    
  exit(EXIT_SUCCESS);
  
}
