#ifndef PROGRAMABESOKERNEL_H_
#define PROGRAMABESOKERNEL_H_

#include "ProgramaBeso.h"

typedef struct{
	char *codigoBeso;
}t_datosKernel;

typedef struct{
	char *mensaje;
	int codigo;
}t_datosMostrarConsola;

void consolaHacerConexionconLoader(t_programaBESO* self);

void realizarHandshakeConLoader(t_programaBESO* self);

void consolaComunicacionLoader(t_programaBESO* self);

#endif /* PROGRAMABESOKERNEL_H_ */
