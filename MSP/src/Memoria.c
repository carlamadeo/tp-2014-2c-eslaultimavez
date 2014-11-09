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

char *memoria;
t_log *MSPlogger;
double cantidadMemoriaPrincipal, cantidadMemoriaSecundaria;
uint16_t modoSustitucionPaginasMSP;
t_list *programas, *marcosLibres, *marcosOcupados;


/***************************************************************************************************\
 *								--Comienzo Creacion Segmento--									 	 *
\***************************************************************************************************/

uint32_t mspCrearSegmento(int pid, int tamanio){

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

	//TODO fijarse si se debe chequear con tamanio o con cantidadPaginas * TAMANIO_PAGINA
	else if ((cantidadPaginas * TAMANIO_PAGINA) > cantidadMemoriaTotal()) {
		log_error(MSPlogger, "Error de Memoria Llena");
		direccionBase = -1;
	}

	else {
		log_info(MSPlogger, "Creando segmento...");
		direccionBase = crearSegmentoConSusPaginas(pid, cantidadPaginas, tamanio);
	}

	return direccionBase;
}


uint32_t crearSegmentoConSusPaginas(int pid, int cantidadPaginas, int tamanio){

	t_segmento *segmento = malloc(sizeof(t_segmento));
	uint32_t direccionBase;
	int i = 0;

	t_programa *programa = encontrarPrograma(pid);

	if(programa != NULL){

		//Si el programa no tiene segmentos, a numero de segmento le pongo 0
		if(list_is_empty(programa->tablaSegmentos))
			segmento->numero = 0;

		else{
			//Si el programa ya tiene segmentos, busco el anterior y en numero de segmento le pongo el numero del anterior + 1
			t_segmento *segmentoAnterior = list_get(programa->tablaSegmentos, list_size(programa->tablaSegmentos) - 1);
			segmento->numero = segmentoAnterior->numero + 1;
		}

		segmento->tamanio = tamanio;
		segmento->tablaPaginas = list_create();

		list_add(programa->tablaSegmentos, segmento);

		for(i = 0; i < cantidadPaginas; i++){
			t_pagina *pagina = malloc(sizeof(t_pagina));
			pagina->numero = i;
			pagina->numeroMarco = NO_EN_MEMORIA;
			list_add(segmento->tablaPaginas, pagina);
		}

		direccionBase = calculoDireccionBase(segmento->numero);

		if (direccionBase == 0)
			log_info(MSPlogger, "Segmento creado correctamente. PID: %d, Tamanio: %d, Direccion base: 0x00000000", pid, tamanio);
		else
			log_info(MSPlogger, "Segmento creado correctamente. PID: %d, Tamanio: %d, Direccion base: %0.8p", pid, tamanio, direccionBase);

		return direccionBase;
	}

	else{
		log_error(MSPlogger, "No existe el programa con PID %d. No se hace nada.", pid);
		log_info(MSPlogger, "Finalizando...");
		return -1;
	}
}


double cantidadMemoriaTotal(){
	return cantidadMemoriaPrincipal + cantidadMemoriaSecundaria;
}

/***************************************************************************************************\
 *								--Comienzo Destruccion Segmento--									 *
\***************************************************************************************************/

void mspDestruirSegmento(int pid, uint32_t direccionBase){

	t_direccion direccionReal = calculoDireccionReal(direccionBase);

	if (direccionBase == 0)
		log_info(MSPlogger, "Comienzo de destruccion del Segmento con Direccion Base 0x00000000 para el PID %d... ", pid);
	else
		log_info(MSPlogger, "Comienzo de destruccion del Segmento con Direccion Base %0.8p para el PID %d... ", direccionBase, pid);

	//Compruebo que me hayan pasado la direccion base del segmento (pagina 0, desplazamiento 0)
	if(direccionReal.numeroPagina != 0 || direccionReal.desplazamiento != 0){
		log_error(MSPlogger, "No se encontro el segmento con base %0.8p para el programa con PID %d. No se hace nada. ", direccionBase, pid);
	}

	else{

		t_programa *programa = encontrarPrograma(pid);

		if(programa != NULL){

			t_segmento * segmento = encontrarSegmento(programa, direccionReal.numeroSegmento);

			if(segmento != NULL){

				borrarPaginasDeMemoriaSecundariaYPrimaria(pid, segmento);

				eliminarSegmentoDeListaDelPrograma(programa, direccionReal.numeroSegmento);

				if (direccionBase == 0)
					log_info(MSPlogger, "Segmento destruido correctamente. PID: %d, Direccion Base: 0x00000000, Numero de Segmento: %d", pid, direccionReal.numeroSegmento);
				else
					log_info(MSPlogger, "Segmento destruido correctamente. PID: %d, Direccion Base: %0.8p, Numero de Segmento: %d", pid, direccionBase, direccionReal.numeroSegmento);
			}

			else
				log_error(MSPlogger, "No se encontro el segmento con base %0.8p para el programa con PID %d. No se hace nada. ", direccionBase, pid);
		}
		else
			log_error(MSPlogger, "No se encontro el programa con PID %d. No se hace nada. ", pid);
	}

}

void eliminarSegmentoDeListaDelPrograma(t_programa *programa, int numeroSegmento){

	bool encontrarSegmento(t_segmento *unSegmento){
		return unSegmento->numero == numeroSegmento;
	}

	t_segmento *segmento = list_find(programa->tablaSegmentos, encontrarSegmento);

	list_destroy_and_destroy_elements(segmento->tablaPaginas, free);

	list_remove_and_destroy_by_condition(programa->tablaSegmentos, encontrarSegmento, free);

}

void borrarPaginasDeMemoriaSecundariaYPrimaria(int pid, t_segmento *segmento){

	t_list *paginasEnMemoria = list_create();
	t_list *paginasEnDisco = list_create();

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

	bool matchMarco(t_marco *unMarco){
		return unMarco->numero == pagina->numeroMarco;
	}

	t_marco *marco = list_find(marcosOcupados, matchMarco);

	list_add(marcosLibres, marco);

	list_remove_by_condition(marcosOcupados, matchMarco);

	cantidadMemoriaPrincipal += TAMANIO_PAGINA;

	//TODO ver como borrar correctamente la memoria
	memset(memoria + marco->inicio, 0, TAMANIO_PAGINA);
}


void borrarPaginaDeDisco(int pid, int numeroSegmento, int numeroPagina){

	char *absolute_path;

	absolute_path = armarPathArchivo(pid, numeroSegmento, numeroPagina);

	if(remove(absolute_path) == -1)
		log_error(MSPlogger, "Ha ocurrido un error al intentar borrar el archivo %s", absolute_path);

	else{
		cantidadMemoriaSecundaria += TAMANIO_PAGINA;
		log_info(MSPlogger, "El archivo de swapping %s se ha eliminado correctamente", absolute_path);
	}

}

/***************************************************************************************************\
 *								--Comienzo Escribir Memoria-- 										 *
\***************************************************************************************************/

bool mspEscribirMemoria(int pid, uint32_t direccionVirtual, char* buffer, int tamanio){

	char *mostrarBuffer = malloc(sizeof(char)*tamanio + 1);
	int cantidadPaginas;
	t_list *paginasAMemoria;
	t_direccion direccionReal = calculoDireccionReal(direccionVirtual);

	t_programa *programa = encontrarPrograma(pid);

	//Compruebo que sea un PID valido
	if(programa == NULL){
		log_error(MSPlogger, "No se ha encontrado el programa con el PID %d", pid);
		return false;
	}

	else{

		t_segmento *segmento = encontrarSegmento(programa, direccionReal.numeroSegmento);

		//Compruebo que el segmento corresponda a ese PID
		if(segmento == NULL){
			log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
			return false;
		}

		else {
			//Compruebo que no me soliciten escribir mas alla de los limites del segmento
			if(isSegmentationFault(segmento->tamanio, direccionReal, tamanio)){
				log_error(MSPlogger, "Se ha excedido los limites del segmento %d. Segmentation Fault", segmento->numero);
				return false;
			}

			t_pagina *pagina = encontrarPagina(segmento, direccionReal.numeroPagina);

			//Compruebo que la pagina pertenezca al segmento
			if(pagina == NULL){
				log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
				return false;
			}

			//Escribo Memoria
			else{

				//Calculo la cantidad de paginas que necesito tener en memoria, ademas de la pagina encontrada por la direccionBase
				cantidadPaginas = calcularCantidadPaginasNecesarias(tamanio, direccionReal.desplazamiento);

				//Creo una lista con todas las paginas que debo pasar a memoria
				paginasAMemoria = crearListaPaginasAPasarAMemoria(cantidadPaginas, pagina, segmento->tablaPaginas);

				buscarPaginasYEscribirMemoria(pid, direccionReal, paginasAMemoria, tamanio, buffer);

				//Esto es para imprimir en el log lo que se escribio en memoria
				memset(mostrarBuffer, 0, tamanio);
				memmove(mostrarBuffer, buffer, tamanio);

				log_info(MSPlogger, "Se ha escrito correctamente en memoria: %s", mostrarBuffer);

				list_destroy(paginasAMemoria);

				return true;
			}
		}
	}

	free(mostrarBuffer);
}

bool isSegmentationFault(int tamanioSegmento, t_direccion direccionReal, int tamanioSolicitado){

	if(tamanioSolicitado > tamanioSegmento -(direccionReal.numeroPagina * TAMANIO_PAGINA + direccionReal.desplazamiento)){
		return true;
	}

	return false;
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


t_list *crearListaPaginasAPasarAMemoria(int cantidadPaginas, t_pagina *pagina, t_list *paginas){

	int i;
	int numeroPagina = pagina->numero;

	t_list *paginasAMemoria = list_create();

	list_add(paginasAMemoria, pagina);
	for(i = 1; i <= cantidadPaginas; i++){

		bool matchPagina(t_pagina *unaPagina){
			return unaPagina->numero == numeroPagina + i;
		}

		t_pagina *paginaSiguiente = list_find(paginas, matchPagina);

		list_add(paginasAMemoria, paginaSiguiente);
	}

	return paginasAMemoria;
}


void buscarPaginasYEscribirMemoria(int pid, t_direccion direccionReal, t_list *paginasAMemoria, int tamanio, char *buffer){

	t_marco *marco;
	int numeroMarco;
	int posicionDondeLeer;
	int contador = 1;
	int tamanioParaPrimeraPagina = TAMANIO_PAGINA - direccionReal.desplazamiento;
	int cantidadPaginas = list_size(paginasAMemoria);
	int faltaEscribir = tamanio;

	void iterarPaginasParaEscribir(t_pagina *pagina){

		if(paginaEstaEnMemoria(pagina)){
			marco = encontrarMarcoPorPagina(pagina);
			seModificoElMarco(marco);
		}

		else if(pagina->numeroMarco == NO_EN_MEMORIA){
			log_info(MSPlogger, "La pagina no se encuentra en memoria");
			marco = llevarPaginaAMemoria(pagina);
		}

		else{
			log_info(MSPlogger, "La pagina se encuentra en memoria secundaria");
			numeroMarco = traerPaginaDeDiscoAMemoria(pid, direccionReal.numeroSegmento, pagina->numero);
			marco = encontrarMarcoPorNumeroMarco(numeroMarco);
			pagina->numeroMarco = numeroMarco;
		}

		seReferencioElMarco(marco);

		//Si debo escribir mas de una pagina debo saber en que posicion se encuentra esa pagina, para ver cuanto escribir
		if(cantidadPaginas != 1){

			//Si es la primera pagina la que debo escribir, escribo desde desplazamiento hasta el final (tamanioParaPrimeraPagina)
			if(contador == 1){
				memmove(memoria + marco->inicio + direccionReal.desplazamiento, buffer, tamanioParaPrimeraPagina);
				faltaEscribir -= tamanioParaPrimeraPagina;
				posicionDondeLeer = TAMANIO_PAGINA - direccionReal.desplazamiento;
			}

			//Si lo que queda por escribir supera o es igual a TAMANIO_PAGINA escribo toda la pagina
			else if(faltaEscribir >= TAMANIO_PAGINA){
				memmove(memoria + marco->inicio, buffer + tamanioParaPrimeraPagina, TAMANIO_PAGINA);
				faltaEscribir -= TAMANIO_PAGINA;
				posicionDondeLeer += TAMANIO_PAGINA;
			}

			//Si es la ultima pagina la que debo escribir, escribo lo que falta
			else if(contador == cantidadPaginas){
				memmove(memoria + marco->inicio, buffer + posicionDondeLeer, tamanio - faltaEscribir);
			}
		}
		//Si solo debo esribir una pagina, escribo directo
		else
			memmove(memoria + marco->inicio + direccionReal.desplazamiento, buffer, tamanio);

		contador++;
	}

	list_iterate(paginasAMemoria, iterarPaginasParaEscribir);
}


/***************************************************************************************************\
 * 								--Comienzo Leer Memoria-- 											 *
\***************************************************************************************************/


bool mspLeerMemoria(int pid, uint32_t direccionVirtual, int tamanio, char *leido){

	t_direccion direccionReal = calculoDireccionReal(direccionVirtual);
	t_list *paginasAMemoria;
	int cantidadPaginas;

	t_programa *programa = encontrarPrograma(pid);

	if(programa == NULL){
		log_error(MSPlogger, "No se ha encontrado el programa con el PID %d", pid);
		return false;
	}

	else{

		t_segmento *segmento = encontrarSegmento(programa, direccionReal.numeroSegmento);

		if(segmento == NULL){
			log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
			return false;
		}

		else{
			//Compruebo que no me soliciten leer mas alla de los limites del segmento
			if(isSegmentationFault(segmento->tamanio, direccionReal, tamanio)){
				log_error(MSPlogger, "Se ha excedido el tamanio del segmento %d. Segmentation Fault", segmento->numero);
				return false;
			}

			t_pagina *pagina = encontrarPagina(segmento, direccionReal.numeroPagina);

			if(pagina == NULL){
				log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
				return false;
			}

			else{
				//Calculo la cantidad de paginas que necesito tener en memoria, ademas de la pagina encontrada por la direccionBase
				cantidadPaginas = calcularCantidadPaginasNecesarias(tamanio, direccionReal.desplazamiento);


				//Creo una lista con todas las paginas que debo pasar a memoria
				paginasAMemoria = crearListaPaginasAPasarAMemoria(cantidadPaginas, pagina, segmento->tablaPaginas);

				if(buscarPaginasYLeerMemoria(pid, direccionReal, paginasAMemoria, tamanio, leido))
					log_info(MSPlogger, "Se ha leido de memoria: %s", leido);

				list_destroy(paginasAMemoria);

				return true;
			}
		}
	}

}


bool buscarPaginasYLeerMemoria(int pid, t_direccion direccionReal, t_list *paginasAMemoria, int tamanio, char *leido){

	int tamanioParaPrimerMarco = TAMANIO_PAGINA - direccionReal.desplazamiento;
	int cantidadPaginas = list_size(paginasAMemoria);
	int numeroMarco;
	int contador = 1;
	int faltaLeer = tamanio;
	int posicionDondeLeer;

	memset(leido, 0, TAMANIO_PAGINA);

	void iterarPaginasParaLeer(t_pagina *pagina){

		//Si la pagina se encuentra en disco, la paso a un marco libre y la borro de disco
		if(pagina->numeroMarco == EN_DISCO){
			log_info(MSPlogger, "La pagina se encuentra en memoria secundaria");
			numeroMarco = traerPaginaDeDiscoAMemoria(pid, direccionReal.numeroSegmento, direccionReal.numeroPagina);
			pagina->numeroMarco = numeroMarco;
		}

		//Si la pagina no se encuentra ni en disco ni en memoria la paso a un marco libre
		else if(pagina->numeroMarco == NO_EN_MEMORIA){
			log_info(MSPlogger, "La pagina no se encuentra en memoria");
			t_marco *nuevoMarco = llevarPaginaAMemoria(pagina);
			numeroMarco = nuevoMarco->numero;
		}

		else
			numeroMarco = pagina->numeroMarco;

		t_marco *marco = encontrarMarcoEnMarcosOcupados(numeroMarco);

		if(marco == NULL || numeroMarco != -1)
			return false;

		else{

			seReferencioElMarco(marco);

			//Si debo leer mas de un marco debo saber en que posicion se encuentra ese marco, para ver cuanto leer
			if(cantidadPaginas != 1){

				//Si es el primer marco el que debo leer, leo desde desplazamiento hasta el final (tamanioParaPrimeraPagina)
				if(contador == 1){
					memmove(leido, memoria + marco->inicio + direccionReal.desplazamiento, tamanioParaPrimerMarco);
					faltaLeer -= tamanioParaPrimerMarco;
					posicionDondeLeer = TAMANIO_PAGINA - direccionReal.desplazamiento;
				}

				//Si lo que queda por leer supera o es igual a TAMANIO_PAGINA leo todo_el marco
				else if(faltaLeer >= TAMANIO_PAGINA){
					memmove(leido + tamanioParaPrimerMarco, memoria + marco->inicio, TAMANIO_PAGINA);
					faltaLeer -= TAMANIO_PAGINA;
					posicionDondeLeer += TAMANIO_PAGINA;
				}

				//Si es el ultimo marco el que debo leer, leo lo que falta
				else if(contador == cantidadPaginas){
					memmove(leido + posicionDondeLeer, memoria + marco->inicio, tamanio - faltaLeer);
				}

			}
			//Si solo debo leer un marco, leo directo
			else
				memmove(leido, memoria + marco->inicio + direccionReal.desplazamiento, tamanio);
		}

		contador++;
	}

	list_iterate(paginasAMemoria, iterarPaginasParaLeer);
	return true;
}


//TODO si da error devolver -1
int traerPaginaDeDiscoAMemoria(int pid, int numeroSegmento, int numeroPagina){

	char *absolute_path;
	char linea[256];
	char buffer[256];
	t_marco *marco;

	log_info(MSPlogger, "Cargando pagina a memoria...");

	absolute_path = armarPathArchivo(pid, numeroSegmento, numeroPagina);
	FILE* file = fopen(absolute_path, "r");

	void copiarPaginaAMemoriaYEliminarDeDisco(t_marco *marco){

		log_info(MSPlogger, "Copiando la pagina en memoria...");

		memcpy(memoria + marco->inicio, buffer, TAMANIO_PAGINA);
		borrarPaginaDeDisco(pid, numeroSegmento, numeroPagina);

		log_info(MSPlogger, "Se ha cargado la Pagina %d del Segmento %d del PID %d en memoria correctamente", numeroPagina, numeroSegmento, pid);

		list_add(marcosOcupados, marco);
		t_programa *programa = encontrarPrograma(pid);
		t_segmento *segmento = encontrarSegmento(programa, numeroSegmento);
		t_pagina *pagina = encontrarPagina(segmento, numeroPagina);
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


bool paginaEstaEnMemoria(t_pagina *pagina){
	return pagina->numeroMarco >= 0;
}


t_marco *llevarPaginaAMemoria(t_pagina *pagina){

	t_marco *marco;

	log_info(MSPlogger, "Se cargara la pagina en memoria...");

	if(list_size(marcosLibres) > 0){
		marco = (t_marco*)list_remove(marcosLibres, 0);
		list_add(marcosOcupados, marco);
		log_info(MSPlogger, "La pagina %d ha sido cargada en memoria correctamente en el marco %d", pagina->numero, marco->numero);
	}

	else if(modoSustitucionPaginasMSP == FIFO)
		marco = sustituirPaginaPorFIFO();

	else
		marco = sustituirPaginaPorCLOCK_MODIFICADO();

	pagina->numeroMarco = marco->numero;

	return marco;
}


t_marco *sustituirPaginaPorFIFO(){

	int pid;
	int numeroSegmento;
	int numeroPagina;

	log_info(MSPlogger, "Comienzo de sustitucion de pagina por FIFO...");

	//Tomo el primer elemento de marcosOcupados y lo elimino de la lista
	t_marco *marco = (t_marco*)list_remove(marcosOcupados, 0);

	//Busco la pagina que esta cargada en el marco para poder llevarla a disco
	corresponderMarcoAPagina(marco, &pid, &numeroSegmento, &numeroPagina);

	log_info(MSPlogger, "Se reemplazara el marco %d", marco->numero);

	llevarPaginaADisco(marco, pid, numeroSegmento, numeroPagina);
	borrarMarcoDeMemoria(marco);

	//Agrego el marco en la ultima posicion de la lista marcosOcupados (para FIFO)
	list_add(marcosOcupados, marco);

	log_info(MSPlogger, "Se sustituyo correctamente la Pagina %d del Segmento %d del PID %d al Marco %d por FIFO", numeroPagina, numeroSegmento, pid, marco->numero);

	return marco;
}


t_marco *sustituirPaginaPorCLOCK_MODIFICADO(){

	int pid;
	int numeroSegmento;
	int numeroPagina;
	t_marco *marco;

	log_info(MSPlogger, "Comienzo de sustitucion de pagina por CLOCK MODIFICADO...");

	bool marcoNoRefNoMod(t_marco *unMarco){
		return unMarco->categoriaClockModificado == NOREFERENCIADA_NOMODIFICADA;
	}

	marco = list_find(marcosOcupados, marcoNoRefNoMod);

	if(marco == NULL){
		bool marcoNoRefMod(t_marco *unMarco){
			return unMarco->categoriaClockModificado == NOREFERENCIADA_MODIFICADA;
		}

		marco = list_find(marcosOcupados, marcoNoRefMod);

		if(marco == NULL){
			bool marcoRefNoMod(t_marco *unMarco){
				return unMarco->categoriaClockModificado == REFERENCIADA_NOMODIFICADA;
			}

			marco = list_find(marcosOcupados, marcoRefNoMod);

			if(marco == NULL){
				bool marcoRefMod(t_marco *unMarco){
					return unMarco->categoriaClockModificado == REFERENCIADA_MODIFICADA;
				}

				marco = list_find(marcosOcupados, marcoRefMod);
			}
		}
	}

	bool matchMarco(t_marco *unMarco){
		return unMarco->numero == marco->numero;
	}

	//Busco la pagina que corresponde al marco para poder eliminarla del disco (necesito pid, numeroSegmento, numeroPagina)
	corresponderMarcoAPagina(marco, &pid, &numeroSegmento, &numeroPagina);
	log_info(MSPlogger, "Se reemplazara el marco %d...", marco->numero);
	llevarPaginaADisco(marco, pid, numeroSegmento, numeroPagina);

	//Remuevo el marco de la lista de marcos ocupados
	list_remove_by_condition(marcosOcupados, matchMarco);
	//Agrego el marco en la ultima posicion de la lista de marcos ocupados (para FIFO)
	list_add(marcosOcupados, marco);

	log_info(MSPlogger, "Se sustituyo correctamente la Pagina %d del Segmento %d del PID %d al Marco %d por CLOCK MODIFICADO", numeroPagina, numeroSegmento, pid, marco->numero);

	return marco;
}

void borrarMarcoDeMemoria(t_marco *marco){
	memset(memoria + marco->inicio, 0, TAMANIO_PAGINA);
}

void seReferencioElMarco(t_marco *marco){

	if(marco->categoriaClockModificado == NOREFERENCIADA_NOMODIFICADA){
		marco->categoriaClockModificado = REFERENCIADA_NOMODIFICADA;
	}

	if(marco->categoriaClockModificado == NOREFERENCIADA_MODIFICADA){
		marco->categoriaClockModificado = REFERENCIADA_MODIFICADA;
	}
}


void seModificoElMarco(t_marco *marco){

	if(marco->categoriaClockModificado == NOREFERENCIADA_NOMODIFICADA)
		marco->categoriaClockModificado = NOREFERENCIADA_MODIFICADA;

	if(marco->categoriaClockModificado == REFERENCIADA_NOMODIFICADA)
		marco->categoriaClockModificado = REFERENCIADA_MODIFICADA;
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

	char *leido = malloc(sizeof(char)*TAMANIO_PAGINA + 1);
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
	char *extension = ".dat";
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


t_direccion calculoDireccionReal(uint32_t direccionLogica){
	t_direccion direccionReal;

	direccionReal.numeroSegmento = (direccionLogica & 0xFFF00000) >> 20;
	direccionReal.numeroPagina = (direccionLogica & 0xFFF00) >> 8;
	direccionReal.desplazamiento = (direccionLogica & 0xFF);

	return direccionReal;
}


t_programa *encontrarPrograma(int pid){

	bool matchPrograma(t_programa *unPrograma){
		return unPrograma->pid == pid;
	}

	if (!list_is_empty(programas))
		return list_find(programas, matchPrograma);
	else
		return NULL;
}


t_segmento * encontrarSegmento(t_programa *programa, int numeroSegmento){

	bool matchSegmento(t_segmento *unSegmento){
		return unSegmento->numero == numeroSegmento;
	}

	return list_find(programa->tablaSegmentos, matchSegmento);

}


t_pagina *encontrarPagina(t_segmento *segmento, int numeroPagina){

	bool matchPagina(t_pagina *unaPagina){
		return unaPagina->numero == numeroPagina;
	}

	return list_find(segmento->tablaPaginas, matchPagina);
}


t_marco *encontrarMarcoEnMarcosOcupados(int numeroMarco){

	bool matchMarco(t_marco *unMarco){
		return unMarco->numero == numeroMarco;
	}

	return list_find(marcosOcupados, matchMarco);
}


t_marco *encontrarMarcoPorPagina(t_pagina *pagina){

	t_list *marcos = list_create();
	list_add_all(marcos, marcosLibres);
	list_add_all(marcos,marcosOcupados);

	bool matchMarco(t_marco *unMarco){
		return unMarco->numero == pagina->numeroMarco;
	}

	t_marco *marco = list_find(marcos, matchMarco);

	list_destroy(marcos);

	return marco;

}

t_marco *encontrarMarcoPorNumeroMarco(int numeroMarco){

	t_list *marcos = list_create();
	list_add_all(marcos, marcosLibres);
	list_add_all(marcos,marcosOcupados);

	bool matchMarco(t_marco *unMarco){
		return unMarco->numero == numeroMarco;
	}

	t_marco *marco = list_find(marcos, matchMarco);

	list_destroy(marcos);

	return marco;

}



/*
bool segmentoYPaginaPorDireccionVirtual(int pid, t_programa *programa, t_segmento *segmento, t_pagina *pagina, uint32_t direccionVirtual){

	int numeroSegmento = calculoNumeroSegmento(direccionVirtual);
	int	numeroPagina = calculoNumeroPagina(direccionVirtual);

	programa = encontrarPrograma(pid);

	if(programa == NULL){
		log_error(MSPlogger, "El programa con PID %d no existe", pid);
		return false;
	}
	else{
		segmento = encontrarSegmento(programa, numeroSegmento);
		if(segmento == NULL){
			log_error(MSPlogger, "La direccion virtual %0.8p no corresponde al espacio de direcciones del PID %d. Segmentation Fault", direccionVirtual, pid);
			return false;
		}
		else{
			pagina = encontrarPagina(segmento, numeroPagina);
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
 */


