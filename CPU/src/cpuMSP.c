
#include "cpuMSP.h"


t_socket_client* cpuConectarConMPS(t_CPU* self) {

	self->socketMSP = socket_createClient();

	if (self->socketMSP  == NULL)
		log_error(logger, "CPU: Error al crear socket con la MSP!");

	if (socket_connect(self->socketMSP, self->ipMsp, self->puertoMSP) == 0)
		log_error(self->loggerCPU, "CPU: Error al hacer el connect con la MSP!");

	else{
		log_info(self->loggerCPU, "CPU: Conectado con la MSP (IP: %s/Puerto: %d)!", self->ipMsp, self->puertoMSP);
		cpuRealizarHandshakeConMSP(self);
	}

	return self->socketMSP;
}

void cpuRealizarHandshakeConMSP(t_CPU* self) {

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

}

int cpuCrearSegmento(t_CPU *self, int pid, int tamanio){

	t_datos_aMSP* datosAEnviar = malloc(sizeof(t_datos_aMSP));
	t_datos_deMSP *datosRecibidos = malloc(sizeof(t_datos_deMSP));
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	datosAEnviar->pid = pid;
	datosAEnviar->tamanio = tamanio;

	if (socket_sendPaquete(self->socketMSP->socket, CREAR_SEGMENTO, sizeof(t_datos_aMSP), datosAEnviar) > 0) {
		log_info(self->loggerCPU, "CPU: Solicitud de creacion de segmento de Tamaño %d para el Proceso con PID %d.", datosAEnviar->tamanio, datosAEnviar->pid);

		if(socket_recvPaquete(self->socketMSP->socket, paquete) >= 0){

			if(paquete->header.type == CREAR_SEGMENTO){
				datosRecibidos = (t_datos_deMSP *) (paquete->data);
				log_info(self->loggerCPU, "CPU: Se recibio de la MSP la direccion base %0.8p ", datosRecibidos->direccionBase);

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
			log_info(self->loggerCPU, "CPU: Error al recibir los datos de creacion de segmento");
			return -1;
		}
	}

	free(datosAEnviar);
	return datosRecibidos->direccionBase;
}


int cpuEscribirMemoria(t_CPU* self, int pid, uint32_t direccionVirtual, char *programa, int tamanio, t_socket* socketNuevoCliente){

	t_escribirSegmentoBeso* escrituraDeCodigo = malloc(sizeof(t_escribirSegmentoBeso));
	t_socket_paquete *paqueteConfirmacionEscritura = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_confirmacionEscritura *unaConfirmacionEscritura = (t_confirmacionEscritura *)malloc(sizeof(t_confirmacionEscritura));

	escrituraDeCodigo->direccionVirtual = direccionVirtual;
	escrituraDeCodigo->pid = pid;
	escrituraDeCodigo->tamanio = tamanio;
	strcpy(escrituraDeCodigo->bufferCodigoBeso, programa);

	log_info(self->loggerCPU, "CPU: Solicitud de escritura de %s en memoria para PID: %d, Direccion Virtual: %0.8p, Tamaño: %d.", escrituraDeCodigo->bufferCodigoBeso, escrituraDeCodigo->pid, escrituraDeCodigo->direccionVirtual, escrituraDeCodigo->tamanio);

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

int cpuLeerMemoria(t_CPU* self, int pid, uint32_t direccionVirtual, char *programa, int tamanio, t_socket* socketNuevoCliente){

	t_datos_aMSPLectura *datosAMSP = malloc(sizeof(t_datos_aMSPLectura));
	t_socket_paquete *paqueteLectura = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_datos_deMSPLectura *unaLectura = (t_datos_deMSPLectura *)malloc(sizeof(t_datos_deMSPLectura));

	datosAMSP->direccionVirtual = direccionVirtual;
	datosAMSP->pid = pid;
	datosAMSP->tamanio = tamanio;

	log_info(self->loggerCPU, "CPU: Solicitud de lectura de memoria para PID: %d, Direccion Virtual: %0.8p, Tamaño: %d.", datosAMSP->pid, datosAMSP->direccionVirtual, datosAMSP->tamanio);

	socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, sizeof(t_datos_aMSPLectura), datosAMSP);

	socket_recvPaquete(socketNuevoCliente, paqueteLectura);

	unaLectura = (t_datos_deMSPLectura *) paqueteLectura->data;
	strcpy(programa, unaLectura->lectura);

	if (unaLectura->estado == ERROR_POR_SEGMENTATION_FAULT){
		//Pasarle el error al kernel
	}

	return unaLectura->estado;
}
