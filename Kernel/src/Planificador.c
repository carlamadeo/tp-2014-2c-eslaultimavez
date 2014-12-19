#include "planificadorMensajesCPU.h"
#include <errno.h>
#include <unistd.h>

int idCPU = 250;
pthread_mutex_t cpuLibreMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cpuOcupadaMutex = PTHREAD_MUTEX_INITIALIZER;

void kernel_comenzar_Planificador(t_kernel* self){


	iretThreadPlanificador = pthread_create(&hiloAtiendeCPU, NULL, (void*) planificadorEscucharConexionesCPU, self);
	if(iretThreadPlanificador) {
		fprintf(stderr,"Error - pthread_create() return code: %d\n",iretThread);
		exit(EXIT_FAILURE);
	}

	iretThreadPlanificador = pthread_create(&hiloMandaEjectutarTCB, NULL, (void*) pasarTCB_Ready_A_Exec, self);
	if(iretThreadPlanificador) {
		fprintf(stderr,"Error - pthread_create() return code: %d\n",iretThread);
		exit(EXIT_FAILURE);
	}

	pthread_join(hiloAtiendeCPU, NULL);
	pthread_join(hiloMandaEjectutarTCB, NULL);

}

void pasarTCB_Ready_A_Exec(t_kernel* self){

	while(1){

		sem_wait(&sem_B);
		sem_wait(&sem_C);

		//Busco si hay algun programa con Prioridad (KM 1)
		bool matchProgramaKM(t_programaEnKernel *programa){
			return (programa->programaTCB->km == 1);
		}

		t_programaEnKernel* programaParaExecKM = list_remove_by_condition(cola_ready, matchProgramaKM);

		if(programaParaExecKM != NULL){

			pthread_mutex_lock(&execMutex);
			list_add(cola_exec, programaParaExecKM); // se agrega el programa en cola EXEC
			pthread_mutex_unlock(&execMutex);

			mostrarHilosEjecutando();
			t_cpu* cpuLibre = list_get(listaDeCPULibres, 0);
			cargarTCBconOtroTCB_VOID(cpuLibre->TCB, programaParaExecKM->programaTCB);
			cpuLibreAOcupada(cpuLibre);

			mandarEjecutarPrograma(self, cpuLibre);
		}


		else if(programaParaExecKM == NULL){

			pthread_mutex_lock(&readyMutex);
			t_programaEnKernel* programaParaExec = list_remove(cola_ready, 0); //se remueve el primer elemento de la cola READY
			pthread_mutex_unlock(&readyMutex);

			if(programaParaExec != NULL){

				pthread_mutex_lock(&execMutex);
				list_add(cola_exec, programaParaExec); // se agrega el programa en cola EXEC
				pthread_mutex_unlock(&execMutex);

				mostrarHilosEjecutando();

				t_cpu* cpuLibre = list_get(listaDeCPULibres, 0);
				cargarTCBconOtroTCB_VOID(cpuLibre->TCB, programaParaExec->programaTCB);
				cpuLibreAOcupada(cpuLibre);

				mandarEjecutarPrograma(self, cpuLibre);
			}

		}
	}

}


//Pregunta si estos son todos los campos!!! Cursos de Stack???
void cargarTCBconOtroTCB_VOID(t_TCB_Kernel* destino, t_TCB_Kernel* origen){

	destino->pid = origen->pid;
	destino->tid = origen->tid;
	destino->km = origen->km;
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

}


void mandarEjecutarPrograma(t_kernel* self, t_cpu* cpuLibre){

	t_TCB_Kernel* unTCBaEXEC = malloc (sizeof(t_TCB_Kernel));
	unTCBaEXEC = cpuLibre->TCB;

	t_QUANTUM* unQuantum = malloc(sizeof(t_QUANTUM));
	unQuantum->quantum = self->quantum;

	//se manda un QUANTUM a CPU
	if(socket_sendPaquete(cpuLibre->socketCPU, QUANTUM, sizeof(t_QUANTUM), unQuantum) < 0)
		log_error(self->loggerPlanificador,"Planificador: Error el enviar un Quantum a CPU");

	if(socket_sendPaquete(cpuLibre->socketCPU, TCB_NUEVO, sizeof(t_TCB_Kernel), unTCBaEXEC) > 0) ////CPU_NUEVO_PCB
		log_info(self->loggerPlanificador,"Planificador: Se envia a ejecutar Hilo { PID: %d, TID: %d, KM: %d } con Quantum: %d", unTCBaEXEC->pid, unTCBaEXEC->tid, unTCBaEXEC->km, unQuantum->quantum);

	else
		log_error(self->loggerPlanificador,"Planificador: Error el enviar un TCB a CPU");

}


void cpuLibreAOcupada(t_cpu *CPU){

	bool matchCPU(t_cpu *unaCPU){
		return unaCPU->id == CPU->id;
	}

	pthread_mutex_lock(&cpuOcupadaMutex);
	t_cpu *cpuBuscada = list_remove_by_condition(listaDeCPULibres,(void*) matchCPU);
	pthread_mutex_unlock(&cpuOcupadaMutex);

	pthread_mutex_lock(&cpuLibreMutex);
	list_add(listaDeCPUExec, cpuBuscada);
	pthread_mutex_unlock(&cpuLibreMutex);
}


void cpuOcupadaALibre(t_cpu *CPU){

	bool matchCPU(t_cpu *unaCPU){
		return unaCPU->id == CPU->id;
	}
	pthread_mutex_lock(&cpuOcupadaMutex);
	t_cpu *cpuBuscada = list_remove_by_condition(listaDeCPUExec,(void*)matchCPU);
	pthread_mutex_unlock(&cpuOcupadaMutex);

	pthread_mutex_lock(&cpuLibreMutex);
	list_add(listaDeCPULibres, cpuBuscada);
	pthread_mutex_unlock(&cpuLibreMutex);

	sem_post(&sem_C);
}



void planificadorEscucharConexionesCPU(t_kernel* self){

	t_socket *socketNuevaConexionCPU;
	//listaCpu =
	fd_set master;   //conjunto maestro de descriptores de fichero
	fd_set read_fds; //conjunto temporal de descriptores de fichero para select()
	int fdmax,i;

	FD_ZERO(&master); /* borra los conjuntos maestro y temporal*/
	FD_ZERO(&read_fds);

	if (!(self->socketCPU = socket_createServer(self->puertoPlanificador)))
		log_error(self->loggerPlanificador, "Planificador: Error al crear un socket para escuchar CPUs. %s", strerror(errno));

	if(!socket_listen(self->socketCPU))
		log_error(self->loggerPlanificador, "Planificador: Error al poner a escuchar al Planificador: %s", strerror(errno));

	log_info(self->loggerPlanificador, "Planificador: Escuchando conexiones entrantes en el puerto: %d", self->puertoPlanificador);

	FD_SET(self->socketCPU->descriptor, &master);
	fdmax = self->socketCPU->descriptor; /* seguir la pista del descriptor de fichero mayor*/


	/* bucle principal*/
	while(1){
		read_fds = master;
		int selectResult = select(fdmax + 1, &read_fds, NULL, NULL, NULL);
		if (selectResult == -1){
			log_error(self->loggerPlanificador, "Planificador: Error en el select del Planificador.");
			exit(1);
		}

		else if (selectResult != 0){

			for(i = 0; i <= fdmax; i++){ //explorar conexiones existentes en busca de datos que leer

				if (FD_ISSET(i, &read_fds)){ //¡¡tenemos datos!!

					if(i == self->socketCPU->descriptor){  //gestionar nuevas conexiones

						if((socketNuevaConexionCPU = socket_acceptClient(self->socketCPU)) == 0)
							log_error(self->loggerPlanificador, "Planificador: Error en el accept Planificador");

						else
							atenderNuevaConexionCPU(self, socketNuevaConexionCPU, &master, &fdmax);
					}

					else{ //sino no es una nueva conexion busca un programa en la lista
						t_cpu* cpuCliente = obtenerCPUSegunDescriptor(self, i);
						atenderCPU(self, cpuCliente, &master);
					}
				}//fin del if FD_ISSET
			}// fin del for de las i
		}//Fin del else grande

	}//fin de while(1)
}


void atenderNuevaConexionCPU(t_kernel* self, t_socket* socketNuevoCliente, fd_set* master, int* fdmax){

	t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));

	if ((socket_recvPaquete(socketNuevoCliente, paquete)) < 0) {
		log_error(self->loggerPlanificador, "Planificador: Conexión cerrada con el CPU.");
		FD_CLR(socketNuevoCliente->descriptor, master);
		close(socketNuevoCliente->descriptor);
	}

	else{
		//Si recibe una señal de la CPU hace el HANDSHAKE
		if (socket_sendPaquete(socketNuevoCliente, HANDSHAKE_PLANIFICADOR, 0, NULL) >= 0){
			idCPU++;
			agregarEnListaDeCPU(self, idCPU, socketNuevoCliente);
		}

		else
			log_error(self->loggerPlanificador, "Planificador: Error al realizar Handshake con la CPU.");
	}

	//Se actualiza del select
	FD_SET(socketNuevoCliente->descriptor, master); /*añadir al conjunto maestro*/
	if (socketNuevoCliente->descriptor > *fdmax)
		*fdmax = socketNuevoCliente->descriptor; /*actualizar el máximo*/

	conexion_cpu(idCPU);

	socket_freePaquete(paquete);
}

void agregarEnListaDeCPU(t_kernel* self, int id, t_socket* socketCPU){

	t_cpu *unaCpu;
	unaCpu = malloc( sizeof(t_cpu) );
	unaCpu->id = id;
	unaCpu->socketCPU = socketCPU;
	unaCpu->TCB = inicializarUnTCB();
	list_add(listaDeCPULibres, unaCpu);

	sem_post(&sem_C);
}

void atenderCPU(t_kernel* self, t_cpu *cpu, fd_set* master){

	t_socket_paquete *paqueteCPUAtendido = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if (socket_recvPaquete(cpu->socketCPU, paqueteCPUAtendido) > 0){

		switch(paqueteCPUAtendido->header.type){

		case TERMINAR_QUANTUM:
			log_info(self->loggerPlanificador, "Planificador: Recibe de CPU \"Fin de Quantum\"");
			ejecutar_TERMINAR_QUANTUM(self, paqueteCPUAtendido);
			cpuOcupadaALibre(cpu);
			break;

		case FINALIZAR_PROGRAMA_EXITO:
			log_info(self->loggerPlanificador, "Planificador: Recibe de CPU \"Finalizacion de programa exitoso\"");
			ejecutar_FINALIZAR_PROGRAMA_EXITO(self, paqueteCPUAtendido);
			//sem_wait(&sem_B);
			cpuOcupadaALibre(cpu);
			break;

		case MENSAJE_DE_ERROR:
			log_info(self->loggerPlanificador, "Planificador: Recibe de CPU \"Mensaje de Error\"");
			ejecutar_UN_MENSAJE_DE_ERROR(self, paqueteCPUAtendido);
			cpuOcupadaALibre(cpu);
			break;

		case INTERRUPCION:
			ejecutar_UNA_INTERRUPCION(self, paqueteCPUAtendido);
			cpuOcupadaALibre(cpu);
			break;

		case TERMINAR_INTERRUPCION:
			ejecutar_FIN_DE_INTERRUPCION(self, paqueteCPUAtendido);
			cpuOcupadaALibre(cpu);
			break;

		case ENTRADA_ESTANDAR:
			ejecutar_UNA_ENTRADA_ESTANDAR(self, cpu, paqueteCPUAtendido);
			break;

		case SALIDA_ESTANDAR:
			ejecutar_UNA_SALIDA_ESTANDAR(self, cpu, paqueteCPUAtendido);
			break;

		case FINALIZAR_HILO_EXITO:
			log_info(self->loggerPlanificador, "Planificador: Recibe de CPU \"Finalizacion de Hilo exitoso\"");
			ejecutar_FINALIZAR_HILO_EXITO(self, paqueteCPUAtendido);
			cpuOcupadaALibre(cpu);
			break;

		case CREAR_HILO:
			ejecutar_UN_CREAR_HILO(self, paqueteCPUAtendido);
			break;

		case JOIN_HILO:
			ejecutar_UN_JOIN_HILO(self, paqueteCPUAtendido);
			break;

		case BLOK_HILO:
			ejecutar_UN_BLOCK_HILO(self, paqueteCPUAtendido);
			break;

		case WAKE_HILO:
			log_info(self->loggerPlanificador, "Planificador: Recibe de CPU \"WAKE\"");
			ejecutar_UN_WAKE_HILO(self, paqueteCPUAtendido);
			break;

		default:
			log_info(self->loggerPlanificador, "Planificador: Se envia un mensaje de error a la Consola");
			ejecutar_UN_MENSAJE_DE_ERROR(self, paqueteCPUAtendido);
			break;
		}
	}

	else{   //fin switch(paqueteCPU->header.type)
		ejecutar_DESCONECTAR_CPU(self, cpu, master);
	}

	socket_freePaquete(paqueteCPUAtendido);
}


void ejecutar_DESCONECTAR_CPU(t_kernel* self, t_cpu* cpu, fd_set* master){

	//1) Primer paso, se elimina de conjunto maestro
	FD_CLR(cpu->socketCPU->descriptor, master);
	close(cpu->socketCPU->descriptor);

	t_cpu* cpuRemovido = NULL;

	//2) Segundo paso, se trae el CPU que se quiere desconectar de la lista de ejecutados
	bool esCpu(t_cpu* cpuEnLista){
		return (cpuEnLista->id == cpu->id);
	}

	sem_wait(&mutex_cpuExec);
	cpuRemovido = list_remove_by_condition(listaDeCPUExec, (void*)esCpu);
	sem_post(&mutex_cpuExec);

	//3) Tercer paso, se pregunta si tiene programa ejecutando
	if(cpuRemovido != NULL){

		bool esPrograma(t_programaEnKernel* programaEnLista){
			return ((programaEnLista->programaTCB->pid == cpuRemovido->TCB->pid) && (programaEnLista->programaTCB->tid == cpuRemovido->TCB->tid));
		}

		sem_wait(&mutex_exec);
		t_programaEnKernel* programaRemovido = list_find(cola_exec, (void*)esPrograma);
		sem_post(&mutex_exec);


		//se le avisa al programa Beso que se desconecto la CPU
		if (socket_sendPaquete(programaRemovido->socketProgramaConsola, ERROR_POR_DESCONEXION_DE_CPU, 0, NULL) >= 0)
			log_error(self->loggerPlanificador, "Planificador: Envia \"Error por desconexion\" a la Consola");
		else
			log_error(self->loggerPlanificador, "Planificador: Envia \"Error por desconexion\" a la Consola");

	}

	else{
		//sino solo borra de la lista de CPUs libres
		sem_wait(&mutex_cpuLibre);
		cpuRemovido = list_remove_by_condition(listaDeCPULibres, (void*)esCpu);
		sem_post(&mutex_cpuLibre);
	}

	sem_wait(&sem_C);

	desconexion_cpu(cpuRemovido->id);
}



t_cpu* obtenerCPUSegunDescriptor(t_kernel* self, int descriptor){

	bool _esCPUDescriptor(t_cpu* cpu) {
		return (cpu->socketCPU->descriptor == descriptor);
	}

	t_cpu* cpuBuscado = list_find(listaDeCPUExec, (void*)_esCPUDescriptor);

	if(cpuBuscado == NULL){
		pthread_mutex_lock(&cpuMutex);
		cpuBuscado = list_find(listaDeCPULibres, (void*)_esCPUDescriptor);
		pthread_mutex_unlock(&cpuMutex);
	}

	//cpuBuscado->socket->descriptor = descriptor;
	return cpuBuscado;
}


t_programaEnKernel* obtenerTCBdeReady(t_kernel* self){

	//sem_wait(&mutex_BloqueoPlanificador);   //desbloquea al planificador!!! ERROR

	if (list_size(cola_new)>0){
		sem_wait(&mutex_new);
		t_programaEnKernel* programa = list_remove(cola_new, 0); //SE REMUEVE EL PRIMER PROGRAMA DE NEW
		sem_post(&mutex_new);

		sem_wait(&mutex_ready);
		list_add(cola_ready, programa);// SE AGREGA UN PROGRAMA EN READY
		sem_post(&mutex_ready);

		mostrarHilosEjecutando();

		return programa;
	}

	return NULL;
}


t_TCB_Kernel* inicializarUnTCB(){
	t_TCB_Kernel* test_TCB = malloc(sizeof(t_TCB_Kernel));

	test_TCB->pid= 0;
	test_TCB->tid= 0;
	test_TCB->km = 0;
	test_TCB->base_segmento_codigo = 0;
	test_TCB->tamanio_segmento_codigo = 0;
	test_TCB->base_stack = 0;
	test_TCB->cursor_stack = 0;
	test_TCB->puntero_instruccion = 0;
	test_TCB->registro_de_programacion[0] = 0;
	test_TCB->registro_de_programacion[1] = 0;
	test_TCB->registro_de_programacion[2] = 0;
	test_TCB->registro_de_programacion[3] = 0;
	test_TCB->registro_de_programacion[4] = 0;
	return test_TCB;
}


void finalizarProgramaEnPlanificacion(t_programaEnKernel* programa){
	sem_wait(&mutex_exit);//BLOQUEO LISTA DE EXIT
	list_add(cola_exit, programa);
	sem_post(&mutex_exit);

	mostrarHilosEjecutando();
}
