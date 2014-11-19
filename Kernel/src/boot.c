#include "boot.h"
#include "commons/protocolStructInBigBang.h"
#include "kernelMSP.h"


void crearTCBKERNEL(t_kernel* self, char* codigoPrograma, int tamanioEnBytes, int pid, int tid){

	uint32_t stack;
	t_programaEnKernel* programaEnElKernel = malloc( sizeof(t_programaEnKernel) );
	log_info(self->loggerKernel, "Boot: Creando TCB...");

	programaEnElKernel->TCB.pid = pid;
	programaEnElKernel->TCB.tid = tid;

	programaEnElKernel->TCB.base_segmento_codigo = kernelCrearSegmento(self, pid, tamanioEnBytes); //beso


	if(programaEnElKernel->TCB.base_segmento_codigo == -1){
		finalizarProgramaEnPlanificacion(programaEnElKernel);
		//return NULL;
	}


	programaEnElKernel->TCB.base_stack = kernelCrearSegmento(self, pid, self->tamanioStack);
	if(programaEnElKernel->TCB.base_stack == -1){
		finalizarProgramaEnPlanificacion(programaEnElKernel);
		//return NULL;
	}

	programaEnElKernel->TCB.cursor_stack = programaEnElKernel->TCB.base_stack;

	//faltan todos los logs
	log_info(self->loggerKernel, "PID %d TID: %d\n",programaEnElKernel->TCB.pid, programaEnElKernel->TCB.tid);

	//Falta escribir la memoria con las SystemCalls
	log_info(self->loggerKernel, "Boot: Completado con éxito!");

	//return programaEnElKernel;
}
