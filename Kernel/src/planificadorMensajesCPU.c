#include "planificadorMensajesCPU.h"


void agregarEnListaDeCPU(){

}

void ejecutar_CPU_TERMINE_UNA_LINEA (t_kernel* self,t_socket* socketNuevoCliente){
	log_info(self->loggerPlanificador, "Planificador: recibe CPU_TERMINE_UNA_LINEA");
	socket_sendPaquete(socketNuevoCliente, CPU_SEGUI_EJECUTANDO, 0, NULL);
	log_info(self->loggerPlanificador, "Planificador: envia CPU_SEGUI_EJECUTANDO");
}

void ejecutar_UNA_INTERRUPCION(){

}


void ejecutar_UNA_ENTRADA_STANDAR(){

}

void ejecutar_UNA_SALIDA_ESTANDAR(){

}

void ejecutar_UN_CREAR_HILO(){

}

void ejecutar_UN_JOIN_HILO(){

}

void ejecutar_UN_BLOK_HILO(){

}

void ejecutar_UN_WAKE_HILO(){

}
