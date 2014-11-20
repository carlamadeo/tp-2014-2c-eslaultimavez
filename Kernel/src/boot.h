#ifndef BOOT_H_
#define BOOT_H_

#include "kernelConfig.h"

void crearTCBKERNEL(t_kernel* self);
int obtenerCodigoBesoSystemCall(t_kernel *self, char *codigoSystemCall);

#endif /* BOOT_H_ */
