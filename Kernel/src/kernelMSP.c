/*
 * kernelMSP.c
 *
 *  Created on: 18/11/2014
 *      Author: utnso
 */

#include "Kernel.h"
#include "kernelMSP.h"
#include "kernelConfig.h"
#include "commons/protocolStructInBigBang.h"
#include <stdlib.h>

void hacer_conexion_con_msp(t_kernel *self) {

	self->socketMSP = socket_createClient();

	if (self->socketMSP == NULL )
		log_error(self->loggerKernel, "Kernel: Error al crear socket con la MSP!");

	if (socket_connect(self->socketMSP, self->ipMsp, self->puertoMsp) == 0)
		log_error(self->loggerKernel, "Kernel: Error al hacer el Boot con la MSP!");

	else{
		log_info(self->loggerKernel, "Kernel: Conectado con la MSP (IP: %s/Puerto: %d)!", self->ipMsp, self->puertoMsp);
		realizarHandshakeConMSP(self);
	}
}


void realizarHandshakeConMSP(t_kernel *self) {

	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if (socket_sendPaquete(self->socketMSP->socket, HANDSHAKE_KERNEL, 0, NULL) > 0)
		log_info(self->loggerKernel, "Kernel se presenta a la MSP!");


	if (socket_recvPaquete(self->socketMSP->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_MSP)
			log_info(self->loggerKernel, "Kernel: Se realizaron los Handshake con la MSP correctamente");
	}

	else
		log_error(self->loggerKernel, "Kernel: Error al recibir los datos de la MSP!");

}


int kernelCrearSegmento(t_kernel *self, int pid, int tamanio){

	t_datos_aMSP* datosAEnviar = malloc(sizeof(t_datos_aMSP));
	t_datos_deMSP *datosRecibidos = malloc(sizeof(t_datos_deMSP));
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	datosAEnviar->pid = pid;
	datosAEnviar->tamanio = tamanio;


	if (socket_sendPaquete(self->socketMSP->socket, CREAR_SEGMENTO, sizeof(t_datos_aMSP), datosAEnviar) > 0) {
		log_info(self->loggerKernel, "Kernel: Solicitud de creacion de segmento de Tamaño %d para el Proceso con PID %d.", datosAEnviar->tamanio, datosAEnviar->pid);

		if(socket_recvPaquete(self->socketMSP->socket, paquete) >= 0){

			if(paquete->header.type == CREAR_SEGMENTO){
				datosRecibidos = (t_datos_deMSP *) (paquete->data);
				log_info(self->loggerKernel, "Kernel: Se recibio de la MSP la direccion base %0.8p ", datosRecibidos->direccionBase);

				if(datosRecibidos->direccionBase < 0){
					//Manejo de errores
					//ERROR_POR_TAMANIO_EXCEDIDO
					//ERROR_POR_MEMORIA_LLENA
					//ERROR_POR_NUMERO_NEGATIVO
					//ERROR_POR_SEGMENTO_INVALIDO
					//ERROR_POR_SEGMENTATION_FAULT
				}
			}
		}

		else{
			log_info(self->loggerKernel, "Kernel: Error al recibir los datos de creacion de segmento");
			return -1;
		}
	}

	free(datosAEnviar);
	return datosRecibidos->direccionBase;
}


int kernelEscribirMemoria(t_kernel* self, int pid, uint32_t direccionVirtual, char *programaBeso, int tamanioBeso, t_socket* socketNuevoCliente){

	t_escribirSegmentoBeso* escrituraDeCodigo = malloc(sizeof(t_escribirSegmentoBeso));
	t_socket_paquete *paqueteConfirmacionEscritura = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_confirmacionEscritura *unaConfirmacionEscritura = (t_confirmacionEscritura *)malloc(sizeof(t_confirmacionEscritura));

	escrituraDeCodigo->direccionVirtual = direccionVirtual;
	escrituraDeCodigo->pid = pid;
	escrituraDeCodigo->tamanio = tamanioBeso;
	escrituraDeCodigo->bufferCodigoBeso = programaBeso;

	log_info(self->loggerKernel, "Kernel: Solicitud de escritura de %s en memoria para PID: %d, Direccion Virtual: %0.8p, Tamaño: %d.", escrituraDeCodigo->bufferCodigoBeso, escrituraDeCodigo->pid, escrituraDeCodigo->direccionVirtual, escrituraDeCodigo->tamanio);
	//memcpy(escrituraDeCodigo->bufferCodigoBeso, programaBeso, strlen(programaBeso)); //importante, ver si tiene o no el /0

	socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA, sizeof(t_escribirSegmentoBeso), escrituraDeCodigo);

	socket_recvPaquete(socketNuevoCliente, paqueteConfirmacionEscritura);

	unaConfirmacionEscritura = (t_confirmacionEscritura *) paqueteConfirmacionEscritura->data;

	switch(unaConfirmacionEscritura->estado){

	case ERROR_POR_SEGMENTATION_FAULT:

		break;
	default:

		break;

	}

	return unaConfirmacionEscritura->estado;
}
