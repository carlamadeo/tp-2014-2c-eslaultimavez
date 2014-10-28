/*
 * Memoria.c
 *
 *  Created on: 14/09/2014
 *      Author: utnso
 */

#include "Memoria.h"
#include "ConfigMSP.h"
#include "commons/log.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "commons/collections/queue.h"

extern char *memoria;
extern t_log *MSPlogger;
extern double cantidadMemoriaPrincipal, cantidadMemoriaSecundaria, cantidadMemoriaTotal;
extern uint16_t modoSustitucionPaginasMSP;
extern t_list *programas, *marcos, *marcosLibres, *marcosOcupados;


/****************************** Comienzo Creacion Segmento ****************************************/

uint32_t crearSegmento(int pid, int tamanio){

	uint32_t direccionBase;
	int cantidadPaginas = tamanio / TAMANIO_PAGINA;

	if (tamanio%TAMANIO_PAGINA > 0) cantidadPaginas++;

	log_info(MSPlogger, "Comienzo de creacion de nuevo Segmento para el PID %d con tamanio %d... ", pid, tamanio);

	if (tamanio > TAMANIO_MAX_SEGMENTO){
		log_error(MSPlogger, "No se ha podido crear el Segmento: El tamanio ingresado es mayor al permitido");
		log_info(MSPlogger, "Finalizando...");
		direccionBase = -1;
	}

	else if(tamanio <= 0){
		log_error(MSPlogger, "No es posible crear un segmento de tamanio menor a 1");
		log_info(MSPlogger, "Finalizando...");
		direccionBase = -1;
	}

	else if ((cantidadPaginas * TAMANIO_PAGINA) > cantidadMemoriaTotal) {
		log_error(MSPlogger, "Error de Memoria Llena");
		direccionBase = -1;
	}

	else {
		log_info(MSPlogger, "Creando segmento...");
		direccionBase = crearSegmentoConSusPaginas(pid, tamanio, cantidadPaginas);
	}

	return direccionBase;
}


uint32_t crearSegmentoConSusPaginas(int pid, int tamanio, int cantidadPaginas){

	t_programa *programa = malloc(sizeof(t_programa));
	t_segmento *segmento = malloc(sizeof(t_segmento));
	t_segmento *segmentoAnterior = malloc(sizeof(t_segmento));
	uint32_t direccionBase;
	int i = 0;

	bool matchPrograma(t_programa *unPrograma){
		return unPrograma->pid == pid;
	}

	if (!list_is_empty(programas)){
		programa = list_find(programas, matchPrograma);

		if(programa != NULL){
			//Si el programa no tiene segmentos, a numero de segmento le pongo 0
			if(list_is_empty(programa->tablaSegmentos)){
				segmento->numero = 0;
			}

			else{
				//Si el programa ya tiene segmentos, busco el anterior y en numero de segmento le pongo el numero del anterior + 1
				segmentoAnterior = list_get(programa->tablaSegmentos, list_size(programa->tablaSegmentos) - 1);
				segmento->numero = segmentoAnterior->numero + 1;
			}

			segmento->tamanio = cantidadPaginas * TAMANIO_PAGINA;
			segmento->tablaPaginas = list_create();

			list_add(programa->tablaSegmentos, segmento);

			for(i = 0; i < cantidadPaginas; i++){
				t_pagina *pagina = malloc(sizeof(t_pagina));
				pagina->numero = i;
				pagina->numeroMarco = NO_EN_MEMORIA;
				list_add(segmento->tablaPaginas, pagina);
			}

			direccionBase = calculoDireccionBase(segmento->numero);
			log_info(MSPlogger, "Segmento creado correctamente. PID: %d, Tamanio: %d, Direccion base: %0.8p", pid, segmento->tamanio, direccionBase);
			//TODO Me parece que esto no va, no disminuye la cantidad de memoria porque todavia no lo guarde en ningun lado
			//cantidadMemoriaTotal -= segmento->tamanio;
			return direccionBase;
		}

		else{
			log_error(MSPlogger, "No existe el programa con PID %d. No se hace nada.", pid);
			log_info(MSPlogger, "Finalizando...");
			return -1;
		}

	}

	else return -1;
	free(segmento);
	free(segmentoAnterior);
}

/****************************** Finalizacion Creacion Segmento ****************************************/


/****************************** Comienzo Destruccion Segmento ****************************************/

void destruirSegmento(int pid, uint32_t direccionBase){

	t_programa *programa = malloc(sizeof(t_programa));
	t_segmento *segmento = malloc(sizeof(t_segmento));

	int numeroSegmento = calculoNumeroSegmento(direccionBase);
	int numeroPagina = calculoNumeroPagina(direccionBase);
	int desplazamiento = calculoDesplazamiento(direccionBase);

	log_info(MSPlogger, "Comienzo de destruccion del Segmento con Direccion Base %0.8p para el PID %d... ", direccionBase, pid);

	//Compruebo que me hayan pasado la direccion base del segmento (pagina 0, desplazamiento 0)
	if(numeroPagina != 0 || desplazamiento != 0){
		log_error(MSPlogger, "No se encontro el segmento con base %0.8p para el programa con PID %d. No se hace nada. ", direccionBase, pid);
	}

	else{

		bool matchPrograma(t_programa *unPrograma){
			return unPrograma->pid == pid;
		}

		programa = list_find(programas, matchPrograma);

		if(programa != NULL){
			bool matchSegmento(t_segmento *unSegmento){
				return unSegmento->numero == numeroSegmento;
			}

			segmento = list_find(programa->tablaSegmentos, matchSegmento);

			if(segmento != NULL){
				borrarPaginasDeMemoriaSecundariaYPrimaria(pid, segmento);
				list_remove_and_destroy_by_condition(programa->tablaSegmentos, matchSegmento, free);
				//TODO Esto se corresponde con el TODO de crearSegmentoConSusPaginas, ver si van!
				//cantidadMemoriaTotal += segmento->tamanio;
				log_info(MSPlogger, "Segmento destruido correctamente. PID: %d, Direccion Base: %0.8p, Numero de Segmento: %d", pid, direccionBase, numeroSegmento);
			}

			else{
				log_error(MSPlogger, "No se encontro el segmento con base %0.8p para el programa con PID %d. No se hace nada. ", direccionBase, pid);
			}
		}

		else{
			log_error(MSPlogger, "No se encontro el programa con PID %d. No se hace nada. ", pid);
		}
	}

}


void borrarPaginasDeMemoriaSecundariaYPrimaria(int pid, t_segmento *segmento){

	t_list *paginasEnMemoria;
	t_list *paginasEnDisco;

	bool matchPaginaEnMemoria(t_pagina *unaPagina){
		return unaPagina->numeroMarco >= 0;
	}

	bool matchPaginaEnDisco(t_pagina *unaPagina){
		return unaPagina->numeroMarco == EN_DISCO;
	}

	//Busco las paginas del segmento que se encuentren en memoria (las que tengan en numeroMarco >= 0)
	paginasEnMemoria = list_filter(segmento->tablaPaginas, matchPaginaEnMemoria);
	//Busco las paginas del segmento que se encuentren en disco (las que tengan en numeroMarco = -2)
	paginasEnDisco = list_filter(segmento->tablaPaginas, matchPaginaEnDisco);

	list_iterate(paginasEnMemoria, borrarPaginaDeMemoria);
	list_iterate(paginasEnDisco, borrarPaginaDeDisco);

	list_destroy_and_destroy_elements(paginasEnMemoria, free);
	list_destroy_and_destroy_elements(paginasEnDisco, free);

}


void borrarPaginaDeMemoria(t_pagina *pagina){

	t_marco *marco;

	bool matchMarco(t_marco *unMarco){
		return unMarco->numero == pagina->numeroMarco;
	}

	marco = list_find(marcos, matchMarco);

	list_add(marcosLibres, marco);

	list_remove_by_condition(marcosOcupados, matchMarco);

	cantidadMemoriaPrincipal += TAMANIO_PAGINA;
	cantidadMemoriaTotal += TAMANIO_PAGINA;

	//TODO ver como borrar correctamente la memoria
	memset(memoria + marco->inicio, 0, TAMANIO_PAGINA);
}


void borrarPaginaDeDisco(int pid, int numeroSegmento, int numeroPagina){

	char *absolute_path;

	absolute_path = armarPathArchivo(pid, numeroSegmento, numeroPagina);

	if(remove(absolute_path) == -1){
		log_error(MSPlogger, "Ha ocurrido un error al intentar borrar el archivo %s", absolute_path);
	}
	else{
		cantidadMemoriaSecundaria += TAMANIO_PAGINA;
		cantidadMemoriaTotal += TAMANIO_PAGINA;
		log_info(MSPlogger, "El archivo de swapping %s se ha eliminado correctamente", absolute_path);
	}

}

/****************************** Finalizacion Destruccion Segmento ****************************************/


/****************************** Comienzo Escribir Memoria ****************************************/

bool escribirMemoria(int pid, uint32_t direccionVirtual, char* buffer, int tamanio){

	uint32_t numeroSegmento, numeroPagina, numeroMarco, desplazamiento;
	int cantidadPaginas;
	t_segmento *segmento = malloc(sizeof(t_segmento));
	t_programa *programa = malloc(sizeof(t_programa));
	t_pagina *pagina = malloc(sizeof(t_pagina));
	t_marco *marco = malloc(sizeof(t_marco));

	numeroSegmento = calculoNumeroSegmento(direccionVirtual);
	numeroPagina = calculoNumeroPagina(direccionVirtual);
	desplazamiento = calculoDesplazamiento(direccionVirtual);

	bool matchPrograma(t_programa *unProgama){
		return unProgama->pid == pid;
	}

	programa = list_find(programas, matchPrograma);

	//Compruebo que sea un PID valido
	if(programa == NULL){
		log_error(MSPlogger, "No se ha encontrado el programa con el PID %d", pid);
		return false;
	}

	else{
		bool matchSegmento(t_segmento *unSegmento){
			return unSegmento->numero == numeroSegmento;
		}

		segmento = list_find(programa->tablaSegmentos, matchSegmento);

		//Compruebo que el segmento corresponda a ese PID
		if(segmento == NULL){
			log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
			return false;
		}

		else{
			//Compruebo que no me soliciten escribir mas del tamanio que posee el segmento
			if(tamanio > segmento->tamanio){
				log_error(MSPlogger, "Se ha excedido el tamanio del segmento %d. Segmentation Fault", segmento->numero);
				return false;
			}

			bool matchPagina(t_pagina *unaPagina){
				return unaPagina->numero == numeroPagina;
			}

			pagina = list_find(segmento->tablaPaginas, matchPagina);

			//Compruebo que la pagina pertenezca al segmento
			if(pagina == NULL){
				log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
				return false;
			}

			else{
				//Calculo la cantidad de paginas que necesito tener en memoria, ademas de la pagina encontrada por la direccionBase
				cantidadPaginas = calcularCantidadPaginasNecesarias(tamanio, desplazamiento);

				if(paginaEstaEnMemoria(pagina)){
					marco = encontrarMarcoPorPagina(pagina);
					seModificoElMarco(marco);
					borrarMarcoDeMemoria(marco);
				}

				else if(pagina->numeroMarco == NO_EN_MEMORIA){
					log_info(MSPlogger, "La pagina no se encuentra en memoria");
					marco = llevarPaginaAMemoria(pagina);
				}

				else{
					log_info(MSPlogger, "La pagina no se encuentra en memoria");
					numeroMarco = traerPaginaDeDiscoAMemoria(pid, segmento->numero, pagina->numero);
					marco = encontrarMarcoPorNumeroMarco(numeroMarco);
					pagina->numeroMarco = numeroMarco;
				}

				seReferencioElMarco(marco);
				memcpy(memoria + marco->inicio, buffer, tamanio);

				memset(buffer, 0, TAMANIO_PAGINA);
				memcpy(buffer, memoria + marco->inicio, tamanio);

				log_info(MSPlogger, "Se ha escrito en la posicion de memoria %0.8p: %s", marco->inicio, buffer);
				return true;
			}
		}
	}
}

int calcularCantidadPaginasNecesarias(int tamanio, int desplazamiento){

	int tamanioExcedente;
	int cantidadNecesaria;

	tamanioExcedente = tamanio - (TAMANIO_PAGINA - desplazamiento);

	if(tamanioExcedente <= 0) return 0;

	else{
		cantidadNecesaria = tamanioExcedente/TAMANIO_PAGINA;
		if(tamanioExcedente % TAMANIO_PAGINA > 0) cantidadNecesaria++;
		return cantidadNecesaria;
	}

}

bool leerMemoria(int pid, uint32_t direccionVirtual, int tamanio, char *leido){

	uint32_t numeroSegmento, numeroPagina, numeroMarco, desplazamiento;
	t_segmento *segmento = malloc(sizeof(t_segmento));
	t_programa *programa = malloc(sizeof(t_programa));
	t_pagina *pagina = malloc(sizeof(t_pagina));
	t_marco *marco = malloc(sizeof(t_marco));

	numeroSegmento = calculoNumeroSegmento(direccionVirtual);
	numeroPagina = calculoNumeroPagina(direccionVirtual);
	desplazamiento = calculoDesplazamiento(direccionVirtual);

	bool matchPrograma(t_programa *unPrograma){
		return unPrograma->pid == pid;
	}

	programa = list_find(programas, matchPrograma);

	if(programa == NULL){
		log_error(MSPlogger, "No se ha encontrado el programa con el PID %d", pid);
		return false;
	}

	else{
		bool matchSegmento(t_segmento *unSegmento){
			return unSegmento->numero == numeroSegmento;
		}

		segmento = list_find(programa->tablaSegmentos, matchSegmento);

		if(segmento == NULL){
			log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
			return false;
		}

		else{

			if(tamanio > segmento->tamanio){
				log_error(MSPlogger, "Se ha excedido el tamanio del segmento %d. Segmentation Fault", segmento->numero);
				return false;
			}

			bool matchPagina(t_pagina *unaPagina){
				return unaPagina->numero == numeroPagina;
			}

			pagina = list_find(segmento->tablaPaginas, matchPagina);

			if(pagina == NULL){
				log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
				return false;
			}

			else{
				if(pagina->numeroMarco == NO_EN_MEMORIA){
					log_error(MSPlogger, "No hay datos en la memoria");	//TODO ver que mensaje tirar en este caso
					return false;
				}

				else{

					if(pagina->numeroMarco == EN_DISCO){
						log_info(MSPlogger, "La pagina no se encuentra en memoria");
						numeroMarco = traerPaginaDeDiscoAMemoria(pid, numeroSegmento, numeroPagina);
						pagina->numeroMarco = numeroMarco;
					}

					else{
						numeroMarco = pagina->numeroMarco;
					}

					bool matchMarco(t_marco *unMarco){
						return unMarco->numero == numeroMarco;
					}

					marco = list_find(marcos, matchMarco);

					if(marco == NULL || numeroMarco == -1){
						log_error(MSPlogger, "Ha ocurrido un error al leer la memoria. Finalizando... ");	//TODO ver que mensaje tirar en este caso
						return false;
					}

					else{
						seReferencioElMarco(marco);
						memcpy(leido, memoria + marco->inicio + desplazamiento, tamanio);
						log_info(MSPlogger, "Se ha leido de la posicion de memoria %0.8p: %s", marco->inicio + desplazamiento, leido);
						return true;
					}
				}
			}
		}
	}
}


bool segmentoYPaginaPorDireccionVirtual(int pid, t_programa *programa, t_segmento *segmento, t_pagina *pagina, uint32_t direccionVirtual){

	int numeroSegmento = calculoNumeroSegmento(direccionVirtual);
	int	numeroPagina = calculoNumeroPagina(direccionVirtual);
	int	desplazamiento = calculoDesplazamiento(direccionVirtual);

	programa = encontrarProgramaPorPid(pid);

	if(programa == NULL){
		log_error(MSPlogger, "El programa con PID %d no existe", pid);
		return false;
	}
	else{
		segmento = encontrarSegmentoEnProgramaPorNumeroDeSegmento(programa, numeroSegmento);
		if(segmento == NULL){
			log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
			return false;
		}
		else{
			pagina = encontrarPaginaEnSegmentoPorNumeroDePagina(segmento, numeroPagina);
			if(pagina == NULL){
				log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
				return false;
			}
			else{
				return true;
			}
		}
	}
}


bool paginaEstaEnMemoria(t_pagina *pagina){
	return pagina->numeroMarco >= 0;
}


t_marco *llevarPaginaAMemoria(t_pagina *pagina){

	t_marco *marco = malloc(sizeof(t_marco));

	log_info(MSPlogger, "Se cargara la pagina en memoria...");

	if(list_size(marcosLibres) > 0){
		marco = (t_marco*)list_remove(marcosLibres, 0);
		list_add(marcosOcupados, marco);
		log_info(MSPlogger, "La pagina ha sido cargada en memoria correctamente en el marco %d", marco->numero);
	}
	else{
		if(modoSustitucionPaginasMSP == FIFO){
			marco = sustituirPaginaPorFIFO();
		}

		else{
			marco = sustituirPaginaPorCLOCK_MODIFICADO();
		}
	}

	pagina->numeroMarco = marco->numero;

	return marco;
}


//TODO si da error devolver -1
int traerPaginaDeDiscoAMemoria(int pid, int numeroSegmento, int numeroPagina){

	char *absolute_path;
	char linea[256];
	char buffer[256];
	t_marco *marco;
	t_programa *programa;
	t_segmento *segmento;
	t_pagina *pagina;

	log_info(MSPlogger, "Cargando pagina a memoria...");

	absolute_path = armarPathArchivo(pid, numeroSegmento, numeroPagina);
	FILE* file = fopen(absolute_path, "r");

	void copiarPaginaAMemoriaYEliminarDeDisco(t_marco *marco){
		log_info(MSPlogger, "Copiando la pagina en memoria...");

		memcpy(memoria + marco->inicio, buffer, TAMANIO_PAGINA);

		borrarPaginaDeDisco(pid, numeroSegmento, numeroPagina);

		log_info(MSPlogger, "Se ha cargado la pagina en memoria correctamente. Direccion de inicio %0.8p", marco->inicio + &memoria);
		queue_push(marcosOcupados, marco);

		bool matchPrograma(t_programa *unPrograma){
			return unPrograma->pid == pid;
		}

		programa = list_find(programas, matchPrograma);

		bool matchSegmento(t_segmento *unSegmento){
			return unSegmento->numero == numeroSegmento;
		}

		segmento = list_find(programa->tablaSegmentos, matchSegmento);

		bool matchPagina(t_pagina *unaPagina){
			return unaPagina->numero == numeroPagina;
		}

		pagina = list_find(segmento->tablaPaginas, matchPagina);

		pagina->numeroMarco = marco->numero;

	}

	if(file == NULL){
		log_error(MSPlogger, "Error al leer el archivo. Finalizando...");
		return -1;
	}
	else{

		while (feof(file) == 0)
		{
			fgets(linea, TAMANIO_PAGINA, file);
			strcat(buffer, linea);
		}

		if(list_size(marcosLibres) > 0){
			marco = (t_marco*)list_remove(marcosLibres, 1);
			copiarPaginaAMemoriaYEliminarDeDisco(marco);
			return marco->numero;
		}

		else{
			log_info(MSPlogger, "No hay marcos libres en memoria");

			if(cantidadMemoriaSecundaria == 0){
				log_error(MSPlogger, "No es posible cargar la pagina. No hay espacio en memoria primaria ni secundaria");
				return -1;
			}

			else{
				if(modoSustitucionPaginasMSP == FIFO){
					marco = sustituirPaginaPorFIFO();
					copiarPaginaAMemoriaYEliminarDeDisco(marco);
					return marco->numero;
				}

				else{
					marco = sustituirPaginaPorCLOCK_MODIFICADO();
					copiarPaginaAMemoriaYEliminarDeDisco(marco);
					return marco->numero;
				}
			}
		}

		fclose(file);
	}

}

t_marco *sustituirPaginaPorFIFO(){

	int pid;
	int numeroSegmento;
	int numeroPagina;
	t_marco *marco;

	log_info(MSPlogger, "Comienzo de sustitucion de pagina por FIFO...");

	marco = (t_marco*)list_remove(marcosOcupados, 0);		//Tomo el primer elemento de marcosOcupados y lo elimino de la lista

	corresponderMarcoAPagina(marco, &pid, &numeroSegmento, &numeroPagina);		//Busco la pagina que esta cargada en el marco para poder llevarla a disco
	log_info(MSPlogger, "Se reemplazara el marco %d", marco->numero);
	llevarPaginaADisco(marco, pid, numeroSegmento, numeroPagina);
	borrarMarcoDeMemoria(marco);
	list_add(marcosOcupados, marco);		//Agrego el marco en la ultima posicion de la lista marcosOcupados (para FIFO)

	log_info(MSPlogger, "Se sustituyo correctamente la Pagina %d del Segmento %d del PID %d al Marco %d por FIFO", numeroPagina, numeroSegmento, pid, marco->numero);

	return marco;
}


t_marco *sustituirPaginaPorCLOCK_MODIFICADO(){

	int pid;
	int numeroSegmento;
	int numeroPagina;
	t_marco *marco;

	bool matchMarcoEn1(t_marco *unMarco){
		unMarco->categoriaClockModificado == NOREFERENCIADA_NOMODIFICADA;
	}

	marco = list_find(marcosOcupados, matchMarcoEn1);

	if(marco == NULL){
		bool matchMarcoEn2(t_marco *unMarco){
			unMarco->categoriaClockModificado == NOREFERENCIADA_MODIFICADA;
		}

		marco = list_find(marcosOcupados, matchMarcoEn2);

		if(marco == NULL){
			bool matchMarcoEn3(t_marco *unMarco){
				unMarco->categoriaClockModificado == REFERENCIADA_NOMODIFICADA;
			}

			marco = list_find(marcosOcupados, matchMarcoEn3);

			if(marco == NULL){
				bool matchMarcoEn4(t_marco *unMarco){
					unMarco->categoriaClockModificado == REFERENCIADA_MODIFICADA;
				}

				marco = list_find(marcosOcupados, matchMarcoEn4);
			}
		}
	}

	bool matchMarco(t_marco *unMarco){
		unMarco->numero == marco->numero;
	}

	corresponderMarcoAPagina(marco, &pid, &numeroSegmento, &numeroPagina);		//Busco la pagina del marco para poder eliminarla del disco (necesito pid, numeroSegmento, numeroPagina)
	log_info(MSPlogger, "Se reemplazara el marco %d...", marco->numero);
	llevarPaginaADisco(marco, pid, numeroSegmento, numeroPagina);

	list_remove_by_condition(marcosOcupados, matchMarco);	//Remuevo el marco de la lista
	list_add(marcosOcupados, marco);						//Agrego el marco en la ultima posicion de la lista (para FIFO)

	return marco;
}

void borrarMarcoDeMemoria(t_marco *marco){
	memset(memoria + marco->inicio, 0, TAMANIO_PAGINA);
}

void seReferencioElMarco(t_marco *marco){

	if(marco->categoriaClockModificado == 0){
		marco->categoriaClockModificado = 2;
	}

	if(marco->categoriaClockModificado == 1){
		marco->categoriaClockModificado = 3;
	}
}


void seModificoElMarco(t_marco *marco){

	if(marco->categoriaClockModificado == 0){
		marco->categoriaClockModificado = 1;
	}

	if(marco->categoriaClockModificado == 2){
		marco->categoriaClockModificado = 3;
	}
}


void corresponderMarcoAPagina(t_marco *marco, int *pid, int *numeroSegmento, int *numeroPagina){

	*pid = -1;
	*numeroSegmento = -1;
	*numeroPagina = -1;
	int segmentoAnterior;
	bool encontrado = false;

	void iterarPagina(t_pagina *unaPagina){
		if(unaPagina->numeroMarco == marco->numero && !encontrado){
			*numeroPagina = unaPagina->numero;
			*numeroSegmento = segmentoAnterior;
			encontrado = true;
		}
	}

	void iterarSegmento(t_segmento *unSegmento){
		if(!encontrado){
			segmentoAnterior = unSegmento->numero;
			list_iterate(unSegmento->tablaPaginas, iterarPagina);
		}
	}

	void iterarPrograma(t_programa *unPrograma){
		if(!encontrado){
			*pid = unPrograma->pid;
			list_iterate(unPrograma->tablaSegmentos, iterarSegmento);
		}
	}

	list_iterate(programas, iterarPrograma);

	if(*pid == -1 || *numeroPagina == -1 || *numeroSegmento == -1){
		log_error(MSPlogger, "Ha ocurrido un error al encontrar el marco");
	}

}

void llevarPaginaADisco(t_marco *marco, int pid, int numeroSegmento, int numeroPagina){

	char *leido = malloc(TAMANIO_PAGINA);
	char *absolute_path;
	t_programa *programa;
	t_segmento *segmento;
	t_pagina *pagina;

	log_info(MSPlogger, "Comienzo de copia de pagina a disco...");

	//TODO ver si el archivo debe pesar 256 Bytes
	absolute_path = armarPathArchivo(pid, numeroSegmento, numeroPagina);

	FILE* file = fopen(absolute_path, "w");

	memcpy(leido, memoria + marco->inicio, TAMANIO_PAGINA);

	fputs(leido, file);
	fclose(file);

	log_info(MSPlogger, "Se ha copiado la Pagina %d, del Segmento %d, del PID %d a disco", numeroPagina, numeroSegmento, pid);

	bool matchPrograma(t_programa *unPrograma){
		return unPrograma->pid == pid;
	}

	programa = list_find(programas, matchPrograma);

	bool matchSegmento(t_segmento *unSegmento){
		return unSegmento->numero == numeroSegmento;
	}

	segmento = list_find(programa->tablaSegmentos, matchSegmento);

	bool matchPagina(t_pagina *unaPagina){
		return unaPagina->numero == numeroPagina;
	}

	pagina = list_find(segmento->tablaPaginas, matchPagina);

	pagina->numeroMarco = -2;

	free(leido);
	free(absolute_path);
}


char *armarPathArchivo(int pid, int numeroSegmento, int numeroPagina){

	char *folder = "../swapp";
	char *extension = ".txt";
	char *absolute_path = malloc(strlen(folder) + sizeof(pid) + sizeof(numeroSegmento) + sizeof(numeroPagina) + strlen(extension) + 1);

	sprintf(absolute_path, "%s/%d%d%d%s", folder, pid, numeroSegmento, numeroPagina, extension);

	return absolute_path;
}


uint32_t calculoDireccionBase(int numeroSegmento){
	//TODO ver por que imprime nil en vez de 0x00000000
	if (numeroSegmento == 0){
		return 0x00000000;
	}
	else{
		return (numeroSegmento << 20) & 0xFFFFFFFF;
	}
}


int calculoNumeroSegmento(uint32_t direccionLogica){
	return (direccionLogica & 0xFFF00000) >> 20;
}


int calculoNumeroPagina(uint32_t direccionLogica){
	return (direccionLogica & 0xFFF00) >> 8;
}


int calculoDesplazamiento(uint32_t direccionLogica){
	return (direccionLogica & 0xFF);
}

t_programa *encontrarProgramaPorPid(int pid){

	bool matchPrograma(t_programa *unPrograma){
		return unPrograma->pid == pid;
	}

	return list_find(programas, matchPrograma);

}


t_segmento * encontrarSegmentoEnProgramaPorNumeroDeSegmento(t_programa *programa, int numeroSegmento){

	bool matchSegmento(t_segmento *unSegmento){
		return unSegmento->numero == numeroSegmento;
	}

	return list_find(programa->tablaSegmentos, matchSegmento);

}


t_pagina *encontrarPaginaEnSegmentoPorNumeroDePagina(t_segmento *segmento, int numeroPagina){

	bool matchPagina(t_pagina *unaPagina){
		return unaPagina->numero == numeroPagina;
	}

	return list_find(segmento->tablaPaginas, matchPagina);

}

t_marco *encontrarMarcoPorPagina(t_pagina *pagina){

	bool matchMarco(t_marco *unMarco){
		return unMarco->numero == pagina->numeroMarco;
	}

	return list_find(marcos, matchMarco);

}

t_marco *encontrarMarcoPorNumeroMarco(int numeroMarco){

	bool matchMarco(t_marco *unMarco){
		return unMarco->numero == numeroMarco;
	}

	return list_find(marcos, matchMarco);

}
