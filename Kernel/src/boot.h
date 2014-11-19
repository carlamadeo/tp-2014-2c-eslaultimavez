#ifndef BOOT_H_
#define BOOT_H_

#include "Kernel.h"

void crearTCBKERNEL(t_kernel* self, char* codigoPrograma, int tamanioEnBytes, int pid, int tid);
int pedirBaseAMSP(t_kernel* self,int pid, char* buffer, int tamanio);


#endif /* BOOT_H_ */
