#ifndef BOOT_H_
#define BOOT_H_

#include "kernelConfig.h"

void crearTCBKERNEL(t_kernel* self);
int obtenerTamanioArchivo(t_kernel *self, FILE *archivoBesoSystemCall);
char *obtenerCodigoArchivo(t_kernel *self, FILE *archivoBesoSystemCall, int tamanio);
void bootEscribirMemoria(t_kernel* self, int pid, uint32_t direccionVirtual, char *programaBeso, int tamanioBeso);
//int obtenerCodigoBesoSystemCall(t_kernel *self, char *codigoSystemCall);

#endif /* BOOT_H_ */
