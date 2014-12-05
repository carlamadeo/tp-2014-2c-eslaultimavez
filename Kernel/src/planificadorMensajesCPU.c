#include "planificadorMensajesCPU.h"
#include "kernelMSP.h"
#include "commons/protocolStructInBigBang.h"

//t_list *listaDeProgramasDisponibles;
//t_list* cola_ready;

void agregarEnListaDeCPU(t_kernel* self,int id,  t_socket* socketCPU){
	t_cpu* unaCpu;
	unaCpu = malloc( sizeof(t_cpu) );
	unaCpu->id = id;
	unaCpu->socket = socketCPU;
	list_add(listaDeCPULibres, unaCpu);
	sem_post(&mutex_cpuLibre);
	log_info(self->loggerPlanificador, "Planificador: tiene una nueva CPU con id: %d",unaCpu->id);
	//free(unaCpu);
}

void ejecutar_CPU_TERMINE_UNA_LINEA (t_kernel* self,t_socket* socketNuevoCliente){
	log_info(self->loggerPlanificador, "Planificador: recibe CPU_TERMINE_UNA_LINEA");
	socket_sendPaquete(socketNuevoCliente, CPU_SEGUI_EJECUTANDO, 0, NULL);
	log_info(self->loggerPlanificador, "Planificador: envia CPU_SEGUI_EJECUTANDO");
}

void ejecutar_FINALIZAR_PROGRAMA_EXITO(t_kernel* self, t_socket *socketNuevaConexionCPU){

	log_info(self->loggerPlanificador, "Planificador: recibe un FINALIZAR_PROGRAMA_EXITO" );

	//1) Primer paso, se recibe un TCB

	t_socket_paquete *paqueteFinalizado = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_TCB_Kernel* tcbFinalizado = (t_TCB_Kernel*) malloc(sizeof(t_TCB_Kernel));

	if(socket_recvPaquete(socketNuevaConexionCPU, paqueteFinalizado) >= 0){
		tcbFinalizado = (t_TCB_Kernel*) paqueteFinalizado->data;
		//printTCBKernel(tcbFinalizado);
	}else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir FINALIZAR_PROGRAMA_EXITO.");


	//2) Segundo se obtiene el socket del programaBeso
	//printf("COLA_EXIT:Buscar PID: %d TID: %d\n",tcbFinalizado->pid,tcbFinalizado->tid);

	bool _tcbParaExit(t_programaEnKernel* tcb) {
		return ((tcb->programaTCB->tid == tcbFinalizado->tid) && (tcb->programaTCB->pid == tcbFinalizado->pid));
	}

	//printf("listaDeProgramasDisponibles Cantidad: %d\n", list_size(listaDeProgramasDisponibles));
	t_programaEnKernel* unTcbProcesado = list_find(listaDeProgramasDisponibles, (void*)_tcbParaExit);
	//printf("COLA_EXIT:Buscar PID: %d TID: %d\n",unTcbProcesado->programaTCB->pid,unTcbProcesado->programaTCB->tid);


	//3) Tercero, mando un mensja
	socket_sendPaquete(unTcbProcesado->socketProgramaConsola, FINALIZAR_PROGRAMA_EXITO, 0 , NULL);

	//4) Se carga en la COLA_EXIT
	list_add(cola_exit,unTcbProcesado);
	printf("COLA_EXIT: Guarda PID: %d TID: %d\n",unTcbProcesado->programaTCB->pid,unTcbProcesado->programaTCB->tid);
}


void ejecutar_UN_CAMBIO_DE_CONTEXTO(t_kernel* self, t_socket *socketNuevaConexionCPU){

	//1) Primer paso, se lo pone a final de READY
	t_socket_paquete *paqueteTCB = (t_socket_paquete*) malloc(sizeof(t_socket_paquete));
	t_TCB_Kernel* unTCBNuevo = (t_TCB_Kernel*) malloc(sizeof(t_TCB_Kernel));

	if(socket_recvPaquete(socketNuevaConexionCPU, paqueteTCB) >= 0){

		if(paqueteTCB->header.type == TCB_NUEVO){

			unTCBNuevo = (t_TCB_Kernel*) paqueteTCB->data;
			t_programaEnKernel *unProgramaCPU = obtenerProgramaDeReady(unTCBNuevo);

			if(unProgramaCPU != NULL){

				list_add(cola_ready, unProgramaCPU);

				//2) Segundo paso, se verifica que la cola de READY no esta vacia

				if(list_size(cola_ready) > 0){

					// se remueve el primer elemento de ready
					t_programaEnKernel *unProgramaTcbReady = list_remove(cola_ready, 0); //SE REMUEVE EL PRIMER PROGRAMA DE NEW

					t_QUANTUM* unQuamtum = malloc(sizeof(t_QUANTUM));
					unQuamtum->quantum = self->quantum;

					//se manda un QUANTUM a CPU
					socket_sendPaquete(socketNuevaConexionCPU, QUANTUM, sizeof(t_QUANTUM), unQuamtum);
					log_info(self->loggerPlanificador, "Planificador: envia un quantum: %d", unQuamtum->quantum);

					//se mande un TCB a CPU
					socket_sendPaquete(socketNuevaConexionCPU, TCB_NUEVO,sizeof(t_TCB_Kernel), unProgramaTcbReady->programaTCB);
					log_info(self->loggerPlanificador, "Planificador: envia TCB_NUEVO con PID: %d TID:%d KM:%d", unProgramaTcbReady->programaTCB->pid, unProgramaTcbReady->programaTCB->tid, unProgramaTcbReady->programaTCB->km );

				}

				else{
					// si no corresponde se queda bloqueado
					log_debug(self->loggerPlanificador, "Planificador: bloqueado, sin Programas Beso. Error, jorge");
					sem_wait(&mutex_new);
				}
			}
		}

		else
			log_error(self->loggerPlanificador, "Planificador: Error al recibir de CPU TCB_NUEVO");
	}

	else
		log_error(self->loggerPlanificador, "Planificador: Error al recibir un paquete del CPU");

	//free(unTCBPadre);
	//socket_freePaquete(paqueteContexto);
}

void recibirUnaDireccion(t_kernel* self,t_socket *socketNuevaConexionCPU,t_interrupcionKernel* unaInterripcion){

	t_socket_paquete *paqueteDireInterrupcion = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	//t_quantumCPU* unQuantum =  (t_quantumCPU*) malloc(sizeof(t_quantumCPU));

	if(socket_recvPaquete(socketNuevaConexionCPU, paqueteDireInterrupcion) >= 0){

		t_interrupcionDireccionKernel* unDireInterrupcion = (t_interrupcionDireccionKernel*) paqueteDireInterrupcion->data;
		unaInterripcion->direccion = unDireInterrupcion->direccion;
		//log_info(self->loggerPlanificador, "CPU: recibe una dirrecion %0.8p",unDireInterrupcion);
	}else
		log_error(self->loggerPlanificador, "CPU: error al un paqueteDireInterrupcion");

	free(paqueteDireInterrupcion);

}
void recibirTCB(t_kernel* self,t_socket *socketNuevaConexionCPU,t_interrupcionKernel* unaInterripcion){

	t_socket_paquete *paqueteInterrupcionTCB = (t_socket_paquete*) malloc(sizeof(t_socket_paquete));
	t_TCB_Kernel* unTCBNuevo = (t_TCB_Kernel*) malloc(sizeof(t_TCB_Kernel));

	printf("CPU manda por este descriptor: %d\n",socketNuevaConexionCPU->descriptor);
	if(socket_recvPaquete(socketNuevaConexionCPU, paqueteInterrupcionTCB) >= 0){

		printf("TCB_NUEVO %d \n", paqueteInterrupcionTCB->header.type);   //no Borrar sirve para como debug Jorge
		if(paqueteInterrupcionTCB->header.type == TCB_NUEVO){

			unTCBNuevo = (t_TCB_Kernel*) paqueteInterrupcionTCB->data;
			//printTCBKernel(unTCBNuevo);
			unaInterripcion->tcb = unTCBNuevo;
			recibirUnaDireccion(self,socketNuevaConexionCPU,unaInterripcion);

		}else
			log_error(self->loggerPlanificador, "CPU: error al recibir un TCB Interrupcion.");

	}else
		log_error(self->loggerPlanificador, "CPU: Error al recibir un paqueteInterrupcionTCB");

	free(paqueteInterrupcionTCB);
}


//Pregunta si estos son todos los campos!!! Cursos de Stack???
t_TCB_Kernel* cargarTCBconOtroTCB(t_TCB_Kernel* destino, t_TCB_Kernel* origen){

	destino->pid = origen->pid;
	destino->tid = origen->tid;

	destino->base_stack = origen->base_stack;
	destino->base_segmento_codigo = origen->base_segmento_codigo;
	destino->tamanio_segmento_codigo = origen->tamanio_segmento_codigo;
	destino->cursor_stack = origen->cursor_stack;
	destino->puntero_instruccion = origen->puntero_instruccion;
	destino->registro_de_programacion[0]= origen->registro_de_programacion[0];
	destino->registro_de_programacion[1]= origen->registro_de_programacion[1];
	destino->registro_de_programacion[2]= origen->registro_de_programacion[2];
	destino->registro_de_programacion[3]= origen->registro_de_programacion[3];
	destino->registro_de_programacion[4]= origen->registro_de_programacion[4];

	return destino;
}


void enviarTCByQUANTUMCPU(t_kernel* self,t_socket *socketNuevaConexionCPU,t_TCB_Kernel* tcbKernel){

	t_QUANTUM* unQuan = malloc(sizeof(t_QUANTUM));
	unQuan->quantum = self->quantum;
	//se manda un QUANTUM a CPU
	socket_sendPaquete(socketNuevaConexionCPU, QUANTUM , sizeof(t_QUANTUM), unQuan);
	log_debug(self->loggerPlanificador, "Planificador: envia un quantum: %d", self->quantum);

	//se manda un TCB a CPU
	socket_sendPaquete(socketNuevaConexionCPU, TCB_NUEVO,sizeof(t_TCB_Kernel), tcbKernel);
	log_debug(self->loggerPlanificador, "Planificador: envia TCB con PID: %d TID:%d KM:%d", tcbKernel->pid, tcbKernel->tid, tcbKernel->km);


}
void ejecutar_UNA_INTERRUPCION(t_kernel* self,t_socket *socketNuevaConexionCPU){

	//1) Primer paso, poner el TCB Recibido en cola BLOCK

	t_interrupcionKernel* unaInterripcion = malloc(sizeof(t_interrupcionKernel));
	recibirTCB(self,socketNuevaConexionCPU,unaInterripcion);

	//log_info(self->loggerPlanificador, "Planificador: recibe una dirrecion %0.8p",unaInterripcion->direccion);
	//printTCBKernel(unaInterripcion->tcb);

	//se lo remueve de la lista listaDeCPULibres
	bool _esTCBBuscado(t_programaEnKernel* tcbProcesado) {
		return ((tcbProcesado->programaTCB->pid == unaInterripcion->tcb->pid) && ((tcbProcesado->programaTCB->tid == unaInterripcion->tcb->tid)));   //ver esta parte importante
	}

	t_programaEnKernel* unTcbEnReady = malloc(sizeof(t_programaEnKernel));
	unTcbEnReady = list_find(cola_ready, (void*)_esTCBBuscado);

	//		se lo agrega en la lista de Sistem Call
	list_add(listaSystemCall,unTcbEnReady);

	//printTCBKernel(unTcbProsesado->programaTCB);
	cargarTCBconOtroTCB(unTcbEnReady->programaTCB,unaInterripcion->tcb);

	//2) Segundo paso, se busca el TCB kernel en la cola_Block

	bool _esKMKERNEL(t_TCB_Kernel* tcb){
		return (tcb->km == 1);
	}

	t_TCB_Kernel* tcbKernel = list_remove_by_condition(cola_block, (void*)_esKMKERNEL);


	//3) Tercer paso, se copia los registros del TCB usuario a TBC Kernel
	cargarTCBconOtroTCB(tcbKernel,unaInterripcion->tcb);
	tcbKernel->puntero_instruccion = unaInterripcion->direccion;


	//4) Cuarto paso, se agrega el TCB kernel en la lista de CPU Libres
	//Se tiene que verificar si hay CPUs Libres

	if (list_size(listaDeCPULibres) > 0){

		list_add(listaDeCPULibres,tcbKernel); //sincronizar
		log_debug(self->loggerPlanificador, "EXEC: Se carga KM=1 en el primer CPU disponible ");
	}else
		log_error(self->loggerPlanificador, "EXEC: error al cargar TCB Kernel NO EXISTES CPUs CONECTADAS");


	//5) Quinto paso, se manda el TCB KERNEL al CPU

	enviarTCByQUANTUMCPU(self,socketNuevaConexionCPU,tcbKernel);

	log_info(self->loggerPlanificador, "Planificador: Listo TCB KERNEL");
	printTCBKernel(tcbKernel);


	log_info(self->loggerPlanificador, "Planificador: LISTO PARA RECIBIR SYSTEM CALLS" );

	t_socket_paquete *paqueteSYSTEMCALLS = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	socket_recvPaquete(socketNuevaConexionCPU, paqueteSYSTEMCALLS);

	printf("Tipo de Instruccion: %d\n",paqueteSYSTEMCALLS->header.type);

	switch(paqueteSYSTEMCALLS->header.type){
	case ENTRADA_ESTANDAR:
		ejecutar_UNA_ENTRADA_STANDAR(self,socketNuevaConexionCPU);
		break;
	case SALIDA_ESTANDAR:
		ejecutar_UNA_SALIDA_ESTANDAR(self,socketNuevaConexionCPU);
		break;
		break;
	case CREAR_HILO:
		ejecutar_UN_CREAR_HILO(self);
		break;
	case JOIN_HILO:
		ejecutar_UN_JOIN_HILO(self);
		break;
	case BLOK_HILO:
		ejecutar_UN_BLOK_HILO(self);
		break;
	case WAKE_HILO:
		ejecutar_UN_WAKE_HILO(self);
		break;
	default:
		log_error(self->loggerPlanificador, "Planificador:Conexión cerrada con CPU.");
		break;

	}//fin switch(paqueteCPU->header.type)

	/*//6) Sexto paso, se queda bloqueado esperando al TCB Kernel

	t_socket_paquete *paqueteKernelTCB = (t_socket_paquete*) malloc(sizeof(t_socket_paquete));
	t_TCB_Kernel* tcbKernelModificado = (t_TCB_Kernel*) malloc(sizeof(t_TCB_Kernel));

	if(socket_recvPaquete(self->socketCPU, paqueteKernelTCB) >= 0){

		if(paqueteKernelTCB->header.type == TCB_NUEVO){
			tcbKernelModificado = (t_TCB_Kernel*) paqueteKernelTCB->data;

		}

		else
			log_error(self->loggerPlanificador, "Planificador: error al recibir el TCB Kernel. ");
	}

	else
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
		unTcbSystemCall->registro_de_programacion[4] = tcbKernelModificado->registro_de_programacion[4];
	}

	else
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
	 */
}

void printfEntradaStandar(t_entrada_estandarKenel* entrada){
	printf("entrada pid: %d\n",entrada->pid);
	printf("entrada tamanio: %d\n",entrada->tamanio);
	printf("entrada tipo: %d\n",entrada->tipo);

}
void ejecutar_UNA_ENTRADA_STANDAR(t_kernel* self, t_socket *socketNuevaConexionCPU){

	//1) Primer paso, se recibe un PID

	t_socket_paquete *paqueteEntrada = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_entrada_estandarKenel* unaEntrada = (t_entrada_estandarKenel*) malloc(sizeof(t_entrada_estandarKenel));

	if(socket_recvPaquete(socketNuevaConexionCPU, paqueteEntrada) >= 0){
		unaEntrada = (t_entrada_estandarKenel*) paqueteEntrada->data;

	}else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UNA_ENTRADA_STANDAR.");


	//2) Segundo paso, se busca la consola(programaBeso)
	bool esProgramaEntrada(t_programaEnKernel* programaEnLista){
		return (programaEnLista->programaTCB->pid == unaEntrada->pid);
	}

	t_programaEnKernel* unPrograma = list_find(listaDeProgramasDisponibles, (void*)esProgramaEntrada);

	//3) Tercer paso, se manda un mensaje a la consola
	log_info(self->loggerPlanificador, "Planificador: valores de la entrada:");
	printfEntradaStandar(unaEntrada);

	if (unaEntrada->tipo == ENTRADA_ESTANDAR_INT ){
		socket_sendPaquete(unPrograma->socketProgramaConsola, ENTRADA_ESTANDAR_INT, sizeof(t_entrada_estandarKenel), unaEntrada);
		log_info(self->loggerPlanificador, "Planificador: envia ENTRADA_ESTANDAR_INT");



		//4) Cuarto paso, se recibe un paquete de consola
		t_socket_paquete *paqueteEntradaINT = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
		t_entrada_numeroKernel* unaEntradaDevueltaINT = (t_entrada_numeroKernel*) malloc(sizeof(t_entrada_numeroKernel));

		if(socket_recvPaquete(unPrograma->socketProgramaConsola, paqueteEntradaINT) >= 0){
			unaEntradaDevueltaINT = (t_entrada_numeroKernel*) paqueteEntradaINT->data;
			log_info(self->loggerPlanificador, "Planificador: recibe un ENTRADA_ESTANDAR_TEXT: %d ",unaEntradaDevueltaINT->numero);
		}
		else
			log_error(self->loggerPlanificador, "Planificador: error al rebicir UNA_ENTRADA_STANDAR_TEXT de consola.");


	}

	else
		if(unaEntrada->tipo == ENTRADA_ESTANDAR_TEXT){
			socket_sendPaquete(unPrograma->socketProgramaConsola, ENTRADA_ESTANDAR_TEXT, 0, NULL);
			socket_sendPaquete(unPrograma->socketProgramaConsola, ENTRADA_ESTANDAR_TEXT, sizeof(t_entrada_estandarKenel), unaEntrada);
			log_info(self->loggerPlanificador, "Planificador: Envia ENTRADA_ESTANDAR_TEXT ");

			//4) Cuarto paso, se recibe un paquete de consola
			t_socket_paquete *paqueteEntradaTexto = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
			t_entrada_textoKernel* unaEntradaDevueltaTexto = (t_entrada_textoKernel*) malloc(sizeof(t_entrada_textoKernel));

			if(socket_recvPaquete(unPrograma->socketProgramaConsola, paqueteEntradaTexto) >= 0){
				unaEntradaDevueltaTexto = (t_entrada_textoKernel*) paqueteEntradaTexto->data;
				log_info(self->loggerPlanificador, "Planificador: Recibe un ENTRADA_ESTANDAR_TEXT: %s ", unaEntradaDevueltaTexto->texto);

				//por ultimo se lo manda a la CPU
				socket_sendPaquete(socketNuevaConexionCPU, ENTRADA_ESTANDAR, sizeof(unaEntradaDevueltaTexto->texto), unaEntradaDevueltaTexto);
				log_debug(self->loggerPlanificador, "Planificador: Envia %s a CPU %s", unaEntradaDevueltaTexto->texto);

				socket_freePaquete(paqueteEntrada);

				free(unaEntradaDevueltaTexto);

			}

			else
				log_error(self->loggerPlanificador, "Planificador: error al rebicir UNA_ENTRADA_STANDAR_TEXT de consola.");

		}

		else
			log_error(self->loggerPlanificador, "Planificador: error en el tipo ENTRADA_ESTANDAR_TEXT ");

	free(unaEntrada);

	/*	//por ultimo se lo manda a la CPU
	socket_sendPaquete(self->socketCPU, ENTRADA_ESTANDAR,sizeof(unaEntradaDevuelta->tamanio), unaEntradaDevuelta);
	log_debug(self->loggerPlanificador, "Planificador: envia ");

	socket_freePaquete(paqueteEntrada);
	free(unaEntrada);
	free(unaEntradaDevuelta);
	socket_freePaquete(paqueteEntradaDevuelto);
	 */
}




void ejecutar_UNA_SALIDA_ESTANDAR(t_kernel* self, t_socket *socketNuevaConexionCPU){

	//1) Primer paso, se recibe un PID y una cadena de texto

	t_socket_paquete *paqueteSalida = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_salida_estandarKernel* unaSalida = (t_salida_estandarKernel*) malloc(sizeof(t_salida_estandarKernel));

	if(socket_recvPaquete(socketNuevaConexionCPU, paqueteSalida) >= 0)
		unaSalida = (t_salida_estandarKernel*) paqueteSalida->data;

	else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UNA_SALIDA_ESTANDAR.");

	//2) Segundo paso, se busca la consola(programaBeso)
	bool esProgramaSalida(t_programaEnKernel* programaEnLista){
		return (programaEnLista->programaTCB->pid == unaSalida->pid);
	}

	t_programaEnKernel* unProgramaSalida = list_find(listaDeProgramasDisponibles, (void*)esProgramaSalida);

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

	if(socket_recvPaquete(self->socketCPU, paqueteTCBPadre) >= 0)
		unTCBPadre = (t_crea_hiloKernel*) paqueteTCBPadre->data;

	else
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
	tcbHiloSecundario->tcb->registro_de_programacion[4] = unTCBPadre->tcb->registro_de_programacion[4];

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

	if(socket_recvPaquete(self->socketCPU, paqueteJOIN) >= 0)
		esTBCJOIN = (t_joinKernelKernel*) paqueteJOIN->data;

	else
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
		}

		else
			log_error(self->loggerPlanificador, "Planificador: error al rebicir un FIN_DE_ESPERAR.");
	}

	else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UN_JOIN de esperar.");

	socket_freePaquete(paqueteEsperar);
	free(esTBCJOIN);
	socket_freePaquete(paqueteJOIN);
}




void ejecutar_UN_BLOK_HILO(t_kernel* self){

	//1) Primer paso, recibe un TCB y un recurso

	t_socket_paquete *paqueteBloquear = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_bloquearKernel* tcbABloquear = (t_bloquearKernel*) malloc(sizeof(t_bloquearKernel));

	if(socket_recvPaquete(self->socketCPU, paqueteBloquear) >= 0)
		tcbABloquear = (t_bloquearKernel*) paqueteBloquear->data;

	else
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

	if(socket_recvPaquete(self->socketCPU, paqueteDespetar) >= 0)
		tcbDespertar = (t_despertarKernel*) paqueteDespetar->data;

	else
		log_error(self->loggerPlanificador, "Planificador: error al rebicir UN_BLOK.");


	//2) Segundo paso,revuene el primer elemento de la lista recurso

	bool _esTCBDespertado(t_procesoBloquea* tcbProcesado) {
		return (tcbProcesado->motivo == tcbDespertar->id_recurso);
	}

	t_procesoBloquea* unTcbProcesado = list_remove_by_condition(listaDeEsperaRecurso, (void*)_esTCBDespertado);


	//3)Tercer paso, se remueve el TCB de la cola Block

	bool _esTCBDesbloqueado(t_procesoBloquea* tcbProcesado) {
		return ((tcbProcesado->tcbKernel->pid == unTcbProcesado->tcbKernel->pid)&&(tcbProcesado->tcbKernel->tid == unTcbProcesado->tcbKernel->tid));
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

t_programaEnKernel* obtenerProgramaDeReady(t_TCB_Kernel* tcb){

	//se lo remueve de la lista cola Ready
	printf("Condicion de busqueda PID: %d TID: %d\n",tcb->pid,tcb->tid);

	bool _esTCBBuscadoReady(t_programaEnKernel* tcbReady) {
		return ((tcbReady->programaTCB->tid == tcb->tid) && (tcbReady->programaTCB->pid == tcb->pid));
	}

	printf("listaDeProgramasDisponibles Cantidad: %d\n", list_size(listaDeProgramasDisponibles));
	t_programaEnKernel* unTcbProcesado = list_find(listaDeProgramasDisponibles, (void*)_esTCBBuscadoReady);

	if(unTcbProcesado == NULL){
		printf("Planificador: No se encontro el TCB\n");
		return NULL;
	}

	unTcbProcesado->programaTCB = tcb;

	printf("Planificador: Un TCB encontrado en READY: PID:%d  TID:%d \n", unTcbProcesado->programaTCB->pid, unTcbProcesado->programaTCB->tid);
	return unTcbProcesado;
}
