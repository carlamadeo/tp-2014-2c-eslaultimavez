/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "CPU_Proceso.h"
#include "codigoESO.h"
#include "cpuMSP.h"
#include "cpuKernel.h"
#include "cpuConfig.h"
#include "ejecucion.h"
#include <sys/types.h>
#include <stdlib.h>


int main(int argc, char** argv) {

	verificar_argumentosCPU(argc, argv);
	t_CPU *self = malloc(sizeof(t_CPU));
	t_ServiciosAlPlanificador* serviciosAlPlanificador = malloc(sizeof(t_ServiciosAlPlanificador));

	int valorCPU;
	t_config *configCPU;

	char *nombreLog = malloc(strlen("logCPU_.log") + sizeof(int) + 1);
	sprintf(nombreLog, "%s%d%s", "logCPU_", getpid(), ".log");
	self->loggerCPU = log_create(nombreLog, "CPU", 1, LOG_LEVEL_DEBUG); //Creo el archivo Log
	free(nombreLog);

	if(!cargarConfiguracionCPU(self, argv[1], configCPU)){
		printf("Archivo de configuracion invalido\n");
		return EXIT_SUCCESS;
	}

	cpuConectarConMPS(self);
	cpuConectarConKernel(self);

	while(1){
		//1) Paso, recibir Quantum
		if (cpuRecibirQuantum(self) || cpuRecibirTCB(self)){
			log_info(self->loggerCPU, "Finalizando CPU..");
			return EXIT_FAILURE;
		}

		else{
			valorCPU = cpuProcesarTCB(self, serviciosAlPlanificador);

			log_info(self->loggerCPU, "CPU: valorCPU == %d",valorCPU);
			int num;

			switch(valorCPU){

			case FINALIZAR_PROGRAMA_EXITO:
				num = cpuFinalizarProgramaExitoso(self, self->tcb);

				if(num == MENSAJE_DE_ERROR){
					log_error(self->loggerCPU, "CPU: error al mandar un FINALIZAR_PROGRAMA_EXITO a la consola");
				}

				break;
			case SIN_ERRORES:
				//ALE: si el tcb ya fue enviado por XXXX aca lo vuelve a enviar!!! NO CONTEPLA ESE CASO
				cpuEnviarPaqueteAPlanificador(self, CAMBIO_DE_CONTEXTO);
				t_TCB_CPU* tcbProcesado = malloc(sizeof(t_TCB_CPU));
				tcbProcesado = self->tcb;

				//se mande un TCB a CPU
				//printTCBCPU(tcbProcesado);
				socket_sendPaquete(self->socketPlanificador->socket, TCB_NUEVO,sizeof(t_TCB_CPU), tcbProcesado);
				log_info(self->loggerCPU, "CPU: envia un CAMBIO_DE_CONTEXTO");
				free(tcbProcesado);
				break;

			case INTERRUPCION:
				cpuEnviaInterrupcion(self);
				//log_info(self->loggerCPU, "CPU: envia una INTERRUPCION");
				break;

			case ENTRADA_ESTANDAR:
				//cpuEnviarPaqueteAPlanificador(self, ENTRADA_ESTANDAR);
				//ALE: el send lo hace en la instruccion INNN e INNC en codigoESO.c
				//se crea una estructura Entrada para mandar
				//t_entrada_estandar* unaEntrada= malloc(sizeof(t_entrada_estandar));
				//unaEntrada->pid = serviciosAlPlanificador->entradaEstandar->pid;
				//unaEntrada->tamanio = serviciosAlPlanificador->entradaEstandar->tamanio;
				//unaEntrada->tipo = serviciosAlPlanificador->entradaEstandar->tipo;

				//se manda una Entrada
				//socket_sendPaquete(self->socketPlanificador->socket,ENTRADA_ESTANDAR,sizeof(t_entrada_estandar), unaEntrada);
				log_info(self->loggerCPU, "CPU: fin ENTRADA_ESTANDAR");
				break;

			case SALIDA_ESTANDAR:
				cpuEnviarPaqueteAPlanificador(self, SALIDA_ESTANDAR);
				//ALE: el send lo hace en la instruccion OUTN y OUTC en codigoESO.c
				//se crea una estructura Salida para mandar
				//t_salida_estandar* unaSalida= malloc(sizeof(t_salida_estandar));
				//unaSalida->pid = serviciosAlPlanificador->salidaEstandar->pid;
				//unaSalida->cadena = serviciosAlPlanificador->salidaEstandar->cadena;

				//se manda una SALIDA_ESTANDAR
				//socket_sendPaquete(self->socketPlanificador->socket,SALIDA_ESTANDAR,sizeof(t_salida_estandar), unaSalida);
				//log_info(self->loggerCPU, "CPU: envia una SALIDA_ESTANDAR");
				break;

			case CREAR_HILO:
				cpuEnviarPaqueteAPlanificador(self, CREAR_HILO);
				//ALE: el send lo hace en la instruccion CREA_ESO (linea: 1257 archivo: codigoESO.c)
				//se crea una estructura CREAR HIJO para mandar
				//t_crea_hilo* unCrearHilo= malloc(sizeof(t_crea_hilo));
				//unCrearHilo->tcb = serviciosAlPlanificador->crearHijo->tcb;

				//se manda un CrearHILO
				//socket_sendPaquete(self->socketPlanificador->socket,CREAR_HILO,sizeof(t_crea_hilo), unCrearHilo);
				//log_info(self->loggerCPU, "CPU: envia un CREAR_HILO");
				break;

			case JOIN_HILO:
				cpuEnviarPaqueteAPlanificador(self, JOIN_HILO);
				//ALE: el send lo hace en la instruccion JOIN_ESO (linea: 1271 archivo: codigoESO.c)
				//se crea una estructura JOIN HIJO para mandar
				//t_join* unJoin= malloc(sizeof(t_join));
				//unJoin->tid_esperar = serviciosAlPlanificador->join->tid_esperar;
				//unJoin->tid_llamador = serviciosAlPlanificador->join->tid_llamador;

				//se manda un JOIN
				//socket_sendPaquete(self->socketPlanificador->socket,JOIN_HILO,sizeof(t_join), unJoin);
				//log_info(self->loggerCPU, "CPU: envia un JOIN_HILO");
				break;

			case BLOK_HILO:
				cpuEnviarPaqueteAPlanificador(self, BLOK_HILO);
				//ALE: el send lo hace en la instruccion BLOK_ESO (linea: 1287 archivo: codigoESO.c)
				//se crea una estructura Block para mandar
				//t_bloquear* unBlock= malloc(sizeof(t_bloquear));
				//unBlock->id_recurso = serviciosAlPlanificador->bloquear->id_recurso;
				//unBlock->tcb = serviciosAlPlanificador->bloquear->tcb;


				//se manda un Block
				//socket_sendPaquete(self->socketPlanificador->socket,BLOK_HILO,sizeof(t_bloquear), unBlock);
				//log_info(self->loggerCPU, "CPU: envia un BLOK_HILO");
				break;

			case WAKE_HILO:
				cpuEnviarPaqueteAPlanificador(self, WAKE_HILO);
				//ALE: el send lo hace en la instruccion WAKE_ESO (linea: 1302 archivo: codigoESO.c)

				//se crea una estructura Block para mandar
				//t_despertar* unDespertar= malloc(sizeof(t_despertar));
				//unDespertar->id_recurso = serviciosAlPlanificador->despear->id_recurso;

				//se manda un WAKE
				//socket_sendPaquete(self->socketPlanificador->socket,WAKE_HILO,sizeof(t_despertar), unDespertar);
				//log_info(self->loggerCPU, "CPU: envia un WAKE_HILO");
				break;

			default:
				//Maneja los errores de la CPU, tine su valor en la variable valorCPU
				// ERROR_POR_DESCONEXION_DE_CPU 110
				// ERROR_POR_DESCONEXION_DE_CONSOLA 111
				// ERROR_POR_EJECUCION_ILICITA 112
				// ERROR_POR_CODIGO_INESPERADO 113
				// ERROR_REGISTRO_DESCONOCIDO 114

				if (socket_sendPaquete(self->socketPlanificador->socket, valorCPU, 0, NULL) <= 0)
					log_info(self->loggerCPU, "CPU: Fallo envio de CPU_TERMINE_UNA_LINEA, PID: %d", self->tcb->pid);
				else
					log_info(self->loggerCPU, "CPU: envia al Planificador un paquete N°: %d", valorCPU);


				if (socket_sendPaquete(self->socketPlanificador->socket, TCB_NUEVO, sizeof(t_TCB_CPU), self->tcb) > 0){
					log_info(self->loggerCPU, "CPU: envia un TCB por un ERROR");
					printTCBCPU(self->tcb);
				}else
					log_error(self->loggerCPU, "CPU: error al envia un TCB en una interrupcion");


				break;
			}

		}
	}

	log_info(self->loggerCPU, "Se desconecto la CPU. Elimino todo");

	close(self->socketPlanificador->socket->descriptor);
	close(self->socketMSP->socket->descriptor);

	free(serviciosAlPlanificador);
	free(self);
	return EXIT_SUCCESS;
}


void verificar_argumentosCPU(int argc, char* argv[]){

	if( argc != 2 ){
		printf("Modo de empleo: ./CPU cpuConfig.cfg\n");
		perror("CPU no recibio las configuraciones");
		exit (EXIT_FAILURE);
	}
}

void printTCBCPU(t_TCB_CPU* unTCB){

	printf("TCB PID: %d \n", unTCB->pid);
	printf("TCB TID: %d \n", unTCB->tid);
	printf("TCB  KM: %d \n", unTCB->km);

	printf("Base Segmento:   %0.8p \n",unTCB->base_segmento_codigo);
	printf("Tamanio Segmento: %d \n",unTCB->tamanio_segmento_codigo);
	printf("Puntero a Instruccion: %0.8p \n",unTCB->puntero_instruccion);

	printf("Base Stack:   %0.8p \n",unTCB->base_stack);
	printf("Cursor Stack: %0.8p \n",unTCB->cursor_stack);

	printf("Regristros A: %d\n", unTCB->registro_de_programacion[0]);
	printf("Regristros B: %d\n", unTCB->registro_de_programacion[1]);
	printf("Regristros C: %d\n", unTCB->registro_de_programacion[2]);
	printf("Regristros D: %d\n", unTCB->registro_de_programacion[3]);
	printf("Regristros E: %d\n", unTCB->registro_de_programacion[4]);

}


t_registros_cpu* cpuInicializarRegistrosCPU(t_CPU* self, t_registros_cpu* registros){

	registros->I = (uint32_t)self->tcb->pid;
	registros->K = (uint32_t)self->tcb->km;
	registros->M = self->tcb->base_segmento_codigo;
	registros->P = self->tcb->puntero_instruccion;
	registros->S = self->tcb->cursor_stack;
	registros->X = self->tcb->base_stack;

	int i;
	for(i = 0; i < 5 ;i++)
		registros->registros_programacion[i] = self->tcb->registro_de_programacion[i];

	return registros;
}

