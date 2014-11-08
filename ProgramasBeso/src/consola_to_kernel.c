#include "consola_to_kernel.h"

void consolaConectarConElLOADER(t_buffer_ConsolaPrograma programaESO) {

	t_socket_client *socketDePrograma;

	socketDePrograma = socket_createClient();
	if (socketDePrograma == NULL ) {
		log_error(logger, "Error al crear socket del Proceso Programa!");
	}
	if (!socket_connect( socketDePrograma, info_conexion_Loader.ip, info_conexion_Loader.port)) {
		log_error(logger, "Error al conectar el Proceso Programa con el Proceso Kernel!");
		perror("Error al conectar el Proceso Programa con el Proceso Kernel!");
	}else{
		log_info(logger, "Programa Conectado con el Kernel (IP:%s/Puerto:%d)!", info_conexion_Loader.ip, info_conexion_Loader.port);
		realizarHandshakeConKernel(socketDePrograma,programaESO);
	}
}

void consolaRealizarHandshakeConLOADER(t_socket_client* socketDelCliente, t_buffer_ConsolaPrograma programaESO) {
	t_info_programa_handshake infoProgramaHandshake;

	infoProgramaHandshake.puertoPrograma = info_conexion_Loader.port;

	log_debug(logger, "El Programa solicita conexion.");


	if (socket_sendPaquete(socketDelCliente->socket, 0, sizeof(int),NULL) > 0) {
		log_info(logger, "Programa con el puerto %d se presenta al LOADER!",infoProgramaHandshake.puertoPrograma);
	}

	t_socket_paquete *paquete_handshake = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));

	if (socket_recvPaquete(socketDelCliente->socket, paquete_handshake) >= 0) {
		log_info(logger, "Información recibida de Conexión con el KERNEL.");

		if(paquete_handshake->header.type == 1){
			t_info_pid* datos_recibidos = (t_info_pid*) (paquete_handshake->data);

			log_info(logger, "Se asigno el ID %d!",datos_recibidos->pid);

			idPrograma = datos_recibidos->pid;

			log_info(logger, "vamos a enviar el codigo.");
			log_info(logger, "%s",programaESO.codigo);
			if (socket_sendPaquete(socketDelCliente->socket, 15, programaESO.codigo, programaESO.codigo) > 0) {
					log_info(logger, "Se envio el programa con exito.");
			}else{
				log_error(logger, "No se pudo enviar el codigo %s",buffer_programa.codigo);
			}
		}
	} else {
		log_error(logger, "Error al recibir los datos del KERNEL!");
	}

	free(paquete_handshake);

	while(1){
		t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));

		if ((socket_recvPaquete(socketDelCliente->socket, paquete)) >= 0) {

			if( paquete->header.type == 110 ){ //ERROR_CREACION_SEGMENTO Y/O INTRODUCCION DE BUFFER
					log_info(logger, "Llego un error en la creacion del segmento.");
					printf("Ha habido un error en la creacion del segmento. Se da por terminado el Programa\n");
					close(socketDelCliente->socket->descriptor);
					exit(-1);
			} else if( paquete->header.type == 55 ){ //Imprimir texto
                t_imprimir_texto* texto_datos = malloc(sizeof(t_imprimir_texto));
                texto_datos = (t_imprimir_texto*) (paquete->data);

                if(socket_recvPaquete(socketDelCliente->socket, paquete) >= 0) {
                	if( paquete->header.type == 55 ){
                		texto = (char *) (paquete->data);

						log_info(logger, "Texto a imprimir: %s | %d", texto, texto);
						printf("Texto recibido del kernel: %s\n", texto);
                	}//
                }

                free(texto_datos);
                free(texto);
			} else if( paquete->header.type == 56 ){ //Imprimir numero
                int numero = (int) (paquete->data);

				log_info(logger, "Numero a imprimir: %d", *(int*) numero);
				printf("Numero recibido del kernel: %d\n", *(int*) numero);
			} else if( paquete->header.type == 114 ){
				log_info(logger, "Segmentation fault recibido");

				printf("SEGMENTATION FAULT. Se da por terminado el Programa\n");
				close(socketDelCliente->socket->descriptor);
				exit(-1);
			} else if( paquete->header.type == 1 ){
				log_info(logger, "PROGRAMA FINALIZADO. El programa se cerrara");

				printf("PROGRAMA FINALIZADO. El programa se cerrara\n");
				close(socketDelCliente->socket->descriptor);
				exit(-1);
			}
		} else {
			log_error(logger, "El KERNEL ha cerrado la conexion.");
			close(socketDelCliente->socket->descriptor);
			exit(-1);
		}

		free(paquete);
	}
}
