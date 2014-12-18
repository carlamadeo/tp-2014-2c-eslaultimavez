#include "planificadorMensajesCPU.h"
#include "kernelMSP.h"
#include "commons/protocolStructInBigBang.h"

pthread_mutex_t sem_interrupcion = PTHREAD_MUTEX_INITIALIZER;
int seBloqueo = 0;

/***************************************************************************************************\
 *								--Comienzo Finalizar Programa con Exito--							*
\***************************************************************************************************/


void ejecutar_FINALIZAR_PROGRAMA_EXITO(t_kernel* self, t_socket_paquete *paqueteTCB){

	t_TCB_Kernel* tcbFinalizado = (t_TCB_Kernel*) malloc(sizeof(t_TCB_Kernel));
	tcbFinalizado = (t_TCB_Kernel*) paqueteTCB->data;

	if(programaBesoExiste(self, tcbFinalizado) != ERROR_POR_DESCONEXION_DE_CONSOLA){

		bool _tcbParaExit(t_programaEnKernel* tcb){
			return ((tcb->programaTCB->tid == tcbFinalizado->tid) && (tcb->programaTCB->pid == tcbFinalizado->pid));
		}

		pthread_mutex_lock(&execMutex);
		t_programaEnKernel* unTcbProcesado = list_find(cola_exec, (void*)_tcbParaExit);
		pthread_mutex_unlock(&execMutex);

		if(unTcbProcesado != NULL){
			socket_sendPaquete(unTcbProcesado->socketProgramaConsola, FINALIZAR_PROGRAMA_EXITO, 0, NULL);
			log_info(self->loggerPlanificador,"Planificador: Envia un FINALIZAR_PROGRAMA_EXITO a una consola");
			desbloquearHilosBloqueadosPorElQueFinalizo(self, unTcbProcesado);
		}
	}

	else
		log_error(self->loggerPlanificador,"Planificador: El Programa ha cerrado la conexion");

}


void desbloquearHilosBloqueadosPorElQueFinalizo(t_kernel *self, t_programaEnKernel* unTcbProcesado){

	int i;

	bool matchHilo(t_BloqueadoPorOtro* tcbBloqueador){
		return ((unTcbProcesado->programaTCB->tid == tcbBloqueador->TIDbloqueador) && (unTcbProcesado->programaTCB->pid == tcbBloqueador->pid));
	}

	t_BloqueadoPorOtro* pidTidBloqueador = list_remove_by_condition(listaBloqueadosPorOtroHilo, (void*) matchHilo);

	if(pidTidBloqueador != NULL){

		int listSize = list_size(pidTidBloqueador->hilosBloqueados);

		for(i = 0; i < listSize; i++){
			int tidBloqueado = list_remove(pidTidBloqueador->hilosBloqueados, i);

			bool _tcbParaReady(t_programaEnKernel* tcb){
				return ((tcb->programaTCB->tid == tidBloqueado) && (tcb->programaTCB->pid == pidTidBloqueador->pid));
			}

			t_programaEnKernel *programaADesbloquear = list_find(listaDeProgramasDisponibles, _tcbParaReady);

			if(programaADesbloquear != NULL){
				pasarProgramaDeBlockAReady(programaADesbloquear->programaTCB, 0);
				log_info(self->loggerPlanificador,"Planificador: Se ha desbloqueado al TCB con PID %d, TID %d", programaADesbloquear->programaTCB->pid, programaADesbloquear->programaTCB->tid);
			}

		}
	}

}



void ejecutar_FINALIZAR_HILO_EXITO(t_kernel* self, t_socket_paquete *paqueteTCB){

	t_TCB_Kernel* tcbFinalizado = (t_TCB_Kernel*) malloc(sizeof(t_TCB_Kernel));
	tcbFinalizado = (t_TCB_Kernel*) paqueteTCB->data;

	if(programaBesoExiste(self, tcbFinalizado) != ERROR_POR_DESCONEXION_DE_CONSOLA){

		bool _tcbParaExit(t_programaEnKernel* tcb){
			return ((tcb->programaTCB->tid == tcbFinalizado->tid) && (tcb->programaTCB->pid == tcbFinalizado->pid));
		}

		pthread_mutex_lock(&execMutex);
		t_programaEnKernel* unTcbProcesado = list_remove_by_condition(cola_exec, (void*)_tcbParaExit);
		pthread_mutex_unlock(&execMutex);

		pthread_mutex_lock(&exitMutex);
		list_add(cola_exit, unTcbProcesado);
		pthread_mutex_unlock(&exitMutex);

		if(unTcbProcesado != NULL){
			desbloquearHilosBloqueadosPorElQueFinalizo(self, unTcbProcesado);
		}

	}

	else
		log_error(self->loggerPlanificador,"Planificador: El Programa ha cerrado la conexion");

}



/***************************************************************************************************\
 *								--Comienzo Terminar Quantum--							            *
\***************************************************************************************************/


void ejecutar_TERMINAR_QUANTUM(t_kernel* self, t_socket_paquete *paqueteTCB){

	t_TCB_Kernel* TCBRecibido = (t_TCB_Kernel*) malloc(sizeof(t_TCB_Kernel));
	TCBRecibido = (t_TCB_Kernel*) paqueteTCB->data;

	int existeProgramaBeso = programaBesoExiste(self, TCBRecibido);

	if(existeProgramaBeso != ERROR_POR_DESCONEXION_DE_CONSOLA){
		pasarProgramaDeExecAReady(TCBRecibido);
	}

}


int programaBesoExiste(t_kernel* self, t_TCB_Kernel* TCBRecibido){

	bool _existeProgramaBeso(t_programaEnKernel *unPrograma){
		return ((unPrograma->programaTCB->pid == TCBRecibido->pid) && (unPrograma->programaTCB->tid == TCBRecibido->tid));
	}

	t_programaEnKernel *programaBuscado = NULL;

	pthread_mutex_lock(&programasBesoDisponibleMutex);
	programaBuscado = list_find(listaDeProgramasDisponibles,(void*)_existeProgramaBeso);
	pthread_mutex_unlock(&programasBesoDisponibleMutex);

	if (programaBuscado == NULL)
		return ERROR_POR_DESCONEXION_DE_CONSOLA;

	return 0;
}


void pasarProgramaDeExecAReady(t_TCB_Kernel *TCB){

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return (unPrograma->programaTCB->pid == TCB->pid) && (unPrograma->programaTCB->tid == TCB->tid) && (unPrograma->programaTCB->km == TCB->km);
	}

	t_programaEnKernel *program = list_get(cola_exec, 0);

	pthread_mutex_lock(&execMutex);
	t_programaEnKernel *programaBuscado = list_remove_by_condition(cola_exec,(void*)matchPrograma);
	pthread_mutex_unlock(&execMutex);

	copiarValoresDosTCBs(programaBuscado->programaTCB, TCB);

	pthread_mutex_lock(&readyMutex);
	list_add(cola_ready, programaBuscado);
	pthread_mutex_unlock(&readyMutex);

	sem_post(&sem_B);
}

/***************************************************************************************************\
 *								--Comienzo Ejecutar una Interrupcion--								*
\***************************************************************************************************/


void ejecutar_UNA_INTERRUPCION(t_kernel* self, t_socket_paquete* paquete){

	pthread_mutex_lock(&sem_interrupcion);
	t_interrupcionKernel* interrupcion = (t_interrupcionKernel*) (paquete->data);

	t_TCB_Kernel *TCBInterrupcion = malloc(sizeof(t_TCB_Kernel));

	convertirLaInterrupcionEnTCB(interrupcion, TCBInterrupcion);

	if(programaBesoExiste(self, TCBInterrupcion) != ERROR_POR_DESCONEXION_DE_CONSOLA){

		//Paso el TCB que llamo la interrupcion a la cola de Block
		t_socket *socketConsola = pasarProgramaDeExecABlock(TCBInterrupcion);

		//Paso el TCB que llamo la interrupcion a la cola de bloqueados por System Calls
		agregarTCBAColaSystemCalls(TCBInterrupcion, interrupcion->direccionKM);

		//Tomo al primer TCB que se encuentre en la lista de bloqueados por System Calls
		t_TCBSystemCalls *TCBSystemCall = list_get(listaSystemCall, 0);

		//Modifico el TCB Kernel con los valores del TCB que llamo la interrupcion
		modificarTCBKM(self->tcbKernel, TCBSystemCall);

		//Paso el TCB Kernel a Ready
		pasarProgramaDeBlockAReady(self->tcbKernel, socketConsola);

		free(interrupcion);
	}

	else
		log_error(self->loggerPlanificador, "Planificador: El Programa ha cerrado la conexion");


}


void ejecutar_FIN_DE_INTERRUPCION(t_kernel* self, t_socket_paquete* paquete){

	//Recibo el TCB que finalizo la interrupcion
	t_finInterrupcionKernel* informacionFinInterrupcion = (t_finInterrupcionKernel*) (paquete->data);

	t_TCB_Kernel *TCBFinInterrupcion = malloc(sizeof(t_TCB_Kernel));

	copiarValoresFinInterrupcionATCB(informacionFinInterrupcion, TCBFinInterrupcion);

	if(programaBesoExiste(self, TCBFinInterrupcion) != ERROR_POR_DESCONEXION_DE_CONSOLA){

		copiarValoresDosTCBs(self->tcbKernel, TCBFinInterrupcion);

		//Vuelvo a bloquear al TCB Kernel
		pasarProgramaDeExecABlock(self->tcbKernel);

		//Busco al TCB que llego en la cola de bloqueados por System Calls y lo elimino
		bool matchTCB(t_TCBSystemCalls *TCB){
			return (TCB->programa->programaTCB->pid == TCBFinInterrupcion->pid) && (TCB->programa->programaTCB->tid == TCBFinInterrupcion->tid);
		}

		t_TCBSystemCalls *TCBFinDeInterrupcionSystemCall = list_remove_by_condition(listaSystemCall, matchTCB);

		if(informacionFinInterrupcion->esJoin != 1 || seBloqueo != 1){
			//Copio los valores de los registros del TCB que se ejecuto y pongo el km en 0
			volverTCBAModoNoKernel(self->tcbKernel, TCBFinDeInterrupcionSystemCall->programa->programaTCB);
			//Saco al TCB de la cola de bloqueados y lo paso a Ready
			pasarProgramaDeBlockAReady(TCBFinDeInterrupcionSystemCall->programa->programaTCB, 0);
			seBloqueo = 0;
		}

		pthread_mutex_unlock(&sem_interrupcion);
	}

else
		log_error(self->loggerPlanificador,"Planificador: La Consola se ha desconectado. No se puede ejecutar la operación FIN_DE_INTERRUPCION");

}


void copiarValoresFinInterrupcionATCB(t_finInterrupcionKernel* informacionFinInterrupcion, t_TCB_Kernel *TCBFinInterrupcion){

	TCBFinInterrupcion->pid = informacionFinInterrupcion->pid;
	TCBFinInterrupcion->tid = informacionFinInterrupcion->tid;
	TCBFinInterrupcion->km = informacionFinInterrupcion->km;
	TCBFinInterrupcion->base_segmento_codigo = informacionFinInterrupcion->base_segmento_codigo;
	TCBFinInterrupcion->tamanio_segmento_codigo = informacionFinInterrupcion->tamanio_segmento_codigo;
	TCBFinInterrupcion->tamanio_segmento_codigo = informacionFinInterrupcion->tamanio_segmento_codigo;
	TCBFinInterrupcion->puntero_instruccion = informacionFinInterrupcion->puntero_instruccion;
	TCBFinInterrupcion->base_stack = informacionFinInterrupcion->base_stack;
	TCBFinInterrupcion->cursor_stack = informacionFinInterrupcion->cursor_stack;
	TCBFinInterrupcion->registro_de_programacion[0] = informacionFinInterrupcion->registro_de_programacion[0];
	TCBFinInterrupcion->registro_de_programacion[1] = informacionFinInterrupcion->registro_de_programacion[1];
	TCBFinInterrupcion->registro_de_programacion[2] = informacionFinInterrupcion->registro_de_programacion[2];
	TCBFinInterrupcion->registro_de_programacion[3] = informacionFinInterrupcion->registro_de_programacion[3];
	TCBFinInterrupcion->registro_de_programacion[4] = informacionFinInterrupcion->registro_de_programacion[4];

}


t_socket *pasarProgramaDeExecABlock(t_TCB_Kernel *TCB){

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return (unPrograma->programaTCB->pid == TCB->pid) && (unPrograma->programaTCB->tid == TCB->tid);
	}

	pthread_mutex_lock(&execMutex);
	t_programaEnKernel *programaBuscado = list_remove_by_condition(cola_exec, matchPrograma);
	pthread_mutex_unlock(&execMutex);

	copiarValoresDosTCBs(programaBuscado->programaTCB, TCB);

	pthread_mutex_lock(&blockMutex);
	list_add(cola_block, programaBuscado);
	pthread_mutex_unlock(&blockMutex);

	return programaBuscado->socketProgramaConsola;
}


void agregarTCBAColaSystemCalls(t_TCB_Kernel* TCBInterrupcion, uint32_t direccionKM){

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return (unPrograma->programaTCB->pid == TCBInterrupcion->pid) && (unPrograma->programaTCB->tid == TCBInterrupcion->tid);
	}

	t_programaEnKernel *programaBuscado = list_find(listaDeProgramasDisponibles, matchPrograma);

	//El programa que se encuentra en la lista de programas disponible no tiene las mismas direcciones que el que busco ahora, por eso actualizo
	copiarValoresDosTCBs(programaBuscado->programaTCB, TCBInterrupcion);

	t_TCBSystemCalls *TCBSystemCall = malloc(sizeof(t_TCBSystemCalls));
	TCBSystemCall->programa = programaBuscado;
	TCBSystemCall->direccionKM = direccionKM;

	//TODO Ver si aca necesito bloquear la lista
	list_add(listaSystemCall, TCBSystemCall);

}


void modificarTCBKM(t_TCB_Kernel *TCBKernel, t_TCBSystemCalls *TCBSystemCall){

	TCBKernel->pid = TCBSystemCall->programa->programaTCB->pid;
	TCBKernel->tid = TCBSystemCall->programa->programaTCB->tid;
	TCBKernel->puntero_instruccion = TCBSystemCall->direccionKM;
	TCBKernel->registro_de_programacion[0] = TCBSystemCall->programa->programaTCB->registro_de_programacion[0];
	TCBKernel->registro_de_programacion[1] = TCBSystemCall->programa->programaTCB->registro_de_programacion[1];
	TCBKernel->registro_de_programacion[2] = TCBSystemCall->programa->programaTCB->registro_de_programacion[2];
	TCBKernel->registro_de_programacion[3] = TCBSystemCall->programa->programaTCB->registro_de_programacion[3];
	TCBKernel->registro_de_programacion[4] = TCBSystemCall->programa->programaTCB->registro_de_programacion[4];

}


void pasarProgramaDeBlockAReady(t_TCB_Kernel *TCB, t_socket *socketConsola){

	int encontrado = 0;

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return (unPrograma->programaTCB->pid == TCB->pid) && (unPrograma->programaTCB->tid == TCB->tid);
	}

	bool esProgramaBloqueadoPorRecurso(t_programaEnKernel *unProgramaBloqueado){
		return (unProgramaBloqueado->programaTCB->pid == TCB->pid) && (unProgramaBloqueado->programaTCB->tid == TCB->tid);
	}

	void busquedaPorLista(t_recurso *recurso){
		t_programaEnKernel *programaBloqueadoPorRecurso = list_find(recurso->listaBloqueados, esProgramaBloqueadoPorRecurso);

		if(programaBloqueadoPorRecurso != NULL) encontrado = 1;
	}

	list_iterate(listaDeRecursos, busquedaPorLista);

	if(!encontrado){
		printf("Block_A_Ready: se encontro programa que no esta bloqueado por recurso\n");
		pthread_mutex_lock(&execMutex);
		t_programaEnKernel *programaBuscado = list_remove_by_condition(cola_block, matchPrograma);
		pthread_mutex_unlock(&execMutex);

		if(TCB->km == 1){
			programaBuscado->socketProgramaConsola = socketConsola;
		}

		programaBuscado->programaTCB = TCB;

		pthread_mutex_lock(&blockMutex);
		list_add(cola_ready, programaBuscado);
		pthread_mutex_unlock(&blockMutex);

		sem_post(&sem_B);
		printf("Block_A_Ready: se cargo un programa a READY correctamente\n");
	}else
		printf("Block_A_Ready: se encontro programa bloqueado por recurso\n");

}


void convertirLaInterrupcionEnTCB(t_interrupcionKernel *interrupcion, t_TCB_Kernel *TCBInterrupcion){

	TCBInterrupcion->pid = interrupcion->pid;
	TCBInterrupcion->tid = interrupcion->tid;
	TCBInterrupcion->km = interrupcion->km;
	TCBInterrupcion->base_segmento_codigo = interrupcion->base_segmento_codigo;
	TCBInterrupcion->tamanio_segmento_codigo = interrupcion->tamanio_segmento_codigo;
	TCBInterrupcion->puntero_instruccion = interrupcion->puntero_instruccion;
	TCBInterrupcion->base_stack = interrupcion->base_stack;
	TCBInterrupcion->cursor_stack = interrupcion->cursor_stack;
	TCBInterrupcion->registro_de_programacion[0] = interrupcion->registro_de_programacion[0];
	TCBInterrupcion->registro_de_programacion[1] = interrupcion->registro_de_programacion[1];
	TCBInterrupcion->registro_de_programacion[2] = interrupcion->registro_de_programacion[2];
	TCBInterrupcion->registro_de_programacion[3] = interrupcion->registro_de_programacion[3];
	TCBInterrupcion->registro_de_programacion[4] = interrupcion->registro_de_programacion[4];

}


void volverTCBAModoNoKernel(t_TCB_Kernel * TCBKernel, t_TCB_Kernel *unTCB){

	unTCB->km = 0;
	unTCB->registro_de_programacion[0] = TCBKernel->registro_de_programacion[0];
	unTCB->registro_de_programacion[1] = TCBKernel->registro_de_programacion[1];
	unTCB->registro_de_programacion[2] = TCBKernel->registro_de_programacion[2];
	unTCB->registro_de_programacion[3] = TCBKernel->registro_de_programacion[3];
	unTCB->registro_de_programacion[4] = TCBKernel->registro_de_programacion[4];
}

/***************************************************************************************************\
 *								--Comienzo Ejecutar una Entrada Estandar--							*
\***************************************************************************************************/


void ejecutar_UNA_ENTRADA_ESTANDAR(t_kernel* self, t_cpu *cpu, t_socket_paquete* paquete){

	t_entrada_estandarKenel* entradaEstandar = (t_entrada_estandarKenel*) paquete->data;


	bool esProgramaEntrada(t_programaEnKernel* programaEnLista){
		return (programaEnLista->programaTCB->pid == entradaEstandar->pid);
	}

	t_programaEnKernel* unPrograma = list_find(listaDeProgramasDisponibles, (void*)esProgramaEntrada);

	if(unPrograma != NULL){

		if (entradaEstandar->tipo == ENTRADA_ESTANDAR_INT){
			entradaEstandar->idCPU = cpu->id;
			socket_sendPaquete(unPrograma->socketProgramaConsola, ENTRADA_ESTANDAR_INT, sizeof(t_entrada_estandarKenel), entradaEstandar);
			log_info(self->loggerPlanificador, "Planificador: Envia ENTRADA_ESTANDAR_INT a Consola");

		}

		else if(entradaEstandar->tipo == ENTRADA_ESTANDAR_TEXT){
			entradaEstandar->idCPU = cpu->id;
			socket_sendPaquete(unPrograma->socketProgramaConsola, ENTRADA_ESTANDAR_TEXT, sizeof(t_entrada_estandarKenel), entradaEstandar);
			log_info(self->loggerPlanificador, "Planificador: Envia ENTRADA_ESTANDAR_TEXT a Consola");
		}

		else
			log_error(self->loggerPlanificador, "Planificador: Error al enviar UNA_ENTRADA_STANDAR de consola.");

	}

	else
		log_error(self->loggerPlanificador, "Planificador: El Programa ha cerrado la conexion");

	free(entradaEstandar);

}


/***************************************************************************************************\
 *								--Comienzo Ejecutar una Salida Estandar--							*
\***************************************************************************************************/

ejecutar_UNA_SALIDA_ESTANDAR(t_kernel* self, t_cpu *cpu, t_socket_paquete* paquete){

	t_salida_estandarKernel* salidaEstandar = (t_salida_estandarKernel*) paquete->data;

	bool esProgramaSalida(t_programaEnKernel* programaEnLista){
		return (programaEnLista->programaTCB->pid == salidaEstandar->pid);
	}

	log_info(self->loggerPlanificador, "Planificador: Se recibio de la CPU %s\n", salidaEstandar->cadena);

	t_programaEnKernel* unProgramaSalida = list_find(listaDeProgramasDisponibles, (void*)esProgramaSalida);

	if(socket_sendPaquete(unProgramaSalida->socketProgramaConsola, SALIDA_ESTANDAR, sizeof(t_salida_estandarKernel), salidaEstandar) >= 0)
		log_debug(self->loggerPlanificador, "Planificador: envia UNA_SALIDA_ESTANDAR a la Consola");

	free(salidaEstandar);
}


/***************************************************************************************************\
 *								   --Comienzo Crear Hilo--							                *
\***************************************************************************************************/

//TODO Ver si lo tengo que mandar a la cola NEW y que pase por el loader en vez de mandarlo directo a la cola READY
void ejecutar_UN_CREAR_HILO(t_kernel* self, t_socket_paquete* paquete){

	char *lecturaEscrituraMSP = malloc(sizeof(char)*self->tamanioStack + 1);
	t_TCB_Kernel* hiloNuevo = malloc(sizeof(t_TCB_Kernel));
	t_crea_hiloKernel *datosRecibidos = (t_crea_hiloKernel*) paquete->data;

	bool matchProgramaEnBlock(t_programaEnKernel *programa){
		return ((programa->programaTCB->pid == datosRecibidos->pid) && ((programa->programaTCB->tid == datosRecibidos->tid)));
	}

	t_programaEnKernel* programaHiloRecibido = list_find(cola_block, matchProgramaEnBlock);

	if(programaHiloRecibido != NULL){
		copiarValoresDosTCBs(hiloNuevo, programaHiloRecibido->programaTCB);

		uint32_t baseStackPadre = programaHiloRecibido->programaTCB->base_stack;
		uint32_t cursorStackPadre = programaHiloRecibido->programaTCB->cursor_stack;

		hiloNuevo->puntero_instruccion = (uint32_t)programaHiloRecibido->programaTCB->registro_de_programacion[1];
		kernelLeerMemoria(self, hiloNuevo->pid, baseStackPadre, lecturaEscrituraMSP, self->tamanioStack);

		hiloNuevo->base_stack = kernelCrearSegmento(self, hiloNuevo->pid, self->tamanioStack);
		hiloNuevo->cursor_stack = hiloNuevo->base_stack + (cursorStackPadre - baseStackPadre);
		kernelEscribirMemoria(self, hiloNuevo->pid, hiloNuevo->base_stack, lecturaEscrituraMSP, self->tamanioStack);

		bool contarTID(t_programaEnKernel *unPrograma){
			return (unPrograma->programaTCB->pid == hiloNuevo->pid);
		}

		hiloNuevo->tid = list_count_satisfying(listaDeProgramasDisponibles, contarTID);

		t_programaEnKernel* programaNuevoHilo = malloc(sizeof(t_programaEnKernel));

		programaNuevoHilo->programaTCB = hiloNuevo;
		programaNuevoHilo->socketProgramaConsola = programaHiloRecibido->socketProgramaConsola;

		list_add(listaDeProgramasDisponibles, programaNuevoHilo);

		pthread_mutex_lock(&readyMutex);
		list_add(cola_ready, programaNuevoHilo);
		pthread_mutex_unlock(&readyMutex);

		sem_post(&sem_B);
	}

	free(lecturaEscrituraMSP);

}


void copiarValoresDosTCBs(t_TCB_Kernel *tcbHasta, t_TCB_Kernel *tcbDesde){
	tcbHasta->base_segmento_codigo = tcbDesde->base_segmento_codigo;
	tcbHasta->base_stack = tcbDesde->base_stack;
	tcbHasta->cursor_stack = tcbDesde->cursor_stack;
	tcbHasta->km = tcbDesde->km;
	tcbHasta->pid = tcbDesde->pid;
	tcbHasta->puntero_instruccion = tcbDesde->puntero_instruccion;
	tcbHasta->registro_de_programacion[0] = tcbDesde->registro_de_programacion[0];
	tcbHasta->registro_de_programacion[1] = tcbDesde->registro_de_programacion[1];
	tcbHasta->registro_de_programacion[2] = tcbDesde->registro_de_programacion[2];
	tcbHasta->registro_de_programacion[3] = tcbDesde->registro_de_programacion[3];
	tcbHasta->registro_de_programacion[4] = tcbDesde->registro_de_programacion[4];
	tcbHasta->tamanio_segmento_codigo = tcbDesde->tamanio_segmento_codigo;
	tcbHasta->tid = tcbDesde->tid;
}


/***************************************************************************************************\
 *							       	--Comienzo JOIN--							                    *
\***************************************************************************************************/

void ejecutar_UN_JOIN_HILO(t_kernel* self, t_socket_paquete* paquete){

	t_joinKernel *joinHilos = (t_joinKernel*) paquete->data;
	seBloqueo = 0;

	bool matchTCBEsperar(t_TCB_Kernel *unTCB){
		return ((unTCB->pid == joinHilos->pid) && (unTCB->tid == joinHilos->tid_esperar));
	}

	//Si el hilo que debe bloquear ya finalizo no bloqueo al tid_llamador
	t_TCB_Kernel *TCBFinalizado = list_find(cola_exit, matchTCBEsperar);

	if(TCBFinalizado != NULL){

		seBloqueo = 1;

		bool matchHilo(t_BloqueadoPorOtro *hiloBloqueador){
			return ((hiloBloqueador->pid == joinHilos->pid) && (hiloBloqueador->TIDbloqueador == joinHilos->tid_esperar));
		}

		t_BloqueadoPorOtro *bloqueador = list_find(listaBloqueadosPorOtroHilo, matchHilo);

		if(bloqueador == NULL){
			t_BloqueadoPorOtro *bloqueadorNuevo = malloc(sizeof(t_BloqueadoPorOtro));
			bloqueadorNuevo->pid = joinHilos->pid;
			bloqueadorNuevo->TIDbloqueador = joinHilos->tid_esperar;
			bloqueadorNuevo->hilosBloqueados = list_create();
			list_add(bloqueadorNuevo->hilosBloqueados, joinHilos->tid_llamador);
			list_add(listaBloqueadosPorOtroHilo, bloqueadorNuevo);

		}

		else
			list_add(bloqueador->hilosBloqueados, joinHilos->tid_llamador);

		log_info(self->loggerPlanificador, "Planificador: Para el PID %d, TID %d, se bloquea al TCB con TID %d", joinHilos->pid, joinHilos->tid_esperar, joinHilos->tid_llamador);
	}

}


/***************************************************************************************************\
 *								        --Comienzo BLOCK--							                *
\***************************************************************************************************/

void ejecutar_UN_BLOCK_HILO(t_kernel* self, t_socket_paquete* paquete){

	t_bloquearKernel *blockHilo = (t_bloquearKernel*) (paquete->data);

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return ((unPrograma->programaTCB->pid == blockHilo->pid) && (unPrograma->programaTCB->tid == blockHilo->tid));
	}

	t_programaEnKernel* programaABloquear = list_find(listaDeProgramasDisponibles, (void*) matchPrograma);

	if(programaABloquear != NULL){

		bool matchRecurso(t_recurso *unRecurso){
			return unRecurso->identificador == blockHilo->id_recurso;
		}

		t_recurso *recursoEncontrado = list_find(listaDeRecursos, (void*) matchRecurso);

		if(recursoEncontrado == NULL){
			t_recurso *recurso = malloc(sizeof(t_recurso));
			recurso->identificador = blockHilo->id_recurso;
			recurso->listaBloqueados = list_create();
			list_add(listaDeRecursos, recurso);
			log_info(self->loggerPlanificador,"Planificador: carga un recurso a la listaDeRecursos :%d", recurso->identificador);
			list_add(recurso->listaBloqueados, programaABloquear);
			log_info(self->loggerPlanificador,"Planificador: carga un programa a la ListeBloqueados con tamanio de la lista: %d", list_size(recurso->listaBloqueados));

		}else{
			list_add(recursoEncontrado->listaBloqueados, programaABloquear);
			log_info(self->loggerPlanificador,"Planificador: se encontro recurso en block BLOCK_HILO");
		}

	}

	else
		log_error(self->loggerPlanificador,"Planificador: La Consola ya no existe. No se puede realizar la operacion BLOCK_HILO");
}


/***************************************************************************************************\
 *								      --Comienzo WAKE--							                    *
\***************************************************************************************************/

void ejecutar_UN_WAKE_HILO(t_kernel* self, t_socket_paquete* paquete){

	t_despertarKernel *recursoDespertarHilo = (t_bloquearKernel*) (paquete->data);

	bool matchRecurso(t_recurso *unRecurso){
		return unRecurso->identificador == recursoDespertarHilo->id_recurso;
	}

	t_recurso *recursoEncontrado = list_find(listaDeRecursos, matchRecurso);

	if(recursoEncontrado != NULL){
		t_programaEnKernel *programaADesbloquear = list_remove(recursoEncontrado->listaBloqueados, 0);

		if(programaADesbloquear != NULL){

			bool matchPrograma(t_programaEnKernel *unPrograma){
				return (unPrograma->programaTCB->pid == programaADesbloquear->programaTCB->pid) && (unPrograma->programaTCB->tid == programaADesbloquear->programaTCB->tid);
			}

			pthread_mutex_lock(&execMutex);
			t_programaEnKernel *programaBuscado = list_remove_by_condition(cola_block, matchPrograma);
			pthread_mutex_unlock(&execMutex);

			pthread_mutex_lock(&blockMutex);
			list_add(cola_ready, programaBuscado);
			pthread_mutex_unlock(&blockMutex);
		}

	}

}


/***************************************************************************************************\
 *						 --Comienzo Enviar Mensaje de Error a Consola--							    *
\***************************************************************************************************/

void ejecutar_UN_MENSAJE_DE_ERROR(t_kernel* self, t_socket_paquete* paquete){

	int mandarMensaje =0;
	char *mensaje = malloc(sizeof(char) * 150);

	t_errorKernel *errorParaConsola = (t_errorKernel*) (paquete->data);

	bool matchProgramaConsola(t_programaEnKernel* programa){
		return (programa->programaTCB->pid == errorParaConsola->pid);
	}

	t_programaEnKernel *programaAEnviar = list_find(listaDeProgramasDisponibles,(void*)matchProgramaConsola);

	if(programaAEnviar != NULL){
		t_entrada_textoKernel *enviarMensaje = malloc(sizeof(t_entrada_textoKernel)); //Uso la misma estructura que para la ENTRADA_ESTANDAR

		switch(errorParaConsola->identificadorError){

		case ERROR_REGISTRO_DESCONOCIDO:

			mensaje = "Ha ocurrido un error por un registro desconocido";

			break;

		case ERROR_POR_EJECUCION_ILICITA:

			mensaje = "Ha ocurrido un error al intentar acceder a un area de memoria protegida";

			break;

		case ERROR_POR_TAMANIO_EXCEDIDO:

			mensaje = "Ha ocurrido un error al solicitar un tamanio de segmento mayor al permitido";

			break;

		case ERROR_POR_MEMORIA_LLENA:

			mensaje = "Ha ocurrido un error de memoria. No hay espacio disponible";

			break;

		case ERROR_POR_NUMERO_NEGATIVO:

			mensaje = "Ha ocurrido un error al solicitar la creacion de un segmento con tamanio menor a 1";

			break;

		case ERROR_POR_SEGMENTO_DESCONOCIDO:

			mensaje = "Ha ocurrido un error al solicitar la destruccion de un segmento invalido";

			break;

		case ERROR_POR_SEGMENTATION_FAULT:

			mensaje = "Ha ocurrido un error por Segmentation Fault";

			break;

		default:

			mensaje = "Ha ocurrido un error desconocido";

			break;
		}

		memset(enviarMensaje->texto, 0, 150);
		memcpy(enviarMensaje->texto, mensaje, strlen(mensaje) + 1);

		if(socket_sendPaquete(programaAEnviar->socketProgramaConsola, MENSAJE_DE_ERROR, sizeof(t_entrada_textoKernel), enviarMensaje) >= 0)
			log_debug(self->loggerPlanificador, "Planificador: Envia ERROR a la Consola");

		free(enviarMensaje);
	}

	else
		log_error(self->loggerPlanificador,"Planificador: El Programa ha cerrado la conexion (ejecutar_UN_MENSAJE_DE_ERROR)");

	free(errorParaConsola);
}



void printTCBKernel(t_TCB_Kernel* unTCB){

	printf("TCB PID: %d \n", unTCB->pid);
	printf("TCB TID: %d \n", unTCB->tid);
	printf("TCB  KM: %d \n", unTCB->km);

	printf("Base Segmento:   %0.8p\n",unTCB->base_segmento_codigo);
	printf("Tamanio Segmento: %d\n",unTCB->tamanio_segmento_codigo);
	printf("Puntero a Instruccion: %0.8p\n",unTCB->puntero_instruccion);

	printf("Base Stack:   %0.8p\n",unTCB->base_stack);
	printf("Cursor Stack: %0.8p\n",unTCB->cursor_stack);

	printf("Regristros A: %d\n", unTCB->registro_de_programacion[0]);
	printf("Regristros B: %d\n", unTCB->registro_de_programacion[1]);
	printf("Regristros C: %d\n", unTCB->registro_de_programacion[2]);
	printf("Regristros D: %d\n", unTCB->registro_de_programacion[3]);
	printf("Regristros E: %d\n", unTCB->registro_de_programacion[4]);

}
