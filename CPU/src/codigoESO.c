#include "codigoESO.h"
#include "CPU.h"


void LOAD (int registro, int32_t numero){
		tcb->registro_de_programacion[registro]=numero;
}

void GETM (int registroA, int registroB){

}

int SETM (int numero, int registroA, int registroB){

	return 1;
}


void MOVR (int primer_registro, int segundo_registro){
	tcb->registro_de_programacion[primer_registro]=tcb->registro_de_programacion[segundo_registro];
}


void ADDR (int primer_registro, int segundo_registro){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]+auxiliar;

}

void SUBR (int primer_registro, int segundo_registro){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]-auxiliar;

}

void MULR (int primer_registro, int segundo_registro){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]*auxiliar;
	}



void MODR (int primer_registro, int segundo_registro){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]%auxiliar;
}

void DIVR (int primer_registro, int segundo_registro){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	if (auxiliar==0){
			log_info(logger, "fallo de division por cero %d", tcb->pid);
			/*salida estandar informando a la consola que hubo un error de division por cero*/
			break;
		}else{
			tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]/auxiliar;
	}
}

void INCR (int registro){

	tcb->registro_de_programacion[registro]++;

}

void DECR (int registro){

	tcb->registro_de_programacion[registro]--;
}

void COMP (int primer_registro, int segundo_registro){
	if (tcb->registro_de_programacion[primer_registro]==tcb->registro_de_programacion[segundo_registro]){
		tcb->registro_de_programacion[0]=1;
	}else{
		tcb->registro_de_programacion[0]=0;
	}

}

void CGEQ (int primer_registro, int segundo_registro){
	if (tcb->registro_de_programacion[primer_registro]>=tcb->registro_de_programacion[segundo_registro]){
		tcb->registro_de_programacion[0]=1;
	}else{
		tcb->registro_de_programacion[0]=0;
	}

}

void CLEQ(int primer_registro, int segundo_registro){
	if (tcb->registro_de_programacion[primer_registro]<=tcb->registro_de_programacion[segundo_registro]){
		tcb->registro_de_programacion[0]=1;
	}else{
		tcb->registro_de_programacion[0]=0;
	}

}
void GOTO (int registro){
	uint32_t auxiliar=tcb->base_segmento_codigo;
	auxiliar+=(uint32_t)tcb->registro_de_programacion[registro];
	tcb->puntero_instruccion=auxiliar;
}

void JMPZ(int numero){
   if(tcb->registro_de_programacion[0]==0){
	   uint32_t auxiliar=(uint32_t)numero;
	   auxiliar+=tcb->base_segmento_codigo;
	   tcb->puntero_instruccion=auxiliar;
   }
}

void JPNZ(int numero){
	   if(tcb->registro_de_programacion[0]!=0){
		   uint32_t auxiliar=(uint32_t)numero;
		   auxiliar+=tcb->base_segmento_codigo;
		   tcb->puntero_instruccion=auxiliar;
	   }
}

void INET(uint32_t direccion){

}
/*
void FLCL(){

}*/

void SHIF (int numero, int registro){
	if(numero>0){
		int32_t auxiliar=tcb->registro_de_programacion[registro];
		int32_t resultado;
		resultado=auxiliar>>numero;
		tcb->registro_de_programacion[registro]=resultado;
	}else{
		int32_t auxiliar=tcb->registro_de_programacion[registro];
		int32_t resultado;
		resultado=auxiliar<<numero;
		tcb->registro_de_programacion[registro]=resultado;
	}

}


void NOPP (){
/*consume quantum sin hacer nada*/
}

int PUSH (int numero, int registroA){
/*apilo los bytes indicados por numero, del registro hacia el stack, actualizo el cursor de stack*/
	return 1;
}

int TAKE (int numero, int registro){

	return 1;
}

void XXXX (){

}



//Instrucciones Protegidas, KM=1   (ninguna de estas operaciones tiene operadores)

void MALC (){

}
void FREE(){

}
void INNC(){

}
void OUTN(){

}
void OUTC(){

}
void CREA(){

}
void  JOIN (){

}

void BLOK(){

}

void WAKE(){

}


//funciones de la system calls
void systemCallsMALLOC(){

}
void systemCallsFREE(){

}
void systemCallsINN(){

}
void systemCallsINC(){

}
void systemCallsOUTN(){

}
void systemCallsOUTC(){

}
void systemCallsCREATE_THREAD(){

}
void systemCallsJOIN_THREAD(){

}
void systemCallsWAIT(){

}

void systemCallsSIGNAL(){

}

void systemCallsSETSEM(){

}
