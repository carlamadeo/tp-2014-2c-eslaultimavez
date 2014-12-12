/*
 * kernelMSP.h
 *
 *  Created on: 18/11/2014
 *      Author: utnso
 */

#ifndef KERNELMSP_H_
#define KERNELMSP_H_

#include "kernelConfig.h"

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
} t_destruirSegmento;

typedef struct{
	int estado;
}t_confirmacion;

typedef struct{
	int pid;
	uint32_t direccionVirtual;
	int tamanio;
}t_datos_aMSPLectura;

typedef struct{
	int estado;
	char lectura[1000];
}t_datos_deMSPLectura;


void hacer_conexion_con_msp(t_kernel* self);
void realizarHandshakeConMSP(t_kernel* self);
uint32_t kernelCrearSegmento(t_kernel *self, int pid, int tamanio);
int kernelDestruirSegmento(t_kernel *self, t_TCB_Kernel *tcb, uint32_t direccionVirtual);
int kernelEscribirMemoria(t_kernel* self, int pid, uint32_t direccionVirtual, char *programaBeso, int tamanioBeso, t_socket* socketNuevoCliente);
int kernelLeerMemoria(t_kernel *self, int pid, uint32_t direccionVirtual, char *programa, int tamanio);

#endif /* KERNELMSP_H_ */
