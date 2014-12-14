#ifndef PROGRAMABESOKERNEL_H_
#define PROGRAMABESOKERNEL_H_

#include "ProgramaBeso.h"

#define TAMANIO_MAXIMO 1000

typedef struct{
	char *codigoBeso;
}t_datosKernel;

typedef struct{
	int codigo;
	char mensaje[TAMANIO_MAXIMO];
}t_datosMostrarConsola;


typedef struct{
	int pid;
	int32_t tamanio;
	int tipo;
	int idCPU;
} t_entrada_estandarConsola;


typedef struct{
	int numero;
	int idCPU;
} t_entrada_numero;

typedef struct{
	char texto[TAMANIO_MAXIMO];
	int idCPU;
} t_entrada_texto;

void consolaHacerConexionconLoader(t_programaBESO* self);
void realizarHandshakeConLoader(t_programaBESO* self);
void consolaComunicacionLoader(t_programaBESO* self, char *parametro);

#endif /* PROGRAMABESOKERNEL_H_ */
