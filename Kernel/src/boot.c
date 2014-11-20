#include "boot.h"
#include "kernelMSP.h"
#include "Kernel.h"
#include "kernelConfig.h"
#include "commons/socketInBigBang.h"
#include <stdlib.h>


void crearTCBKERNEL(t_kernel* self){

	log_info(self->loggerKernel, "Boot: Creando TCB...");

	self->tcbKernel = malloc(sizeof(t_TCB_Kernel));
	self->tcbKernel->pid = 125;
	self->tcbKernel->tid = 465;
	self->tcbKernel->km  = 1;

	self->tcbKernel->tamanio_segmento_codigo = obtenerCogidoBesoSystemCall(self);
	self->tcbKernel->base_segmento_codigo = kernelCrearSegmento(self, self->tcbKernel->pid, self->tcbKernel->tamanio_segmento_codigo);
	self->tcbKernel->puntero_instruccion  = self->tcbKernel->base_segmento_codigo;

	self->tcbKernel->base_stack = kernelCrearSegmento(self, self->tcbKernel->pid, self->tamanioStack);
	self->tcbKernel->cursor_stack = self->tcbKernel->base_stack;

	self->tcbKernel->registro_de_programacion[0] = 0;
	self->tcbKernel->registro_de_programacion[1] = 0;
	self->tcbKernel->registro_de_programacion[2] = 0;
	self->tcbKernel->registro_de_programacion[3] = 0;
	log_info(self->loggerKernel, "Boot: Completado con éxito!");

	//return programaEnElKernel;
}


//esta funcion tiene que leer el archivo
int obtenerCogidoBesoSystemCall(t_kernel *self){

	FILE *archivoBesoSystemCall = fopen("../../ArchivosConfiguracion/besoSystemCall.bc", "r");

	if(archivoBesoSystemCall == 0){
		log_error(self->loggerKernel, "Kernel: Error al abrir el archivo besoSystemCall");
		exit(-1);
	}

	fseek(archivoBesoSystemCall, 0, SEEK_END);	//Me coloco al final del fichero para saber el tamanio
	size_t sizeArchivoBeso = ftell(archivoBesoSystemCall);
	fseek(archivoBesoSystemCall, 0, SEEK_SET);	//Me coloco al principio del fichero para leerlo

	//falta hacer el SEND y mandarlo a la msp


	return sizeArchivoBeso;
}
