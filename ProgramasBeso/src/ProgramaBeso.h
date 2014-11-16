#ifndef PROGRAMABESO_H_
#define PROGRAMABESO_H_

#include "commons/log.h"
#include <commons/socketInBigBang.h>

typedef struct {
	t_socket_client *socketKernel;
	t_log *loggerProgramaBESO;
	int puertoLoader;
	char *ipLoader;
	char *codigo;
} t_programaBESO;


void verificar_argumentosProgramaBeso(int argc, char* argv[]);

t_programaBESO* consolaCargarConfiguracion(char* config_file);

void consolaExtraer_programaBeso (t_programaBESO* self,char *parametro);

#endif /* PROGRAMABESO_H_ */
