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
	t_socket_client * socket_ProgramaBESO;
	t_log* loggerProgramaBESO;
	int puertoLoader;
	char* ipLoader;
	char* codigo;
} t_programaBESO;

typedef struct {
	char* ip;
	int port;
} t_info_conexion;

typedef struct {
	int pid;
} t_info_pid;



t_info_conexion info_conexion_Loader;


t_programaBESO*  consolaCargarConfiguracion(char* config_file);
void verificar_argumentosProgramaBeso(int argc, char* argv[]);
void consolaExtraer_programaESO (t_programaBESO* self,char *parametro);

#endif /* CONSOLAPROGRAMA_H	_ */
