/*
 * Consola.c
 *
 *  Created on: 13/09/2014
 *      Author: utnso
 */

#include "Consola.h"
#include "Memoria.h"
#include "Programa.h"

extern t_log *MSPlogger;
extern t_list *programas;
extern t_list *marcosLibres;

void *mspLanzarhiloMSPCONSOLA() {

	int opcion;
	do
	{
		system("clear");
		printf("\n-----------Consola de la MSP-----------\n\n");
		printf("¿Que operacion desea realizar?\n\n");
		printf("1 - Crear Segmento\n");
		printf("2 - Destruir Segmento\n");
		printf("3 - Escribir Memoria\n");
		printf("4 - Leer Memoria\n");
		printf("5 - Ver Tabla de Segmentos\n");
		printf("6 - Ver Tabla de Paginas\n");
		printf("7 - Listar Marcos\n");
		printf("\nIngrese su opcion (0 para finalizar): ");

		scanf("%d", &opcion);

		switch (opcion)
		{
			case 0:
				break;
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

	}while(opcion != 0);
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
	crearSegmento(pid, tamanio);
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
	destruirSegmento(pid, direccionBase);
	getchar();
}

void consolaEscribirMemoria() {
	int pid;
	uint32_t direccionVirtual = 0;
	int tamanio;
	char *texto = malloc(sizeof(char)*TAMANIO_PAGINA + 1);

	printf("\n###################ESCRIBIR MEMORIA###################\n\n");
	printf("Ingrese el numero PID del programa: ");
	scanf("%d", &pid);
	printf("Ingrese la direccion virtual donde desea escribir: ");
	scanf("%d", &direccionVirtual);
	printf("Ingrese el tamanio de lo que desea escribir: ");
	scanf("%d", &tamanio);
	printf("Ingrese lo que desea escribir: ");
	scanf("%s", texto);

	escribirMemoria(pid, direccionVirtual, texto, tamanio);

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

	leido = malloc(sizeof(char)*tamanio + 1);

	leerMemoria(pid, direccionVirtual, tamanio, leido);

	free(leido);
	getchar();
}

void imprimirTablaDeSegmentos() {
	uint32_t direccionBase;
	int pid;
	int cantSegmentos = 0;

	if(list_size(programas) == 0){
		log_warning(MSPlogger, "No hay Programas. Nada que mostrar");
	}

	else{
		log_info(MSPlogger, "TABLA DE SEGMENTOS:");
		void mostrarSegmento(t_segmento *unSegmento){
			direccionBase = calculoDireccionBase(unSegmento->numero);
			log_info(MSPlogger,"Segmento#: %d | Tamanio: %d | Direccion Base: %0.8p | Pertenece a Programa: %d", unSegmento->numero, unSegmento->tamanio, direccionBase, pid);
			cantSegmentos+=1;
		}

		void mostrarPrograma(t_programa *unPrograma){
			pid = unPrograma->pid;
			list_iterate(unPrograma->tablaSegmentos, mostrarSegmento);
		}

		list_iterate(programas, mostrarPrograma);

		if(cantSegmentos == 0){
			log_info(MSPlogger, "No hay Segmentos. Nada que mostrar");
		}
	}
	getchar();
}

void consolaImprimirTablaDePaginas() {
	int pid;
	int numeroSegmento;
	char *enMemoria;
	t_programa *programa = malloc(sizeof(t_programa));

	printf("Ingrese el numero PID del programa: ");
	scanf("%d", &pid);

	log_info(MSPlogger, "TABLA DE PAGINAS PARA EL PID %d:", pid);

	bool matchPrograma(t_programa *unPrograma){
		return unPrograma->pid == pid;
	}

	if (list_is_empty(programas)){
		log_info(MSPlogger, "No hay programas. Nada que mostrar");
	}

	else{
		programa = list_find(programas, matchPrograma);

		if(programa == NULL){
			log_error(MSPlogger, "No existe el programa con PID %d", pid);
		}

		else{
			void mostrarPagina(t_pagina *unaPagina){
				if(unaPagina->numeroMarco != NO_EN_MEMORIA){
					enMemoria = "Si";
				}
				else{
					enMemoria = "No";
				}
				log_info(MSPlogger,"Pagina#: %d | En Memoria Principal: %s | Pertenece a Segmento: %d", unaPagina->numero, enMemoria, numeroSegmento);
			}

			void mostrarSegmento(t_segmento *unSegmento){
				numeroSegmento = unSegmento->numero;
				list_iterate(unSegmento->tablaPaginas, mostrarPagina);
			}

			if(list_is_empty(programa->tablaSegmentos)){
				log_warning(MSPlogger, "No hay paginas para el PID %d. Nada que mostrar", pid);
			}

			list_iterate(programa->tablaSegmentos, mostrarSegmento);
		}

	}
	getchar();
}

void imprimirMarcos() {
	int pid, numeroSegmento;
	int cantidadMarcosOcupados = 0;

	printf("\n######################TABLA DE MARCOS######################\n");

	void mostrarMarcosLibres(t_marco *unMarco){
		log_info(MSPlogger, "Marco#: %d | PID: - | Numero de Segmento: - | Numero de Pagina: -", unMarco->numero);
	}

	void iterarPagina(t_pagina *unaPagina){
		if(unaPagina->numeroMarco >= 0){
			log_info(MSPlogger, "Marco#: %d | PID: %d | Numero de Segmento: %d | Numero de Pagina: %d", unaPagina->numeroMarco, pid, numeroSegmento, unaPagina->numero);
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

	printf("\n----------------------Marcos Libres----------------------\n\n");
	if(list_size(marcosLibres) > 0){
		list_iterate(marcosLibres, mostrarMarcosLibres);
	}
	printf("\n---------------------Marcos Ocupados---------------------\n\n");
	if(list_size(programas) > 0){
		list_iterate(programas, iterarPrograma);
	}

	if(cantidadMarcosOcupados == 0){
		printf("No hay marcos ocupados que mostrar\n");
	}

	getchar();
}


