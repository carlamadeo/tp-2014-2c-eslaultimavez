#ifndef MSP_CPU_H_
#define MSP_CPU_H_

#include <semaphore.h>
#include <stdint.h>
#include "commons/socketInBigBang.h"

//CPU envia info para crear segmento
typedef struct{
	int pid;
	int tamanio;
}t_datos_deCPUCrearSegmento;

//CPU envia info para destruir segmento
typedef struct{
	int pid;
	uint32_t direccionBase;
}t_datos_deCPUDestruirSegmento;

//CPU recibe la direccion base al crear el segmento y info de error al destruir segmento
typedef struct{
	uint32_t recibido;
}t_datos_aCPUSegmento;

//CPU envia datos para leer la memoria
typedef struct{
	int pid;
	uint32_t direccionVirtual;
	int tamanio;
}t_datos_deCPULectura;

//CPU recibe el estado (si se pudo leer o hubo error ERROR_POR_SEGMENTATION_FAULT o SIN_ERRORES) y lo leido
typedef struct{
	int estado;
	char *lectura;
}t_datos_aCPULectura;

//CPU envia datos para escribir la memoria
typedef struct{
	int pid;
	uint32_t direccionVirtual;
	char buffer[1000];
	int tamanio;
}t_datos_deCPUEscritura;

//CPU recibe el estado de la solicitud (si se pudo escribir o hubo error ERROR_POR_SEGMENTATION_FAULT o SIN_ERRORES)
typedef struct{
	int estado;
}t_datos_aCPUEscritura;

void* mspLanzarHiloCPU(t_socket  *socketCPU);

void crearSegmentoCPU(t_socket  *socketCPU, t_socket_paquete *paquete);

void destruirSegmentoCPU(t_socket  *socketCPU, t_socket_paquete *paquete);

void escribirMemoriaCPU(t_socket  *socketKernel, t_socket_paquete *paquete);

void leerMemoriaCPU(t_socket  *socketCPU, t_socket_paquete *paquete);

#endif /* CPU_H_ */
