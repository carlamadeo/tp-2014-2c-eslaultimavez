#ifndef CONSOLAPROGRAMA_H_
#define CONSOLAPROGRAMA_H_
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/socketInBigBang.h>
#include <commons/protocolStructInBigBang.h>

#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define PATH_CONFIG "../config" //Esto hay que borrar luego porque se pasa por variable
#define PATH_LOG "logs/trace.log" //Donde esta el archivo LOG

typedef struct {
	char* ip;
	int port;
} t_info_conexion;

typedef struct {
	int pid;
} t_info_pid;


typedef struct{
	char* codigo;
} t_buffer_ConsolaPrograma;
t_buffer_ConsolaPrograma buffer_programa;
t_info_conexion info_conexion_Loader;





t_log* logger;

t_socket* socketKernel;
int idPrograma;

void consolaCargarConfiguracion(); //Carga la configuracion de la consola
t_buffer_ConsolaPrograma consolaExtraer_programaESO(char *parametro);


#endif /* CONSOLAPROGRAMA_H	_ */
