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
		log_info(self->loggerProgramaBESO, "Consola: Envia al Kernel: HANDSHAKE_PROGRAMA ");

	if (socket_recvPaquete(self->socketKernel->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_LOADER)
			log_info(self->loggerProgramaBESO, "Consola: Recibe del Kernel: HANDSHAKE_LOADER");
	}
	else
		log_error(self->loggerProgramaBESO, "Consola: Error al recibir los datos del Kernel.");

}

void consolaComunicacionLoader(t_programaBESO* self, char *parametro){
	int num;
	int tamanio;
	char *texto;

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
	t_datosKernel* datosAKernel = malloc(sizeof(t_datosKernel));

	datosAKernel->codigoBeso = self->codigo;

	log_info(self->loggerProgramaBESO, "Consola: Espera respuesta del kernel");

	int fin= 0;
	while(fin == 0){
		if (socket_recvPaquete(self->socketKernel->socket, paquete) >= 0){

			switch(paquete->header.type){

			case FINALIZAR_PROGRAMA_EXITO:
				log_info(self->loggerProgramaBESO,"Consola: FINALIZA CON EXITO, vamos los pibes!!!");
				fin=1;
				break;
			case ENTRADA_ESTANDAR_INT:
				log_info(self->loggerProgramaBESO,"Consola: recibe una ENTRADA_ESTANDAR_INT");
				printf("Ingrese el numero PID del programa: \n");
				scanf("%d", &num);

				t_entrada_numero* unNum = malloc(sizeof(t_entrada_numero));
				unNum->numero= num;

				//se manda un texto al planificador
				socket_sendPaquete(self->socketKernel->socket, QUANTUM, sizeof(t_entrada_numero), unNum);
				log_info(self->loggerProgramaBESO, "Consola: envia un texto: %s", unNum->numero);

				break;
			case ENTRADA_ESTANDAR_TEXT:

				log_info(self->loggerProgramaBESO,"Consola: recibe una ENTRADA_ESTANDAR_TEXT");

				t_socket_paquete *paqueteEntrada = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
				t_entrada_estandarConsola* entradaConsola = malloc(sizeof(t_entrada_estandarConsola));

				if(paquete->header.type == ENTRADA_ESTANDAR_TEXT){

					if(socket_recvPaquete(self->socketKernel->socket, paqueteEntrada) >= 0){
						entradaConsola = (t_entrada_estandarConsola*) paqueteEntrada->data;

						tamanio = entradaConsola->tamanio;

						texto = malloc(sizeof(char)*tamanio + 1);
						memset(texto, 0, tamanio);
						printf("Ingrese lo que desea escribir: \n");
						fgets(texto, tamanio, stdin);


						t_entrada_texto* unTexto = malloc(sizeof(t_entrada_texto));
						unTexto->texto = texto;

						//se manda un texto al planificador
						socket_sendPaquete(self->socketKernel->socket, QUANTUM, sizeof(t_entrada_texto), unTexto);
						log_info(self->loggerProgramaBESO, "Consola: envia un texto: %s", unTexto->texto);

					}else
						log_error(self->loggerProgramaBESO, "Consola: error al rebicir el mensaje UNA_ENTRADA_STANDAR.");
				}else
					log_error(self->loggerProgramaBESO,"Consola: error al recibir el paquete ENTRADA_ESTANDAR_TEXT");


				break;
			case ERROR_POR_TAMANIO_EXCEDIDO:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error por tamaño de segmento excedido");
				break;
			case ERROR_POR_MEMORIA_LLENA:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error por memoria llena");
				break;
			case ERROR_POR_NUMERO_NEGATIVO:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error por solicitar un tamaño de segmento negativo");
				break;
			case ERROR_POR_SEGMENTO_INVALIDO:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error por segmento invalido");
				break;
			case ERROR_POR_SEGMENTATION_FAULT:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error del tipo Segmentation Fault");
				break;
			default:
				log_info(self->loggerProgramaBESO,"Consola: Recibe OK");
				break;

			}// fin del switch

		}
		else{
			log_error(self->loggerProgramaBESO, "Consola: El Kernel ha cerrado la conexion.");
			close(self->socketKernel->socket->descriptor);
			exit(-1);
		}
	}

	free(texto);
	//free(datosAKernel->codigoBeso);
	free(datosAKernel);
	socket_freePaquete(paquete);
	close(self->socketKernel->socket->descriptor);
}

