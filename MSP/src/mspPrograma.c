/*
 * Programa.c
 *
 *  Created on: 29/09/2014
 *      Author: utnso
 */

#include "mspPrograma.h"
#include <stdlib.h>

extern t_list *programas;


t_programa *crearPrograma(int pid){
	t_programa *programa = malloc(sizeof(t_programa));

	programa->pid = pid;
	programa->tablaSegmentos = list_create();

	list_add(programas, programa);
	return programa;
}
