/*
 * Memoria.h
 *
 *  Created on: 14/09/2014
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#define TAMANIO_PAGINA 256 //Bytes
#define TAMANIO_MAX_SEGMENTO 1048576 //Bytes
#define NO_EN_MEMORIA -1	//La pagina no esta ni en disco ni en memoria
#define EN_DISCO -2	//La pagina se encuentra en el disco

#define NOREFERENCIADA_NOMODIFICADA 1
#define NOREFERENCIADA_MODIFICADA 2
#define REFERENCIADA_NOMODIFICADA 3
#define REFERENCIADA_MODIFICADA 4

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include "Programa.h"

typedef struct{
	int numero;
	int tamanio;
	t_list *tablaPaginas;
}t_segmento;

typedef struct{
	int numero;
	int numeroMarco;	//Se pone en -1 si no esta cargado ni en memoria principal ni secundaria. Se pone en -2 si esta en disco.
}t_pagina;

typedef struct{
	int numero;
	uint32_t inicio;					//Este inicio es relativo a la memoria, es decir el inicio del primer marco sera 0
	int categoriaClockModificado;
}t_marco;



/**
* @NAME: crearSegmento
* @DESC: Verifica si hay espacio en MR o MV. Si hay, agrega a tabla de segmentos con su tabla de paginas.
* Devuelve la direccion base del segmento creado.
* Si no se puede crear el segmento devuelve -1
*/
uint32_t crearSegmento(int pid, int tamanio);

/**
* @NAME: destruirSegmento
* @DESC: Destruye el segmento con base "direccionBase" para el pid "pid"
*/
void destruirSegmento(int pid, uint32_t direccionBase);

/**
* @NAME: borrarPaginasDeMemoriaSecundariaYPrimaria
* @DESC: Busca las paginas que se encuentren en memoria principal y secundaria y las borra
*/
void borrarPaginasDeMemoriaSecundariaYPrimaria(int pid, t_segmento *segmento);

/**
* @NAME: calculoDireccionBase
* @DESC: Calcula la direccion base de un segmento a partir del numero de segmento
*/
uint32_t calculoDireccionBase(int numeroSegmento);

/**
* @NAME: calculoNumeroSegmento
* @DESC: Calcula el numero de segmento a partir de una direccion logica
*/
int calculoNumeroSegmento(uint32_t direccionLogica);

/**
* @NAME: calculoNumeroPagina
* @DESC: Calcula el numero de pagina a partir de una direccion logica
*/
int calculoNumeroPagina(uint32_t direccionLogica);

/**
* @NAME: calculoNumeroPagina
* @DESC: Calcula el desplazamiento a partir de una direccion logica
*/
int calculoDesplazamiento(uint32_t direccionLogica);

/**
* @NAME: escribirMemoria
* @DESC: Escribe en la direccion virtual indicada buffer.
* Hace las validaciones de violaciones de memoria correspondientes
*/
bool escribirMemoria(int pid, uint32_t direccionVirtual, char* buffer, int tamanio);

/**
* @NAME: leerMemoria
* @DESC: Lee de la direccion virtual indicada y lo carga en leido.
* Hace las validaciones de violaciones de memoria correspondientes
*/
bool leerMemoria(int pid, uint32_t direccionVirtual, int tamanio, char *leido);


int traerPaginaDeDiscoAMemoria(int pid, int numeroSegmento, int numeroPagina);
uint32_t crearSegmentoConSusPaginas(int pid, int tamanio, int cantidadPaginas);
void borrarSegmentosEnMemoria(int pid, t_list *paginas);
void borrarSegmentosEnDisco(int pid, t_list *paginas);
char *armarPathArchivo(int pid, int numeroSegmento, int numeroPagina);
void borrarPaginaDeDisco(int pid, int numeroSegmento, int numeroPagina);
void borrarPaginaDeMemoria(t_pagina *pagina);
void seReferencioElMarco(t_marco *marco);
void seModificoElMarco(t_marco *marco);
t_marco *sustituirPaginaPorCLOCK_MODIFICADO();
t_marco *sustituirPaginaPorFIFO();
void corresponderMarcoAPagina(t_marco *marco, int *pid, int *numeroSegmento, int *numeroPagina);
void llevarPaginaADisco(t_marco *marco, int pid, int numeroSegmento, int numeroPagina);
t_programa *encontrarProgramaPorPid(int pid);
t_segmento *encontrarSegmentoEnProgramaPorNumeroDeSegmento(t_programa *programa, int numeroSegmento);
t_pagina *encontrarPaginaEnSegmentoPorNumeroDePagina(t_segmento *segmento, int numeroPagina);
bool segmentoYPaginaPorDireccionVirtual(int pid, t_programa *programa, t_segmento *segmento, t_pagina *pagina, uint32_t direccionVirtual);
bool paginaEstaEnMemoria(t_pagina *pagina);
t_marco *encontrarMarcoPorPagina(t_pagina *pagina);
t_marco *encontrarMarcoPorNumeroMarco(int numeroMarco);
t_marco *llevarPaginaAMemoria(t_pagina *pagina);


#endif /* MEMORIA_H_ */
