#ifndef BOOT_H_
#define BOOT_H_

#include "Kernel.h"

typedef struct{
	int pid;
	int tamanio;
}t_datos_aMSP;

typedef struct{
	uint32_t direccionBase;
}t_datos_deMSP;


void hacer_conexion_con_msp(t_kernel* self);
void realizarHandshakeConMSP(t_kernel* self);
void crearTCBKERNEL(t_kernel* self, char* codigoPrograma, int tamanioEnBytes, int pid, int tid);
int pedirBaseAMSP(t_kernel* self,int pid, char* buffer, int tamanio);


#endif /* BOOT_H_ */
