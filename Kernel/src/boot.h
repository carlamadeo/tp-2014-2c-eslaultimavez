#ifndef BOOT_H_
#define BOOT_H_


#include "Kernel.h"


void hacer_conexion_con_msp(t_kernel* self);
void realizarHandshakeConMSP(t_kernel* self);
t_programaEnKernel* crearTCB(t_kernel* self, char* codigoPrograma, int tamanioEnBytes, int pid, int tid);
int pedirBaseAMSP(t_kernel* self,int pid, char* buffer, int tamanio);


#endif /* BOOT_H_ */
