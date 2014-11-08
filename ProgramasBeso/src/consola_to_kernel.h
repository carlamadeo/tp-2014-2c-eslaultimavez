#ifndef CONSOLA_TO_LOADER_H_
#define CONSOLA_TO_LOADER_H_

#include "Consola.h"


//Estructura para lo que quieran. Poner las structs necesarias.
	typedef struct { //HANDSHAKE_PROGRAMACONSOLA
		char* programaConsola;
		int puertoProgramaConsola;
	} t_info_programa_handshake;

void consolaConectarConElLOADER(t_buffer_ConsolaPrograma programa);
void consolaRealizarHandshakeConLOADER(t_socket_client* socketDelCliente, t_buffer_ConsolaPrograma programa);

#endif /* CONSOLA_TO_LOADER_H_ */
