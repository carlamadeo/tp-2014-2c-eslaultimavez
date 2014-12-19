#include "commons/socketInBigBang.h"
#include "cpuMSP.h"
#include "cpuConfig.h"
#include "commons/protocolStructInBigBang.h"
#include <stdlib.h>


void cpuConectarConMPS(t_CPU *self) {

	self->socketMSP = socket_createClient();

	if (self->socketMSP == NULL)
		log_error(self->loggerCPU, "CPU: Error al crear socket con la MSP!");

	if (socket_connect(self->socketMSP, self->ipMsp, self->puertoMSP) == 0)
		log_error(self->loggerCPU, "CPU: Error al hacer el connect con la MSP!");

	else{
		//printf("IP: %s y Puerto:  %d\n", self->ipMsp,self->puertoMSP);
		log_info(self->loggerCPU, "CPU: Conectado con la MSP (IP: %s/Puerto: %d)!", self->ipMsp, self->puertoMSP);
		cpuRealizarHandshakeConMSP(self);
	}
}


void cpuRealizarHandshakeConMSP(t_CPU *self) {

	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if (socket_sendPaquete(self->socketMSP->socket, HANDSHAKE_CPU, 0, NULL) > 0)
		log_info(self->loggerCPU, "CPU Se presenta a la MSP!");


	if (socket_recvPaquete(self->socketMSP->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_MSP)
			log_info(self->loggerCPU, "CPU: Se realizaron los Handshake con la MSP correctamente");
		else
			log_error(self->loggerCPU, "CPU: Error al recibir los datos de la MSP!");
	}

	else
		log_error(self->loggerCPU, "CPU: Error al recibir paquete de la MSP.");

	free(paquete);

}


uint32_t cpuCrearSegmento(t_CPU *self, int pid, int tamanio){

	t_datos_aMSP* datosAEnviar = malloc(sizeof(t_datos_aMSP));
	t_datos_deMSP *datosRecibidos = malloc(sizeof(t_datos_deMSP));
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	uint32_t direccionBase;

	datosAEnviar->pid = pid;
	datosAEnviar->tamanio = tamanio;

	if (socket_sendPaquete(self->socketMSP->socket, CREAR_SEGMENTO, sizeof(t_datos_aMSP), datosAEnviar) > 0) {
		log_info(self->loggerCPU, "CPU: Solicitud de creacion de segmento de Tamaño %d para el Proceso con PID %d.", datosAEnviar->tamanio, datosAEnviar->pid);

		if(socket_recvPaquete(self->socketMSP->socket, paquete) >= 0){

			if(paquete->header.type == CREAR_SEGMENTO){

				datosRecibidos = (t_datos_deMSP *) (paquete->data);

				direccionBase = datosRecibidos->direccionBase;

				log_info(self->loggerCPU, "CPU: Se recibio de la MSP la Direccion Base  de segmento %0.8p ", direccionBase);

				if(datosRecibidos->direccionBase == SIN_ERRORES)
					log_info(self->loggerCPU, "CPU: Se creo correctamente el segmento solicitado. Direccion base: %0.8p correctamente", direccionBase);
			}
		}

		else{
			log_info(self->loggerCPU, "CPU: Error al recibir los datos de creacion de segmento");
			return -1;
		}
	}

	free(datosAEnviar);
	free(paquete);
	free(datosRecibidos);
	return direccionBase;
}


int cpuDestruirSegmento(t_CPU *self, uint32_t direccionVirtual){

	t_destruirSegmento* destruir_segmento = malloc(sizeof(t_destruirSegmento));
	t_socket_paquete *paqueteConfirmacionDestruccionSegmento = malloc(sizeof(t_socket_paquete));
	t_confirmacion* confirmacion = malloc(sizeof(t_confirmacion));
	int intConfirmacion;

	destruir_segmento->pid = self->tcb->pid;
	destruir_segmento->direccionVirtual = direccionVirtual;

	socket_sendPaquete(self->socketMSP->socket, DESTRUIR_SEGMENTO, sizeof(t_destruirSegmento), destruir_segmento);

	socket_recvPaquete(self->socketMSP->socket, paqueteConfirmacionDestruccionSegmento);

	confirmacion = (t_confirmacion *) paqueteConfirmacionDestruccionSegmento->data;

	if(confirmacion->estado == SIN_ERRORES)
		log_info(self->loggerCPU, "CPU: Se destruyo el segmento con Direccion Virtual %0.8p correctamente", direccionVirtual);

	intConfirmacion = confirmacion->estado;

	free(paqueteConfirmacionDestruccionSegmento);
	free(destruir_segmento);
	free(confirmacion);
	return intConfirmacion;
}


int cpuEscribirMemoria(t_CPU *self, uint32_t direccionVirtual, char *programa, int tamanio){

	t_escribirSegmentoBeso* escrituraDeCodigo = malloc(sizeof(t_escribirSegmentoBeso));
	t_socket_paquete *paqueteConfirmacionEscritura = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_confirmacion *unaConfirmacionEscritura = (t_confirmacion *)malloc(sizeof(t_confirmacion));
	int intConfirmacion;

	escrituraDeCodigo->direccionVirtual = direccionVirtual;
	int pid = 0;
	if(self->tcb->km == 0) pid = self->tcb->pid;
	escrituraDeCodigo->pid = pid;
	escrituraDeCodigo->tamanio = tamanio;
	strcpy(escrituraDeCodigo->bufferCodigoBeso, programa);

	log_info(self->loggerCPU, "CPU: Solicitud de escritura de %s en memoria para PID: %d, Direccion Virtual: %0.8p, Tamaño: %d.", escrituraDeCodigo->bufferCodigoBeso, escrituraDeCodigo->pid, escrituraDeCodigo->direccionVirtual, escrituraDeCodigo->tamanio);

	socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA, sizeof(t_escribirSegmentoBeso), escrituraDeCodigo);

	socket_recvPaquete(self->socketMSP->socket, paqueteConfirmacionEscritura);

	unaConfirmacionEscritura = (t_confirmacion *) paqueteConfirmacionEscritura->data;

	if(unaConfirmacionEscritura->estado == SIN_ERRORES)
		log_info(self->loggerCPU, "CPU: Se escribio correctamente en memoria, Direccion Virtual %0.8p", direccionVirtual);

	intConfirmacion = unaConfirmacionEscritura->estado;

	free(escrituraDeCodigo);
	free(paqueteConfirmacionEscritura);
	free(unaConfirmacionEscritura);
	return intConfirmacion;
}


int cpuLeerMemoria(t_CPU *self, uint32_t direccionVirtual, char *programa, int tamanio){

	t_datos_aMSPLectura *datosAMSP = malloc(sizeof(t_datos_aMSPLectura));
	t_socket_paquete *paqueteLectura = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_datos_deMSPLectura *unaLectura = (t_datos_deMSPLectura *)malloc(sizeof(t_datos_deMSPLectura));

	int estado;

	datosAMSP->direccionVirtual = direccionVirtual;
	int pid = 0;
	if(self->tcb->km == 0) pid = self->tcb->pid;
	datosAMSP->pid = pid;
	datosAMSP->tamanio = tamanio;

	log_info(self->loggerCPU, "CPU: Solicitud de lectura de memoria para PID: %d, Direccion Virtual: %0.8p, Tamaño: %d.", datosAMSP->pid, datosAMSP->direccionVirtual, datosAMSP->tamanio);

	socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, sizeof(t_datos_aMSPLectura), datosAMSP);

	socket_recvPaquete(self->socketMSP->socket, paqueteLectura);

	unaLectura = (t_datos_deMSPLectura *) paqueteLectura->data;

	memset(programa, 0, tamanio);
	memcpy(programa, unaLectura->lectura, tamanio);  //en esta linea rompe Para que se usa un programa y como se carga

	estado = unaLectura->estado;

	free(unaLectura);
	return estado;
}

int cpuLeerMemoriaSinKM(t_CPU *self, uint32_t direccionVirtual, char *programa, int tamanio){

	t_datos_aMSPLectura *datosAMSP = malloc(sizeof(t_datos_aMSPLectura));
	t_socket_paquete *paqueteLectura = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_datos_deMSPLectura *unaLectura = (t_datos_deMSPLectura *)malloc(sizeof(t_datos_deMSPLectura));

	int estado;

	datosAMSP->direccionVirtual = direccionVirtual;
	datosAMSP->pid = self->tcb->pid;
	datosAMSP->tamanio = tamanio;

	log_info(self->loggerCPU, "CPU: Solicitud de lectura de memoria para PID: %d, Direccion Virtual: %0.8p, Tamaño: %d.", datosAMSP->pid, datosAMSP->direccionVirtual, datosAMSP->tamanio);

	socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, sizeof(t_datos_aMSPLectura), datosAMSP);

	socket_recvPaquete(self->socketMSP->socket, paqueteLectura);

	unaLectura = (t_datos_deMSPLectura *) paqueteLectura->data;

	memset(programa, 0, tamanio + 1);
	memcpy(programa, unaLectura->lectura, tamanio + 1);  //en esta linea rompe Para que se usa un programa y como se carga

	estado = unaLectura->estado;

	free(unaLectura);
	return estado;
}


