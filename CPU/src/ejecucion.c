/*
 * ejecucion.c
 *
 *  Created on: 26/11/2014
 *      Author: utnso
 */


#include "cpuKernel.h"
#include "cpuMSP.h"
#include "ejecucion.h"
#include "codigoESO.h"


char *instrucciones_eso[] = {"LOAD", "GETM", "SETM", "MOVR", "ADDR", "SUBR", "MULR", "MODR", "DIVR", "INCR", "DECR",
		"COMP", "CGEQ", "CLEQ", "GOTO", "JMPZ", "JPNZ", "INTE", "SHIF", "NOPP", "PUSH", "TAKE", "XXXX", "MALC", "FREE", "INNN",
		"INNC", "OUTN", "OUTC", "CREA", "JOIN", "BLOK", "WAKE"};


int cpuProcesarTCB(t_CPU *self){

	int tamanio = sizeof(char) * 4;
	char *datosDeMSP = malloc(sizeof(tamanio) + 1);
	int estado_ejecucion_instruccion, estado, encontrado, indice, num;
	int salida = 0;
	int error = 0;
	int esJoin = 0;

	//COPIO LA ESTRUCTURA DEL TCB AL hilo_log
	//hilo_log = (t_hilo_log *) self->tcb;
	//ejecucion_hilo(hilo_log, self->quantum);

	log_info(self->loggerCPU, "CPU: Comienzo a procesar el TCB de pid: %d y direccion: %0.8p", self->tcb->pid, self->tcb->puntero_instruccion);

	while(!salida && ((self->quantum > 0) || (self->tcb->km == 1))){
		printTCBCPU(self->tcb);
		encontrado = 0;
		indice = 0;

		if(self->tcb->km == 1)
			log_info(self->loggerCPU, "CPU: ------------------QUANTUM infinito por KM 1------------------");

		else
			log_info(self->loggerCPU, "CPU: ------------------QUANTUM = %d------------------", self->quantum);

		estado = cpuLeerMemoria(self, self->tcb->puntero_instruccion, datosDeMSP, tamanio);

		if ((estado < 0) && (estado != SIN_ERRORES)){
			log_error(self->loggerCPU, "CPU: error al intentar cpuLeerMemoria, con N°: %d", estado);
			estado_ejecucion_instruccion = ERROR_DE_LECTURA_DE_MEMORIA;
		}

		else{

			while ((encontrado == 0) && (indice <= CANTIDAD_INSTRUCCIONES)){

				if(strncmp(instrucciones_eso[indice], datosDeMSP, 4) == 0){
					encontrado = 1;
					estado_ejecucion_instruccion = ejecutar_instruccion(self, indice, &esJoin);
				}

				indice++;
			}

			self->quantum = self->quantum - 1;

			if((self->quantum == 0) && (self->tcb->km == 0) && (estado_ejecucion_instruccion == SIN_ERRORES))
				estado_ejecucion_instruccion = TERMINAR_QUANTUM;
		}

		switch(estado_ejecucion_instruccion){

		case FINALIZAR_PROGRAMA_EXITO:

			//TODO Sacar todos los printf
			if(self->tcb->km == 0 && self->tcb->tid == 0){
				printf("CPU: ENVIA FINALIZAR PROGRAMA EXITO\n");
				num = cpuFinalizarProgramaExitoso(self);
				salida = 1;
			}

			else if(self->tcb->km == 0 && self->tcb->tid != 0){
				printf("CPU: ENVIA FINALIZAR HILO EXITO\n");
				num = cpuFinalizarHiloExitoso(self);
				salida = 1;
			}

			else{
				if(esJoin){
					printf("CPU: ENVIA FINALIZAR INTERRUPCION PARA JOIN\n");
					cpuFinalizarInterrupcion(self, 1);
					esJoin = 0;
				}
				else{
					printf("CPU: ENVIA FINALIZAR INTERRUPCION\n");
					cpuFinalizarInterrupcion(self, 0);

				}
				salida = 1;
			}
			break;

		case TERMINAR_QUANTUM:
			printf("CPU: ENVIA TERMINAR QUANTUM\n");
			salida = 1;
			cpuTerminarQuantum(self);
			break;

		case INTERRUPCION:
			salida = 1;
			break;

		case SIN_ERRORES:
			break;

		default:
			enviarMensajeDeErrorAKernel(self, estado_ejecucion_instruccion);
			salida = 1;
			error = 1;
			//Enviar a kernel para que mande a consola el problema
			break;
		}
	}

	free(datosDeMSP);
	return error;

}


int determinar_registro(char registro){

	switch(registro){
	case 'A': return 0;
	case 'B': return 1;
	case 'C': return 2;
	case 'D': return 3;
	case 'E': return 4;
	case 'M': return 6;
	case 'P': return 7;
	case 'X': return 8;
	case 'S': return 9;
	default: return -1;
	}
}


int ejecutar_instruccion(t_CPU *self, int linea, int *esJoin){
	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

	int estado = 0;
	parametros = list_create();

	self->tcb->puntero_instruccion += 4; //avanzo el puntero de instruccion
	cpuInicializarRegistrosCPU(self, registros_cpu);
	//cambio_registros(registros_cpu);
	free(registros_cpu);

	log_info(self->loggerCPU, "CPU: TCB KM %d", self->tcb->km);
	usleep(self->retardo);

	//-----------------------------------INICIO SWITCH - CASE--------------------------------

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


	case MALC:
		estado = MALC_ESO(self);
		break;

	case FREE:
		estado = FREE_ESO(self);
		break;
	case INNN:
		estado = INNN_ESO(self);
		break;
	case INNC:
		estado = INNC_ESO(self);
		break;
	case OUTN:
		estado = OUTN_ESO(self);
		break;
	case OUTC:
		estado = OUTC_ESO(self);
		break;
	case CREA:
		estado = CREA_ESO(self);
		break;
	case JOIN:
		estado = JOIN_ESO(self);
		*esJoin = 1;
		break;
	case BLOK:
		estado = BLOK_ESO(self);
		break;
	case WAKE:
		estado = WAKE_ESO(self);
		break;
		/***************************************************************************************************\
		 *								--FIN SYSTEMCALL--									 	 *
	\***************************************************************************************************/


	default:
		log_error(self->loggerCPU, "CPU: error en el switch-case, instruccion no encontrada:\n %d", self->tcb->pid);
		printf("CPU: error en el switch-case, instruccion no encontrada:\n %d", self->tcb->pid);
		//free(lectura_en_msp);
		//free(cpu_leer_memoria);
		estado = ERROR_POR_CODIGO_INESPERADO;
		break;
	}

	/****************************\
										  	  FIN SWITCH - CASE
										\****************************/
	return estado;
}
