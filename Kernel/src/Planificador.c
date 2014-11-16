#include "Planificador.h"
#include <errno.h>

void kernel_comenzar_Planificador(t_kernel* self){

	planificadorEscuchaCPU(self);
}


void planificadorEscuchaCPU(t_kernel* self){
	t_socket *socketEscucha, *socketNuevaConexion;
	listaCpu = list_create();
	fd_set master;   //conjunto maestro de descriptores de fichero
	fd_set read_fds; //conjunto temporal de descriptores de fichero para select()
	int fdmax,i;

	FD_ZERO(&master); /* borra los conjuntos maestro y temporal*/
	FD_ZERO(&read_fds);

	if (!(socketEscucha = socket_createServer(self->puertoPlanificador))){
		log_error(self->loggerPlanificador, "Planificador: Error al crear un socket para escuchar CPUs. %s", strerror(errno));
	}

	if(!socket_listen(socketEscucha)){
		log_error(self->loggerPlanificador, "Planificador:Error al poner a escuchar al Planificador: %s", strerror(errno));
	}

	log_info(self->loggerPlanificador, "Planificador: Ya se esta escuchando conexiones entrantes en el puerto: %d",self->puertoPlanificador);

	FD_SET(socketEscucha->descriptor, &master);
	fdmax = socketEscucha->descriptor; /* seguir la pista del descriptor de fichero mayor*/

	/* bucle principal*/
	while(1){
		read_fds = master;
		//printf("antes select: %d\n",  1111);
		int selectResult = select(fdmax+1, &read_fds, NULL, NULL, NULL);
		log_info(self->loggerPlanificador,"Planificador: Select= %d",selectResult);
		//printf("after select: %d\n",  1111);
		if (selectResult == -1){
			log_error(self->loggerPlanificador, "Planificador: Error en el select del Planificador.");
			exit(1);
		}else if (selectResult == 0){

		} else{
			for(i = 0; i <= fdmax; i++){ //explorar conexiones existentes en busca de datos que leer
				if (FD_ISSET(i, &read_fds)){ //¡¡tenemos datos!!

					log_info(self->loggerPlanificador," Planificador:Se encontraron datos en el elemento de la lista i=%d, descriptorEscucha=%d",i,socketEscucha->descriptor);


					if(i == socketEscucha->descriptor){  //gestionar nuevas conexiones
						if((socketNuevaConexion = socket_acceptClient(socketEscucha))==0) {
							log_error(self->loggerPlanificador, "Loader: Error en el accep  Planificador");
						}else  {
							log_debug(self->loggerPlanificador, "Loader: ACCEP completo! ");
							atenderNuevaConexionCPU(self, socketNuevaConexion, &master, &fdmax);
						}

					}else{ //sino no es una nueva conexion busca un programa en la lista
						log_debug(self->loggerPlanificador, "Planificador:Mensaje del Programa descriptor= %d.", i);
						t_cpu* cpuCliente = obtenerCPUSegunDescriptor(self,i);
						log_debug(self->loggerPlanificador, "Planificador: Mensaje del CPU.", cpuCliente->id);
						atienderCPU(self,cpuCliente, &master);
					}
				}//fin del if FD_ISSET
			}// fin del for de las i
		}//Fin del else grande

	}//fin de while(1)
}


void atenderNuevaConexionCPU(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master, int* fdmax){

}

void atienderCPU(t_kernel* self,t_cpu* programa, fd_set* master){

}


t_cpu* obtenerCPUSegunDescriptor(t_kernel* self,int descriptor){

	log_info(self->loggerPlanificador,"Planificador:Obteniendo CPU segun el descriptor %d",descriptor);

	bool _esProgramaDescriptor(t_cpu* cpu) {
		return (cpu->socket->descriptor == descriptor);
	}
	t_cpu* cpuBuscado = list_find(listaCpu, (void*)_esProgramaDescriptor);
	log_info(self->loggerPlanificador," Planificador: Se encontro programa %d",cpuBuscado->id);
	return cpuBuscado;
}
