/* Autores: Argenis Chang, Vicente Santacoloma
 * Carnet: 08-10220, 08-11044
 * Fecha: 16-03-2011.
 * Contenido: Declaracion de la estructura lista.
 */

#ifndef LISTA
#define LISTA
#include <stdio.h>
#include <stdlib.h>

/* Creacion de la estructura lista con dos campos: Datos y Siguiente.
 */ 
struct Lista {

  char * data;
  struct Lista *next;

};

void	Lista_Add (struct Lista **, char *);
char* 	Lista_Remove (struct Lista **);
int 	Lista_Tam (struct Lista *);
void 	Lista_toArray (struct Lista *, char* (*)[]);
void 	Lista_Print (struct Lista *);
void 	Lista_Clean (struct Lista *);

#endif