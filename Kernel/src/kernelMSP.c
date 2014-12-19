#include "kernelMSP.h"
#include "commons/protocolStructInBigBang.h"

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
		log_info(self->loggerKernel, "Kernel: Envia HANDSHAKE_KERNEL ");

	if (socket_recvPaquete(self->socketMSP->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_MSP)
			log_info(self->loggerKernel, "Kernel: Se realizaron los Handshake con la MSP correctamente");
	}

	else
		log_error(self->loggerKernel, "Kernel: Error al recibir los datos de la MSP!");

	socket_freePaquete(paquete);
}


uint32_t kernelCrearSegmento(t_kernel *self, int pid, int tamanio){

	t_datos_aMSP* datosAEnviar = malloc(sizeof(t_datos_aMSP));
	t_datos_deMSP *datosRecibidos = malloc(sizeof(t_datos_deMSP));
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	uint32_t estado;

	datosAEnviar->pid = pid;
	datosAEnviar->tamanio = tamanio;

	if (socket_sendPaquete(self->socketMSP->socket, CREAR_SEGMENTO, sizeof(t_datos_aMSP), datosAEnviar) > 0) {
		log_info(self->loggerKernel, "Kernel: Solicitud de creacion de segmento de Tamaño %d para el Proceso con PID %d.", datosAEnviar->tamanio, datosAEnviar->pid);

		if(socket_recvPaquete(self->socketMSP->socket, paquete) >= 0){

			if(paquete->header.type == CREAR_SEGMENTO){
				datosRecibidos = (t_datos_deMSP *) (paquete->data);
				estado = datosRecibidos->direccionBase;

				if(estado >= 0)
					log_info(self->loggerKernel, "Kernel: Se recibio de la MSP la direccion base %0.8p ", estado);
			}
		}

		else{
			log_error(self->loggerKernel, "Kernel: Error al recibir los datos de creacion de segmento");
			return -1; //TODO ver que error mandar al no recibir un paquete
		}
	}

	socket_freePaquete(paquete);
	free(datosAEnviar);
	free(datosRecibidos);
	return estado;
}


int kernelDestruirSegmento(t_kernel *self, t_TCB_Kernel *tcb, uint32_t direccionVirtual){

	t_destruirSegmento* destruir_segmento = malloc(sizeof(t_destruirSegmento));
	t_socket_paquete *paqueteConfirmacionDestruccionSegmento = malloc(sizeof(t_socket_paquete));
	t_confirmacion* confirmacion = malloc(sizeof(t_confirmacion));
	int estado;

	destruir_segmento->pid = tcb->pid;
	destruir_segmento->direccionVirtual = direccionVirtual;

	socket_sendPaquete(self->socketMSP->socket, DESTRUIR_SEGMENTO, sizeof(t_destruirSegmento), destruir_segmento);

	socket_recvPaquete(self->socketMSP->socket, paqueteConfirmacionDestruccionSegmento);

	confirmacion = (t_confirmacion *) paqueteConfirmacionDestruccionSegmento->data;

	estado = confirmacion->estado;

	if(estado == SIN_ERRORES)
		log_info(self->loggerKernel, "Kernel: Se destruyo el segmento con base virtual %0.8p correctamente", direccionVirtual);

	free(destruir_segmento);
	free(paqueteConfirmacionDestruccionSegmento);
	free(confirmacion);

	return estado;
}

int kernelLeerMemoria(t_kernel *self, int pid, uint32_t direccionVirtual, char *programa, int tamanio){

	t_datos_aMSPLectura *datosAMSP = malloc(sizeof(t_datos_aMSPLectura));
	t_socket_paquete *paqueteLectura = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_datos_deMSPLectura *unaLectura = (t_datos_deMSPLectura *)malloc(sizeof(t_datos_deMSPLectura));
	int estado;

	datosAMSP->direccionVirtual = direccionVirtual;
	datosAMSP->pid = pid;
	datosAMSP->tamanio = tamanio;

	log_info(self->loggerKernel, "Kernel: Solicitud de lectura de memoria para PID: %d, Direccion Virtual: %0.8p, Tamaño: %d.", datosAMSP->pid, datosAMSP->direccionVirtual, datosAMSP->tamanio);

	socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, sizeof(t_datos_aMSPLectura), datosAMSP);

	socket_recvPaquete(self->socketMSP->socket, paqueteLectura);

	unaLectura = (t_datos_deMSPLectura *) paqueteLectura->data;

	memset(programa, 0, tamanio);
	memcpy(programa, unaLectura->lectura, tamanio);  //en esta linea rompe Para que se usa un programa y como se carga

	estado = unaLectura->estado;

	free(datosAMSP);
	free(paqueteLectura);
	free(unaLectura);
	return estado;
}


int kernelEscribirMemoria(t_kernel* self, int pid, uint32_t direccionVirtual, char *programaBeso, int tamanioBeso){

	t_escribirSegmentoBeso* escrituraDeCodigo = malloc(sizeof(t_escribirSegmentoBeso));
	t_socket_paquete *paqueteConfirmacionEscritura = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_confirmacionEscritura *unaConfirmacionEscritura = (t_confirmacionEscritura *)malloc(sizeof(t_confirmacionEscritura));
	int estado;

	escrituraDeCodigo->direccionVirtual = direccionVirtual;
	escrituraDeCodigo->pid = pid;
	escrituraDeCodigo->tamanio = tamanioBeso;
	memmove(escrituraDeCodigo->bufferCodigoBeso, programaBeso, tamanioBeso);

	log_info(self->loggerKernel, "Kernel: Solicitud de escritura de %s en memoria para PID: %d, Direccion Virtual: %0.8p, Tamaño: %d.", escrituraDeCodigo->bufferCodigoBeso, escrituraDeCodigo->pid, escrituraDeCodigo->direccionVirtual, escrituraDeCodigo->tamanio);

	socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA, sizeof(t_escribirSegmentoBeso), escrituraDeCodigo);

	socket_recvPaquete(self->socketMSP->socket, paqueteConfirmacionEscritura);

	unaConfirmacionEscritura = (t_confirmacionEscritura *) paqueteConfirmacionEscritura->data;

	estado = unaConfirmacionEscritura->estado;

	if(unaConfirmacionEscritura->estado == SIN_ERRORES)
		log_info(self->loggerKernel, "Kernel: Se ha escrito en memoria correctamente");

	free(escrituraDeCodigo);
	free(paqueteConfirmacionEscritura);
	free(unaConfirmacionEscritura);
	return estado;
}
