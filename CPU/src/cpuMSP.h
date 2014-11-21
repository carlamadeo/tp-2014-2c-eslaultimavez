#ifndef CPUMPS_H_
#define CPUMPS_H_

#include "CPU.h"

typedef struct{
	int pid;
	int tamanio;
}t_datos_aMSP;

typedef struct{
	uint32_t direccionBase;
}t_datos_deMSP;

typedef struct{
	int estado;
}t_confirmacionEscritura;

typedef struct {
	int pid;
	int tamanio;
} t_crearSegmentoBeso;

typedef struct {
	int pid;
	uint32_t direccionVirtual;
	char bufferCodigoBeso[1000];
	int tamanio;
} t_escribirSegmentoBeso;

typedef struct{
	int pid;
	uint32_t direccionVirtual;
	int tamanio;
}t_datos_aMSPLectura;

typedef struct{
	int estado;
	char lectura[1000];
}t_datos_deMSPLectura;

t_socket_client* cpuConectarConMPS(t_CPU* self);
void cpuRealizarHandshakeConMSP(t_CPU* self);
int cpuCrearSegmento(t_CPU *self, int pid, int tamanio);
int cpuEscribirMemoria(t_CPU* self, int pid, uint32_t direccionVirtual, char *programaBeso, int tamanioBeso, t_socket* socketNuevoCliente);
int cpuLeerMemoria(t_CPU* self, int pid, uint32_t direccionVirtual, char *programa, int tamanio, t_socket* socketNuevoCliente);

#endif /* CPUMPS_H_ */
