/*
 * Programa.c
 *
 *  Created on: 29/09/2014
 *      Author: utnso
 */

#include "Programa.h"

t_programa *crearPrograma(int pid){
	t_programa *programa = malloc(sizeof(t_programa));

	programa->pid = pid;
	programa->tablaSegmentos = list_create();
	return programa;
}
