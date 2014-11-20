#ifndef BOOT_H_
#define BOOT_H_

#include "kernelConfig.h"

void crearTCBKERNEL(t_kernel* self);
int obtenerCogidoBesoSystemCall(t_kernel *self);

#endif /* BOOT_H_ */
