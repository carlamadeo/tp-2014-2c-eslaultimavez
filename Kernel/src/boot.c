#include "boot.h"
#include "Loader.h"
#include "kernelMSP.h"
#include "kernelConfig.h"
#include <stdlib.h>
#include <errno.h>


void crearTCBKERNEL(t_kernel* self){

	log_info(self->loggerKernel, "Boot: Creando TCB Kernel Mode...");

	FILE *archivoBesoSystemCall;
	self->tcbKernel = malloc(sizeof(t_TCB_Kernel));
	self->tcbKernel->pid = 0;
	self->tcbKernel->tid = 0;
	self->tcbKernel->km  = 1;

	self->tcbKernel->tamanio_segmento_codigo = obtenerTamanioArchivo(self, archivoBesoSystemCall);

	char *codigoSC = obtenerCodigoArchivo(self, archivoBesoSystemCall, self->tcbKernel->tamanio_segmento_codigo);

	self->tcbKernel->base_segmento_codigo = kernelCrearSegmento(self, self->tcbKernel->pid, self->tcbKernel->tamanio_segmento_codigo);
	self->tcbKernel->puntero_instruccion  = self->tcbKernel->base_segmento_codigo;

	bootEscribirMemoria(self, self->tcbKernel->pid, self->tcbKernel->base_segmento_codigo, codigoSC, self->tcbKernel->tamanio_segmento_codigo);

	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	if(socket_recvPaquete(self->socketMSP->socket, paquete) >= 0){
		if(paquete->header.type == ESCRIBIR_MEMORIA){ //que bonito!
			self->tcbKernel->base_stack = kernelCrearSegmento(self, self->tcbKernel->pid, self->tamanioStack);
			self->tcbKernel->cursor_stack = self->tcbKernel->base_stack;
			log_info(self->loggerKernel,"Boot: La direccion de base de stack es: %0.8p para el PID: %d y TID:%d", self->tcbKernel->base_stack, self->tcbKernel->pid, self->tcbKernel->tid);
		}

	}


	self->tcbKernel->registro_de_programacion[0] = 0;
	self->tcbKernel->registro_de_programacion[1] = 0;
	self->tcbKernel->registro_de_programacion[2] = 0;
	self->tcbKernel->registro_de_programacion[3] = 0;



	sem_wait(&mutex_block);
	list_add(cola_block,self->tcbKernel);
	sem_post(&mutex_block);
	log_info(self->loggerLoader,"Boot: Agrego un elemento a la Cola Block con el PID:%d  TID:%d ", self->tcbKernel->pid, self->tcbKernel->tid);

	//return programaEnElKernel;
}

int obtenerTamanioArchivo(t_kernel *self, FILE *archivoBesoSystemCall){

	archivoBesoSystemCall = fopen("../Resources/besoSystemCall.bc", "r");

	if(archivoBesoSystemCall == 0){
		log_error(self->loggerKernel, "Kernel: Error al abrir el archivo besoSystemCall");
		exit(-1);
	}

	fseek(archivoBesoSystemCall, 0, SEEK_END);
	size_t sizeArchivoBesoSC = ftell(archivoBesoSystemCall);

	fclose(archivoBesoSystemCall);
	return sizeArchivoBesoSC;
}

char *obtenerCodigoArchivo(t_kernel *self, FILE *archivoBesoSystemCall, int tamanio){

	archivoBesoSystemCall = fopen("../Resources/besoSystemCall.bc", "r");

	if(archivoBesoSystemCall == 0){
		log_error(self->loggerKernel, "Kernel: Error al abrir el archivo besoSystemCall");
		exit(-1);
	}

	fseek(archivoBesoSystemCall, 0, SEEK_SET);

	char *codigoSystemCall = malloc(sizeof(char)*tamanio);

	fread(codigoSystemCall, 1, tamanio, archivoBesoSystemCall);

	fclose(archivoBesoSystemCall);
	return codigoSystemCall;
}

void bootEscribirMemoria(t_kernel* self, int pid, uint32_t direccionVirtual, char *programaBeso, int tamanioBeso){

	t_escribirSegmentoBeso* escrituraDeCodigo = malloc(sizeof(t_escribirSegmentoBeso));
	t_socket_paquete *paqueteConfirmacionEscritura = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_confirmacionEscritura *unaConfirmacionEscritura = (t_confirmacionEscritura *)malloc(sizeof(t_confirmacionEscritura));

	escrituraDeCodigo->direccionVirtual = direccionVirtual;
	escrituraDeCodigo->pid = pid;
	escrituraDeCodigo->tamanio = tamanioBeso;
	strcpy(escrituraDeCodigo->bufferCodigoBeso, programaBeso);

	log_info(self->loggerKernel, "Boot: Solicitud de escritura de %s en memoria para PID: %d, Direccion Virtual: %0.8p, Tamaño: %d.", escrituraDeCodigo->bufferCodigoBeso, escrituraDeCodigo->pid, escrituraDeCodigo->direccionVirtual, escrituraDeCodigo->tamanio);

	socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA, sizeof(t_escribirSegmentoBeso), escrituraDeCodigo);

}
