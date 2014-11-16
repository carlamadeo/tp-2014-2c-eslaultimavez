/*
 * mspConfig.c
 *
 *  Created on: 14/09/2014
 *      Author: utnso
 */

#include "mspConfig.h"
#include "Memoria.h"
#include "commons/log.h"
#include "commons/config.h"
#include <stdint.h>
#include "commons/string.h"
#include <stdlib.h>
#include <string.h>

t_log *MSPlogger;

t_config *MSPConfig;
int puertoMSP;
double cantidadMemoriaPrincipal, cantidadMemoriaSecundaria;	  //Bytes
uint16_t modoSustitucionPaginasMSP;

char *memoria;
t_list *programas;
t_list *marcosLibres;
t_list *marcosOcupados;


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

	memset(memoria, 0, cantidadMemoriaPrincipal);

	if (memoria == 0) {
		log_error(MSPlogger, "No se pudo alocar la memoria, finalizando...");
		return false;
	}

	log_info(MSPlogger, "Memoria reservada correctamente");
	log_info(MSPlogger, "Se reservo %g MegaBytes para el archivo de paginacion.", cantidadMemoriaSecundaria);

	cantidadMemoriaSecundaria *= MEGABYTES_A_BYTES; //Los paso a Bytes

	programas = list_create();
	marcosLibres = list_create();
	marcosOcupados = list_create();

	cargarMarcos(marcosLibres);

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
		marco->categoriaClockModificado = NOREFERENCIADA_NOMODIFICADA;
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
	list_destroy_and_destroy_elements(marcosLibres, free);
	list_destroy_and_destroy_elements(marcosOcupados, free);

	free(memoria);
	config_destroy(MSPConfig);
}