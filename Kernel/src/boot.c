#include "boot.h"

void hacer_conexion_con_msp(t_kernel* self) {

	self->socketMSP = socket_createClient();

	if (self->socketMSP == NULL ) {
		log_error(self->loggerKernel, "Kernel: Error al crear socket con la MSP!");
	}

	if (!socket_connect( self->socketMSP, self->ipMsp, self->puertoMsp)) {
		log_error(self->loggerKernel, "Kernel: Error al hacer el Boot con la MSP!");
		perror("Kernel: Kernel: Error al hacer el Boot con la MSP!");

	}

	log_info(self->loggerKernel, "Kernel: Conectado con la MSP (IP:%s/Puerto:%d)!", self->ipMsp, self->puertoMsp);

	realizarHandshakeConMSP(self);


}

void realizarHandshakeConMSP(t_kernel* self) {

	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if (socket_sendPaquete(self->socketMSP->socket, CREAR_SEGMENTO, 0, NULL) > 0) {
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
	int pid= 4000;
	int tid= 53;
	crearTCB(self, code,a, pid, tid);
}


t_programaEnKernel* crearTCB(t_kernel* self, char* codigoPrograma, int tamanioEnBytes, int pid, int tid){

	int stack;
	t_programaEnKernel* programaEnElKernel = malloc( sizeof(t_programaEnKernel) );
	log_info(self->loggerKernel, "Kernel: Crear un TCB.");

	//t_medatada_program* metadata = metadata_desde_literal(codigoPrograma);

	programaEnElKernel->PCB.pid=pid;
	programaEnElKernel->PCB.tid= tid;

	//ver porque rompre
	//programaEnElKernel->PCB.puntero_instruccion = metadata->instruccion_inicio;


	programaEnElKernel->PCB.base_segmento_codigo= pedirBaseAMSP(self,pid, codigoPrograma, tamanioEnBytes);

	if(programaEnElKernel->PCB.base_segmento_codigo == -1){
		finalizarProgramaEnPlanificacion(programaEnElKernel);
		return NULL;
	}


	programaEnElKernel->PCB.base_stack = pedirBaseAMSP(self,pid, "", stack);
	if(programaEnElKernel->PCB.base_stack == -1){
		finalizarProgramaEnPlanificacion(programaEnElKernel);
		return NULL;
	}

	programaEnElKernel->PCB.cursor_stack = programaEnElKernel->PCB.base_stack;

	//faltan todos los logs
	log_info(self->loggerKernel, "PID %d TID: %d\n",programaEnElKernel->PCB.pid, programaEnElKernel->PCB.tid);

	return programaEnElKernel;
}


int pedirBaseAMSP(t_kernel* self,int pid, char* buffer, int tamanio){
	int direccionLogica;
	t_envio_num_EnKernel* datos = malloc(sizeof(t_envio_num_EnKernel));
	datos->num = tamanio;
	datos->pid = pid;
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if (socket_sendPaquete(self->socketMSP->socket, BASE_MSP, sizeof(t_envio_num_EnKernel), datos) > 0) {
		log_info(self->loggerKernel, "-Mando Tamaño del Segmento %d para el proceso %d!",datos->num,datos->pid);
		if(socket_recvPaquete(self->socketMSP->socket, paquete) >= 0){
			if(paquete->header.type == BASE){
				datos = (t_envio_num_EnKernel*) paquete->data;
				direccionLogica = datos->num;
				log_info(self->loggerKernel, "RECIBIDOS DATOS: PID=%d / Indice Segmento=%d", datos->pid, direccionLogica);
			}
			if(paquete->header.type == ERROR_EXCEPCION){
				return -1;
			}
		}else{
			return -1;
		}
	}
	if(buffer!=""){
		t_envio_bytes_EnKernel* bufferMsg = malloc(sizeof(t_envio_bytes_EnKernel));
		bufferMsg->base = datos->num;
		bufferMsg->offset = 0;
		bufferMsg->pid = pid;
		bufferMsg->tamanio = tamanio;
		if(socket_sendPaquete(self->socketMSP->socket, BASE_MSP, sizeof(t_envio_bytes_EnKernel), bufferMsg) > 0){
			if(socket_sendPaquete(self->socketMSP->socket, BASE, tamanio, buffer) > 0){
				if(socket_recvPaquete(self->socketMSP->socket, paquete) >= 0){
					if(paquete->header.type == ERROR_EXCEPCION){
						log_info(self->loggerKernel, "Kernel: Error");
					}
				}
			}else{
				return -1;
			}
		}
	}
	free(paquete);
	free(datos);
	return direccionLogica;
}
