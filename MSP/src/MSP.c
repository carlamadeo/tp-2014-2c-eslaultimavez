/*
 ============================================================================
 Name        : MSP.c
 Author      : utnso
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ConfigMSP.h"
#include "Consola.h"
#include <commons/protocolStructInBigBang.h>


t_log *MSPlogger;
pthread_t threadConsola;
int idproc;

int main(void) {

	MSPlogger = log_create("logMSP.log", "MSP", 1, LOG_LEVEL_TRACE);
	cargarConfiguracionMSP("../resources/configMSP.cfg");

	idproc = getpid();
	system("clear");
	printf("************** Iniciado MPS (PID: %d) ***************\n",idproc);

	pthread_create(&threadConsola, NULL, iniciarConsolaMSP, NULL);

	log_info(MSPlogger, "Iniciando consola de la MSP...");


	t_socket_header header;
	fd_set master;
	fd_set read_fds;

	int max_desc = 0;
	int nuevo_sock;
	int listener;
	int i, se_desconecto;
	int fin = false;
	int numeroCPU=1;
	t_hilo hiloCPU[8]; //indica cantidad maxima de hilos cpu.

	log_info(MSPlogger, "MPS: Iniciado.");

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	/****************************** Creacion Listener ****************************************/
	log_info(MSPlogger, "****************** CREACION DEL LISTENER *****************\n");
	// Uso puerto seteado en el archivo de configuracion
	crear_listener(puertoMSP, &listener);

	agregar_descriptor(listener, &master, &max_desc);

	log_info(MSPlogger, "MPS: Esperando conexiones...");
	/***************************** LOGICA PRINCIPAL ********************************/
	while(!fin)
	{

		FD_ZERO (&read_fds);
		read_fds = master;
		if((select(max_desc+1, &read_fds, NULL, NULL, NULL/*&tvDemora*/)) == -1)
				{
					log_error(MSPlogger, "MPS: error en el select()");
				}

				for(i = 0; i <= max_desc; i++)
				{
					//otrosDescriptor = 1;
					if (FD_ISSET(i, &read_fds) )
					{
						if (i == listener)
						{
							/* nueva conexion */
							log_info(MSPlogger, "NUEVA CONEXION");

							aceptar_conexion(&listener, &nuevo_sock);
							//agregar_descriptor(nuevo_sock, &master, &max_desc);
							recibir_header(nuevo_sock, &header, &master, &se_desconecto);

							switch (header.type)
							{
								case 90://INICIO_KERNEL:
									log_info(MSPlogger, "MPS: INICIO KERNEL");
									//kernel(nuevo_sock, &master);
									agregar_descriptor(nuevo_sock, &master, &max_desc); //Aqui agrego a descriptor a principal
									break;
								case 91://NUEVO_CPU:
									log_info(MSPlogger, "MPS: NUEVO CPU");
									log_info(MSPlogger, "Creando hilo para CPU...");
									hiloCPU[numeroCPU].sock=nuevo_sock;
									pthread_create (&hiloCPU[numeroCPU].tid, NULL, (void*) hiloCPU, (void*)&hiloCPU[numeroCPU]);
									//paso descriptor en estructura para que la agregue y la atienda el hilo.
									numeroCPU++;
									//nuevoCPU(nuevo_sock, &master);
									break;

								case 92://NUEVA_CONSOLA:
									log_info(MSPlogger, "MPS: NUEVA CONSOLA");
									nuevaConsola(nuevo_sock, header);

									break;

								default:
									break;
							}
						}
					}

				}
	}



	pthread_exit(threadConsola);
	log_destroy(MSPlogger);
	return EXIT_SUCCESS;

}
