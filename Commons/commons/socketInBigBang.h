#ifndef SOCKET_H_
#define SOCKET_H_

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_CONEXIONES 10
#define BUFF_SIZE 1024

typedef struct {
	int descriptor;
	struct sockaddr_in *address;
} t_socket;

typedef struct {
	t_socket *socket;
	t_socket *socketServer;
} t_socket_client;

typedef struct {
	char data[BUFF_SIZE];
	int size;
} t_socket_buffer ;

typedef struct {
	int8_t type;
	int16_t length;
} __attribute__((packed)) t_socket_header;

typedef struct {
	t_socket_header header;
	//char data[BUFF_SIZE];
	void* data;
} __attribute__((packed)) t_socket_paquete;

typedef struct { //para hilos
	pthread_t tid;
	int32_t fdPipe[2]; // fdPipe[0] de lectura/ fdPipe[1] de escritura
	t_socket socket;
	int sock;
} t_hiloBigBang;

t_socket* socket_createServer(int port);
t_socket_client* socket_createClient();
int socket_listen(t_socket* socketEscucha);
int socket_connect(t_socket_client *cliente, char *ipServer, int portServer);
t_socket* socket_acceptClient(t_socket* socketEscucha);
t_socket_buffer *socket_recv(t_socket *emisor);
int socket_recvPaquete(t_socket* emisor, t_socket_paquete* paquete);
int socket_send(t_socket *destinatario, t_socket_buffer *buffer);
int socket_sendPaquete(t_socket *destinatario, int tipo, int tamanio, void *contenido);
char* socket_getIP(t_socket* socket);
void socket_freePaquete(t_socket_paquete* paquete);

#endif /* SOCKET_H_ */
