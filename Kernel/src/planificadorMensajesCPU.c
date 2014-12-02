#include "planificadorMensajesCPU.h"


void agregarEnListaDeCPU(int id,  t_socket* socketCPU){
	t_cpu* unaCpu;
	unaCpu = malloc( sizeof(t_cpu) );
	unaCpu->id = id;
	unaCpu->socket = socketCPU;
	list_add(listaDeCPULibres,unaCpu);
	//free(unaCpu);
}

void ejecutar_CPU_TERMINE_UNA_LINEA (t_kernel* self,t_socket* socketNuevoCliente){
	log_info(self->loggerPlanificador, "Planificador: recibe CPU_TERMINE_UNA_LINEA");
	socket_sendPaquete(socketNuevoCliente, CPU_SEGUI_EJECUTANDO, 0, NULL);
	log_info(self->loggerPlanificador, "Planificador: envia CPU_SEGUI_EJECUTANDO");
}

void ejecutar_UN_CAMBIO_DE_CONTEXTO(t_kernel* self, t_TCB_Kernel* tcb){

	//1) Primer paso, se lo pone a final de READY

	t_programaEnKernel *unProgramaCPU = obtenerProgramaDeReady(tcb);
	list_add(cola_ready,unProgramaCPU);

	//2) Segundo paso, se verifica que la cola de READY no esta vacia

	printf("Que tiene lista: %d", list_size(cola_ready));
	if(list_size(cola_ready)>0){
		//log_info(self->loggerPlanificador, "test2");

		// se remueve el primer elemento de ready
		t_TCB_Kernel* tcbReady = malloc(sizeof(t_TCB_Kernel));
		tcbReady = list_remove(cola_ready, 0); //SE REMUEVE EL PRIMER PROGRAMA DE NEW

		t_QUAMTUM* unQuamtum = malloc(sizeof(t_QUAMTUM));
		unQuamtum->quamtum = self->quamtum;

		//se manda un QUAMTUM a CPU
		socket_sendPaquete(self->socketCPU, QUAMTUM,sizeof(t_QUAMTUM), unQuamtum);
		log_info(self->loggerPlanificador, "Planificador: envia un quamtum: %d", unQuamtum->quamtum);

		//se mande un TCB a CPU
		socket_sendPaquete(self->socketCPU, TCB_NUEVO,sizeof(t_TCB_Kernel), tcbReady);
		log_info(self->loggerPlanificador, "Planificador: envia TCB_NUEVO con PID: %d TID:%d KM:%d", tcbReady->pid,tcbReady->tid,tcbReady->km );


	}else{
		log_info(self->loggerPlanificador, "test3");
		// si no corresponde se queda bloqueado
		log_debug(self->loggerPlanificador, "Planificador: bloqueado, sin Programas Beso. Error, jorge");
		sem_wait(&mutex_new);

	}


	//free(unTCBPadre);
	//socket_freePaquete(paqueteContexto);

}

void ejecutar_UNA_INTERRUPCION(t_kernel* self){

	//1) Primer paso, poner el TCB Recibido en cola BLOCK

	t_socket_paquete *paqueteCPU = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_interrupcionKernel* unaInterrupcion = (t_interrupcionKernel*) malloc(sizeof(t_interrupcionKernel));

	if(socket_recvPaquete(self->socketCPU, paqueteCPU) >= 0){
		unaInterrupcion = (t_interrupcionKernel*) paqueteCPU->data;

		//se lo remueve de la lista listaDeCPULibres
		bool _esTCBBuscado(t_TCB_Kernel* tcbProcesado) {
			return ((tcbProcesado->pid == unaInterrupcion->tcb->pid) && ((tcbProcesado->tid == unaInterrupcion->tcb->tid)));   //ver esta parte importante
		}
		t_TCB_Kernel* unTcbProsesado = list_remove_by_condition(listaDeCPULibres, (void*)_esTCBBuscado);

		//Se borrar el TCB encontrado, hacer funcion que libere recursos
		unTcbProsesado->pid = -1;
		unTcbProsesado->tid = -1;


		//se lo agrega en la lista de Sistem Call
		list_add(listaSystemCall,unTcbProsesado);


	}else{
		unaInterrupcion->tcb = NULL;
		unaInterrupcion->direccion= 0;
		log_error(self->loggerPlanificador, "Planificador: error en recibir una INTERRUPCION" );
	}

	socket_freePaquete(paqueteCPU);
	log_info(self->loggerPlanificador, "Planificador: recibe una INTERRUPCION");
	log_info(self->loggerPlanificador, "Planificador: TCB PID:%d  TCB TID:%d DIRECCION:%d", unaInterrupcion->tcb->pid,unaInterrupcion->tcb->tid,unaInterrupcion->direccion);


	//2) Segundo paso, se busca el TCB kernel en la cola_Block

	bool _esKMKERNEL(t_TCB_Kernel* tcb) {
		return (tcb->km == 1);
	}
	t_TCB_Kernel* tcbKernel = list_remove_by_condition(cola_block, (void*)_esKMKERNEL);


	//3) Tercer paso, se copia los registros del TCB usuario a TBC Kernel

	tcbKernel->pid = unaInterrupcion->tcb->pid;
	tcbKernel->tid = unaInterrupcion->tcb->tid;
	tcbKernel->puntero_instruccion = unaInterrupcion->direccion;
	tcbKernel->registro_de_programacion[0]= unaInterrupcion->tcb->registro_de_programacion[0];
	tcbKernel->registro_de_programacion[1]= unaInterrupcion->tcb->registro_de_programacion[1];
	tcbKernel->registro_de_programacion[2]= unaInterrupcion->tcb->registro_de_programacion[2];
	tcbKernel->registro_de_programacion[3]= unaInterrupcion->tcb->registro_de_programacion[3];

	//4) Cuarto paso, se agrega el TCB kernel en la lista de CPU Libres

	t_cpu* unaCpuKernel;
	unaCpuKernel = malloc( sizeof(t_cpu) );
	unaCpuKernel->id = 712;
	unaCpuKernel->socket = self->socketCPU;
	unaCpuKernel->TCB = tcbKernel;

	//Se tiene que verificar si hay CPUs Libres
	if (list_size(listaDeCPULibres)>0){
		list_add(listaDeCPULibres,unaCpuKernel); //sincronizar
		log_debug(self->loggerPlanificador, "EXEC: Se carga KM=1 en el primer CPU disponible ");
	}else
		log_error(self->loggerPlanificador, "EXEC: error al cargar TCB Kernel NO EXISTES CPUs CONECTADAS");

	//5) Quiento paso, se manda el TCB KERNEL al CPU
	t_TCB_Kernel* tcbKernelaCPU = malloc(sizeof(t_TCB_Kernel));
	tcbKernelaCPU = tcbKernel;

	t_QUAMTUM* unQuamtum = malloc(sizeof(t_QUAMTUM));
	unQuamtum->quamtum = 0;

	//se manda un QUAMTUM a CPU
	socket_sendPaquete(self->socketCPU, QUAMTUM,sizeof(t_QUAMTUM), unQuamtum);
	log_debug(self->loggerPlanificador, "Planificador: envia un quamtum: %d", unQuamtum->quamtum);

	//se manda un TCB a CPU
	socket_sendPaquete(self->socketCPU, TCB_NUEVO,sizeof(t_TCB_Kernel), tcbKernelaCPU);
	log_debug(self->loggerPlanificador, "Planificador: envia TCB_NUEVO con PID: %d TID:%d KM:%d", tcbKernelaCPU->pid,tcbKernelaCPU->tid,tcbKernelaCPU->km);




	//6) Sexto paso, se queda bloqueado esperando al TCB Kernel

	t_socket_paquete *paqueteKernelTCB = (t_socket_paquete*) malloc(sizeof(t_socket_paquete));
	t_TCB_Kernel* tcbKernelModificado = (t_TCB_Kernel*) malloc(sizeof(t_TCB_Kernel));

	if(socket_recvPaquete(self->socketCPU, paqueteKernelTCB) >= 0){
		if(paqueteKernelTCB->header.type == TCB_NUEVO){
			tcbKernelModificado = (t_TCB_Kernel*) paqueteKernelTCB->data;

		}else
			log_error(self->loggerPlanificador, "Planificador: error al recibir el TCB Kernel. ");
	}else
		log_error(self->loggerPlanificador, "Planificador: error al un Paquete de CPU, que contiene al TCB Kernel.");


	//7) Septimo paso, sacar de la lista System Call
	//se lo remueve de la lista listaDeCPULibres
	bool _esTCBSystemCall(t_TCB_Kernel* tcbSystemCall) {
		return ((tcbSystemCall->pid == unaInterrupcion->tcb->pid) && ((tcbSystemCall->tid == unaInterrupcion->tcb->tid)));   //ver esta parte importante
	}
	t_TCB_Kernel* unTcbSystemCall = list_remove_by_condition(listaSystemCall, (void*)_esTCBSystemCall);

	//Se cargan el TCB usuario con los valores del TCB Kernel modificado
	if(unTcbSystemCall != NULL){
		unTcbSystemCall->puntero_instruccion = tcbKernelModificado->puntero_instruccion;
		unTcbSystemCall->cursor_stack = tcbKernelModificado->cursor_stack;
		unTcbSystemCall->registro_de_programacion[0] = tcbKernelModificado->registro_de_programacion[0];
		unTcbSystemCall->registro_de_programacion[1] = tcbKernelModificado->registro_de_programacion[1];
		unTcbSystemCall->registro_de_programacion[2] = tcbKernelModificado->registro_de_programacion[2];
		unTcbSystemCall->registro_de_programacion[3] = tcbKernelModificado->registro_de_programacion[3];

	}else
		log_error(self->loggerPlanificador, "Planificador: error al traer un TCB de la lista listaSystemCall.");

	//por ultimo se lo manda a la cola de READY
	list_add(cola_ready,unTcbSystemCall);


	free(unaInterrupcion);
	free(unaCpuKernel);
	free(tcbKernelaCPU);
	free(unQuamtum);
	socket_freePaquete(paqueteKernelTCB);
	free(tcbKernelModificado);
	free(unTcbSystemCall);
}


void ejecutar_UNA_ENTRADA_STANDAR(t_kernel* self){

	//1) Primer paso, se recibe un PID

	t_socket_paquete *paqueteEntrada = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_entrada_estandarKenel* unaEntrada = (t_entrada_estandarKenel*) malloc(sizeof(t_entrada_estandarKenel));

	if(socket_recvPaquete(self->socketCPU, paqueteEntrada) >= 0){
		unaEntrada = (t_entrada_estandarKenel*) paqueteEntrada->data;

	}else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UNA_ENTRADA_STANDAR.");


	//2) Segundo paso, se busca la consola(programaBeso)

	bool esProgramaEntrada(t_programaEnKernel* programaEnLista){
		return (programaEnLista->programaTCB->pid == unaEntrada->pid);
	}
	t_programaEnKernel* unPrograma = list_find(cola_exec, (void*)esProgramaEntrada);

	//3) Terce paso, se manda un mensaje a la consola

	if (unaEntrada->tipo == ENTRADA_ESTANDAR_INT ){
		socket_sendPaquete(unPrograma->socketProgramaConsola, ENTRADA_ESTANDAR_INT,sizeof(t_entrada_estandarKenel), unaEntrada);
		log_debug(self->loggerPlanificador, "Planificador: envia ENTRADA_ESTANDAR_INT");
	}else
		if(unaEntrada->tipo == ENTRADA_ESTANDAR_TEXT){
			socket_sendPaquete(unPrograma->socketProgramaConsola, ENTRADA_ESTANDAR_TEXT,sizeof(t_entrada_estandarKenel), unaEntrada);
			log_debug(self->loggerPlanificador, "Planificador: envia ENTRADA_ESTANDAR_TEXT ");

		}

	//4) Cuarto paso, se recibe un paquete de consola
	t_socket_paquete *paqueteEntradaDevuelto = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_entrada_estandarKenel* unaEntradaDevuelta = (t_entrada_estandarKenel*) malloc(sizeof(t_entrada_estandarKenel));

	if(socket_recvPaquete(unPrograma->socketProgramaConsola, paqueteEntradaDevuelto) >= 0){
		unaEntradaDevuelta = (t_entrada_estandarKenel*) paqueteEntradaDevuelto->data;

	}else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UNA_ENTRADA_STANDAR de consola.");


	//por ultimo se lo manda a la CPU

	socket_sendPaquete(self->socketCPU, ENTRADA_ESTANDAR,sizeof(unaEntradaDevuelta->tamanio), unaEntradaDevuelta);
	log_debug(self->loggerPlanificador, "Planificador: envia ");


	socket_freePaquete(paqueteEntrada);
	free(unaEntrada);
	free(unaEntradaDevuelta);
	socket_freePaquete(paqueteEntradaDevuelto);
}




void ejecutar_UNA_SALIDA_ESTANDAR(t_kernel* self){

	//1) Primer paso, se recibe un PID y una cadena de texto

	t_socket_paquete *paqueteSalida = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_salida_estandarKernel* unaSalida = (t_salida_estandarKernel*) malloc(sizeof(t_salida_estandarKernel));

	if(socket_recvPaquete(self->socketCPU, paqueteSalida) >= 0){
		unaSalida = (t_salida_estandarKernel*) paqueteSalida->data;

	}else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UNA_SALIDA_ESTANDAR.");


	//2) Segundo paso, se busca la consola(programaBeso)

	bool esProgramaSalida(t_programaEnKernel* programaEnLista){
		return (programaEnLista->programaTCB->pid == unaSalida->pid);
	}
	t_programaEnKernel* unProgramaSalida = list_find(cola_exec, (void*)esProgramaSalida);


	//3) Terce paso, se manda un mensaje a la consola

	socket_sendPaquete(unProgramaSalida->socketProgramaConsola, SALIDA_ESTANDAR,sizeof(t_salida_estandarKernel), unaSalida);
	log_debug(self->loggerPlanificador, "Planificador: envia UNA_SALIDA_ESTANDAR");


	free(unaSalida);
	socket_freePaquete(paqueteSalida);
}




void ejecutar_UN_CREAR_HILO(t_kernel* self){

	//1)Primer paso, recibe un TCB

	t_socket_paquete *paqueteTCBPadre = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_crea_hiloKernel* unTCBPadre = (t_crea_hiloKernel*) malloc(sizeof(t_crea_hiloKernel));

	if(socket_recvPaquete(self->socketCPU, paqueteTCBPadre) >= 0){
		unTCBPadre = (t_crea_hiloKernel*) paqueteTCBPadre->data;

	}else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UN_CREAR_HILO.");



	//2) Segundo paso, crear un TCB hilo

	t_crea_hiloKernelSecundario* tcbHiloSecundario = malloc (sizeof(t_crea_hiloKernelSecundario));
	tcbHiloSecundario->ramaDelHiloPrincipal = unTCBPadre->tcb->pid+1000;
	tcbHiloSecundario->tcb->pid = unTCBPadre->tcb->pid;
	tcbHiloSecundario->tcb->tid = unTCBPadre->tcb->tid;
	tcbHiloSecundario->tcb->km  = unTCBPadre->tcb->km;

	tcbHiloSecundario->tcb->base_segmento_codigo    = unTCBPadre->tcb->base_segmento_codigo;
	tcbHiloSecundario->tcb->tamanio_segmento_codigo = unTCBPadre->tcb->tamanio_segmento_codigo;
	tcbHiloSecundario->tcb->puntero_instruccion     = unTCBPadre->tcb->base_segmento_codigo;

	tcbHiloSecundario->tcb->base_stack = kernelCrearSegmento(self,tcbHiloSecundario->tcb->pid, self->tamanioStack);
	tcbHiloSecundario->tcb->cursor_stack = tcbHiloSecundario->tcb->base_stack;

	tcbHiloSecundario->tcb->registro_de_programacion[0] = unTCBPadre->tcb->registro_de_programacion[0];
	tcbHiloSecundario->tcb->registro_de_programacion[1] = unTCBPadre->tcb->registro_de_programacion[1];
	tcbHiloSecundario->tcb->registro_de_programacion[2] = unTCBPadre->tcb->registro_de_programacion[2];
	tcbHiloSecundario->tcb->registro_de_programacion[3] = unTCBPadre->tcb->registro_de_programacion[3];


	//3) Tercer paso, se manda a Ready para planificarlo

	list_add(cola_ready,tcbHiloSecundario);

	free(tcbHiloSecundario);
	free(unTCBPadre);
	socket_freePaquete(paqueteTCBPadre);
}

void ejecutar_UN_JOIN_HILO(t_kernel* self){

	//1) Primer paso, recibe un TID llamador y un TID espera

	t_socket_paquete *paqueteJOIN = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_joinKernelKernel* esTBCJOIN = (t_joinKernelKernel*) malloc(sizeof(t_joinKernelKernel));

	if(socket_recvPaquete(self->socketCPU, paqueteJOIN) >= 0){
		esTBCJOIN = (t_joinKernelKernel*) paqueteJOIN->data;

	}else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UN_JOIN.");


	//2) Segundo paso,se manda un hilo a block

	bool tidBuscadoLlamador(t_programaEnKernel* programaEnLista){
		return (programaEnLista->programaTCB->pid == esTBCJOIN->tid_llamador);
	}
	t_programaEnKernel* unTidBuscadoLLamador = list_find(cola_ready, (void*)tidBuscadoLlamador);

	list_add(cola_block,unTidBuscadoLLamador);



	//3) Tercer paso,se manda un hilo a exec

	bool tibBuscadoEspera(t_programaEnKernel* programaEnLista){
		return (programaEnLista->programaTCB->pid == esTBCJOIN->tid_esperar);
	}
	t_programaEnKernel* unTidBuscadoEspera = list_find(cola_block, (void*)tibBuscadoEspera);

	list_add(cola_exec,unTidBuscadoEspera);


	//4) Cuarto paso, se espera que el hilo llamador avise que termino de ejecutar
	t_socket_paquete *paqueteEsperar = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));

	if ((socket_recvPaquete(self->socketCPU, paqueteEsperar)) < 0) {
		if (paqueteEsperar->header.type == FIN_DE_ESPERAR){

			//se lo remueve de la lista colaBlock
			bool _esTCBBuscado(t_TCB_Kernel* tcbProcesado) {
				return (tcbProcesado->tid == unTidBuscadoEspera->programaTCB->tid);
			}

			t_TCB_Kernel* unTcbProsesado = list_remove_by_condition(cola_block, (void*)_esTCBBuscado);

			list_add(cola_ready,unTcbProsesado);
		}else
			log_error(self->loggerPlanificador, "Planificador: error al rebicir un FIN_DE_ESPERAR.");
	}else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UN_JOIN de esperar.");

	socket_freePaquete(paqueteEsperar);
	free(esTBCJOIN);
	socket_freePaquete(paqueteJOIN);
}




void ejecutar_UN_BLOK_HILO(t_kernel* self){

	//1) Primer paso, recibe un TCB y un recurso

	t_socket_paquete *paqueteBloquear = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_bloquearKernel* tcbABloquear = (t_bloquearKernel*) malloc(sizeof(t_bloquearKernel));

	if(socket_recvPaquete(self->socketCPU, paqueteBloquear) >= 0){
		tcbABloquear = (t_bloquearKernel*) paqueteBloquear->data;

	}else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UN_BLOK.");


	//2) Segundo paso,se manda el TCB a block

	list_add(cola_block,tcbABloquear->tcb);

	// Tercer paso, se pone en lista de espera el recurso

	list_add(listaDeEsperaRecurso,tcbABloquear->tcb);

	free(tcbABloquear);
	socket_freePaquete(paqueteBloquear);
}

void ejecutar_UN_WAKE_HILO(t_kernel* self){

	//1) Primer paso, recibe un un recurso

	t_socket_paquete *paqueteDespetar = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_despertarKernel* tcbDespertar = (t_despertarKernel*) malloc(sizeof(t_despertarKernel));

	if(socket_recvPaquete(self->socketCPU, paqueteDespetar) >= 0){
		tcbDespertar = (t_despertarKernel*) paqueteDespetar->data;

	}else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UN_BLOK.");


	//2) Segundo paso,revuene el primer elemento de la lista recurso

	bool _esTCBDespertado(t_procesoBloquea* tcbProcesado) {
		return (tcbProcesado->motivo == tcbDespertar->id_recurso);
	}

	t_procesoBloquea* unTcbProsesado = list_remove_by_condition(listaDeEsperaRecurso, (void*)_esTCBDespertado);


	//3)Tercer paso, se remueve el TCB de la cola Block

	bool _esTCBDesbloqueado(t_procesoBloquea* tcbProcesado) {
		return ((tcbProcesado->tcbKernel->pid == unTcbProsesado->tcbKernel->pid)&&(tcbProcesado->tcbKernel->tid == unTcbProsesado->tcbKernel->tid));
	}

	t_procesoBloquea* tcbDesbloqueado = list_remove_by_condition(cola_block, (void*)_esTCBDesbloqueado);


	//4)Cuarto paso, se lo agrega  al final de la cola Ready

	list_add(cola_ready,tcbDesbloqueado);

	free(tcbDespertar);
	socket_freePaquete(paqueteDespetar);
}




void printTCBKernel(t_TCB_Kernel* unTCB){

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

t_programaEnKernel* obtenerProgramaDeReady(t_TCB_Kernel* tcb){

	//se lo remueve de la lista cola Ready
	t_programaEnKernel* unTcbProsesado = malloc(sizeof(t_programaEnKernel));
	printf("condicion PID: %d TID:%d\n",tcb->pid,tcb->tid);
	bool _esTCBBuscadoReady(t_TCB_Kernel* tcbReady) {
		return ((tcbReady->tid == tcb->tid) &&(tcbReady->pid == tcb->pid));
	}

	printf("listaDeProgramasDisponibles cantidad: %d \n ", list_size(listaDeProgramasDisponibles));
	unTcbProsesado = list_remove_by_condition(listaDeProgramasDisponibles, (void*)_esTCBBuscadoReady);

	printf("Planificador: un TCB encontrado en READY: PID:%d  TID:%d \n", unTcbProsesado->programaTCB->pid, unTcbProsesado->programaTCB->tid);
	return unTcbProsesado;
}
