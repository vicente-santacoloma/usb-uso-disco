/* Autores: Argenis Chang, Vicente Santacoloma
 * Carnet: 08-10220, 08-11044
 * Fecha: 16-03-2011
 * Contenido: Archivo que contiene las operaciones basicas de lista, 
 *	      (agregar, remover, tamano, imprimir) ademas contiene 
 *            un procedimiento para convertir de lista a arreglos y
 *	      para liberar luego espacio de memoria de la lista creada.
 */

#include <stdio.h>
#include <stdlib.h>
#include "Lista.h"

/* Funcion que se encarga de agregar elementos a la lista (en nuestro caso para agregar procesos a la lista).
 * Parametros de entrada: Lista, elemento de tipo caracter (contiene informacion del proceso).
 * Parametro de retorno: Ninguno.
 */ 
void Lista_Add (struct Lista **p, char *i) {

  struct Lista *n = (struct Lista*)malloc(sizeof(struct Lista));
  
  n->data = i;
  n->next = NULL;
  
  if (*p != NULL)
    n->next = *p;
  
  *p = n;
}

/* Funcion que se encarga de eliminar elementos de la lista.
 * Parametros de entrada: Lista.
 * Parametro de retorno: Retorna la informacion que contiene la lista (en nuestro caso del proceso).
 */ 
char * Lista_Remove (struct Lista **p) {
  
  struct Lista *aux = *p;
  char * i = NULL;
  
  if (*p != NULL) {
   
    i = aux -> data;
    *p = (*p)->next;
    aux->next = NULL;
    free(aux);
  }
  return i;
}

/* Funcion que se encarga de imprimir el tamano de la lista.
 * Parametros de entrada: Lista.
 * Parametro de retorno: Tamano de la lista.
 */ 
int Lista_Tam (struct Lista *l) {

  int k = 0;

  while (l != NULL) {
    
    l = l->next;
    k++;
  }

  return k;

}

/* Funcion que se encarga de cambiar la lista a un arreglo.
 * Parametros de entrada: Lista, Apuntador a arreglo.
 * Parametro de retorno: Ninguno.
 */ 
void Lista_toArray (struct Lista *l, char* (*array)[] ) {
  
  int tam = Lista_Tam(l);
  int k;
  
  for (k = 0; k<tam; k++) {
   
   (*array)[k] = l->data;
    l = l->next;
    
  }
  
}

/* Funcion que se encarga de imprimir todo el contenido de la lista.
 * Parametros de entrada: Lista.
 * Parametro de retorno: Ninguno.
 */ 
void Lista_Print (struct Lista *l) {
	
  if (l == NULL) 
    printf("La lista esta vacia\n");
  
  while (l!=NULL) {
    
    printf("%s\n", l->data);
    l = l->next;	
    
  }
  
  printf("\n");
}

/* Funcion que se encarga de eliminar todos los elementos de la lista.
 * Parametros de entrada: Lista.
 * Parametro de retorno: Ninguno.
 */ 
void Lista_Clean (struct Lista *l) {
  
 struct Lista *aux;
  
 while (l != NULL) {
    
    aux = l;
    l = l->next;
    free(aux->data);
    free(aux);
    
  }
  
}