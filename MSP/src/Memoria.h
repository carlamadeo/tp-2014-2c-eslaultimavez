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

#include "mspPrograma.h"
#include <stdint.h>

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

typedef struct{
	int numeroPagina;
	int numeroSegmento;
	int desplazamiento;
}t_direccion;



/**
* @NAME: crearSegmento
* @DESC: Verifica si hay espacio en MR o MV. Si hay, agrega a tabla de segmentos con su tabla de paginas
* Devuelve la direccion base del segmento creado
* Si no se puede crear el segmento devuelve -1
*/
uint32_t mspCrearSegmento(int pid, int tamanio);


/**
* @NAME: crearSegmentoConSusPaginas
* @DESC: Crea el segmento con sus paginas
*/
uint32_t crearSegmentoConSusPaginas(int pid, int cantidadPaginas, int tamanio);

/**
* @NAME: cantidadMemoriaTotal
* @DESC: Devuelve la cantidad de memoria total, es decir la suma de memoria real y secundaria
*/
double cantidadMemoriaTotal();

/**
* @NAME: destruirSegmento
* @DESC: Destruye el segmento con base "direccionBase" para el pid "pid"
* Si alguna pagina del segmento se encontraba en memoria la elimina de memoria
* Si alguna pagina del segmento se encontraba en disco la elimina del disco
*/
void mspDestruirSegmento(int pid, uint32_t direccionBase);

/**
* @NAME: eliminarSegmentoDeListaDelPrograma
* @DESC: Destruye el segmento determinado por numeroSegmento de la tabla de segmentos del programa
*/
void eliminarSegmentoDeListaDelPrograma(t_programa *programa, int numeroSegmento);

/**
* @NAME: borrarPaginasDeMemoriaSecundariaYPrimaria
* @DESC: Busca las paginas que se encuentren en memoria principal y secundaria y las borra
*/
void borrarPaginasDeMemoriaSecundariaYPrimaria(int pid, t_segmento *segmento);

/**
* @NAME: borrarPaginaDeMemoria
* @DESC: Borra la pagina ingresada por parametro de memoria
* Libera el marco agregandolo en la lista de marcosLibres y eliminandolo de la lista de marcosOcupados
*/
void borrarPaginaDeMemoria(t_pagina *pagina);

/**
* @NAME: borrarPaginaDeMemoria
* @DESC: Borra las paginas que (ingresan como parametro) de disco
*/
void borrarPaginaDeDisco(int pid, int numeroSegmento, int numeroPagina);

/**
* @NAME: escribirMemoria
* @DESC: Escribe en la direccion virtual indicada lo que me pasan en buffer.
* Hace las validaciones de violaciones de memoria correspondientes
* Devuelve true si pudo escribir, false si no pudo
*/
bool mspEscribirMemoria(int pid, uint32_t direccionVirtual, char* buffer, int tamanio);


bool isSegmentationFault(int tamanioSegmento, t_direccion direccionReal, int tamanioSolicitado);
/**
* @NAME: calcularCantidadPaginasNecesarias
* @DESC: Calcula la cantidad de paginas necesarias en memoria para leer/escribir la memoria
* No considera la pagina que pasan por la direccion base, es decir, se deberan pasar a memoria
* la cantidad de paginas devueltas y la que me indique la direccion base
*/
int calcularCantidadPaginasNecesarias(int tamanio, int desplazamiento);

/**
* @NAME: crearListaPaginasAPasarAMemoria
* @DESC: Crea y devuelve una lista con las paginas que seran pasadas a memoria
*/
t_list *crearListaPaginasAPasarAMemoria(int cantidadPaginas, t_pagina *pagina, t_list *paginas);

/**
* @NAME: buscarPaginasYEscribirMemoria
* @DESC: Escribe la memoria con buffer en los distintos marcos, recorriendo la lista de paginas
*/
void buscarPaginasYEscribirMemoria(int pid, t_direccion direccionReal, t_list *paginasAMemoria, int tamanio, char *buffer);

/**
* @NAME: leerMemoria
* @DESC: Lee de la direccion virtual indicada y lo carga en leido.
* Hace las validaciones de violaciones de memoria correspondientes
*/
bool mspLeerMemoria(int pid, uint32_t direccionVirtual, int tamanio, char *leido);

/**
* @NAME: buscarPaginasYLeerMemoria
* @DESC: Lee la memoria de los distintos marcos, recorriendo la lista de paginas y lo guarda en leido
*/
bool buscarPaginasYLeerMemoria(int pid, t_direccion direccionReal, t_list *paginasAMemoria, int tamanio, char *leido);

/**
* @NAME: traerPaginaDeDiscoAMemoria
* @DESC: Lleva la pagina que se encuentra en disco a memoria. Borra la pagina del disco,
* actualiza el numero de marco en la pagina y devuelve el numero de marco donde la cargo.
*/
int traerPaginaDeDiscoAMemoria(int pid, int numeroSegmento, int numeroPagina);

/**
* @NAME: calculoDireccionReal
* @DESC: Calcula el numero de pagina, numero de segmento y desplazamiento a partir de una direccionLogica
*/
t_direccion calculoDireccionReal(uint32_t direccionLogica);


/**
* @NAME: calculoDireccionBase
* @DESC: Calcula la direccion base de un segmento a partir del numero de segmento
*/
uint32_t calculoDireccionBase(int numeroSegmento);


/**
* @NAME: encontrarPrograma
* @DESC: Busca el programa dentro de la lista de programas por el pid y devuelve el programa encontrado
*/
t_programa *encontrarPrograma(int pid);

t_segmento *encontrarSegmento(t_programa *programa, int numeroSegmento);
t_pagina *encontrarPagina(t_segmento *segmento, int numeroPagina);
t_marco *encontrarMarcoEnMarcosOcupados(int numeroMarco);

void borrarMarcoDeMemoria(t_marco *marco);
void borrarSegmentosEnMemoria(int pid, t_list *paginas);
void borrarSegmentosEnDisco(int pid, t_list *paginas);
char *armarPathArchivo(int pid, int numeroSegmento, int numeroPagina);

void seReferencioElMarco(t_marco *marco);
void seModificoElMarco(t_marco *marco);
t_marco *sustituirPaginaPorCLOCK_MODIFICADO();
t_marco *sustituirPaginaPorFIFO();
void corresponderMarcoAPagina(t_marco *marco, int *pid, int *numeroSegmento, int *numeroPagina);
void llevarPaginaADisco(t_marco *marco, int pid, int numeroSegmento, int numeroPagina);

bool paginaEstaEnMemoria(t_pagina *pagina);
t_marco *encontrarMarcoPorPagina(t_pagina *pagina);
t_marco *encontrarMarcoPorNumeroMarco(int numeroMarco);
t_marco *llevarPaginaAMemoria(t_pagina *pagina);


//int calculoNumeroSegmento(uint32_t direccionLogica);
//int calculoNumeroPagina(uint32_t direccionLogica);
//int calculoDesplazamiento(uint32_t direccionLogica);
//bool segmentoYPaginaPorDireccionVirtual(int pid, t_programa *programa, t_segmento *segmento, t_pagina *pagina, uint32_t direccionVirtual);

#endif /* MEMORIA_H_ */
