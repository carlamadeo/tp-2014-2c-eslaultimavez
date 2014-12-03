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


typedef struct{
	int pid;
	int32_t tamanio;
	int tipo;
} t_entrada_estandarConsola;


typedef struct{
	int numero;
} t_entrada_numero;

typedef struct{
	char* texto;
} t_entrada_texto;

void consolaHacerConexionconLoader(t_programaBESO* self);
void realizarHandshakeConLoader(t_programaBESO* self);
void consolaComunicacionLoader(t_programaBESO* self, char *parametro);

#endif /* PROGRAMABESOKERNEL_H_ */
