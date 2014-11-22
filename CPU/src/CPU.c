/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "CPU.h"
#include "cpuMSP.h"
#include "cpuKernel.h"
#include "cpuConfig.h"
#include <stdlib.h>
#include "commons/panel.h"
#include "commons/cpu.h"

char *instrucciones_eso[] = {"LOAD", "GETM", "SETM", "MOVR", "ADDR", "SUBR", "MULR", "MODR", "DIVR", "INCR", "DECR",
		"COMP", "CGEQ", "CLEQ", "GOTO", "JMPZ", "JPNZ", "INTE", "SHIF", "NOPP", "PUSH", "TAKE", "XXXX", "MALC", "FREE", "INNN",
		"INNC", "OUTN", "OUTC", "CREA", "JOIN", "BLOK", "WAKE"};

int main(int argc, char** argv) {

	verificar_argumentosCPU(argc, argv);
	t_CPU *self = malloc(sizeof(t_CPU));
	t_config *configCPU;

	if(!cargarConfiguracionCPU(argv[1], self, configCPU)){
		printf("Archivo de configuracion invalido\n");
		return EXIT_SUCCESS;
	}

	self->loggerCPU = log_create("logCPU.log", "CPU", 1, LOG_LEVEL_DEBUG);
	self->socketMSP = cpuConectarConMPS(self);
	self->socketPlanificador = conectarCPUConKernel(self);

	char *nombreLog = malloc(strlen("logCPU_.log") + sizeof(int) + 1);

	sprintf(nombreLog, "%s%d%s", "logCPU_", getpid(), ".log");

	self->loggerCPU = log_create(nombreLog, "CPU", 1, LOG_LEVEL_DEBUG); //Creo el archivo Log



	while(1){
		t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if(socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0){

			if(paquete->header.type == TCB_NUEVO){
				t_TCB_nuevo* nuevo_tcb= (t_TCB_nuevo*)paquete->data;
				self->tcb = nuevo_tcb->tcb;
				self->quantum = nuevo_tcb->quantum;
				t_hilo_log* hilo_log = (t_hilo_log *) nuevo_tcb->tcb;
				comienzo_ejecucion(hilo_log, self->quantum);
				int estado_ejecucion = cpuProcesar_tcb(self);
				switch(estado_ejecucion){
				case SIN_ERRORES:
					//loguear
					break;
				case MENSAJE_DE_ERROR:
					//loguear
					// hacer el cambio de contexto con mensaje de error
					break;
				}

			}
			else
				log_error(self->loggerCPU, "Se recibio un codigo inesperado de KERNEL en main de cpu: %d", paquete->header.type);
		}

		else {
			log_info(self->loggerCPU, "CPU: Error al recibir un paquete.");
			exit(-1);
		}

		free(paquete);
		usleep(100);
	}//Fin del while 1



	/* De aca en adelante, se tiene que eliminar porque se desconecto la CPU */

	log_info(self->loggerCPU, "Se desconecto la CPU. Elimino todo");
	exit(0);

	close(self->socketPlanificador->socket->descriptor);
	close(self->socketMSP->socket->descriptor);

	destruirConfiguracionCPU(self, configCPU);
}


int cpuProcesar_tcb(t_CPU* self){

	log_info(self->loggerCPU, "CPU: Comienzo a procesar el TCB de pid: %d", self->tcb->pid);

	/*pregunto por KM del tcb a ejecutar*/
	int seguir_ejecucion = 1;

	while(seguir_ejecucion){
		//pido los primeros 4 bytes especificados por PID+Puntero_Instruccion
		t_lectura_MSP * lecturaDeMSP = malloc(sizeof(t_lectura_MSP));
		t_CPU_LEER_MEMORIA* unCPU_LEER_MEMORIA = malloc(sizeof(t_CPU_LEER_MEMORIA));
		//se hace el control para saber a donde apuntar dependiendo de si se trata un tcb usuario o kernel...
		int pid_proceso = 0;
		if (self->tcb->km == 0){
			pid_proceso = self->tcb->pid;
		}
		unCPU_LEER_MEMORIA->pid = pid_proceso;
		unCPU_LEER_MEMORIA->tamanio = sizeof(char)*4;
		unCPU_LEER_MEMORIA->direccionVirtual = self->tcb->puntero_instruccion;

		int estado_lectura = cpuLeerMemoria(self, unCPU_LEER_MEMORIA->pid, unCPU_LEER_MEMORIA->direccionVirtual, lecturaDeMSP->data, unCPU_LEER_MEMORIA->tamanio, self->socketMSP->socket);
		if (!(estado_lectura==SIN_ERRORES)){
			return MENSAJE_DE_ERROR;
		}

		//logueo de lectura sin errores

		//Esto es para darse cuenta de que instruccion se trata
		int encontrado = 0;
		int indice = 0;
		while (!encontrado && indice < CANTIDAD_INSTRUCCIONES){
			if(strncmp(instrucciones_eso[indice], lecturaDeMSP->data, 4) == 0){
				ejecutar_instruccion(indice, self);
				encontrado = 1;
			}
			indice++;
		}

		if(!encontrado){
			log_error(self->loggerCPU, "CPU: Recibio un codigo inesperado de la MSP");
			//Aca tenemos que salir de la funcion porque se recibio cualquier cosa de la MSP!!
		}

		//Sleep para que no se tilde
		usleep(100);

		t_CPU_TERMINE_UNA_LINEA* unTerminoLinea = malloc(sizeof(t_CPU_TERMINE_UNA_LINEA));
		unTerminoLinea->pid = self->tcb->pid;
		unTerminoLinea->tid = self->tcb->tid;

		log_info(self->loggerCPU, "CPU: Envia al Planificador CPU_TERMINE_UNA_LINEA");

		if (socket_sendPaquete(self->socketPlanificador->socket, CPU_TERMINE_UNA_LINEA, sizeof(t_CPU_TERMINE_UNA_LINEA), unTerminoLinea) <= 0)
			log_info(self->loggerCPU, "CPU: Fallo pedido de datos de Consola %d", self->tcb->pid);

		else{
			t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

			if(socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0){

				switch(paquete->header.type){

				case CPU_SEGUI_EJECUTANDO:
					break;
				case KERNEL_FIN_TCB_QUANTUM:
					cambioContexto(self);
					seguir_ejecucion = 0;
					break;
				case ERROR_POR_DESCONEXION_DE_CONSOLA:
					log_info(self->loggerCPU, "CPU: Recibe un ERROR_POR_DESCONEXION_DE_CONSOLA");
					return MENSAJE_DE_ERROR;
				default:
					log_info(self->loggerCPU, "CPU: Recibe un inesperado al mandar un CPU_TERMINE_UNA_LINEA");
					return MENSAJE_DE_ERROR;
				}
			}

			else{
				log_info(self->loggerCPU, "CPU: Error al esperar un paquete");
				return MENSAJE_DE_ERROR;
			}
		}

	}//fin while(1)

	return SIN_ERRORES;
}

void cambioContexto(t_CPU* self){

	//antes de hacer el send, deberia actualizarce el STACK
	//el stack sufre modificaciones cuando se ejecutan las instrucciones ESO, no se actualiza, lo que se actualiza es el TCB.
	if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0)
		log_error(self->loggerCPU, "CPU: Falló cambio de conexto");

	else
		log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO");

}

int determinar_registro(char registro){

	switch(registro){
	case 'A': return 0;
	case 'B': return 1;
	case 'C': return 2;
	case 'D': return 3;
	case 'E': return 4;
	case 'F': return 5;
	default: return -1;
	}

} //fin while procesar TCB


void ejecutar_instruccion(int linea, t_CPU* self){

	t_CPU_LEER_MEMORIA* unCPU_LEER_MEMORIA = malloc(sizeof(t_CPU_LEER_MEMORIA)); //pid+puntero_instruccion+tamanio_parametros
	t_socket_paquete *paquete_MSP = malloc(sizeof(t_socket_paquete));

	unCPU_LEER_MEMORIA->pid = self->tcb->pid;
	self->tcb->puntero_instruccion += 4; //avanzo el puntero de instruccion
	unCPU_LEER_MEMORIA->direccionVirtual = self->tcb->puntero_instruccion;

	log_info(self->loggerCPU, "Se ejecutara la instruccion %s", instrucciones_eso[linea]);
	t_list* parametros = create_list();

	switch(linea){
	case LOAD:

		unCPU_LEER_MEMORIA->tamanio = 5;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA) <= 0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if(paquete_MSP->header.type == LEER_MEMORIA){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion LOAD" );
				char registro;
				int32_t numero;
				memcpy(&(registro), paquete_MSP->data, sizeof(char));
				memcpy(&(numero), (paquete_MSP->data) + sizeof(char),sizeof(int32_t));
				int reg = determinar_registro(registro);
				list_add(parametros, registro);
				list_add(parametros, numero);
				char* nom_instruccion = "LOAD";
				ejecucion_instruccion(nom_instruccion, parametros);
				LOAD_ESO(reg, numero, self->tcb);

			}
		}

		break;

	case GETM:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA) <= 0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if(paquete_MSP->header.type == LEER_MEMORIA){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion GETM");
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				GETM_ESO(regA,regB, self->tcb);

			}
		}

		break;

	case SETM:

		unCPU_LEER_MEMORIA->tamanio = 6;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if( paquete_MSP->header.type == LEER_MEMORIA ){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SETM");
				char registroA, registroB;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int));
				memcpy(&(registroA), (paquete_MSP->data)+ sizeof(int), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(int)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				SETM_ESO(numero,regA,regB, self->tcb);
			}
		}

		break;

	case MOVR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA) <= 0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if( paquete_MSP->header.type == LEER_MEMORIA ){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MOVR");
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				MOVR_ESO(regA, regB, self->tcb);
			}
		}

		break;

	case ADDR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA) <= 0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d",self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if(paquete_MSP->header.type == LEER_MEMORIA){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion ADDR");
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				ADDR_ESO(regA, regB, self->tcb);
			}
		}

		break;

	case SUBR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA) <= 0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SUBR" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				SUBR_ESO(regA, regB, self->tcb);

			}
		}


		break;

	case MULR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if( paquete_MSP->header.type == LEER_MEMORIA ){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MULR" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				MULR_ESO(regA, regB, self->tcb);
			}
		}

		break;

	case MODR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MODR" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				MODR_ESO(regA, regB, self->tcb);
			}
		}

		break;
	case DIVR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion DIVR" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				DIVR_ESO(regA, regB, self->tcb);
			}
		}

		break;

	case INCR:

		unCPU_LEER_MEMORIA->tamanio = 1;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d",self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion INCR" );
				char registro;
				memcpy(&(registro), (paquete_MSP->data), sizeof(char));
				int reg = determinar_registro(registro);
				INCR_ESO(reg,self->tcb);
			}
		}

		break;

	case DECR:

		unCPU_LEER_MEMORIA->tamanio = 1;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion DECR" );
				char registro;
				memcpy(&(registro), (paquete_MSP->data), sizeof(char));
				int reg = determinar_registro(registro);
				DECR_ESO(reg,self->tcb);
			}
		}

		break;

	case COMP:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d",self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion COMP" );
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				COMP_ESO(regA,regB, self->tcb);
			}
		}

		break;

	case CGEQ:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion CGEQ" );
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				CGEQ_ESO(regA,regB, self->tcb);
			}
		}

		break;

	case CLEQ:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion CLEQ" );
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				CLEQ_ESO(regA,regB, self->tcb);
			}
		}

		break;
	case GOTO:

		unCPU_LEER_MEMORIA->tamanio = 1;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion GOTO" );
				char registro;
				memcpy(&(registro), paquete_MSP->data, sizeof(char));
				int reg = determinar_registro(registro);
				GOTO_ESO(reg,self->tcb);
			}
		}

		break;

	case JMPZ:

		unCPU_LEER_MEMORIA->tamanio = 4;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion JMPZ" );
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				//GETM_ESO(numero, self->tcb);
			}
		}

		break;

	case JPNZ:

		unCPU_LEER_MEMORIA->tamanio = 4;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion JPNZ" );
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				//GETM_ESO(numero, self->tcb);
			}
		}

		break;

	case INTE:

		unCPU_LEER_MEMORIA->tamanio = 4;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion INTE" );
				uint32_t direccion;
				memcpy(&(direccion), paquete_MSP->data, sizeof(uint32_t));
				INTE_ESO(direccion, self->tcb);
			}
		}

		break;

	case SHIF:

		unCPU_LEER_MEMORIA->tamanio = 5;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SHIF" );
				char registro;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				memcpy(&(registro), (paquete_MSP->data) + sizeof(int32_t),sizeof(char));
				int reg = determinar_registro(registro);
				SHIF_ESO(numero, reg, self->tcb);
			}
		}

		break;

	case NOPP: break;

	case PUSH:

		unCPU_LEER_MEMORIA->tamanio = 5;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion PUSH" );
				char registro;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				memcpy(&(registro), (paquete_MSP->data) + sizeof(int32_t),sizeof(char));
				int reg = determinar_registro(registro);
				PUSH_ESO(numero, reg, self->tcb);
			}
		}

		break;

	case TAKE:

		unCPU_LEER_MEMORIA->tamanio = 5;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion TAKE" );
				char registro;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				memcpy(&(registro), (paquete_MSP->data) + sizeof(int32_t),sizeof(char));
				int reg = determinar_registro(registro);
				TAKE_ESO(numero, reg, self->tcb);
			}
		}

		break;
	case XXXX:
		XXXX_ESO(self->tcb);
		fin_ejecucion();
		break;
	case MALC:
		if(self->tcb->km==1){
			MALC_ESO(self->tcb);
		}else{
			log_error(self->loggerCPU, "CPU: ejecucion ilicita de PID:\n %d", self->tcb->pid);
			if (socket_sendPaquete(self->socketPlanificador->socket,MENSAJE_DE_ERROR,sizeof(int),&(self->tcb->pid))<=0){
				log_error(self->loggerCPU, "CPU: fallo: MENSAJE_DE_ERROR\n");
			}else{
				if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0){
					log_error(self->loggerCPU, "CPU: fallo: CAMBIO_DE_CONTEXTO\n");
				}else{
					log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO\n");
				}
			}
		}
		break;

	case FREE:
		if(self->tcb->km==1){
			FREE_ESO(self->tcb);
		}else{
			log_error(self->loggerCPU, "CPU: ejecucion ilicita de PID:\n %d", self->tcb->pid);
			if (socket_sendPaquete(self->socketPlanificador->socket,MENSAJE_DE_ERROR,sizeof(int),&(self->tcb->pid))<=0){
				log_error(self->loggerCPU, "CPU: fallo: MENSAJE_DE_ERROR\n");
			}else{
				if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0){
					log_error(self->loggerCPU, "CPU: fallo: CAMBIO_DE_CONTEXTO\n");
				}else{
					log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO\n");
				}
			}
		}
		break;
	case INNN:
		if(self->tcb->km==1){
			INNN_ESO(self->tcb);
		}else{
			log_error(self->loggerCPU, "CPU: ejecucion ilicita de PID:\n %d", self->tcb->pid);
			if (socket_sendPaquete(self->socketPlanificador->socket,MENSAJE_DE_ERROR,sizeof(int),&(self->tcb->pid))<=0){
				log_error(self->loggerCPU, "CPU: fallo: MENSAJE_DE_ERROR\n");
			}else{
				if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0){
					log_error(self->loggerCPU, "CPU: fallo: CAMBIO_DE_CONTEXTO\n");
				}else{
					log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO\n");
				}
			}
		}

		break;
	case INNC:
		if(self->tcb->km==1){
			INNC_ESO(self->tcb);
		}else{
			log_error(self->loggerCPU, "CPU: ejecucion ilicita de PID:\n %d", self->tcb->pid);
			if (socket_sendPaquete(self->socketPlanificador->socket,MENSAJE_DE_ERROR,sizeof(int),&(self->tcb->pid))<=0){
				log_error(self->loggerCPU, "CPU: fallo: MENSAJE_DE_ERROR\n");
			}else{
				if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0){
					log_error(self->loggerCPU, "CPU: fallo: CAMBIO_DE_CONTEXTO\n");
				}else{
					log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO\n");
				}
			}
		}

		break;
	case OUTN:
		if(self->tcb->km==1){
			OUTN_ESO(self->tcb);
		}else{
			log_error(self->loggerCPU, "CPU: ejecucion ilicita de PID:\n %d", self->tcb->pid);
			if (socket_sendPaquete(self->socketPlanificador->socket,MENSAJE_DE_ERROR,sizeof(int),&(self->tcb->pid))<=0){
				log_error(self->loggerCPU, "CPU: fallo: MENSAJE_DE_ERROR\n");
			}else{
				if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0){
					log_error(self->loggerCPU, "CPU: fallo: CAMBIO_DE_CONTEXTO\n");
				}else{
					log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO\n");
				}
			}
		}

		break;

	case OUTC:
		if(self->tcb->km==1){
			OUTC_ESO(self->tcb);
		}else{
			log_error(self->loggerCPU, "CPU: ejecucion ilicita de PID:\n %d", self->tcb->pid);
			if (socket_sendPaquete(self->socketPlanificador->socket,MENSAJE_DE_ERROR,sizeof(int),&(self->tcb->pid))<=0){
				log_error(self->loggerCPU, "CPU: fallo: MENSAJE_DE_ERROR\n");
			}else{
				if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0){
					log_error(self->loggerCPU, "CPU: fallo: CAMBIO_DE_CONTEXTO\n");
				}else{
					log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO\n");
				}
			}
		}

		break;

	case CREA:

		if(self->tcb->km==1){
			CREA_ESO(self->tcb);
		}else{
			log_error(self->loggerCPU, "CPU: ejecucion ilicita de PID:\n %d", self->tcb->pid);
			if (socket_sendPaquete(self->socketPlanificador->socket,MENSAJE_DE_ERROR,sizeof(int),&(self->tcb->pid))<=0){
				log_error(self->loggerCPU, "CPU: fallo: MENSAJE_DE_ERROR\n");
			}else{
				if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0){
					log_error(self->loggerCPU, "CPU: fallo: CAMBIO_DE_CONTEXTO\n");
				}else{
					log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO\n");
				}
			}
		}

		break;

	case JOIN:
		if(self->tcb->km==1){
			JOIN_ESO(self->tcb);
		}else{
			log_error(self->loggerCPU, "CPU: ejecucion ilicita de PID:\n %d", self->tcb->pid);
			if (socket_sendPaquete(self->socketPlanificador->socket,MENSAJE_DE_ERROR,sizeof(int),&(self->tcb->pid))<=0){
				log_error(self->loggerCPU, "CPU: fallo: MENSAJE_DE_ERROR\n");
			}else{
				if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0){
					log_error(self->loggerCPU, "CPU: fallo: CAMBIO_DE_CONTEXTO\n");
				}else{
					log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO\n");
				}
			}
		}

		break;

	case BLOK:

		if(self->tcb->km==1){
			BLOK_ESO(self->tcb);
		}else{
			log_error(self->loggerCPU, "CPU: ejecucion ilicita de PID:\n %d", self->tcb->pid);
			if (socket_sendPaquete(self->socketPlanificador->socket,MENSAJE_DE_ERROR,sizeof(int),&(self->tcb->pid))<=0){
				log_error(self->loggerCPU, "CPU: fallo: MENSAJE_DE_ERROR\n");
			}else{
				if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0){
					log_error(self->loggerCPU, "CPU: fallo: CAMBIO_DE_CONTEXTO\n");
				}else{
					log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO\n");
				}
			}
		}


		break;

	default:
		log_error(self->loggerCPU, "CPU: error en el switch-case, instruccion no encontrada:\n %d", self->tcb->pid);
		printf("CPU: error en el switch-case, instruccion no encontrada:\n %d", self->tcb->pid);
		break;

		free(unCPU_LEER_MEMORIA);
		free(paquete_MSP);
		usleep(100);
	}

}


void verificar_argumentosCPU(int argc, char* argv[]){
	if( argc != 2 ){
		printf("Modo de empleo: ./CPU cpuConfig.cfg\n");
		perror("CPU no recibio las configuraciones");
		exit (EXIT_FAILURE);
	}
}
