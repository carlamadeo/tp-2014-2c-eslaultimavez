#include "cpuKernel.h"
#include "CPU_Proceso.h"
#include <stdlib.h>


void cpuConectarConKernel(t_CPU *self){

	log_info(self->loggerCPU, "CPU: Conectando con el Planificador...");

	self->socketPlanificador = socket_createClient();

	if (self->socketPlanificador  == NULL )
		log_error(self->loggerCPU, "Error al crear socket del Proceso CPU!");

	if (socket_connect(self->socketPlanificador, self->ipPlanificador, self->puertoPlanificador) == 0)
		log_error(self->loggerCPU, "CPU: Error al hacer el connect con el Planificador!");

	else
		cpuRealizarHandshakeConKernel(self);

}


void cpuRealizarHandshakeConKernel(t_CPU *self){

	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	log_info(self->loggerCPU, "La CPU solicita HANDSHAKE con KERNEL.");

	if (socket_sendPaquete(self->socketPlanificador->socket, HANDSHAKE_CPU, 0, NULL) > 0)
		log_info(self->loggerCPU, "CPU le envia HANDSHAKE al KERNEL!");

	if (socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0) {

		if(paquete->header.type == HANDSHAKE_PLANIFICADOR)
			log_info(self->loggerCPU, "CPU: Recibe del Planificador HANDSHAKE_PLANIFICADOR con descriptor %d", self->socketPlanificador->socket->descriptor);

		else
			log_error(self->loggerCPU, "CPU: Error al recibir HANDSHAKE_PLANIFICADOR del Planificador");
	}

	else
		log_error(self->loggerCPU, "CPU: Error al recibir paquete del Planificador.");

	free(paquete);
}


int cpuRecibirTCB(t_CPU *self){

	t_socket_paquete *paquetePlanificadorTCB = (t_socket_paquete*) malloc(sizeof(t_socket_paquete));
	t_TCB_CPU* unTCBNuevo = (t_TCB_CPU*) malloc(sizeof(t_TCB_CPU));

	if(socket_recvPaquete(self->socketPlanificador->socket, paquetePlanificadorTCB) >= 0){

		//printf("TCB_NUEVO %d \n", paquetePlanificadorTCB->header.type);   //no Borrar sirve para como debug Jorge
		if(paquetePlanificadorTCB->header.type == TCB_NUEVO){

			unTCBNuevo = (t_TCB_CPU*) paquetePlanificadorTCB->data;
			self->tcb = unTCBNuevo;
			printTCBCPU(self->tcb);
			//log_debug(self->loggerCPU, "CPU: recibio un TCB_NUEVO con PID: %d TID:%d KM:%d", self->tcb->pid, self->tcb->tid, self->tcb->km);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error al recibir de planificador TCB_NUEVO");
			return 0;
		}
	}

	else{
		log_error(self->loggerCPU, "CPU: Error al recibir un paquete del planificador");
		return 0;
	}

	free(paquetePlanificadorTCB);
	return 1;
}


int cpuRecibirQuantum(t_CPU *self){

	t_socket_paquete *paquetePlanificadorQuantum = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	//t_quantumCPU* unQuantum =  (t_quantumCPU*) malloc(sizeof(t_quantumCPU));

	if(socket_recvPaquete(self->socketPlanificador->socket, paquetePlanificadorQuantum) >= 0){

		if(paquetePlanificadorQuantum->header.type == QUANTUM){
			t_quantumCPU* unQuantum = (t_quantumCPU*) paquetePlanificadorQuantum->data;
			self->quantum = unQuantum->quantumCPU;

			log_debug(self->loggerCPU, "CPU: Recibe un quantum: %d",self->quantum);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error al recibir un quantum");
			return 0;
		}
	}

	else{
		log_error(self->loggerCPU, "CPU: Error al recibir un paquete del planificador");
		return 0;
	}

	free(paquetePlanificadorQuantum);
	return 1;
}


void cpuTerminarQuantum(t_CPU *self){

	//antes de hacer el send, deberia actualizarce el STACK
	//el stack sufre modificaciones cuando se ejecutan las instrucciones ESO, no se actualiza, lo que se actualiza es el TCB.
	if (socket_sendPaquete(self->socketPlanificador->socket, TERMINAR_QUANTUM, sizeof(t_TCB_CPU), self->tcb) <= 0)
		log_error(self->loggerCPU, "CPU: Falló envio TERMINAR_QUANTUM");

	else
		log_info(self->loggerCPU, "CPU: Envia al Planificador: TERMINAR_QUANTUM");
}


int cpuEnviaInterrupcion(t_CPU *self){

	t_interrupcion *interrupcion = malloc(sizeof(t_interrupcion));

	//Horrible, pero sino no anda
	interrupcion->pid = self->tcb->pid;
	interrupcion->tid = self->tcb->tid;
	interrupcion->km = self->tcb->km;
	interrupcion->base_segmento_codigo = self->tcb->base_segmento_codigo;
	interrupcion->tamanio_segmento_codigo = self->tcb->tamanio_segmento_codigo;
	interrupcion->tamanio_segmento_codigo = self->tcb->tamanio_segmento_codigo;
	interrupcion->puntero_instruccion = self->tcb->puntero_instruccion;
	interrupcion->base_stack = self->tcb->base_stack;
	interrupcion->cursor_stack = self->tcb->cursor_stack;
	interrupcion->registro_de_programacion[0] = self->tcb->registro_de_programacion[0];
	interrupcion->registro_de_programacion[1] = self->tcb->registro_de_programacion[1];
	interrupcion->registro_de_programacion[2] = self->tcb->registro_de_programacion[2];
	interrupcion->registro_de_programacion[3] = self->tcb->registro_de_programacion[3];
	interrupcion->registro_de_programacion[4] = self->tcb->registro_de_programacion[4];
	interrupcion->direccionKM = self->unaDireccion;

	if (socket_sendPaquete(self->socketPlanificador->socket, INTERRUPCION, sizeof(t_interrupcion), interrupcion) > 0)
		log_info(self->loggerCPU, "CPU: Envia un TCB y una Direccion %0.8p en una Interrupcion.", interrupcion->direccionKM);

	else{
		log_error(self->loggerCPU, "CPU: Error al enviar datos de interrupcion al Kernel");
		return MENSAJE_DE_ERROR;
	}

	return SIN_ERRORES;

}


int cpuFinalizarProgramaExitoso(t_CPU *self){

	if (socket_sendPaquete(self->socketPlanificador->socket, FINALIZAR_PROGRAMA_EXITO, sizeof(t_TCB_CPU), self->tcb) <= 0){
		log_error(self->loggerCPU, "CPU: Error de finalizacion de proceso %d", self->tcb->pid);
		return MENSAJE_DE_ERROR;
	}

	else
		log_info(self->loggerCPU, "CPU: Finalizacion de proceso correctamente PID = %d", self->tcb->pid);

	return SIN_ERRORES;
}


int cpuFinalizarInterrupcion(t_CPU *self){

	socket_sendPaquete(self->socketPlanificador->socket, TERMINAR_INTERRUPCION, sizeof(t_TCB_CPU), self->tcb);

	return SIN_ERRORES;
}


int cpuSolicitarEntradaEstandar(t_CPU *self, int tamanio, int tipo){

	t_entrada_estandar *envioEntradaEstandar = malloc(sizeof(t_entrada_estandar));

	envioEntradaEstandar->pid = self->tcb->pid;
	envioEntradaEstandar->tamanio = tamanio;
	envioEntradaEstandar->tipo = tipo;    //JORGE ESTO ESTA MAL!!!!!!!!!!!!!!!!!!!!!!

	if (socket_sendPaquete(self->socketPlanificador->socket, ENTRADA_ESTANDAR, sizeof(t_entrada_estandar), envioEntradaEstandar) <= 0){  //22 corresponde a interrupcion
		log_error(self->loggerCPU, "CPU: Ha ocurrido un error al solicitar una entrada estandar al Kernel para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		return MENSAJE_DE_ERROR;
	}

	else
		log_info(self->loggerCPU, "CPU: La solicitud de entrada estandar al Kernel para PID %d y TID: %d se realizo correctamente", self->tcb->pid, self->tcb->tid);

	free(envioEntradaEstandar);
	return SIN_ERRORES;
}


int reciboEntradaEstandarINT(t_CPU *self, int *recibido){

	t_entrada_numeroCPU *datosRecibidos = malloc(sizeof(t_entrada_numeroCPU));
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if(socket_recvPaquete(self->socketPlanificador->socket, paquete) > 0){

		if(paquete->header.type == ENTRADA_ESTANDAR_INT){
			datosRecibidos = (t_entrada_numeroCPU *) (paquete->data);
			recibido = datosRecibidos->numero;
		}

		else {
			printf("SE RECIBIO INT: %d\n", paquete->header.type);
			log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado al recibir una entrada estandar INT del Kernel para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			return ERROR_POR_CODIGO_INESPERADO;
		}
	}

	else
		log_error(self->loggerCPU, "CPU: Ha ocurrido un error al recibir una entrada estandar INT del Kernel para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

	socket_freePaquete(paquete);
	free(datosRecibidos);
	return SIN_ERRORES;
}


int reciboEntradaEstandarCHAR(t_CPU *self, char *recibido, int tamanio){

	t_entrada_charCPU *datosRecibidos = malloc(sizeof(t_entrada_charCPU));
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if(socket_recvPaquete(self->socketPlanificador->socket, paquete) > 0){

		if(paquete->header.type == ENTRADA_ESTANDAR_TEXT){
			datosRecibidos = (t_entrada_charCPU *) (paquete->data);
			memset(recibido, 0, 10); //TODO ver bien esto!!!
			memcpy(recibido, datosRecibidos->entradaEstandar, tamanio + 1);
		}

		else {
			log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado al recibir una entrada estandar TEXT del Kernel para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			return ERROR_POR_CODIGO_INESPERADO;
		}
	}

	else
		log_info(self->loggerCPU, "CPU: Ha ocurrido un error al recibir una entrada estandar TEXT del Kernel para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

	socket_freePaquete(paquete);
	free(datosRecibidos);
	return SIN_ERRORES;
}


int cpuEnviarSalidaEstandar(t_CPU *self, char *salidaEstandar, int tamanio){

	t_salida_estandar *salida_estandar = malloc(sizeof(t_salida_estandar));
	salida_estandar->pid = self->tcb->pid;
	memset(salida_estandar->cadena, 0, tamanio);
	memcpy(salida_estandar->cadena, salidaEstandar, tamanio + 1);

	if (socket_sendPaquete(self->socketPlanificador->socket, SALIDA_ESTANDAR, sizeof(t_salida_estandar), salida_estandar) <= 0){
		log_error(self->loggerCPU, "CPU: Ha ocurrido un error al enviar una salida estandar al Kernel para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		return MENSAJE_DE_ERROR;
	}

	free(salida_estandar);
	return SIN_ERRORES;
}

void enviarMensajeDeErrorAKernel(t_CPU *self, int codigoError){
	t_error *error = malloc(sizeof(t_error));
	error->pid = self->tcb->pid;
	error->tid = self->tcb->tid;
	error->identificadorError = codigoError;

	if (socket_sendPaquete(self->socketPlanificador->socket, MENSAJE_DE_ERROR, sizeof(t_error), error) >= 0)
		log_error(self->loggerCPU, "CPU: Se ha enviado un mensaje de error al Kernel para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

}
