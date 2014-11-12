/*
 * Programa.c
 *
 *  Created on: 29/09/2014
 *      Author: utnso
 */

#include "mspPrograma.h"
#include "mspConfig.h"
#include <stdlib.h>

t_list *programas;

t_programa *buscarCrearPrograma(int pid){

	bool matchPrograma(t_programa *unPrograma){
		return unPrograma->pid == pid;
	}

	t_programa *programaEncontrado = list_find(programas, matchPrograma);

	if(programaEncontrado == NULL){
		t_programa *programa = malloc(sizeof(t_programa));

		programa->pid = pid;
		programa->tablaSegmentos = list_create();
		list_add(programas, programa);
		return programa;
	}

	return programaEncontrado;

}
