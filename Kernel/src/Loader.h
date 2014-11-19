#ifndef LOADER_H_
#define LOADER_H_

#include "Kernel.h"
#include "kernelConfig.h"

pthread_t programaBesoHilo, conexionConMSPHilo;
t_list* listaDeProgramas;

typedef struct{
	char *codigoBeso;
}t_codigoBeso;


typedef struct{
	uint32_t direccionBase;
}t_msp_DireccionBase;


typedef struct {
	t_TCB_Kernel programaTCB;
	t_socket* socketProgramaConsola;
} t_programa;

void kernel_comenzar_Loader(t_kernel* self);

t_programa* obtenerProgramaConsolaSegunDescriptor(t_kernel* self,int descriptor);

void atienderProgramaConsola(t_kernel* self,t_programa* programa, fd_set* master);

void atenderNuevaConexionPrograma(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master, int* fdmax);

t_TCB_Kernel* loaderCrearTCB(t_kernel* self, char *programaBeso, t_socket* socketNuevoCliente, int tamanioBeso);

void loaderEscuchaProgramaBeso(t_kernel* self);

void loaderTrabajaConMSP(t_kernel* self);


#endif /* BOOT_H_ */
