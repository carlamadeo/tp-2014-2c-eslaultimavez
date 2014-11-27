/*
 * ejecucion.c
 *
 *  Created on: 26/11/2014
 *      Author: utnso
 */


#include "cpuKernel.h"
#include "ejecucion.h"
#include "codigoESO.h"

t_list* parametros;

char *instrucciones_eso[] = {"LOAD", "GETM", "SETM", "MOVR", "ADDR", "SUBR", "MULR", "MODR", "DIVR", "INCR", "DECR",
		"COMP", "CGEQ", "CLEQ", "GOTO", "JMPZ", "JPNZ", "INTE", "SHIF", "NOPP", "PUSH", "TAKE", "XXXX", "MALC", "FREE", "INNN",
		"INNC", "OUTN", "OUTC", "CREA", "JOIN", "BLOK", "WAKE"};

int cpuProcesarTCB(t_CPU *self){

	int estado;
	int tamanio = sizeof(char) * 4;
	char *datosDeMSP = malloc(sizeof(tamanio) + 1);
	int terminarLinea;

	//ejecucion_hilo(hilo_log, self->quantum);
	log_info(self->loggerCPU, "CPU: Comienzo a procesar el TCB de pid: %d", self->tcb->pid);

	while(self->quantum > 0){

		estado = cpuLeerMemoria(self, self->tcb->puntero_instruccion, datosDeMSP, tamanio);

		//estado puede ser SIN_ERRORES o ERROR_POR_SEGMENTATION_FAULT
		//TODO Ver manejo de errores con el Kernel!!!

		int encontrado = 0;
		int indice = 0;
		while (!encontrado && indice <= CANTIDAD_INSTRUCCIONES){

			if(strncmp(instrucciones_eso[indice], datosDeMSP, 4) == 0){

				encontrado = 1;
				int estado_ejecucion_instruccion = ejecutar_instruccion(self, indice);
				/*
				switch(estado_ejecucion_instruccion){
				case SIN_ERRORES:
					log_info(self->loggerCPU, "CPU: instruccion ejecutada SIN_ERRORES");
					break;
				case ERROR_POR_EJECUCION_ILICITA:
					return ERROR_POR_EJECUCION_ILICITA;
				case MENSAJE_DE_ERROR:
					return MENSAJE_DE_ERROR;
				default:
					return ERROR_POR_CODIGO_INESPERADO;
				}*/
			}

			indice++;
		}

		usleep(100);

		terminarLinea = cpuEnviaTermineUnaLinea(self);

		switch(terminarLinea){

		case CPU_SEGUI_EJECUTANDO:
			log_info(self->loggerCPU, "CPU: recibe un CPU_SEGUI_EJECUTANDO");
			break;

		case KERNEL_FIN_TCB_QUANTUM:
			cpuCambioContexto(self);
			return SIN_ERRORES;

		case ERROR_POR_DESCONEXION_DE_CONSOLA:
			log_error(self->loggerCPU, "CPU: Recibe un ERROR_POR_DESCONEXION_DE_CONSOLA.");
			return ERROR_POR_DESCONEXION_DE_CONSOLA;

		default:
			log_error(self->loggerCPU, "CPU: Recibe un codigo inesperado al mandar un CPU_TERMINE_UNA_LINEA.");
			return MENSAJE_DE_ERROR;
		}

		self->quantum = self->quantum - 1;
	}

	return estado;
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
}


int ejecutar_instruccion(t_CPU *self, int linea){


	int estado_bloque;
	int estado = 0;
	parametros = list_create();

	self->tcb->puntero_instruccion += 4; //avanzo el puntero de instruccion

	log_info(self->loggerCPU, "CPU: Se ejecutara la instruccion %s", instrucciones_eso[linea]);

									/****************************\
									 	 INICIO SWITCH - CASE
									\****************************/
	switch(linea){

	case LOAD:
		estado = LOAD_ESO(self);
		break;

	case GETM:
		estado = GETM_ESO(self);
		break;

	case SETM:
		estado = SETM_ESO(self);
		break;

	case MOVR:
		estado = MOVR_ESO(self);
		break;

	case ADDR:
		estado = ADDR_ESO(self);
		break;

	case SUBR:
		estado = SUBR_ESO(self);
		break;

	case MULR:
		estado = MULR_ESO(self);
		break;

	case MODR:
		estado = MODR_ESO(self);
		break;

	case DIVR:
		estado = DIVR_ESO(self);
		break;

	case INCR:
		estado = INCR_ESO(self);
		break;

	case DECR:
		estado = DECR_ESO(self);
		break;

	case COMP:
		estado = COMP_ESO(self);
		break;

	case CGEQ:
		estado = CGEQ_ESO(self);
		break;

	case CLEQ:
		estado = CLEQ_ESO(self);
		break;

	case GOTO:
		estado = GOTO_ESO(self);
		break;

	case JMPZ:
		estado = JMPZ_ESO(self);
		break;

	case JPNZ:
		estado = JPNZ_ESO(self);
		break;

	case INTE:
		estado = INTE_ESO(self);
		break;

	case SHIF:
		estado = SHIF_ESO(self);
		break;

	case NOPP:
		estado = NOPP_ESO(self);
		break;

	case PUSH:
		estado = PUSH_ESO(self);
		break;

	case TAKE:
		estado = TAKE_ESO(self);
		break;

	case XXXX:
		estado = XXXX_ESO(self);
		break;

		/***************************************************************************************************\
		 *								--Comienzo SYSTEMCALL--									 	 *
		\***************************************************************************************************/

/*
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
		break;*/

		/***************************************************************************************************\
		 *								--FIN SYSTEMCALL--									 	 *
	\***************************************************************************************************/


	default:
		log_error(self->loggerCPU, "CPU: error en el switch-case, instruccion no encontrada:\n %d", self->tcb->pid);
		printf("CPU: error en el switch-case, instruccion no encontrada:\n %d", self->tcb->pid);
		//free(lectura_en_msp);
		//free(cpu_leer_memoria);
		usleep(100);
		estado_bloque = ERROR_POR_CODIGO_INESPERADO;
		break;
	}

										/****************************\
										  	  FIN SWITCH - CASE
										\****************************/

	usleep(100);
	return estado_bloque;
}

