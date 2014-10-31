/*
 * ConfigMSP.c
 *
 *  Created on: 14/09/2014
 *      Author: utnso
 */

#include "ConfigMSP.h"
#include "Memoria.h"
#include "commons/log.h"
#include "commons/config.h"
#include <stdint.h>
#include "commons/string.h"
#include <stdlib.h>
#include "commons/collections/queue.h"

extern t_log *MSPlogger;

t_config *MSPConfig;
int puertoMSP;
double cantidadMemoriaPrincipal, cantidadMemoriaSecundaria, cantidadMemoriaTotal;	  //Bytes
uint16_t modoSustitucionPaginasMSP;

char *memoria;
t_list *programas; //TODO Solo para pruebas, eliminar y ver donde se crearia
t_list *marcos;
t_list *marcosLibres;
t_list *marcosOcupados; //Para FIFO


const char configProperties[][25] = {"PUERTO", "CANTIDAD_MEMORIA", "CANTIDAD_SWAP", "SUST_PAGS"};

bool cargarConfiguracionMSP(char *config) {

	MSPConfig = config_create(config);

	if(!validarConfiguracionMSP()) return false;

	puertoMSP = config_get_int_value(MSPConfig, "PUERTO");
	cantidadMemoriaPrincipal = config_get_double_value(MSPConfig, "CANTIDAD_MEMORIA");
	cantidadMemoriaSecundaria = config_get_double_value(MSPConfig, "CANTIDAD_SWAP");

	if(string_equals_ignore_case(config_get_string_value(MSPConfig, "SUST_PAGS"), "FIFO"))
		modoSustitucionPaginasMSP = FIFO;
	else
		modoSustitucionPaginasMSP = CLOCK_MODIFICADO;

	log_info(MSPlogger, "Reservando %g KiloBytes de memoria principal... ", cantidadMemoriaPrincipal);

	cantidadMemoriaPrincipal *= KILOBYTES_A_BYTES; //Los paso a Bytes

	//Alocar el tamanio obtenido por archivo de configuracion a la memoria
	memoria = malloc(cantidadMemoriaPrincipal);

	if (memoria == 0) {
		log_error(MSPlogger, "No se pudo alocar la memoria, finalizando...");
		return false;
	}

	log_info(MSPlogger, "Memoria reservada correctamente");
	log_info(MSPlogger, "Se reservo %g MegaBytes para el archivo de paginacion.", cantidadMemoriaSecundaria);

	cantidadMemoriaSecundaria *= MEGABYTES_A_BYTES; //Los paso a Bytes

	cantidadMemoriaTotal = cantidadMemoriaPrincipal + cantidadMemoriaSecundaria;

	programas = list_create();
	marcos = list_create();
	marcosLibres = list_create();
	marcosOcupados = list_create();

	cargarMarcos(marcos);
	cargarMarcos(marcosLibres);

	t_programa *prog = malloc(sizeof(t_programa)); //TODO Solo para pruebas, eliminar!!
	prog = crearPrograma(1); //TODO Solo para pruebas, eliminar!!
	list_add(programas, prog); //TODO Solo para pruebas, eliminar!!

	return true;
}

void cargarMarcos(t_list *listaMarcos){
	int cantidadMarcos;
	int i;
	uint32_t inicioMarco = 0;

	cantidadMarcos = cantidadMemoriaPrincipal/TAMANIO_PAGINA;

	for(i = 0; i < cantidadMarcos; i++){
		t_marco *marco = malloc(sizeof(t_marco));

		marco->numero = i;
		marco->inicio = inicioMarco;
		marco->categoriaClockModificado = 0;
		list_add(listaMarcos, marco);
		inicioMarco+=TAMANIO_PAGINA;
	}
}


bool validarConfiguracionMSP(){
	bool existen = true;
	int elements = sizeof(configProperties)/sizeof(configProperties[0]);
	int i;

	for(i = 0; i < elements; i++) {
		if(!config_has_property(MSPConfig, &configProperties[i])) {
			existen = false;
			break;
		}
	}

	return existen;
}


void destruirConfiguracionMSP(){
	list_destroy_and_destroy_elements(programas, free);
	list_destroy_and_destroy_elements(marcos, free);
	list_destroy_and_destroy_elements(marcosLibres, free);
	queue_destroy_and_destroy_elements(marcosOcupados, free);

	free(memoria);
	config_destroy(MSPConfig);
}
