#include "planificadorMensajesCPU.h"
#include "kernelMSP.h"
#include "commons/protocolStructInBigBang.h"

/***************************************************************************************************\
 *								--Comienzo Finalizar Programa con Exito--							*
\***************************************************************************************************/

//TODO Se deben liberar los recursos!!
//Pasa el programa finalizado de la cola exec a la cola exit
void ejecutar_FINALIZAR_PROGRAMA_EXITO(t_kernel* self, t_socket_paquete *paqueteTCB){

	t_TCB_Kernel* tcbFinalizado = (t_TCB_Kernel*) malloc(sizeof(t_TCB_Kernel));
	tcbFinalizado = (t_TCB_Kernel*) paqueteTCB->data;

	bool _tcbParaExit(t_programaEnKernel* tcb){
		return ((tcb->programaTCB->tid == tcbFinalizado->tid) && (tcb->programaTCB->pid == tcbFinalizado->pid));
	}

	t_programaEnKernel* unTcbProcesado = list_remove_by_condition(cola_exec, (void*)_tcbParaExit);

	if(unTcbProcesado != NULL){
		socket_sendPaquete(unTcbProcesado->socketProgramaConsola, FINALIZAR_PROGRAMA_EXITO, 0 ,NULL);

		pthread_mutex_lock(&exitMutex);
		list_add(cola_exit, unTcbProcesado);
		pthread_mutex_unlock(&exitMutex);

		desbloquearHilosBloqueadosPorElQueFinalizo(unTcbProcesado);
	}

	else
		log_error(self->loggerPlanificador, "Planificador: No se encontro ningun Programa. Esto es en ejecutar_FINALIZAR_PROGRAMA_EXITO");

}


void desbloquearHilosBloqueadosPorElQueFinalizo(t_programaEnKernel* unTcbProcesado){

	int i;

	bool matchHilo(t_BloqueadoPorOtro* tcbBloqueador){
		return ((unTcbProcesado->programaTCB->tid == tcbBloqueador->TIDbloqueador) && (unTcbProcesado->programaTCB->pid == tcbBloqueador->pid));
	}

	t_BloqueadoPorOtro* pidTidBloqueador = list_remove_by_condition(listaBloqueadosPorOtroHilo, matchHilo);

	if(pidTidBloqueador != NULL){

		int listSize = list_size(pidTidBloqueador->hilosBloqueados);

		for(i = 0; i < listSize; i++){
			int tidBloqueado = list_remove(pidTidBloqueador->hilosBloqueados, i);

			bool _tcbParaReady(t_programaEnKernel* tcb){
				return ((tcb->programaTCB->tid == tidBloqueado) && (tcb->programaTCB->pid == pidTidBloqueador->pid));
			}

			t_programaEnKernel *programaADesbloquear = list_remove_by_condition(listaDeProgramasDisponibles, _tcbParaReady);

			if(programaADesbloquear != NULL){
				pthread_mutex_lock(&blockMutex);
				pasarProgramaDeBlockAReady(programaADesbloquear->programaTCB);
				pthread_mutex_unlock(&blockMutex);
			}
		}

	}

}



/***************************************************************************************************\
 *								--Comienzo Terminar Quantum--							            *
\***************************************************************************************************/

//TODO Falta ver si hay algun TCB en modo kernel, que tiene prioridad!!!
//TODO Falta ver que pasa si no hay cpu disponible
void ejecutar_TERMINAR_QUANTUM(t_kernel* self, t_socket_paquete *paqueteTCB){

	t_TCB_Kernel* TCBRecibido = (t_TCB_Kernel*) malloc(sizeof(t_TCB_Kernel));
	TCBRecibido = (t_TCB_Kernel*) paqueteTCB->data;

	log_info(self->loggerPlanificador, "Planificador: recibe un TCB procesado de la CPU.");
	printTCBKernel(TCBRecibido);
	pasarProgramaDeExecAReady(TCBRecibido);

}


void pasarProgramaDeExecAReady(t_TCB_Kernel *TCB){

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return (unPrograma->programaTCB->pid == TCB->pid) && (unPrograma->programaTCB->tid == TCB->tid) && (unPrograma->programaTCB->km == TCB->km);
	}

	t_programaEnKernel *program = list_get(cola_exec, 0);

	pthread_mutex_lock(&execMutex);
	t_programaEnKernel *programaBuscado = list_remove_by_condition(cola_exec,(void*)matchPrograma);
	pthread_mutex_unlock(&execMutex);

	programaBuscado->programaTCB = TCB;

	pthread_mutex_lock(&readyMutex);
	list_add(cola_ready, programaBuscado);
	pthread_mutex_unlock(&readyMutex);

	sem_post(&sem_B);
}

/***************************************************************************************************\
 *								--Comienzo Ejecutar una Interrupcion--							*
\***************************************************************************************************/


void ejecutar_UNA_INTERRUPCION(t_kernel* self, t_socket_paquete* paquete){

	sem_wait(&sem_interrupcion);
	t_interrupcionKernel* interrupcion = (t_interrupcionKernel*) (paquete->data);

	t_TCB_Kernel *TCBInterrupcion = malloc(sizeof(t_TCB_Kernel));

	convertirLaInterrupcionEnTCB(interrupcion, TCBInterrupcion); //QUE FEO!!

	pasarProgramaDeExecABlock(TCBInterrupcion);

	agregarTCBAColaSystemCalls(TCBInterrupcion, interrupcion->direccionKM);

	//TODO No se si el programa lo tengo que tomar de esta cola o de la de BLOCK
	t_TCBSystemCalls *TCBSystemCall = list_get(listaSystemCall, 0);

	modificarTCBKM(self->tcbKernel, TCBSystemCall);

	pasarProgramaDeBlockAReady(self->tcbKernel);

	free(interrupcion);

}


void ejecutar_FIN_DE_INTERRUPCION(t_kernel* self, t_socket_paquete* paquete){

	t_TCB_Kernel* tcbFinInterrupcion = (t_TCB_Kernel*) (paquete->data);

	pasarProgramaDeExecABlock(self->tcbKernel);

	bool matchTCB(t_TCBSystemCalls *TCB){
		return (TCB->programa->programaTCB->pid == tcbFinInterrupcion->pid) && (TCB->programa->programaTCB->tid == tcbFinInterrupcion->tid);
	}

	t_TCBSystemCalls *TCBFinInterrupcion = list_remove_by_condition(listaSystemCall, matchTCB);

	TCBFinInterrupcion->programa->programaTCB->km = 0;

	volverTCBAModoNoKernel(self->tcbKernel, TCBFinInterrupcion->programa->programaTCB);

	pasarProgramaDeBlockAReady(TCBFinInterrupcion->programa->programaTCB);

	sem_post(&sem_interrupcion);

}


void pasarProgramaDeExecABlock(t_TCB_Kernel *TCB){

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return (unPrograma->programaTCB->pid == TCB->pid) && (unPrograma->programaTCB->tid == TCB->tid);
	}

	pthread_mutex_lock(&execMutex);
	t_programaEnKernel *programaBuscado = list_remove_by_condition(cola_exec, matchPrograma);
	pthread_mutex_unlock(&execMutex);

	programaBuscado->programaTCB = TCB;

	pthread_mutex_lock(&blockMutex);
	list_add(cola_block, programaBuscado);
	pthread_mutex_unlock(&blockMutex);
}


void agregarTCBAColaSystemCalls(t_TCB_Kernel* TCBInterrupcion, uint32_t direccionKM){

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return (unPrograma->programaTCB->pid == TCBInterrupcion->pid) && (unPrograma->programaTCB->tid == TCBInterrupcion->tid) && (unPrograma->programaTCB->km == TCBInterrupcion->km);
	}

	t_programaEnKernel *programaBuscado = list_remove_by_condition(listaDeProgramasDisponibles, matchPrograma);

	//El programa que se encuentra en la lista de programas disponible no tiene las mismas direcciones que el que busco ahora, por eso actualizo
	programaBuscado->programaTCB = TCBInterrupcion;
	list_add(listaDeProgramasDisponibles, programaBuscado);

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


void pasarProgramaDeBlockAReady(t_TCB_Kernel *TCB){

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return unPrograma->programaTCB->pid == TCB->pid;
	}

	pthread_mutex_lock(&execMutex);
	t_programaEnKernel *programaBuscado = list_remove_by_condition(cola_block, matchPrograma);
	pthread_mutex_unlock(&execMutex);

	programaBuscado->programaTCB = TCB;

	pthread_mutex_lock(&blockMutex);
	list_add(cola_ready, programaBuscado);
	pthread_mutex_unlock(&blockMutex);

	sem_post(&sem_B);

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

	if (entradaEstandar->tipo == ENTRADA_ESTANDAR_INT){

		socket_sendPaquete(unPrograma->socketProgramaConsola, ENTRADA_ESTANDAR_INT, sizeof(t_entrada_estandarKenel), entradaEstandar);

		log_info(self->loggerPlanificador, "Planificador: Envia ENTRADA_ESTANDAR_INT");

		t_socket_paquete *paqueteEntradaINT = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if(socket_recvPaquete(unPrograma->socketProgramaConsola, paqueteEntradaINT) >= 0){

			t_entrada_numeroKernel* unaEntradaDevueltaINT = (t_entrada_numeroKernel*) malloc(sizeof(t_entrada_numeroKernel));
			unaEntradaDevueltaINT = (t_entrada_numeroKernel*) paqueteEntradaINT->data;

			log_info(self->loggerPlanificador, "Planificador: Recibe un ENTRADA_ESTANDAR_INT: %d", unaEntradaDevueltaINT->numero);

			if(socket_sendPaquete(cpu->socketCPU, ENTRADA_ESTANDAR_INT, sizeof(t_entrada_numeroKernel), unaEntradaDevueltaINT) <= 0)
				log_debug(self->loggerPlanificador, "Planificador: Envia %d a CPU", unaEntradaDevueltaINT->numero);

			socket_freePaquete(paqueteEntradaINT);
			free(unaEntradaDevueltaINT);
		}

		else
			log_error(self->loggerPlanificador, "Planificador: Error al recibir UNA_ENTRADA_STANDAR_TEXT de consola.");

	}

	else if(entradaEstandar->tipo == ENTRADA_ESTANDAR_TEXT){

		socket_sendPaquete(unPrograma->socketProgramaConsola, ENTRADA_ESTANDAR_TEXT, sizeof(t_entrada_estandarKenel), entradaEstandar);

		log_info(self->loggerPlanificador, "Planificador: Envia ENTRADA_ESTANDAR_TEXT a Consola");

		t_socket_paquete *paqueteEntradaTexto = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if(socket_recvPaquete(unPrograma->socketProgramaConsola, paqueteEntradaTexto) >= 0){

			t_entrada_textoKernel* unaEntradaDevueltaTexto = (t_entrada_textoKernel*) malloc(sizeof(t_entrada_textoKernel));
			unaEntradaDevueltaTexto = (t_entrada_textoKernel*) (paqueteEntradaTexto->data);

			log_info(self->loggerPlanificador, "Planificador: Recibe un ENTRADA_ESTANDAR_TEXT: %s", unaEntradaDevueltaTexto->texto);

			if(socket_sendPaquete(cpu->socketCPU, ENTRADA_ESTANDAR_TEXT, sizeof(t_entrada_textoKernel), unaEntradaDevueltaTexto) <= 0)
				log_debug(self->loggerPlanificador, "Planificador: Envia %s a CPU", unaEntradaDevueltaTexto->texto);

			socket_freePaquete(paqueteEntradaTexto);
			free(unaEntradaDevueltaTexto);

		}

		else
			log_error(self->loggerPlanificador, "Planificador: Error al Recibir UNA_ENTRADA_STANDAR_TEXT de consola.");
	}


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

	if(socket_sendPaquete(unProgramaSalida->socketProgramaConsola, SALIDA_ESTANDAR, sizeof(t_salida_estandarKernel), salidaEstandar) <= 0)
		log_debug(self->loggerPlanificador, "Planificador: envia UNA_SALIDA_ESTANDAR a la Consola");

	free(salidaEstandar);
}


/***************************************************************************************************\
 *								   --Comienzo Crear Hilo--							                *
\***************************************************************************************************/

//TODO Ver si lo tengo que mandar a la cola NEW y que pase por el loader en vez de mandarlo directo a la cola READY
void ejecutar_UN_CREAR_HILO(t_kernel* self, t_socket_paquete* paquete){

	t_TCBHiloKernel *hiloCreado = (t_TCBHiloKernel*) paquete->data;

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return ((unPrograma->programaTCB->pid == hiloCreado->pid) && (unPrograma->programaTCB->tid == hiloCreado->tid));
	}

	t_programaEnKernel* programaHiloCreado = list_find(listaDeProgramasDisponibles, matchPrograma);

	pthread_mutex_lock(&readyMutex);
	list_add(cola_ready, programaHiloCreado);
	pthread_mutex_unlock(&readyMutex);

	sem_post(&sem_B);
}


/***************************************************************************************************\
 *							       	--Comienzo JOIN--							                    *
\***************************************************************************************************/

void ejecutar_UN_JOIN_HILO(t_kernel* self, t_socket_paquete* paquete){

	t_joinKernel *joinHilos = (t_joinKernel*) paquete->data;

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return ((unPrograma->programaTCB->pid == joinHilos->pid) && (unPrograma->programaTCB->tid == joinHilos->tid_llamador));
	}

	t_programaEnKernel* programaTIDLlamador = list_find(listaDeProgramasDisponibles, matchPrograma);

	pthread_mutex_lock(&blockMutex);
	list_add(cola_block, programaTIDLlamador);
	pthread_mutex_unlock(&blockMutex);


	bool matchHilo(t_BloqueadoPorOtro *hiloBloqueador){
		return ((hiloBloqueador->pid == joinHilos->pid) && (hiloBloqueador->TIDbloqueador == joinHilos->tid_esperar));
	}

	t_BloqueadoPorOtro *bloqueador = list_find(listaBloqueadosPorOtroHilo, matchHilo);

	if(bloqueador == NULL){
		bloqueador->pid = joinHilos->pid;
		bloqueador->TIDbloqueador = joinHilos->tid_esperar;
		bloqueador->hilosBloqueados = list_create();
		list_add(listaBloqueadosPorOtroHilo, bloqueador);
	}

	list_add(bloqueador->hilosBloqueados, joinHilos->tid_llamador);

}


/***************************************************************************************************\
 *								        --Comienzo BLOCK--							                *
\***************************************************************************************************/

void ejecutar_UN_BLOCK_HILO(t_kernel* self, t_socket_paquete* paquete){

	t_bloquearKernel *blockHilo = (t_bloquearKernel*) (paquete->data);

	bool matchPrograma(t_programaEnKernel *unPrograma){
		return ((unPrograma->programaTCB->pid == blockHilo->pid) && (unPrograma->programaTCB->tid == blockHilo->tid) && (unPrograma->programaTCB->km == blockHilo->km));
	}

	t_programaEnKernel* programaABloquear = list_find(listaDeProgramasDisponibles, matchPrograma);

	pthread_mutex_lock(&blockMutex);
	list_add(cola_block, programaABloquear);
	pthread_mutex_unlock(&blockMutex);

	bool matchRecurso(t_recurso *unRecurso){
		return unRecurso->identificador == blockHilo->id_recurso;
	}

	t_recurso *recursoEncontrado = list_find(listaDeRecursos, matchRecurso);

	if(recursoEncontrado == NULL){
		t_recurso *recurso = malloc(sizeof(t_recurso));

		recurso->identificador = blockHilo->id_recurso;
		recurso->listaBloqueados = list_create();
		list_add(listaDeRecursos, recurso);
	}

	else
		list_add(recursoEncontrado->listaBloqueados, programaABloquear);

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

		if(programaADesbloquear != NULL)
			pasarProgramaDeBlockAReady(programaADesbloquear->programaTCB);

	}

}


/***************************************************************************************************\
 *						 --Comienzo Enviar Mensaje de Error a Consola--							    *
\***************************************************************************************************/

void ejecutar_UN_MENSAJE_DE_ERROR(t_kernel* self, t_socket_paquete* paquete){

	char *mensaje = malloc(sizeof(char) * 150);

	t_errorKernel *errorParaConsola = (t_errorKernel*) (paquete->data);

	bool matchProgramaConsola(t_programaEnKernel* programa){
		return (programa->programaTCB->pid == errorParaConsola->pid);
	}

	t_programaEnKernel *programaAEnviar = list_remove_by_condition(listaDeProgramasDisponibles, matchProgramaConsola);

	t_entrada_textoKernel *enviarMensaje = malloc(sizeof(t_entrada_textoKernel)); //Uso la misma estructura que para la ENTRADA_ESTANDAR

	switch(errorParaConsola->identificadorError){

	case ERROR_REGISTRO_DESCONOCIDO:

		mensaje = "Ha ocurrido un error al por un registro desconocido";

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

		mensaje = "Ha ocurrido un error al de Segmentation Fault";

		break;

	default:
		break;
	}

	memset(enviarMensaje->texto, 0, 150);
	memcpy(enviarMensaje->texto, mensaje, strlen(mensaje) + 1);

	if(socket_sendPaquete(programaAEnviar->socketProgramaConsola, MENSAJE_DE_ERROR, sizeof(t_entrada_textoKernel), enviarMensaje) <= 0)
		log_debug(self->loggerPlanificador, "Planificador: Envia ERROR a la Consola");

	free(mensaje);
	free(programaAEnviar);
	free(enviarMensaje);
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
