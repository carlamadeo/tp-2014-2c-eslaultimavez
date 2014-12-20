/*
 * Consola.c
 *
 *  Created on: 13/09/2014
 *      Author: utnso
 */

#include "mspConsola.h"
#include "MSP.h"
#include "Memoria.h"
#include "commons/log.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

t_MSP *self;

void *mspLanzarhiloConsola() {

	int opcion;
	while(1)
	{
		printf("\n-----------Consola de la MSP-----------\n\n");
		printf("¿Que operacion desea realizar?\n\n");
		printf("1 - Crear Segmento\n");
		printf("2 - Destruir Segmento\n");
		printf("3 - Escribir Memoria\n");
		printf("4 - Leer Memoria\n");
		printf("5 - Ver Tabla de Segmentos\n");
		printf("6 - Ver Tabla de Paginas\n");
		printf("7 - Listar Marcos\n");
		printf("\nIngrese su opcion\n");

		scanf("%d", &opcion);

		switch (opcion)
		{
			case 1:
				system("clear");
				consolaCrearSegmento();
				printf("\nPresione ENTER para volver al menu ");
				getchar();
				break;

			case 2:
				system("clear");
				consolaDestruirSegmento();
				printf("\nPresione ENTER para volver al menu ");
				getchar();
				break;

			case 3:
				system("clear");
				consolaEscribirMemoria();
				printf("\nPresione ENTER para volver al menu ");
				getchar();
				break;

			case 4:
				system("clear");
				consolaLeerMemoria();
				printf("\nPresione ENTER para volver al menu ");
				getchar();
				break;

			case 5:
				system("clear");
				imprimirTablaDeSegmentos();
				printf("\nPresione ENTER para volver al menu ");
				getchar();
				break;

			case 6:
				system("clear");
				consolaImprimirTablaDePaginas();
				printf("\nPresione ENTER para volver al menu ");
				getchar();
				break;

			case 7:
				system("clear");
				imprimirMarcos();
				printf("\nPresione ENTER para volver al menu ");
				getchar();
				break;

			default:
				break;
		}

	}
	return NULL;
}


void consolaCrearSegmento() {
	int pid;
	int tamanio;

	printf("\n###################CREACION DE SEGMENTO###################\n\n");
	printf("Ingrese el numero PID del programa: ");
	scanf("%d", &pid);
	printf("Ingrese el tamanio del segmento a crear: ");
	scanf("%d", &tamanio);
	mspCrearSegmento(pid, tamanio);
	getchar();
}

void consolaDestruirSegmento() {
	int pid;
	uint32_t direccionBase = 0;

	printf("\n###################DESTRUCCION DE SEGMENTO###################\n\n");
	printf("Ingrese el numero PID del programa: ");
	scanf("%d", &pid);
	printf("Ingrese la direccion base del segmento a destruir: ");
	scanf("%d", &direccionBase);
	mspDestruirSegmento(pid, direccionBase);
	getchar();
}

void consolaEscribirMemoria() {
	int pid;
	uint32_t direccionVirtual = 0;
	int tamanio;
	char *texto;

	printf("\n###################ESCRIBIR MEMORIA###################\n\n");
	printf("Ingrese el numero PID del programa: ");
	scanf("%d", &pid);

	printf("Ingrese la direccion virtual donde desea escribir: ");
	scanf("%d", &direccionVirtual);

	printf("Ingrese el tamanio de lo que desea escribir: ");
	scanf("%d", &tamanio);
	while(getchar() != '\n');

	texto = malloc(sizeof(char)*tamanio + 1);
	memset(texto, 0, tamanio);

	printf("Ingrese lo que desea escribir: ");
	fgets(texto, tamanio, stdin);

	mspEscribirMemoria(pid, direccionVirtual, texto, tamanio);

	free(texto);
	getchar();
}

void consolaLeerMemoria() {
	int pid;
	uint32_t direccionVirtual = 0;
	int tamanio;
	char *leido;

	printf("\n###################LEER MEMORIA###################\n\n");
	printf("Ingrese el numero PID del programa: ");
	scanf("%d", &pid);
	printf("Ingrese la direccion virtual desde donde desea leer: ");
	scanf("%d", &direccionVirtual);
	printf("Ingrese el tamanio del contenido a leer: ");
	scanf("%d", &tamanio);

	leido = malloc(sizeof(char)*TAMANIO_PAGINA + 1);

	mspLeerMemoria(pid, direccionVirtual, tamanio, leido);

	free(leido);
	getchar();
}

void imprimirTablaDeSegmentos() {
	uint32_t direccionBase;
	int pid;
	int cantSegmentos = 0;

	if(list_size(self->programas) == 0)
		log_warning(self->logMSP, "No hay Programas. Nada que mostrar");

	else{

		log_info(self->logMSP, "TABLA DE SEGMENTOS:");
		void mostrarSegmento(t_segmento *unSegmento){
			direccionBase = calculoDireccionBase(unSegmento->numero);

			if(direccionBase == 0)
				log_info(self->logMSP,"Segmento#: %d | Tamanio: %d | Direccion Base: 0x00000000 | Pertenece a Programa: %d", unSegmento->numero, unSegmento->tamanio, pid);
			else
				log_info(self->logMSP,"Segmento#: %d | Tamanio: %d | Direccion Base: %0.8p | Pertenece a Programa: %d", unSegmento->numero, unSegmento->tamanio, direccionBase, pid);

			cantSegmentos+=1;
		}

		void mostrarPrograma(t_programa *unPrograma){
			pid = unPrograma->pid;
			list_iterate(unPrograma->tablaSegmentos, mostrarSegmento);
		}

		list_iterate(self->programas, mostrarPrograma);

		if(cantSegmentos == 0)
			log_info(self->logMSP, "No hay Segmentos. Nada que mostrar");

	}
	getchar();
}

void consolaImprimirTablaDePaginas() {
	int pid;
	int numeroSegmento;
	char *enMemoria = malloc(sizeof(char)*2 + 1);

	printf("Ingrese el numero PID del programa: ");
	scanf("%d", &pid);

	log_info(self->logMSP, "TABLA DE PAGINAS PARA EL PID %d:", pid);

	bool matchPrograma(t_programa *unPrograma){
		return unPrograma->pid == pid;
	}

	if (list_is_empty(self->programas))
		log_info(self->logMSP, "No hay programas. Nada que mostrar");


	else{
		t_programa *programa = list_find(self->programas, matchPrograma);

		if(programa == NULL)
			log_error(self->logMSP, "No existe el programa con PID %d", pid);

		else{
			void mostrarPagina(t_pagina *unaPagina){
				if(unaPagina->numeroMarco != NO_EN_MEMORIA){
					enMemoria = "Si";
				}
				else{
					enMemoria = "No";
				}
				log_info(self->logMSP,"Pagina#: %d | En Memoria Principal: %s | Pertenece a Segmento: %d", unaPagina->numero, enMemoria, numeroSegmento);
			}

			void mostrarSegmento(t_segmento *unSegmento){
				numeroSegmento = unSegmento->numero;
				list_iterate(unSegmento->tablaPaginas, mostrarPagina);
			}

			if(list_is_empty(programa->tablaSegmentos)){
				log_warning(self->logMSP, "No hay paginas para el PID %d. Nada que mostrar", pid);
			}

			list_iterate(programa->tablaSegmentos, mostrarSegmento);
		}

	}
	getchar();
}

void imprimirMarcos() {
	int pid, numeroSegmento;
	int cantidadMarcosOcupados = 0;

	printf("\n#############################################TABLA DE MARCOS#############################################\n");

	void mostrarMarcosLibres(t_marco *unMarco){
		log_info(self->logMSP, "Marco#: %d | PID: - | Numero de Segmento: - | Numero de Pagina: -", unMarco->numero);
	}



	void iterarPagina(t_pagina *unaPagina){
		if(unaPagina->numeroMarco >= 0){
			bool matchMarco(t_marco *unMarco){
						return unMarco->numero == unaPagina->numeroMarco;
			}
			t_marco *marco = list_find(self->marcosOcupados, matchMarco);
			log_info(self->logMSP, "Marco#: %d | PID: %d | Numero de Segmento: %d | Numero de Pagina: %d | Valor Clock: %d", unaPagina->numeroMarco, pid, numeroSegmento, unaPagina->numero, marco->categoriaClockModificado);
			cantidadMarcosOcupados+=1;
		}
	}

	void iterarSegmento(t_segmento *unSegmento){
		numeroSegmento = unSegmento->numero;
		list_iterate(unSegmento->tablaPaginas, iterarPagina);
	}

	void iterarPrograma(t_programa *unPrograma){
		pid = unPrograma->pid;
		list_iterate(unPrograma->tablaSegmentos, iterarSegmento);
	}

	log_info(self->logMSP, "\n----------------------------------------------Marcos Libres----------------------------------------------\n\n");

	if(list_size(self->marcosLibres) > 0)
		list_iterate(self->marcosLibres, mostrarMarcosLibres);

	log_info(self->logMSP, "\n---------------------------------------------Marcos Ocupados---------------------------------------------\n\n");

	if(list_size(self->programas) > 0)
		list_iterate(self->programas, iterarPrograma);

	if(cantidadMarcosOcupados == 0)
		log_info(self->logMSP,"No hay marcos ocupados que mostrar");

	getchar();
}


