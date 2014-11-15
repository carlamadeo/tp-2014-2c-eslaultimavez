#include "boot.h"

void hacer_conexion_con_msp(t_kernel* self) {

	self->socketMSP = socket_createClient();

	if (self->socketMSP == NULL )
		log_error(self->loggerKernel, "Kernel: Error al crear socket con la MSP!");

	if (socket_connect(self->socketMSP, self->ipMsp, self->puertoMsp) == 0)
		log_error(self->loggerKernel, "Kernel: Error al hacer el Boot con la MSP!");

	else{
		log_info(self->loggerKernel, "Kernel: Conectado con la MSP (IP:%s/Puerto:%d)!", self->ipMsp, self->puertoMsp);
		realizarHandshakeConMSP(self);
	}

}

void realizarHandshakeConMSP(t_kernel* self) {

	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if (socket_sendPaquete(self->socketMSP->socket, HANDSHAKE_KERNEL, 0, NULL) > 0)
		log_info(self->loggerKernel, "Kernel se presenta a la MSP!");


	if (socket_recvPaquete(self->socketMSP->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_MSP)
			log_info(self->loggerKernel, "Kernel: Se realizaron los Handshake con la MSP correctamente");
	}

	else
		log_error(self->loggerKernel, "Kernel: Error al recibir los datos de la MSP!");


	char* code;
	int a = 34;
	int pid = 1;
	int tid = 53;
	crearTCBKERNEL(self, code, a, pid, tid);
}


void crearTCBKERNEL(t_kernel* self, char* codigoPrograma, int tamanioEnBytes, int pid, int tid){

	uint32_t stack;
	t_programaEnKernel* programaEnElKernel = malloc( sizeof(t_programaEnKernel) );
	log_info(self->loggerKernel, "Kernel: Crear un TCB.");

	//t_medatada_program* metadata = metadata_desde_literal(codigoPrograma);

	programaEnElKernel->TCB.pid = pid;
	programaEnElKernel->TCB.tid = tid;

	//ver porque rompre
	//programaEnElKernel->PCB.puntero_instruccion = metadata->instruccion_inicio;


	programaEnElKernel->TCB.base_segmento_codigo = kernelCrearSegmento(self, pid, tamanioEnBytes); //beso


	if(programaEnElKernel->TCB.base_segmento_codigo == -1){
		finalizarProgramaEnPlanificacion(programaEnElKernel);
		//return NULL;
	}


	programaEnElKernel->TCB.base_stack = kernelCrearSegmento(self, pid, self->tamanioStack);
	if(programaEnElKernel->TCB.base_stack == -1){
		finalizarProgramaEnPlanificacion(programaEnElKernel);
		//return NULL;
	}

	programaEnElKernel->TCB.cursor_stack = programaEnElKernel->TCB.base_stack;

	//faltan todos los logs
	log_info(self->loggerKernel, "PID %d TID: %d\n",programaEnElKernel->TCB.pid, programaEnElKernel->TCB.tid);

	//return programaEnElKernel;
}


int kernelCrearSegmento(t_kernel* self, int pid, int tamanio){

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
				log_info(self->loggerKernel, "Kernel: Datos recibidos MSP %0.8p ", datosRecibidos->direccionBase);

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
	free(datosRecibidos);
	return datosRecibidos->direccionBase;
}
