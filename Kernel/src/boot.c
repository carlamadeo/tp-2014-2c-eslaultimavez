#include "boot.h"

void hacer_conexion_con_msp(t_kernel* self) {

	self->socketMSP = socket_createClient();

	if (self->socketMSP == NULL ) {
		log_error(self->loggerKernel, "Kernel: Error al crear socket con la MSP!");
	}

	if (socket_connect( self->socketMSP, self->ipMsp, self->puertoMsp)==0) {
		log_error(self->loggerKernel, "Kernel: Error al hacer el Boot con la MSP!");
	}else{
		log_info(self->loggerKernel, "Kernel: Conectado con la MSP (IP:%s/Puerto:%d)!", self->ipMsp, self->puertoMsp);
	}



	realizarHandshakeConMSP(self);


}

void realizarHandshakeConMSP(t_kernel* self) {

	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	//log_info(self->loggerKernel, "MATEE...");

	if (socket_sendPaquete(self->socketMSP->socket, HANDSHAKE_KERNEL, 0, NULL) > 0) {
		log_info(self->loggerKernel, "KERNEL se presenta a la MSP!");
	}

	if (socket_recvPaquete(self->socketMSP->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_MSP){
			log_info(self->loggerKernel,"Se realizaron los HANDSHAKE con la MSP correctamente");
		}
	} else {
		log_error(self->loggerKernel, "Error al recibir los datos de la MSP!");
	}

	char* code;
	int a= 34;
	int pid= 1;
	int tid= 53;
	crearTCB(self, code,a, pid, tid);
}


t_programaEnKernel* crearTCB(t_kernel* self, char* codigoPrograma, int tamanioEnBytes, int pid, int tid){

	uint32_t stack;
	t_programaEnKernel* programaEnElKernel = malloc( sizeof(t_programaEnKernel) );
	log_info(self->loggerKernel, "Kernel: Crear un TCB.");

	//t_medatada_program* metadata = metadata_desde_literal(codigoPrograma);

	programaEnElKernel->TCB.pid=pid;
	programaEnElKernel->TCB.tid= tid;

	//ver porque rompre
	//programaEnElKernel->PCB.puntero_instruccion = metadata->instruccion_inicio;


	programaEnElKernel->TCB.base_segmento_codigo= kernelCrearSegmento(self,pid, tamanioEnBytes); //beso


	if(programaEnElKernel->TCB.base_segmento_codigo == -1){
		finalizarProgramaEnPlanificacion(programaEnElKernel);
		return NULL;
	}


	programaEnElKernel->TCB.base_stack = kernelCrearSegmento(self,pid, self->tamanioStack);
	if(programaEnElKernel->TCB.base_stack == -1){
		finalizarProgramaEnPlanificacion(programaEnElKernel);
		return NULL;
	}

	programaEnElKernel->TCB.cursor_stack = programaEnElKernel->TCB.base_stack;

	//faltan todos los logs
	log_info(self->loggerKernel, "PID %d TID: %d\n",programaEnElKernel->TCB.pid, programaEnElKernel->TCB.tid);

	return programaEnElKernel;
}


int kernelCrearSegmento(t_kernel* self,int pid, int tamanio){

	int direccionLogica;
	t_envio_num_EnKernel* datos = malloc(sizeof(t_envio_num_EnKernel));
	datos->num = tamanio;
	datos->pid = pid;
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if (socket_sendPaquete(self->socketMSP->socket, CREAR_SEGMENTO, sizeof(t_envio_num_EnKernel), datos) > 0) {
		log_info(self->loggerKernel, "Kernel: Mando Tamaño del Segmento %d para el proceso %d!",datos->num,datos->pid);
		printf("TOY aca");
		if(socket_recvPaquete(self->socketMSP->socket, paquete) >= 0){
			if(paquete->header.type == CREAR_SEGMENTO){
				datos = (t_envio_num_EnKernel*) paquete->data;
				direccionLogica = datos->num;
				//log_info(self->loggerKernel, "RECIBIDOS DATOS: PID=%d / Indice Segmento=%d", datos->pid, direccionLogica);
				log_info(self->loggerKernel, "RECIBIDOS DATOS");
			}
		}else{
			log_info(self->loggerKernel, "KERNEL: ERROR DATOS NO RECIBIDOS");
			return -1;
		}
	}

	free(paquete);
	free(datos);
	return direccionLogica;
}
