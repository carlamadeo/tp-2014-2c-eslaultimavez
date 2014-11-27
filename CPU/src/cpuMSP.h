#ifndef CPUMPS_H_
#define CPUMPS_H_

#include <stdint.h>
#include "CPU_Proceso.h"

typedef struct{
	int pid;
	int tamanio;
}t_datos_aMSP;

typedef struct{
	uint32_t direccionBase;
}t_datos_deMSP;

typedef struct{
	int estado;
}t_confirmacion;

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

typedef struct{
	int pid;
	uint32_t direccionVirtual;
} t_destruirSegmento;


void cpuConectarConMPS();
void cpuRealizarHandshakeConMSP();
int cpuCrearSegmento(int pid, int tamanio);
int cpuDestruirSegmento();
int cpuEscribirMemoria(int pid, uint32_t direccionVirtual, char *programa, int tamanio);
//int cpuLeerMemoria(int pid, uint32_t direccionVirtual, char *programa, int tamanio);
int cpuLeerMemoria(t_CPU *self, char *programa, int tamanio);

#endif /* CPUMPS_H_ */
