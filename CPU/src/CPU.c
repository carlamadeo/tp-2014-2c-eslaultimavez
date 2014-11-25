/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

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

	self->socketMSP = cpuConectarConMPS(self);
	self->socketPlanificador = conectarCPUConKernel(self);

	// Creacion del Log del cpu
	char *nombreLog = malloc(strlen("logCPU_.log") + sizeof(int) + 1);
	sprintf(nombreLog, "%s%d%s", "logCPU_", getpid(), ".log");
	self->loggerCPU = log_create(nombreLog, "CPU", 1, LOG_LEVEL_DEBUG); //Creo el archivo Log



	while(1){

		t_registros_cpu* registros_cpu=malloc(sizeof(t_registros_cpu));

		t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if(socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0){

			if(paquete->header.type == TCB_NUEVO){
				t_TCB_nuevo* nuevo_tcb= (t_TCB_nuevo *)paquete->data;
				self->tcb = nuevo_tcb->tcb;
				self->quantum = nuevo_tcb->quantum;
				registros_cpu->I = (uint32_t)nuevo_tcb->tcb->pid;
				registros_cpu->K = (uint32_t)nuevo_tcb->tcb->km;
				registros_cpu->M = nuevo_tcb->tcb->base_segmento_codigo;
				registros_cpu->P = nuevo_tcb->tcb->puntero_instruccion;
				registros_cpu->S = nuevo_tcb->tcb->cursor_stack;
				registros_cpu->X = nuevo_tcb->tcb->base_stack;
				int i;
				for(i=0; i<=4; i++){
					registros_cpu->registros_programacion[i]=nuevo_tcb->tcb->registro_de_programacion[i];
				}
				hilo_log = (t_hilo_log *) nuevo_tcb->tcb;
				comienzo_ejecucion(hilo_log, self->quantum);
				//manda a ejecutar el tcb
				int estado_ejecucion = cpuProcesar_tcb(self);
				//controlo el estado de la ejecucion
				switch(estado_ejecucion){
				case SIN_ERRORES:
					log_info(self->loggerCPU, "CPU: finalizo ejecucion de TID: %d", self->tcb->tid);
					break;
				case MENSAJE_DE_ERROR:
					log_error(self->loggerCPU, "CPU: finalizacion erronea de TID: %d", self->tcb->tid);
					break;
				}

			}else{
				log_error(self->loggerCPU, "Se recibio un codigo inesperado de KERNEL en main de cpu: %d", paquete->header.type);
				}
		}else {
			log_info(self->loggerCPU, "CPU: Error al recibir un paquete.");
				}

		free(paquete);
		free(registros_cpu);
		usleep(100);
	}//Fin del while 1


	/* De aca en adelante, se tiene que eliminar porque se desconecto la CPU */

	log_info(self->loggerCPU, "Se desconecto la CPU. Elimino todo");
	return EXIT_SUCCESS;

	close(self->socketPlanificador->socket->descriptor);
	close(self->socketMSP->socket->descriptor);

	destruirConfiguracionCPU(self, configCPU);
}


int cpuProcesar_tcb(t_CPU* self){
	ejecucion_hilo(hilo_log, self->quantum);
	log_info(self->loggerCPU, "CPU: Comienzo a procesar el TCB de pid: %d", self->tcb->pid);

	while(1){
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

		int estado_lectura = cpuLeerMemoria(self, unCPU_LEER_MEMORIA->direccionVirtual, lecturaDeMSP->data, unCPU_LEER_MEMORIA->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			return MENSAJE_DE_ERROR;
		}

		//Esto es para darse cuenta de que instruccion se trata
		int encontrado = 0;
		int indice = 0;
		while (!encontrado && indice < CANTIDAD_INSTRUCCIONES){
			if(strncmp(instrucciones_eso[indice], lecturaDeMSP->data, 4) == 0){
				int estado_ejecucion_instruccion = ejecutar_instruccion(indice, self);
				switch(estado_ejecucion_instruccion){
				case SIN_ERRORES:
					log_error(self->loggerCPU, "CPU: instruccion ejecutada SIN_ERRORES");
					break;
				case ERROR_POR_EJECUCION_ILICITA:
					return ERROR_POR_EJECUCION_ILICITA;
				case MENSAJE_DE_ERROR:
					return MENSAJE_DE_ERROR;
				default:
					return ERROR_POR_CODIGO_INESPERADO;
				}
				encontrado = 1;
			}
			indice++;
		}

		if(!encontrado){
			log_error(self->loggerCPU, "CPU: Recibio un codigo inesperado de la MSP");
			return ERROR_CODIGO_INESPERADO;
		}


		free(lecturaDeMSP);
		free(unCPU_LEER_MEMORIA);
		usleep(100);

		t_CPU_TERMINE_UNA_LINEA* terminoLinea = malloc(sizeof(t_CPU_TERMINE_UNA_LINEA));
		terminoLinea->pid = self->tcb->pid;
		terminoLinea->tid = self->tcb->tid;

		log_info(self->loggerCPU, "CPU: Envia al Planificador CPU_TERMINE_UNA_LINEA.");

		if (socket_sendPaquete(self->socketPlanificador->socket, CPU_TERMINE_UNA_LINEA, sizeof(t_CPU_TERMINE_UNA_LINEA), terminoLinea) <= 0){
			log_info(self->loggerCPU, "CPU: fallo envio de CPU_TERMINE_UNA_LINEA, PID: %d", self->tcb->pid);
		}else{

			t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
			if(socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0){

				switch(paquete->header.type){

				case CPU_SEGUI_EJECUTANDO:
					break;
				case KERNEL_FIN_TCB_QUANTUM:
					cambioContexto(self);
					return SIN_ERRORES;
				case ERROR_POR_DESCONEXION_DE_CONSOLA:
					log_info(self->loggerCPU, "CPU: Recibe un ERROR_POR_DESCONEXION_DE_CONSOLA.");
					return ERROR_POR_DESCONEXION_DE_CONSOLA;
				default:
					log_info(self->loggerCPU, "CPU: Recibe un codigo inesperado al mandar un CPU_TERMINE_UNA_LINEA.");
					return MENSAJE_DE_ERROR;
				}
			}else{
				log_info(self->loggerCPU, "CPU: Error al esperar un paquete del planificador.");
				return MENSAJE_DE_ERROR;
			}
		}
		free(terminoLinea);
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


int ejecutar_instruccion(int linea, t_CPU* self){

	t_CPU_LEER_MEMORIA* cpu_leer_memoria = malloc(sizeof(t_CPU_LEER_MEMORIA)); //pid+puntero_instruccion+tamanio_parametros
	t_lectura_MSP * lectura_en_msp = malloc(sizeof(t_lectura_MSP));
	int estado_lectura = 0;
	cpu_leer_memoria->pid = self->tcb->pid;
	self->tcb->puntero_instruccion += 4; //avanzo el puntero de instruccion
	cpu_leer_memoria->direccionVirtual = self->tcb->puntero_instruccion;

	log_info(self->loggerCPU, "CPU: Se ejecutara la instruccion %s", instrucciones_eso[linea]);
	t_list* parametros = (t_list *)list_create();
	char registro, registroA, registroB;
	int numero;
	int regA, regB, reg;
	uint32_t direccion;
	int estado_bloque;
									/****************************\
									 	 INICIO SWITCH - CASE
									\****************************/

	switch(linea){

	case LOAD:
		cpu_leer_memoria->tamanio = 5;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
				if (!(estado_lectura==SIN_ERRORES)){
					estado_bloque = MENSAJE_DE_ERROR;
					break;
				} else {

					log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion LOAD" );
					memcpy(&(registro), lectura_en_msp->data, sizeof(char));
					memcpy(&(numero), (lectura_en_msp->data) + sizeof(char),sizeof(int));
					reg = determinar_registro(registro);
					list_add(parametros, (void *)registro);
					list_add(parametros, (void *)numero);
					ejecucion_instruccion(linea, (void *)parametros);
					LOAD_ESO(reg, numero, self->tcb);
					estado_bloque = SIN_ERRORES;
					break;
				}

	case GETM:
		cpu_leer_memoria->tamanio = 2;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{


				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion GETM");
				memcpy(&(registroA), lectura_en_msp->data, sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				regA = determinar_registro(registroA);
				regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				ejecucion_instruccion(linea, parametros);
				GETM_ESO(regA,regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}

	case SETM:

		cpu_leer_memoria->tamanio = 6;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		} else {

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SETM");
				memcpy(&(numero), lectura_en_msp->data, sizeof(int));
				memcpy(&(registroA), (lectura_en_msp->data)+ sizeof(int), sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data)+ sizeof(int)+ sizeof(char),sizeof(char));
				regA = determinar_registro(registroA);
				regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				list_add(parametros, (void *)numero);
				ejecucion_instruccion(linea, parametros);
				SETM_ESO(numero,regA,regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
				}
	case MOVR:

		cpu_leer_memoria->tamanio = 2;
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MOVR");
				memcpy(&(registroA), (lectura_en_msp->data), sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				regA = determinar_registro(registroA);
				regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				ejecucion_instruccion(linea, parametros);
				MOVR_ESO(regA, regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case ADDR:

		cpu_leer_memoria->tamanio = 2;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion ADDR");
				memcpy(&(registroA), (lectura_en_msp->data), sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				regA = determinar_registro(registroA);
				regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				ejecucion_instruccion(linea, parametros);
				ADDR_ESO(regA, regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case SUBR:

		cpu_leer_memoria->tamanio = 2;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SUBR");
				memcpy(&(registroA), (lectura_en_msp->data), sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				regA = determinar_registro(registroA);
				regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				ejecucion_instruccion(linea, parametros);
				SUBR_ESO(regA, regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case MULR:

		cpu_leer_memoria->tamanio = 2;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		} else {

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MULR");
				memcpy(&(registroA), (lectura_en_msp->data), sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				regA = determinar_registro(registroA);
				regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				ejecucion_instruccion(linea, parametros);
				MULR_ESO(regA, regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case MODR:

		cpu_leer_memoria->tamanio = 2;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		} else {
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MODR");
				memcpy(&(registroA), (lectura_en_msp->data), sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				regA = determinar_registro(registroA);
				regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				ejecucion_instruccion(linea, parametros);
				MODR_ESO(regA, regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case DIVR:

		cpu_leer_memoria->tamanio = 2;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion DIVR");
				memcpy(&(registroA), (lectura_en_msp->data), sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				ejecucion_instruccion(linea, parametros);
				DIVR_ESO(regA, regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case INCR:

		cpu_leer_memoria->tamanio = 1;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "recibiendo parametros de instruccion INCR" );
				memcpy(&(registro), (lectura_en_msp->data), sizeof(char));
				reg = determinar_registro(registro);
				list_add(parametros, (void *)registro);
				ejecucion_instruccion(linea, parametros);
				INCR_ESO(reg,self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case DECR:

		cpu_leer_memoria->tamanio = 1;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "recibiendo parametros de instruccion DECR" );
				memcpy(&(registro), (lectura_en_msp->data), sizeof(char));
				reg = determinar_registro(registro);
				list_add(parametros, (void *)registro);
				ejecucion_instruccion(linea, parametros);
				DECR_ESO(reg,self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case COMP:

		cpu_leer_memoria->tamanio = 2;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion COMP");
				memcpy(&(registroA), (lectura_en_msp->data), sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				regA = determinar_registro(registroA);
				regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				ejecucion_instruccion(linea, parametros);
				COMP_ESO(regA,regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}

	case CGEQ:

		cpu_leer_memoria->tamanio = 2;
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion CGEQ");
				memcpy(&(registroA), (lectura_en_msp->data), sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				regA = determinar_registro(registroA);
				regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				ejecucion_instruccion(linea, parametros);
				CGEQ_ESO(regA,regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case CLEQ:

		cpu_leer_memoria->tamanio = 2;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion CLEQ");
				memcpy(&(registroA), (lectura_en_msp->data), sizeof(char));
				memcpy(&(registroB), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				regA = determinar_registro(registroA);
				regB = determinar_registro(registroB);
				list_add(parametros, (void *)registroA);
				list_add(parametros, (void *)registroB);
				ejecucion_instruccion(linea, parametros);
				CLEQ_ESO(regA,regB, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case GOTO:

		cpu_leer_memoria->tamanio = 1;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "recibiendo parametros de instruccion GOTO" );
				memcpy(&(registro), (lectura_en_msp->data), sizeof(char));
				reg = determinar_registro(registro);
				list_add(parametros, (void *)registro);
				ejecucion_instruccion(linea, parametros);
				GOTO_ESO(reg,self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case JMPZ:

		cpu_leer_memoria->tamanio = 4;
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion JMPZ" );
				memcpy(&(direccion), lectura_en_msp->data, sizeof(uint32_t));
				list_add(parametros, (void *)direccion);
				ejecucion_instruccion(linea, parametros);
				JMPZ_ESO(direccion, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case JPNZ:

		cpu_leer_memoria->tamanio = 4;
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion JPNZ" );
				memcpy(&(direccion), lectura_en_msp->data, sizeof(uint32_t));
				list_add(parametros, (void *)direccion);
				ejecucion_instruccion(linea, parametros);
				JPNZ_ESO(direccion, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}

	case INTE:

		cpu_leer_memoria->tamanio = 4;
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion INTE" );
				memcpy(&(direccion), lectura_en_msp->data, sizeof(uint32_t));
				list_add(parametros, (void *)direccion);
				ejecucion_instruccion(linea, parametros);
				INTE_ESO(direccion, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case SHIF:

		cpu_leer_memoria->tamanio = 5;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SHIF" );
				memcpy(&(numero), lectura_en_msp->data, sizeof(int32_t));
				memcpy(&(registro), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				reg = determinar_registro(registro);
				list_add(parametros, (void *)numero);
				list_add(parametros, (void *)registro);
				ejecucion_instruccion(linea, parametros);
				SHIF_ESO(numero, reg, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case NOPP: break;

	case PUSH:

		cpu_leer_memoria->tamanio = 5;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion PUSH" );
				memcpy(&(numero), lectura_en_msp->data, sizeof(int32_t));
				memcpy(&(registro), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				reg = determinar_registro(registro);
				list_add(parametros, (void *)numero);
				list_add(parametros, (void *)registro);
				ejecucion_instruccion(linea, parametros);
				PUSH_ESO(numero, reg, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case TAKE:

		cpu_leer_memoria->tamanio = 5;
		estado_lectura = cpuLeerMemoria(self, cpu_leer_memoria->direccionVirtual, lectura_en_msp->data,cpu_leer_memoria->tamanio);
		if (!(estado_lectura==SIN_ERRORES)){
			estado_bloque = MENSAJE_DE_ERROR;
			break;
		}else{
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion TAKE" );
				memcpy(&(numero), lectura_en_msp->data, sizeof(int32_t));
				memcpy(&(registro), (lectura_en_msp->data) + sizeof(char),sizeof(char));
				reg = determinar_registro(registro);
				list_add(parametros, (void *)numero);
				list_add(parametros, (void *)registro);
				ejecucion_instruccion(linea, parametros);
				TAKE_ESO(numero, reg, self->tcb);
				estado_bloque = SIN_ERRORES;
				break;
		}
	case XXXX:
		fin_ejecucion();
		XXXX_ESO(self->tcb);
		estado_bloque = SIN_ERRORES;
		break;

		/***************************************************************************************************\
		 *								--Comienzo SYSTEMCALL--									 	 *
		\***************************************************************************************************/


	case MALC:
		if(self->tcb->km==1){
			log_info(self->loggerCPU, "CPU: ejecutando instruccion MALC" );
			estado_bloque = MALC_ESO(self->tcb);
		}else{
			estado_bloque = ERROR_POR_EJECUCION_ILICITA;
			break;
		}
		break;

	case FREE:
		if(self->tcb->km==1){
			estado_bloque = FREE_ESO(self->tcb);
		}else{
			estado_bloque = ERROR_POR_EJECUCION_ILICITA;
			break;
		}
		break;

	case INNN:
		if(self->tcb->km==1){
			estado_bloque = INNN_ESO(self->tcb);
		}else{
			estado_bloque = ERROR_POR_EJECUCION_ILICITA;
			break;
		}
		break;
	case INNC:
		if(self->tcb->km==1){
			estado_bloque = INNC_ESO(self->tcb);
		}else{
			estado_bloque = ERROR_POR_EJECUCION_ILICITA;
			break;
		}
		break;
	case OUTN:
		if(self->tcb->km==1){
			estado_bloque = OUTN_ESO(self->tcb);
		}else{
			estado_bloque = ERROR_POR_EJECUCION_ILICITA;
			break;
		}
		break;

	case OUTC:
		if(self->tcb->km==1){
			estado_bloque = OUTC_ESO(self->tcb);
		}else{
			estado_bloque = ERROR_POR_EJECUCION_ILICITA;
			break;
		}
		break;

	case CREA:

		if(self->tcb->km==1){
			estado_bloque = CREA_ESO(self->tcb);
		}else{
			estado_bloque = ERROR_POR_EJECUCION_ILICITA;
			break;
		}
		break;

	case JOIN:
		if(self->tcb->km==1){
			estado_bloque = JOIN_ESO(self->tcb);
		}else{
			estado_bloque = ERROR_POR_EJECUCION_ILICITA;
			break;
		}
		break;

	case BLOK:

		if(self->tcb->km==1){
			estado_bloque = BLOK_ESO(self->tcb);
		}else{
			estado_bloque = ERROR_POR_EJECUCION_ILICITA;
			break;
		}
		break;

	/***************************************************************************************************\
	 *								--FIN SYSTEMCALL--									 	 *
	\***************************************************************************************************/


	default:
		log_error(self->loggerCPU, "CPU: error en el switch-case, instruccion no encontrada:\n %d", self->tcb->pid);
		printf("CPU: error en el switch-case, instruccion no encontrada:\n %d", self->tcb->pid);
		free(lectura_en_msp);
		free(cpu_leer_memoria);
		usleep(100);
		estado_bloque = ERROR_POR_CODIGO_INESPERADO;
		break;
	}

	free(lectura_en_msp);
	free(cpu_leer_memoria);
	usleep(100);

	return estado_bloque;
}

									/****************************\
									  	  FIN SWITCH - CASE
									\****************************/


void verificar_argumentosCPU(int argc, char* argv[]){
	if( argc != 2 ){
		printf("Modo de empleo: ./CPU cpuConfig.cfg\n");
		perror("CPU no recibio las configuraciones");
		exit (EXIT_FAILURE);
	}
}
