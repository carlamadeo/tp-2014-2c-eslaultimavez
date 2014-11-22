#ifndef LOADER_H_
#define LOADER_H_

#include "kernelConfig.h"

pthread_t programaBesoHilo, conexionConMSPHilo;


typedef struct{
	char *codigoBeso;
}t_codigoBeso;


typedef struct{
	uint32_t direccionBase;
}t_msp_DireccionBase;


void kernel_comenzar_Loader(t_kernel* self);

t_programaEnKernel* obtenerProgramaConsolaSegunDescriptor(t_kernel* self,int descriptor);

void atienderProgramaConsola(t_kernel* self,t_programaEnKernel* programa, fd_set* master);

void atenderNuevaConexionPrograma(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master, int* fdmax);

t_TCB_Kernel* loaderCrearTCB(t_kernel* self, char *programaBeso, t_socket* socketNuevoCliente, int tamanioBeso);

void loaderEscuchaProgramaBeso(t_kernel* self);

void loaderTrabajaConMSP(t_kernel* self);

void loaderValidarEscrituraEnMSP(t_kernel* self, t_socket* socketNuevoCliente, int unaRespuesta);

#endif /* BOOT_H_ */
