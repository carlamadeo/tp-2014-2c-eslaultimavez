#ifndef CONSOLA_TO_LOADER_H_
#define CONSOLA_TO_LOADER_H_

#include "ProgramaBeso.h"


//Estructura para lo que quieran. Poner las structs necesarias.
	typedef struct { //HANDSHAKE_PROGRAMACONSOLA
		char* programaConsola;
		int puertoProgramaConsola;
	} t_info_programa_handshake;

void consolaConectarConElLOADER(t_programaBESO* self);
void consolaRealizarHandshakeConLOADER(t_programaBESO* self);

#endif /* CONSOLA_TO_LOADER_H_ */
