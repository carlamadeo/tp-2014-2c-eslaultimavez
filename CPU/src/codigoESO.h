#ifndef CODIGO_ESO_H_
#define CODIGO_ESO_H_

#include "CPU.h"

//Instrucciones de usuario


void LOAD (int registro, int32_t numero); //Carga en el registro, el número dado.

void GETM (int registroA, int registroB);	//Obtiene el valor de memoria apuntado por el segundo registro.
											//El valor obtenido lo asigna en el primer registro.

int SETM (int numero, int registroA, int registroB); //Pone tantos bytes desde el segundo registro,
													//hacia la memoria apuntada por el primer registro.


void MOVR (int primer_registro, int segundo_registro); //Copia el valor del segundo registro hacia el primero.

void ADDR (int registroA, int registroB); //Suma el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.


void SUBR (int registroA, int registroB); //Resta el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.

void MULR (int registroA, int registroB);//Multiplica el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.

void MODR (int registroA, int registroB); //Obtiene el resto de la división del primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.

void DIVR (int registroA, int registroB); //Divide el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el
										//registro A; a menos que el segundo operando sea 0, en cuyo caso se asigna el flag de ZERO_DIV
										//y no se hace la operación.

void INCR (int registroA); //Incrementar una unidad al registro.
void DECR (int registroA); //Decrementa una unidad al registro.


void COMP (int registroA, int registroB); //Compara si el primer registro es igual al segundo.
										//De ser verdadero, se almacena el valor 1.
										//De lo contrario el valor 0.
										//El resultado de la operación se almacena en el registro A.
void CGEQ (int registroA, int registroB); //Compara si el primer registro es mayor o igual al segundo.
										//De ser verdadero, se almacena el valor 1. De lo contrario el valor 0.
										//El resultado de la operación se almacena en el registro A.


void CLEQ (int registroA, int registroB); //Compara si el primer registro es menor o igual al segundo.
										//De ser verdadero, se almacena el valor 1.De lo contrario el valor 0.
										//El resultado de la operación se almacena en el registro A.

void GOTO (int registroA); //Altera el flujo de ejecución para ejecutar la instrucción apuntada por el registro.
						 //El valor es el desplazamiento desde el inicio del programa.

void JMPZ(int direccion);   //Altera el flujo de ejecución, solo si el valor del registro A es cero,
                       //para ejecutar la instrucción apuntada por el registro.
                      //El valor es el desplazamiento desde el inicio del programa.

void JPNZ(int direccion); //Altera el flujo de ejecución, solo si el valor del registro A no es cero,
					 //para ejecutar la instrucción apuntada por el registro.
					//El valor es el desplazamiento desde el inicio del programa.

void INET(uint32_t direccion); 	//Interrumpe la ejecución del programa para ejecutar la rutina del kernel
								//que se encuentra en la posición apuntada por la direccion.
								//El ensamblador admite ingresar una cadena indicando el nombre,
								//que luego transformará en el número correspondiente. Los posibles valores son:
								//“MALC”, “FREE”, “INNN”, “INNC”, “OUTN”, “OUTC”, “BLOK”, “WAKE”, “CREA” y “JOIN”.
								//Invoca al servicio correspondiente en el proceso Kernel.
								//Notar que el hilo en cuestión debe bloquearse tras una interrupción.

/*void FLCL(); //Limpia el registro de flags.*/


void SHIF (int numero, int registroA); //Desplaza los bits del registro, tantas veces como se indique en el Número.
								   //De ser desplazamiento positivo, se considera hacia la derecha.
								   //De lo contrario hacia la izquierda.

void NOPP (); //Consume un ciclo del CPU sin hacer nada.

int PUSH (int numero, int registroA); //Apila los primeros bytes, indicado por el número, del registro hacia el stack.
									 //Modifica el valor del registro cursor de stack de forma acorde.

int TAKE (int numero, int registro); //Desapila los primeros bytes, indicado por el número, del stack hacia el registro.
									//Modifica el valor del registro de stack de forma acorde.

void XXXX (); //Finaliza la ejecución.

//fin de instrucciones de usuario





//Instrucciones Protegidas, KM=1   (ninguna de estas operaciones tiene operadores)

void MALC ();  //Reserva una cantidad de memoria especificada por el registro A.
			  //La direccion de esta se almacena en el registro A.
			 //Crea en la MSP un nuevo segmento del tamaño especificado asociado al programa en ejecución.

void FREE();  //Libera la memoria apuntada por el registro A.
			 //Solo se podrá liberar memoria alocada por la instrucción de MALC.
			//Destruye en la MSP el segmento indicado en el registro A.

void INNC(); //Pide por consola del programa que se ingrese una cadena
			 //no más larga de lo indicado por el registro B.
            //La misma será almacenada en la posición de memoria apuntada por el registro A.
           //Invoca al servicio correspondiente en el proceso Kernel.

void OUTN();//Imprime por consola del programa el número, con signo almacenado en el registro A.
           //Invoca al servicio correspondiente en el proceso Kernel.

void OUTC(); //Imprime por consola del programa una cadena de tamaño indicado
            //por el registro B que se encuentra en la direccion apuntada por el registro A.
		   //Invoca al servicio correspondiente en el proceso Kernel.

void CREA();  //Crea un hilo, hijo del TCB que ejecutó la llamada al sistema correspondiente.
             //El nuevo hilo tendrá su Program Counter apuntado al número almacenado en el registro B.
            //El identificador del nuevo hilo se almacena en el registro A.
           //Para lograrlo debe generar un nuevo TCB como copia del TCB actual,
          //asignarle un nuevo TID correlativo al actual, cargar en el Puntero de Instrucción la rutina donde
         //comenzará a ejecutar el nuevo hilo (registro B), pasarlo de modo Kernel a modo Usuario,
        //duplicar el segmento de stack desde la base del stack, hasta el cursor del stack.
       //Asignar la base y cursor de forma acorde (tal que la diferencia entre cursor y base se mantenga igual)
      //y luego invocar al servicio correspondiente en el proceso Kernel con el TCB recién generado.

void  JOIN ();//Bloquea el programa que ejecutó la llamada al sistema hasta que el hilo con el identificador
             //almacenado en el registro A haya finalizado.
            //Invoca al servicio correspondiente en el proceso Kernel.

void BLOK();  //Bloquea el programa que ejecutó la llamada al sistema
			 //hasta que el recurso apuntado por B se libere.
            //La evaluación y decisión de si el recurso está libre o no es hecha por la llamada al sistema WAIT pre-compilada.

void WAKE(); //Desbloquea al primer programa bloqueado por el recurso apuntado por B.
			//La evaluación y decisión de si el recurso está libre o no es hecha por la llamada al sistema SIGNAL pre-compilada.

//Fin de Instrucciones Protegidas




//Funciones llamadas al sistema

void MALLOC(); //Ejecuta la operación de MALC.
void FREE(); //Ejecuta la operación de FREE.
void INN(); //Ejecuta la operación de INNN.
void INC(); //Ejecuta la operación de INNC.
void OUTN();//Ejecuta la operación de OUTN.
void OUTC(); //Ejecuta la operación de OUTC.
void CREATE_THREAD();//Ejecuta la operación de CREA.
void JOIN_THREAD(); //Ejecuta la operación de JOIN.
void WAIT(); //Toma el valor apuntado por el registro B, resta una unidad, guarda en la memoria
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
