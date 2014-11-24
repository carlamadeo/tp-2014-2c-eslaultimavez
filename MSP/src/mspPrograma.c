/*
 * Programa.c
 *
 *  Created on: 29/09/2014
 *      Author: utnso
 */

#include "mspPrograma.h"
#include "mspConfig.h"
#include "MSP.h"
#include <stdlib.h>

t_MSP *self;

t_programa *buscarCrearPrograma(int pid){

	bool matchPrograma(t_programa *unPrograma){
		return unPrograma->pid == pid;
	}

	t_programa *programaEncontrado = list_find(self->programas, matchPrograma);

	if(programaEncontrado == NULL){
		t_programa *programa = malloc(sizeof(t_programa));

		programa->pid = pid;
		programa->tablaSegmentos = list_create();
		list_add(self->programas, programa);
		return programa;
	}

	return programaEncontrado;

}
