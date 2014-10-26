#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/protocolStructInBigBang.h>
#include "hiloCPU.h"
/*
void* hiloCPU(t_hilo *hiloCPU) {

	t_socket_header header;
	fd_set master;
	fd_set read_fds;

	int max_desc = 0;
	int nuevo_sock;
	int listener;
	int i, se_desconecto;
	int fin = 0;//FALSE;

	log_info(MSPlogger, "HILO CPU: Iniciado.");

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	// Agrego descriptor de comunicacion con plataforma por pipe
	agregar_descriptor(hiloCPU->fdPipe[0], &master, &max_desc);

	/****************************** Creacion Listener ****************************************/
	//crear_listener(PUERTO, &listener);  //ver si es necesario
	//agregar_descriptor(listener, &master, &max_desc);
/*
	agregar_descriptor(hiloCPU->sock, &master, &max_desc);
	
	/***************************** LOGICA PRINCIPAL ********************************/
/*	while(!fin)
	{

		FD_ZERO (&read_fds);
		read_fds = master;

		if((select(max_desc+1, &read_fds, NULL, NULL, NULL&tvDemora)) == -1)
		{
			log_error(MSPlogger, "ORQUESTADOR: error en el select()");
		}

		for(i = 0; i <= max_desc; i++)
		{
			//otrosDescriptor = 1;
			if (FD_ISSET(i, &read_fds) )
			{
				if (i == listener)
				{
					//nueva conexion
					log_info(MSPlogger, "MPS HILO CPU: NUEVA CONEXION");

					aceptar_conexion(&listener, &nuevo_sock);
					//agregar_descriptor(nuevo_sock, &master, &max_desc);

					recibir_header(nuevo_sock, &header, &master, &se_desconecto);

					switch (header.type)
					{
					
						default:
							break;
					}
				}
				else if (i == hiloCPU->fdPipe[0]) {

					log_info(MSPlogger, "MPS HILO CPU: Recibo mensaje desde MPS principal por Pipe");
					initHeader(&header);
					read (hiloCPU->fdPipe[0], &header, sizeof(t_socket_header));

					log_debug(MSPlogger, "MPS HILO CPU: mensaje recibido '%d'", header.type);

					if (header.type == 100FINALIZAR) {
						log_debug(MSPlogger, "MPS HILO CPU: '%d' ES FINALIZAR", header.type);
						fin = 1;//1 = TRUE
						FD_CLR(hiloCPU->fdPipe[0], &master);
						break;
					}

				} 
				else if (i != listener && i != hiloCPU->fdPipe[0]) 
				{

					log_debug(MSPlogger, "MPS HILO CPU: recibo mensaje socket %d", i);
					recibir_header(i, &header, &master, &se_desconecto);
					log_debug(MSPlogger, "MPS HILO CPU: el tipo de mensaje es: %d\n", header.type);

					if(se_desconecto)
					{
						log_info(MSPlogger, "MPS HILO CPU: Se desconecto el socket %d", i);
						// TODO chequear si se desconecto cpu y borrarlo de las estructuras
						// Quito el descriptor del set
						FD_CLR(i, &master);
					}

					if ((header.type == 95DATOS_CPU) && (se_desconecto != 1))
					{
						puts("MPS HILO CPU: Recibo datos de la CPU");

					}

				}
			}
		}
	}
}*/
				
