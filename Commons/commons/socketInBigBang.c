#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "socketInBigBang.h"

t_socket* socket_Create();
t_socket* socket_getServerFromAddress(struct sockaddr_in socketAddress);


/*
 * @NAME: socket_createServer
 * @DESC: Crea un socket y devuelve un struct con el estado de creacion y el descriptor correspondiente.
 * @PARAMS:port - puerto de escucha
 */
t_socket* socket_createServer(int port)
{
	t_socket* socketServer = NULL;

	struct sockaddr_in socketInfo;
	int optval = 1;

	if ((socketServer = socket_Create()) == NULL) {
		return NULL;
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketServer->descriptor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = INADDR_ANY; //Notar que aca no se usa inet_addr()
	socketInfo.sin_port = htons(port);

	// Vincular el socket con una direccion de red almacenada en 'socketInfo'.
	if (bind(socketServer->descriptor, (struct sockaddr*) &socketInfo, sizeof(socketInfo)) != 0) {
		free(socketServer);
		return NULL;
	}


	//printf("socket_createServer: %d\n", socketServer->descriptor);
	return socketServer;
}

t_socket_client* socket_createClient() {
	t_socket *nuevoSocket;

	if ((nuevoSocket = socket_Create()) == NULL) {
		return NULL;
	}

	t_socket_client *socketCliente = (t_socket_client *)malloc(sizeof(t_socket_client));
	socketCliente->socket = nuevoSocket;
	return socketCliente;
}

/*
 * @NAME: socket_listen
 * @DESC: Pone al socketEscucha a escuchar conexiones entrantes y devuelve 1 si tuvo exito o 0 si no.
 * @PARAMS:
 *              socketEscucha
 */
int socket_listen(t_socket* socketEscucha) {
	// Escuchar nuevas conexiones entrantes.
	if (listen(socketEscucha->descriptor, MAX_CONEXIONES) != 0) {
		return 0;
	}

	return 1;
}

/*
 * @NAME: socket_connect
 * @DESC: Conecta al socket client al servidor en ipServer y portServer
 * @PARAMS:
 *              cliente - socket cliente
 *              ipServer - direccion IP del server al que desea conectarse
 *              portServer - puerto del server al que desea conectarse
 */
int socket_connect(t_socket_client *cliente, char *ipServer, int portServer) {
	struct sockaddr_in socketAddress;

	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr = inet_addr(ipServer);
	socketAddress.sin_port = htons(portServer);

	//printf("programa ip y puerto: %d\n",  portServer);

	// Conectar el socket con la direccion 'socketInfo'.
	if (connect(cliente->socket->descriptor, (struct sockaddr*) &socketAddress, sizeof(socketAddress))
			!= 0) {
		return 0; //false,error value
	}

	cliente->socketServer = socket_getServerFromAddress(socketAddress);

	//printf(" numero importante %d\n",  cliente->socket->descriptor);
	return cliente->socket->descriptor;
}

/*
 * @NAME: socket_acceptClient
 * @DESC: Acepta una conexion entrante y devuelve el descriptor asignado a ese cliente.
 * @PARAMS:
 *              socketEscucha
 */
t_socket* socket_acceptClient(t_socket* socketEscucha) {
	t_socket* socketClient = (t_socket *)malloc(sizeof(t_socket));
	socketClient->address = (struct sockaddr_in *)malloc( sizeof(struct sockaddr_in) );
	int addrlen = sizeof(struct sockaddr_in);

	int socketNuevaConexion;
	// Aceptar una nueva conexion entrante. Se genera un nuevo descriptor con la nueva conexion.
	if ((socketNuevaConexion = accept(socketEscucha->descriptor, (struct sockaddr*) (socketClient->address), (void *)&addrlen)) < 0)
	{
		free(socketClient->address);
		free(socketClient);
		return NULL;
	}

	socketClient->descriptor = socketNuevaConexion;
	return socketClient;
}

/*
 * @NAME: socket_recv
 * @DESC: Recibe un mensaje de un socket cliente y devuelve el buffer donde fue recibido.
 * @PARAMS:
 *              destinatario
 */
t_socket_buffer* socket_recv(t_socket* emisor) {
	t_socket_buffer *buffer = malloc( sizeof(t_socket_buffer) );
	int nbytesRecibidos;

	if ((nbytesRecibidos = recv(emisor->descriptor, buffer->data, BUFF_SIZE, 0)) > 0) {
		buffer->size = nbytesRecibidos;
		return buffer;
	}

	free(buffer);
	return NULL;
}

int socket_recvPaquete(t_socket* emisor, t_socket_paquete* paquete) {

	t_socket_header header;
	//char *buffer;
	void *headerBuffer;
	void *buffer;
	int bytesRecibidos;

	//Primero: Recibir el header para saber cuando ocupa el payload.
	headerBuffer = malloc(3);
	if (recv(emisor->descriptor, headerBuffer, sizeof(t_socket_header), MSG_WAITALL) <= 0)
		return -1;

	memcpy(&(header.type), headerBuffer, 1);
	memcpy(&(header.length), headerBuffer + 1, 2);
	paquete->header = header;

	free(headerBuffer);

	if (header.length > 0){
		// Segundo: Alocar memoria suficiente para el payload.
		//buffer = (char *)malloc(header.length);
		buffer = malloc(header.length);

		// Tercero: Recibir el payload.
		if((bytesRecibidos = recv(emisor->descriptor, buffer, header.length, MSG_WAITALL)) < 0){
			free(buffer);
			return -1;
		}

		paquete->data = malloc(header.length);
		memcpy(paquete->data, buffer, header.length);
		free(buffer);
	}
	else{
		paquete->data = NULL;
		bytesRecibidos = 0;
	}

	return bytesRecibidos;
}

int socket_send(t_socket *destinatario, t_socket_buffer *buffer) {
	if (send(destinatario->descriptor, buffer->data, buffer->size, 0) >= 0) {
		return 1;
	}

	return 0;
}

int socket_sendPaquete(t_socket *destinatario, int tipo, int tamanio, void *contenido) {

	//t_socket_paquete paquete;
	void* paquete;
	paquete = malloc(tamanio + 3);
	memcpy(paquete, &tipo, 1);
	memcpy(paquete + 1, &tamanio, 2);
	if (tamanio > 0){
		memcpy(paquete + 3, contenido, tamanio);
	}

	if (send(destinatario->descriptor, paquete, tamanio + 3, 0) >= 0) {
		free(paquete);
		return 1;
	}

	free(paquete);
	return 0;
}

t_socket* socket_Create() {
	int socketDescriptor;
	t_socket* nuevoSocket = NULL;

	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return NULL;
	}

	nuevoSocket = (t_socket *)malloc(sizeof(t_socket));
	nuevoSocket->descriptor = socketDescriptor;
	return nuevoSocket;
}


t_socket* socket_getServerFromAddress(struct sockaddr_in socketAddress) {
	t_socket *socketServer = (t_socket *)malloc(sizeof(t_socket));

	socketServer->address = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	socketServer->address->sin_family = socketAddress.sin_family;
	socketServer->address->sin_addr.s_addr = socketAddress.sin_addr.s_addr;
	socketServer->address->sin_port = socketAddress.sin_port;

	return socketServer;
}

char* socket_getIP(t_socket* socket) {
	char* ip = inet_ntoa(socket->address->sin_addr);
	//char* ipReturn = (char*)malloc(sizeof(char)*strlen(ip));
	//strcpy(ipReturn, ip);

	char* ipReturn = strdup(ip);
	return ipReturn;
}


void socket_freePaquete(t_socket_paquete* paquete){
	//free(paquete->data);
	free(paquete);
}
