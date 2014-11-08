#ifndef CODIGO_ESO_H_
#define CODIGO_ESO_H_

#include "CPU.h"

//Instrucciones de usuario


void LOAD_ESO (int registro, int32_t numero, t_TCB* tcb); //Carga en el registro, el número dado.

void GETM_ESO (int primer_registro, int segundo_registro, t_TCB* tcb);	//Obtiene el valor de memoria apuntado por el segundo registro.
											//El valor obtenido lo asigna en el primer registro.

void SETM_ESO (int numero, int primer_registro, int segundo_registro, t_TCB* tcb); //Pone tantos bytes desde el segundo registro,
													//hacia la memoria apuntada por el primer registro.


void MOVR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb); //Copia el valor del segundo registro hacia el primero.

void ADDR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb); //Suma el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.


void SUBR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb); //Resta el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.

void MULR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb);//Multiplica el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.

void MODR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb); //Obtiene el resto de la división del primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.

void DIVR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb); //Divide el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el
										//registro A; a menos que el segundo operando sea 0, en cuyo caso se asigna el flag de ZERO_DIV
										//y no se hace la operación.

void INCR_ESO (int registro, t_TCB* tcb); //Incrementar una unidad al registro.
void DECR_ESO (int registro, t_TCB* tcb); //Decrementa una unidad al registro.


void COMP_ESO (int primer_registro, int segundo_registro, t_TCB* tcb); //Compara si el primer registro es igual al segundo.
										//De ser verdadero, se almacena el valor 1.
										//De lo contrario el valor 0.
										//El resultado de la operación se almacena en el registro A.
void CGEQ_ESO (int primer_registro, int segundo_registro, t_TCB* tcb); //Compara si el primer registro es mayor o igual al segundo.
										//De ser verdadero, se almacena el valor 1. De lo contrario el valor 0.
										//El resultado de la operación se almacena en el registro A.


void CLEQ_ESO (int primer_registro, int segundo_registro, t_TCB* tcb); //Compara si el primer registro es menor o igual al segundo.
										//De ser verdadero, se almacena el valor 1.De lo contrario el valor 0.
										//El resultado de la operación se almacena en el registro A.

void GOTO_ESO (int registro, t_TCB* tcb); //Altera el flujo de ejecución para ejecutar la instrucción apuntada por el registro.
						 //El valor es el desplazamiento desde el inicio del programa.

void JMPZ_ESO(int direccion, t_TCB* tcb);   //Altera el flujo de ejecución, solo si el valor del registro A es cero,
                       //para ejecutar la instrucción apuntada por el registro.
                      //El valor es el desplazamiento desde el inicio del programa.

void JPNZ_ESO(int direccion, t_TCB* tcb); //Altera el flujo de ejecución, solo si el valor del registro A no es cero,
					 //para ejecutar la instrucción apuntada por el registro.
					//El valor es el desplazamiento desde el inicio del programa.

void INTE_ESO(uint32_t direccion, t_TCB* tcb); 	//Interrumpe la ejecución del programa para ejecutar la rutina del kernel
								//que se encuentra en la posición apuntada por la direccion.
								//El ensamblador admite ingresar una cadena indicando el nombre,
								//que luego transformará en el número correspondiente. Los posibles valores son:
								//“MALC”, “FREE”, “INNN”, “INNC”, “OUTN”, “OUTC”, “BLOK”, “WAKE”, “CREA” y “JOIN”.
								//Invoca al servicio correspondiente en el proceso Kernel.
								//Notar que el hilo en cuestión debe bloquearse tras una interrupción.

/*void FLCL(); //Limpia el registro de flags.*/


void SHIF_ESO (int numero, int registro, t_TCB* tcb); //Desplaza los bits del registro, tantas veces como se indique en el Número.
								   //De ser desplazamiento positivo, se considera hacia la derecha.
								   //De lo contrario hacia la izquierda.

void NOPP_ESO (); //Consume un ciclo del CPU sin hacer nada.

void PUSH_ESO (int numero, int registro, t_TCB* tcb); //Apila los primeros bytes, indicado por el número, del registro hacia el stack.
									 //Modifica el valor del registro cursor de stack de forma acorde.

void TAKE_ESO (int numero, int registro, t_TCB* tcb); //Desapila los primeros bytes, indicado por el número, del stack hacia el registro.
									//Modifica el valor del registro de stack de forma acorde.

void XXXX_ESO (t_TCB* tcb); //Finaliza la ejecución.

//fin de instrucciones de usuario





//Instrucciones Protegidas, KM=1   (ninguna de estas operaciones tiene operadores)

void MALC_ESO ();  //Reserva una cantidad de memoria especificada por el registro A.
			  //La direccion de esta se almacena en el registro A.
			 //Crea en la MSP un nuevo segmento del tamaño especificado asociado al programa en ejecución.

void FREE_ESO();  //Libera la memoria apuntada por el registro A.
			 //Solo se podrá liberar memoria alocada por la instrucción de MALC.
			//Destruye en la MSP el segmento indicado en el registro A.

void INNC_ESO(); //Pide por consola del programa que se ingrese una cadena
			 //no más larga de lo indicado por el registro B.
            //La misma será almacenada en la posición de memoria apuntada por el registro A.
           //Invoca al servicio correspondiente en el proceso Kernel.

void OUTN_ESO();//Imprime por consola del programa el número, con signo almacenado en el registro A.
           //Invoca al servicio correspondiente en el proceso Kernel.

void OUTC_ESO(); //Imprime por consola del programa una cadena de tamaño indicado
            //por el registro B que se encuentra en la direccion apuntada por el registro A.
		   //Invoca al servicio correspondiente en el proceso Kernel.

void CREA_ESO();  //Crea un hilo, hijo del TCB que ejecutó la llamada al sistema correspondiente.
             //El nuevo hilo tendrá su Program Counter apuntado al número almacenado en el registro B.
            //El identificador del nuevo hilo se almacena en el registro A.
           //Para lograrlo debe generar un nuevo TCB como copia del TCB actual,
          //asignarle un nuevo TID correlativo al actual, cargar en el Puntero de Instrucción la rutina donde
         //comenzará a ejecutar el nuevo hilo (registro B), pasarlo de modo Kernel a modo Usuario,
        //duplicar el segmento de stack desde la base del stack, hasta el cursor del stack.
       //Asignar la base y cursor de forma acorde (tal que la diferencia entre cursor y base se mantenga igual)
      //y luego invocar al servicio correspondiente en el proceso Kernel con el TCB recién generado.

void  JOIN_ESO ();//Bloquea el programa que ejecutó la llamada al sistema hasta que el hilo con el identificador
             //almacenado en el registro A haya finalizado.
            //Invoca al servicio correspondiente en el proceso Kernel.

void BLOK_ESO();  //Bloquea el programa que ejecutó la llamada al sistema
			 //hasta que el recurso apuntado por B se libere.
            //La evaluación y decisión de si el recurso está libre o no es hecha por la llamada al sistema WAIT pre-compilada.

void WAKE_ESO(); //Desbloquea al primer programa bloqueado por el recurso apuntado por B.
			//La evaluación y decisión de si el recurso está libre o no es hecha por la llamada al sistema SIGNAL pre-compilada.

//Fin de Instrucciones Protegidas




//Funciones llamadas al sistema

void MALLOC(); //Ejecuta la operación de MALC.
void FREE_ESO(); //Ejecuta la operación de FREE.
void INNN_ESO(); //Ejecuta la operación de INNN.
void INNC_ESO(); //Ejecuta la operación de INNC.
void OUTN_ESO();//Ejecuta la operación de OUTN.
void OUTC_ESO(); //Ejecuta la operación de OUTC.
void CREATE_THREAD();//Ejecuta la operación de CREA.
void JOIN_THREAD(); //Ejecuta la operación de JOIN.
void WAIT_ESO(); //Toma el valor apuntado por el registro B, resta una unidad, guarda en la memoria
             //correspondiente el nuevo valor y evalúa si este contador es negativo o cero.
            //De serlo, invoca a la operación de BLOK;
           //de lo contrario no. Altera el registro A por uso interno.
void SIGNAL(); //Toma el valor apuntado por el registro B, suma una unidad, guarda en la memoria
              //correspondiente el nuevo valor y evalúa si este contador es negativo o cero.
             //De serlo, invoca a la operación de WAKE; de lo contrario no.
            //Altera el registro A por uso interno.
void SETSEM(); //Fija el valor del recurso apuntado por el registro B al valor del registro D.
               //Altera el registro A por uso interno.

//fin de funciones llamadas al sistema





#endif /* CODIGO_ESO_H_ */
