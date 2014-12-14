#include "programaBesoKernel.h"
#include "commons/protocolStructInBigBang.h"
#include <errno.h>
#include <sys/socket.h>
#include <sys/sendfile.h>


void consolaHacerConexionconLoader(t_programaBESO* self){

	self->socketKernel = socket_createClient();

	if (self->socketKernel == NULL )
		log_error(self->loggerProgramaBESO, "Consola: Error al crear socket con el Kernel!");

	if (socket_connect(self->socketKernel, self->ipLoader, self->puertoLoader) == 0)
		log_error(self->loggerProgramaBESO, "Consola: Error al conectarse con el Kernel.");

	else{
		log_info(self->loggerProgramaBESO, "Consola: Conectado con el Kernel (IP: %s/Puerto: %d)!", self->ipLoader, self->puertoLoader);
		realizarHandshakeConLoader(self);
	}

}


void realizarHandshakeConLoader(t_programaBESO* self){

	t_socket_paquete *paquete = (t_socket_paquete*) malloc(sizeof(t_socket_paquete));

	if (socket_sendPaquete(self->socketKernel->socket, HANDSHAKE_PROGRAMA, 0, NULL) > 0)
		log_info(self->loggerProgramaBESO, "Consola: Envia al Kernel: HANDSHAKE_PROGRAMA");

	if (socket_recvPaquete(self->socketKernel->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_LOADER)
			log_info(self->loggerProgramaBESO, "Consola: Recibe del Kernel: HANDSHAKE_LOADER");
	}
	else
		log_error(self->loggerProgramaBESO, "Consola: Error al recibir los datos del Kernel.");

}


void consolaComunicacionLoader(t_programaBESO* self, char *parametro){

	int numero;
	char *texto;
	char *numeroEnChar;

	FILE *archivoBeso = fopen(parametro, "r");

	if(archivoBeso == 0){
		log_error(self->loggerProgramaBESO, "Ha ocurrido un problema al abrir el archivo");
		exit(-1);
	}

	fseek(archivoBeso, 0, SEEK_END);	//Me coloco al final del fichero para saber el tamanio
	size_t sizeArchivoBeso = ftell(archivoBeso);
	fseek(archivoBeso, 0, SEEK_SET);	//Me coloco al principio del fichero para leerlo

	off_t offset = 0;
	//int i;
	t_socket_header header;
	header.length = sizeof(header) + sizeArchivoBeso;

	if(send(self->socketKernel->socket->descriptor, &header, sizeof(t_socket_header), 0) == sizeof(t_socket_header) && sendfile(self->socketKernel->socket->descriptor, archivoBeso->_fileno, &offset, sizeArchivoBeso) == sizeArchivoBeso)
		log_info(self->loggerProgramaBESO, "Consola: Los datos del Programa Beso se enviaron correctamente al Kernel");

	else{
		log_error(self->loggerProgramaBESO, "Consola: No se pudo enviar el archivo");
		exit(-1);
	}

	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_datosKernel *datosAKernel = malloc(sizeof(t_datosKernel));
	t_entrada_estandarConsola *recibidoDelKernel = malloc(sizeof(t_entrada_estandarConsola));
	t_datosMostrarConsola *salidaEstandar = malloc(sizeof(t_datosMostrarConsola));
	t_entrada_texto *entradaError = malloc(sizeof(t_entrada_texto));

	datosAKernel->codigoBeso = self->codigo;

	log_info(self->loggerProgramaBESO, "Consola: Espera respuesta del kernel");

	int fin = 0;
	while(fin == 0){

		if (socket_recvPaquete(self->socketKernel->socket, paquete) >= 0){

			switch(paquete->header.type){

			case FINALIZAR_PROGRAMA_EXITO:
				log_info(self->loggerProgramaBESO,"Consola: El programa \"%s\" ha sido ejecutado con exito", parametro);
				fin = 1;
				break;

			case ENTRADA_ESTANDAR_INT:

				recibidoDelKernel = (t_entrada_estandarConsola*) (paquete->data);

				t_entrada_numero *unNumero = malloc(sizeof(t_entrada_numero));

				unNumero->idCPU = recibidoDelKernel->idCPU;

				numeroEnChar = malloc(sizeof(int));
				memset(numeroEnChar, 0, sizeof(int));
				printf("Ingrese ENTER para entrar al modo Entrada Estandar");
				while(getchar() != '\n');
				printf("Ingrese el numero que desea enviar al Kernel: ");
				fgets(numeroEnChar, sizeof(int), stdin);
				unNumero->numero = atoi(numeroEnChar);

				if(socket_sendPaquete(self->socketKernel->socket, ENTRADA_ESTANDAR_INT, sizeof(t_entrada_numero), unNumero) > 0)
					log_info(self->loggerProgramaBESO, "Consola: Envia un numero: %d", unNumero->numero);

				else
					log_info(self->loggerProgramaBESO, "Consola: Error al enviar un numero.");

				free(unNumero);
				free(numeroEnChar);
				//while(getchar() != '\n');
				break;

			case ENTRADA_ESTANDAR_TEXT:

				recibidoDelKernel = (t_entrada_estandarConsola*) (paquete->data);

				t_entrada_texto* unTexto = malloc(sizeof(t_entrada_texto));

				unTexto->idCPU = recibidoDelKernel->idCPU;

				texto = malloc(sizeof(char)*recibidoDelKernel->tamanio + 1);
				memset(texto, 0, recibidoDelKernel->tamanio + 1);
				memset(unTexto->texto, 0, 10);
				printf("Ingrese ENTER para entrar al modo Entrada Estandar");
				while(getchar() != '\n');
				printf("Ingrese el texto que desea enviar al Kernel: ");
				fgets(texto, recibidoDelKernel->tamanio + 1, stdin);

				memcpy(unTexto->texto, texto, recibidoDelKernel->tamanio);

				if(socket_sendPaquete(self->socketKernel->socket, ENTRADA_ESTANDAR_TEXT, sizeof(t_entrada_texto), unTexto) > 0){
					log_info(self->loggerProgramaBESO, "Consola: envia un texto  :%s", unTexto->texto);
					log_info(self->loggerProgramaBESO, "Consola: envia un CPU id :%d", unTexto->idCPU);
				}

				else
					log_info(self->loggerProgramaBESO, "Consola: Error al enviar un texto.");

				free(unTexto);
				free(texto);
				free(recibidoDelKernel);
				//while(getchar() != '\n');
				break;

			case SALIDA_ESTANDAR:

				salidaEstandar = (t_datosMostrarConsola*) (paquete->data);
				printf("Se ha recibido del Kernel: %s\n", salidaEstandar->mensaje);

				free(salidaEstandar);

				break;

			//TODO Esto no se desde donde se manda
			case ERROR_POR_DESCONEXION_DE_CPU:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error del tipo ERROR_POR_DESCONEXION_DE_CPU");
				fin = 1;
				break;

			case ERROR_POR_DESCONEXION_DE_CONSOLA:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error del tipo ERROR_POR_DESCONEXION_DE_CONSOLA");
				fin = 1;
				break;

			case ERROR_POR_EJECUCION_ILICITA:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error del tipo ERROR_POR_EJECUCION_ILICITA");
				fin = 1;
				break;

			case ERROR_POR_CODIGO_INESPERADO:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error del tipo ERROR_POR_CODIGO_INESPERADO");
				fin = 1;
				break;

			case ERROR_REGISTRO_DESCONOCIDO:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error del tipo ERROR_REGISTRO_DESCONOCIDO");
				fin = 1;
				break;

			default:
				entradaError = (t_entrada_texto*) (paquete->data);
				log_error(self->loggerProgramaBESO, "Consola: Se ha recibido un mensaje de error del Kernel: %s.", entradaError->texto);
				fin = 1;

				free(recibidoDelKernel);
				break;

			}// fin del switch

		}
		else{
			log_error(self->loggerProgramaBESO, "Consola: El Kernel ha cerrado la conexion.");
			close(self->socketKernel->socket->descriptor);

			exit(-1);
		}
	}


	log_debug(self->loggerProgramaBESO, "Consola: Finalizando...");
	//free(datosAKernel->codigoBeso);
	free(datosAKernel);
	socket_freePaquete(paquete);
	close(self->socketKernel->socket->descriptor);

}

