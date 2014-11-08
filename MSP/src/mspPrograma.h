/*
 * Programa.h
 *
 *  Created on: 29/09/2014
 *      Author: utnso
 */

#ifndef PROGRAMA_H_
#define PROGRAMA_H_

#include <commons/collections/list.h>

typedef struct{
	int pid;
	t_list *tablaSegmentos;
}t_programa;

t_programa *crearPrograma(int pid);

#endif /* PROGRAMA_H_ */
