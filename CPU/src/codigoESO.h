#ifndef CODIGO_ESO_H_
#define CODIGO_ESO_H_

#include "CPU_Proceso.h"


//Instrucciones Protegidas, KM=1   (ninguna de estas operaciones tiene operadores)

int MALC_ESO ();  //Reserva una cantidad de memoria especificada por el registro A.
			  //La direccion de esta se almacena en el registro A.
			 //Crea en la MSP un nuevo segmento del tamaño especificado asociado al programa en ejecución.

int FREE_ESO();  //Libera la memoria apuntada por el registro A.
			 //Solo se podrá liberar memoria alocada por la instrucción de MALC.
			//Destruye en la MSP el segmento indicado en el registro A.

int INNC_ESO(); //Pide por consola del programa que se ingrese una cadena
			 //no más larga de lo indicado por el registro B.
            //La misma será almacenada en la posición de memoria apuntada por el registro A.
           //Invoca al servicio correspondiente en el proceso Kernel.

int OUTN_ESO();//Imprime por consola del programa el número, con signo almacenado en el registro A.
           //Invoca al servicio correspondiente en el proceso Kernel.

int OUTC_ESO(); //Imprime por consola del programa una cadena de tamaño indicado
            //por el registro B que se encuentra en la direccion apuntada por el registro A.
		   //Invoca al servicio correspondiente en el proceso Kernel.

int CREA_ESO();  //Crea un hilo, hijo del TCB que ejecutó la llamada al sistema correspondiente.
             //El nuevo hilo tendrá su Program Counter apuntado al número almacenado en el registro B.
            //El identificador del nuevo hilo se almacena en el registro A.
           //Para lograrlo debe generar un nuevo TCB como copia del TCB actual,
          //asignarle un nuevo TID correlativo al actual, cargar en el Puntero de Instrucción la rutina donde
         //comenzará a ejecutar el nuevo hilo (registro B), pasarlo de modo Kernel a modo Usuario,
        //duplicar el segmento de stack desde la base del stack, hasta el cursor del stack.
       //Asignar la base y cursor de forma acorde (tal que la diferencia entre cursor y base se mantenga igual)
      //y luego invocar al servicio correspondiente en el proceso Kernel con el TCB recién generado.

int  JOIN_ESO ();//Bloquea el programa que ejecutó la llamada al sistema hasta que el hilo con el identificador
             //almacenado en el registro A haya finalizado.
            //Invoca al servicio correspondiente en el proceso Kernel.

int BLOK_ESO();  //Bloquea el programa que ejecutó la llamada al sistema
			 //hasta que el recurso apuntado por B se libere.
            //La evaluación y decisión de si el recurso está libre o no es hecha por la llamada al sistema WAIT pre-compilada.

int WAKE_ESO(); //Desbloquea al primer programa bloqueado por el recurso apuntado por B.
			//La evaluación y decisión de si el recurso está libre o no es hecha por la llamada al sistema SIGNAL pre-compilada.

//Fin de Instrucciones Protegidas




//Funciones llamadas al sistema
/*
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




*/
#endif /* CODIGO_ESO_H_ */
