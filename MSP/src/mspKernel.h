#ifndef MSP_KERNEL_H_
#define MSP_KERNEL_H_

#include <semaphore.h>
#include <stdint.h>
#include "commons/socketInBigBang.h"

//Kernel envia info para crear segmento
typedef struct{
	int pid;
	int tamanio;
}t_datos_deKernelCrearSegmento;

//Kernel envia info para destruir segmento
typedef struct{
	int pid;
	uint32_t direccionBase;
}t_datos_deKernelDestruirSegmento;

//Kernel recibe la direccion base al crear el segmento y info de error al destruir segmento
typedef struct{
	uint32_t recibido;
}t_datos_aKernelSegmento;

//Kernel envia datos para leer la memoria
typedef struct{
	int pid;
	uint32_t direccionVirtual;
	int tamanio;
}t_datos_deKernelLectura;

//Kernel recibe el estado (si se pudo leer o hubo error ERROR_POR_SEGMENTATION_FAULT o SIN_ERRORES) y lo leido
typedef struct{
	int estado;
	char *lectura;
}t_datos_aKernelLectura;

//Kernel envia datos para escribir la memoria
typedef struct{
	int pid;
	uint32_t direccionVirtual;
	char *buffer;
	int tamanio;
}t_datos_deKernelEscritura;

//Kernel recibe el estado de la solicitud (si se pudo escribir o hubo error ERROR_POR_SEGMENTATION_FAULT o SIN_ERRORES)
typedef struct{
	int estado;
}t_datos_aKernelEscritura;

void* mspLanzarHiloKernel(t_socket  *socketKernel);

void crearSegmentoKernel(t_socket  *socketKernel, t_socket_paquete *paquete);

void destruirSegmentoKernel(t_socket  *socketKernel, t_socket_paquete *paquete);

void leerMemoriaKernel(t_socket  *socketKernel, t_socket_paquete *paquete);

#endif /* KERNEL_H_ */
