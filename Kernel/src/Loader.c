#include "Planificador.h"
#include "Loader.h"
#include "commons/protocolStructInBigBang.h"
#include "kernelMSP.h"
#include <errno.h>
#include <unistd.h>

int unPIDGlobal = 1;

pthread_mutex_t readyMutex;
pthread_mutex_t newMutex;
//t_list *listaDeProgramasDisponibles;
//t_list* cola_new;

pthread_mutex_t mutexLoader = PTHREAD_MUTEX_INITIALIZER;

void kernel_comenzar_Loader(t_kernel* self){

	//el primero para crear TCB y cargarlos en la cola_NEW
	iretThreadLoader = pthread_create(&hiloMandarNew, NULL, (void*) escuchar_conexiones_programa, self);
	if(iretThreadLoader)
		log_error(self->loggerLoader, "Loader: Error al crear el hilo hiloMandaeNew");


	//el segundo para sacar de a lista New pasarlo a la cola_READY
	iretThreadLoader = pthread_create(&hiloMandarReady, NULL, (void*) pasarProgramaNewAReady, self);
	if(iretThreadLoader) {
		log_error(self->loggerLoader, "Loader: Error al crear el hilo hiloMandarReady");
	}


	pthread_join(hiloMandarNew, NULL);
	pthread_join(hiloMandarReady, NULL);
}


void escuchar_conexiones_programa(t_kernel* self){

	t_socket *socketEscucha, *socketNuevaConexion;
	fd_set master;   //conjunto maestro de descriptores de fichero
	fd_set read_fds; //conjunto temporal de descriptores de fichero para select()
	int fdmax,i;

	FD_ZERO(&master); /* borra los conjuntos maestro y temporal*/
	FD_ZERO(&read_fds);

	if (!(socketEscucha = socket_createServer(self->puertoLoader)))
		log_error(self->loggerLoader, "Loader: Error al crear socket para escuchar Programas: %s", strerror(errno));


	if(!socket_listen(socketEscucha))
		log_error(self->loggerLoader, "Loader: Error al poner a escuchar al Loader: %s", strerror(errno));

	else
		log_info(self->loggerLoader, "Loader: Escuchando conexiones entrantes en el puerto: %d",self->puertoLoader);

	FD_SET(socketEscucha->descriptor, &master);
	fdmax = socketEscucha->descriptor; /* seguir la pista del descriptor de fichero mayor*/

	/* bucle principal*/
	while(1){

		read_fds = master;
		//printf("antes select: %d\n",  1111);
		int selectResult = select(fdmax+1, &read_fds, NULL, NULL, NULL);
		log_info(self->loggerLoader,"Loader: Select = %d",selectResult);
		//printf("after select: %d\n",  1111);

		if (selectResult == -1){
			log_error(self->loggerLoader, "Error en el select del Loader.");
			exit(1);
		}

		else if (selectResult == 0)
			log_error(self->loggerLoader, "Error en el select del Loader: selectResult==0 ");

		else{

			for(i = 0; i <= fdmax; i++){ //explorar conexiones existentes en busca de datos que leer

				if (FD_ISSET(i, &read_fds)){ //¡¡tenemos datos!!

					log_info(self->loggerLoader,"Se encontraron datos en el elemento de la lista i = %d, descriptorEscucha = %d",i,socketEscucha->descriptor);

					if(i == socketEscucha->descriptor){  //gestionar nuevas conexiones

						if((socketNuevaConexion = socket_acceptClient(socketEscucha)) == 0)
							log_error(self->loggerLoader, "Loader: Error en el accept");

						else{
							log_debug(self->loggerLoader, "Loader: Accept completo!");
							atenderNuevaConexionPrograma(self, socketNuevaConexion, &master, &fdmax);
						}
					}

					else{ //sino no es una nueva conexion busca un programa en la lista
						log_debug(self->loggerLoader, "Loader: Mensaje del Programa descriptor = %d.", i);
						t_programaEnKernel* programaCliente = obtenerProgramaConsolaSegunDescriptor(self, i);
						log_debug(self->loggerLoader, "Loader: Mensaje del Programa PID = %d.", programaCliente->programaTCB->pid);
						atenderProgramaConsola(self, programaCliente, &master);
						//exit(1);
					}
				}//fin del if FD_ISSET
			}// fin del for de las i
		}//Fin del else grande

	}//fin de while(1)
}


void pasarProgramaNewAReady(t_kernel* self){

	log_debug(self->loggerLoader,"New_A_Ready: Comienza a ejecutarse hilo de New a Ready");

	while(1){
		sem_wait(&sem_A);

		t_programaEnKernel* programaParaReady = malloc(sizeof(t_programaEnKernel));

		log_info(self->loggerLoader,"New_A_Ready: Tamanio de la cola New: %d", list_size(cola_new));
		if(list_size(cola_new)>0){
			pthread_mutex_lock(&newMutex);
			programaParaReady = list_remove(cola_new, 0); //se remueve el primer elemento de la cola NEW
			pthread_mutex_unlock(&newMutex);

			log_info(self->loggerLoader,"New_A_Ready: Mueve de New a Ready el proceso con PID:%d TID:%d KM:%d", programaParaReady->programaTCB->pid,programaParaReady->programaTCB->tid,programaParaReady->programaTCB->km);

			pthread_mutex_lock(&readyMutex);
			list_add(cola_ready, programaParaReady); // se agrega el programa buscado a la cola READY
			pthread_mutex_unlock(&readyMutex);

			log_info(self->loggerLoader,"Hilo NEW_to_READY: tamanio de la cola New: %d", list_size(cola_new));
			log_info(self->loggerLoader,"Hilo NEW_to_READY: tamanio de la cola Ready: %d",list_size(cola_ready));
			sem_post(&sem_B);

		}
	}

}


//Busca una conexion ya existente
t_programaEnKernel* obtenerProgramaConsolaSegunDescriptor(t_kernel* self,int descriptor){
	log_info(self->loggerLoader,"Loader: comienza busqueda descriptor");
	log_info(self->loggerLoader,"Loader: Buscando el descriptor %d de una Consola",descriptor);

	bool _esCPUDescriptor(t_programaEnKernel* programaBeso) {
		return (programaBeso->socketProgramaConsola->descriptor == descriptor);
	}

	t_programaEnKernel* descriptorBuscado = list_find(listaDeProgramasDisponibles, (void*)_esCPUDescriptor); //MMM ver esto

	//	if(descriptorBuscado == NULL){
	//		sem_wait(&mutex_cpuLibre);
	//		descriptorBuscado = list_find(listaDeProgramasDisponibles, (void*)_esCPUDescriptor);
	//		sem_post(&mutex_cpuLibre);
	//	}

	log_info(self->loggerLoader,"Loader: Se encontro un Programa con PID: %d TID: %d KM: %d", descriptorBuscado->programaTCB->pid, descriptorBuscado->programaTCB->tid,descriptorBuscado->programaTCB->km);
	//cpuBuscado->socket->descriptor = descriptor;
	log_info(self->loggerLoader,"Loader: termina busqueda descriptor");
	return descriptorBuscado;
}



void atenderProgramaConsola(t_kernel* self, t_programaEnKernel* programa, fd_set* master){

	t_socket_paquete *paquetePrograma = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if ((socket_recvPaquete(programa->socketProgramaConsola, paquetePrograma)) < 0) {

		log_debug(self->loggerLoader, "El programa Beso con PID: %d ha cerrado la conexion.", programa->programaTCB->pid);
		FD_CLR(programa->socketProgramaConsola->descriptor, master); // eliminar del conjunto maestro
		close(programa->socketProgramaConsola->descriptor);

		eliminarPrograma(self, programa);

		log_info(self->loggerLoader,"Loader: Cantidad de TCBs en la cola NEW  :%d", list_size(cola_new));
		log_info(self->loggerLoader,"Loader: Cantidad de TCBs en la cola READY:%d", list_size(cola_ready));
		log_info(self->loggerLoader,"Loader: Cantidad de TCBs en la cola EXEC :%d", list_size(cola_exec));
		log_info(self->loggerLoader,"Loader: Cantidad de TCBs en la cola BLOCK:%d", list_size(cola_block));
		log_info(self->loggerLoader,"Loader: Cantidad de TCBs en la cola EXIT :%d", list_size(cola_exit));

	}

	else if (paquetePrograma->header.type == ENTRADA_ESTANDAR_TEXT){

		log_info(self->loggerLoader,"Loader: recibe un ENTRADA_ESTANDAR_TEXT");

		t_entrada_textoKernel* unaEntradaDevueltaTexto = (t_entrada_textoKernel*) malloc(sizeof(t_entrada_textoKernel));

		unaEntradaDevueltaTexto = (t_entrada_textoKernel*) (paquetePrograma->data);

		log_info(self->loggerLoader,"Loader: recibe un texto de una consola :%s", unaEntradaDevueltaTexto->texto);
		log_info(self->loggerLoader,"Loader: recube una CPU con ID: %d", unaEntradaDevueltaTexto->idCPU);

		t_cpu* cpuTexto = cpuPorIDEncontrado(self, unaEntradaDevueltaTexto->idCPU);

		if(socket_sendPaquete(cpuTexto->socketCPU, ENTRADA_ESTANDAR_TEXT, sizeof(t_entrada_textoKernel), unaEntradaDevueltaTexto) >= 0)
			log_debug(self->loggerPlanificador, "Planificador: Envia %s a CPU", unaEntradaDevueltaTexto->texto);

		free(unaEntradaDevueltaTexto);
	}

	else if (paquetePrograma->header.type == ENTRADA_ESTANDAR_INT){

		t_entrada_numeroKernel* unaEntradaDevueltaINT = (t_entrada_numeroKernel*) malloc(sizeof(t_entrada_numeroKernel));
		unaEntradaDevueltaINT = (t_entrada_numeroKernel*) paquetePrograma->data;

		log_info(self->loggerLoader,"Loader: recibe un int por Consola: %d", unaEntradaDevueltaINT->numero);
		log_info(self->loggerLoader,"Loader: va a enviar al CPU con ID: %d", unaEntradaDevueltaINT->idCPU);

		t_cpu* cpuINT = cpuPorIDEncontrado(self, unaEntradaDevueltaINT->idCPU);

		if(socket_sendPaquete(cpuINT->socketCPU, ENTRADA_ESTANDAR_INT, sizeof(t_entrada_numeroKernel), unaEntradaDevueltaINT) >= 0)
			log_debug(self->loggerPlanificador, "Planificador: Envia %d a CPU", unaEntradaDevueltaINT->numero);

		free(unaEntradaDevueltaINT);

	}//else

	socket_freePaquete(paquetePrograma);

}

void eliminarPrograma(t_kernel* self, t_programaEnKernel* programa){

	bool hayDisponible, hayExec, hayBlock, hayNew, hayReady;
	bool primeraVez = true;

	void eliminarSegmentos(t_programaEnKernel* programaEliminar){
		if(programaEliminar->programaTCB->pid == programa->programaTCB->pid){

			if(primeraVez){
				kernelDestruirSegmento(self, programaEliminar->programaTCB, programaEliminar->programaTCB->base_segmento_codigo);
				primeraVez = false;
			}

			kernelDestruirSegmento(self, programaEliminar->programaTCB, programaEliminar->programaTCB->base_stack);
		}
	}

	list_iterate(listaDeProgramasDisponibles, eliminarSegmentos);

	bool esProgramaDesconectado(t_programaEnKernel* programaEnLista){
		return (programaEnLista->programaTCB->pid == programa->programaTCB->pid) && (programaEnLista->programaTCB->km == 0);
	}

	hayDisponible = list_any_satisfy(listaDeProgramasDisponibles, (void*)esProgramaDesconectado);

	while(hayDisponible){

		pthread_mutex_lock(&programasBesoDisponibleMutex);
		list_remove_by_condition(listaDeProgramasDisponibles, (void*)esProgramaDesconectado);
		pthread_mutex_unlock(&programasBesoDisponibleMutex);

		hayDisponible = list_any_satisfy(listaDeProgramasDisponibles, (void*)esProgramaDesconectado);
	}

	hayExec = list_any_satisfy(cola_exec, (void*)esProgramaDesconectado);

	while(hayExec){

		pthread_mutex_lock(&exitMutex);
		pthread_mutex_lock(&execMutex);
		list_add(cola_exit, list_remove_by_condition(cola_exec, (void*)esProgramaDesconectado));
		pthread_mutex_unlock(&execMutex);
		pthread_mutex_unlock(&exitMutex);

		hayExec = list_any_satisfy(cola_exec, (void*)esProgramaDesconectado);

	}

	hayBlock = list_any_satisfy(cola_block, (void*)esProgramaDesconectado);

	while(hayBlock){

		pthread_mutex_lock(&exitMutex);
		pthread_mutex_lock(&blockMutex);
		list_add(cola_exit, list_remove_by_condition(cola_block, (void*)esProgramaDesconectado));
		pthread_mutex_unlock(&blockMutex);
		pthread_mutex_unlock(&exitMutex);

		hayBlock = list_any_satisfy(cola_block, (void*)esProgramaDesconectado);

	}

	hayNew = list_any_satisfy(cola_new, (void*)esProgramaDesconectado);

	while(hayNew){

		pthread_mutex_lock(&exitMutex);
		pthread_mutex_lock(&newMutex);
		list_add(cola_exit, list_remove_by_condition(cola_new, (void*)esProgramaDesconectado));
		pthread_mutex_unlock(&newMutex);
		pthread_mutex_unlock(&exitMutex);

		hayNew = list_any_satisfy(cola_new, (void*)esProgramaDesconectado);

	}

	hayReady = list_any_satisfy(cola_ready, (void*)esProgramaDesconectado);

	while(hayReady){

		pthread_mutex_lock(&exitMutex);
		pthread_mutex_lock(&newMutex);
		list_add(cola_exit, list_remove_by_condition(cola_ready, (void*)esProgramaDesconectado));
		pthread_mutex_unlock(&newMutex);
		pthread_mutex_unlock(&exitMutex);

		hayReady = list_any_satisfy(cola_new, (void*)esProgramaDesconectado);

	}

}



t_cpu* cpuPorIDEncontrado(t_kernel* self,int idCPU){

	bool esCPU(t_cpu* cpuEntrada){
		return (cpuEntrada->id == idCPU);
	}

	t_cpu* cpuEncontrada = list_find(listaDeCPUExec, (void*)esCPU);
	log_info(self->loggerLoader,"Loader: encuentra CPU con ID: %d",cpuEncontrada->id );
	log_info(self->loggerLoader,"Loader: encuentra CPU con descriptor: %d", cpuEncontrada->socketCPU->descriptor);

	return cpuEncontrada;
}

//Esta funcion tiene que mandar crear los TCB con ayuda de la msp y luego ponerlo en la cola de nuevo

//primero el programaBeso le manda al la Loadear HANDSHAKE_PROGRAMA
//segundo el programaBeso se bloquea esperando una nuespuesta, en este caso el loader manda
//tercero el programa

void atenderNuevaConexionPrograma(t_kernel* self, t_socket* socketNuevoCliente, fd_set* master, int* fdmax){

	t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));

	if ((socket_recvPaquete(socketNuevoCliente, paquete)) < 0) {
		log_error(self->loggerLoader, "Loader: Error o conexión cerrada por el Cliente.");
		FD_CLR(socketNuevoCliente->descriptor, master);
		close(socketNuevoCliente->descriptor);
	}

	else{

		if (socket_sendPaquete(socketNuevoCliente, HANDSHAKE_LOADER, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola HANDSHAKE_LOADER");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar los datos de la Consola.");

		//se recibe el codigo del archivo Beso
		t_socket_header header;

		if(recv(socketNuevoCliente->descriptor, &header, sizeof(t_socket_header), NULL) != sizeof(t_socket_header))
			log_error(self->loggerLoader, "Loader: No se ha podido recibir la informacion de la Consola");

		int sizePrograma = header.length - sizeof(t_socket_header);
		char *programaBeso = malloc(sizePrograma);
		memset(programaBeso, 0, sizePrograma + 1);

		if(recv(socketNuevoCliente->descriptor, programaBeso, sizePrograma, NULL) != sizePrograma)
			log_error(self->loggerLoader, "Loader: No se ha podido recibir el programa Beso de la Consola");

		else
			log_info(self->loggerLoader,"Loader: Recibio un programaBeso de Consola: %s", programaBeso);

		t_TCB_Kernel* unTCBenLoader = loaderCrearTCB(self, programaBeso, socketNuevoCliente, sizePrograma);
		log_info(self->loggerLoader, "Loader: TCB completo.");

		if(unTCBenLoader != NULL){

			//al TCB se lo agrega al final de la Cola NEW con su socket correspondiente
			t_programaEnKernel *unPrograma = malloc(sizeof(t_programaEnKernel));
			unPrograma->programaTCB = unTCBenLoader;
			unPrograma->socketProgramaConsola = socketNuevoCliente;

			log_info(self->loggerPlanificador,"Planificador: Cola NEW tamanio antes de un programa: %d ",list_size(cola_new));

			pthread_mutex_lock(&newMutex);
			list_add(cola_new, unPrograma);
			pthread_mutex_unlock(&newMutex);

			log_info(self->loggerPlanificador,"Planificador: Cola NEW tamanio despues de un programa: %d ",list_size(cola_new));

			list_add(listaDeProgramasDisponibles, unPrograma);

			log_info(self->loggerLoader,"Loader: Agrego un elemento a la Cola New con el PID:%d  TID:%d ", unTCBenLoader->pid, unTCBenLoader->tid);
			//sem_post(&mutex_BloqueoPlanificador);   //bloquea al planificador hasta que la lista sea distinta de new

			//Luego se tiene que actualizar las lista que se usan en el select
			FD_SET(socketNuevoCliente->descriptor, master); /*añadir al conjunto maestro*/

			if (socketNuevoCliente->descriptor > *fdmax) {
				log_info(self->loggerLoader, "Se actualiza y añade el conjunto maestro %d", socketNuevoCliente->descriptor);
				*fdmax = socketNuevoCliente->descriptor; /*actualizar el máximo*/
			}

			sem_post(&sem_A);
		}

		else{
			//se tiene que hacer dos cosas
			//1) avisarle al programa que tiene un error
			log_error(self->loggerLoader, "Loader: Error al crear unTCB.");
			//2) avisar a la MSP que hay un error y destruir segmento
		}

		//free(unPrograma);
		//free(programaBeso);
	}//fin de else

	socket_freePaquete(paquete);
}


//Tiene que hacer el HANDSHAKE con la MSP
//Para luego crear segmento y escribir en memoria

t_TCB_Kernel* loaderCrearTCB(t_kernel* self, char *programaBeso, t_socket* socketNuevoCliente, int tamanioBeso){

	t_TCB_Kernel* unTCB = malloc(sizeof(t_TCB_Kernel));

	unTCB->pid = unPIDGlobal;
	unTCB->km = 0;

	bool contarTID(t_programaEnKernel *unPrograma){
		return (unPrograma->programaTCB->pid == unTCB->pid);
	}

	unTCB->tid = list_count_satisfying(listaDeProgramasDisponibles, contarTID);

	unPIDGlobal ++;

	unTCB->tamanio_segmento_codigo = tamanioBeso;
	unTCB->base_segmento_codigo = kernelCrearSegmento(self, unTCB->pid, tamanioBeso);
	unTCB->puntero_instruccion = unTCB->base_segmento_codigo;
	//funciona de escribir en memoria
	int unaRespuesta = kernelEscribirMemoria(self, unTCB->pid, unTCB->base_segmento_codigo, programaBeso, tamanioBeso);

	//Validar check de ERROR y si hay un error mandar a ProgramaBeso
	loaderValidarEscrituraEnMSP(self, socketNuevoCliente, unaRespuesta);


	unTCB->base_stack = kernelCrearSegmento(self, unTCB->pid, self->tamanioStack);
	unTCB->cursor_stack = unTCB->base_stack;
	log_info(self->loggerLoader,"Loader: La direccion de base de stack es: %0.8p para el PID: %d y TID:%d", self->tcbKernel->base_stack, self->tcbKernel->pid, self->tcbKernel->tid);
	unTCB->registro_de_programacion[0] = 0;
	unTCB->registro_de_programacion[1] = 0;
	unTCB->registro_de_programacion[2] = 0;
	unTCB->registro_de_programacion[3] = 0;
	unTCB->registro_de_programacion[4] = 0;
	return unTCB;
}



void loaderValidarEscrituraEnMSP(t_kernel* self, t_socket* socketNuevoCliente, int unaRespuesta){

	switch(unaRespuesta){

	case ERROR_POR_TAMANIO_EXCEDIDO:
		if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_TAMANIO_EXCEDIDO, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola: ERROR_POR_TAMANIO_EXCEDIDO");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar a Consola: ERROR_POR_TAMANIO_EXCEDIDO");
		break;
	case ERROR_POR_MEMORIA_LLENA:
		if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_MEMORIA_LLENA, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola: ERROR_POR_MEMORIA_LLENA");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar a Consola: ERROR_POR_TAMANIO_EXCEDIDO");
		break;
	case ERROR_POR_NUMERO_NEGATIVO:
		if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_NUMERO_NEGATIVO, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola: ERROR_POR_NUMERO_NEGATIVO");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar a Consola: ERROR_POR_TAMANIO_EXCEDIDO");
		break;
	case ERROR_POR_SEGMENTO_INVALIDO:
		if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_SEGMENTO_INVALIDO, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola: ERROR_POR_SEGMENTO_INVALIDO");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar a Consola: ERROR_POR_TAMANIO_EXCEDIDO");
		break;
	case ERROR_POR_SEGMENTATION_FAULT:
		if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_TAMANIO_EXCEDIDO, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola: ERROR_POR_SEGMENTATION_FAULT");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar a Consola: ERROR_POR_SEGMENTATION_FAULT");
		break;
	default:
		log_info(self->loggerLoader, "Loader: Escribio en la MSP, correctamente");
		break;

	}// fin del switch

}
