/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "ProgramaBeso.h"

/**
  * @paramsc Es la cantidad de parametros que recibe main
  * @paramv Son los parametros. Se guardan en un vector [param1, param2, .., paramN]
*/
int main( int argc, char **argv ) {

	verificar_argumentosProgramaBeso(argc,argv);
	char* config_file = argv[1];
	t_programaBESO* self = consolaCargarConfiguracion(config_file); // Cargo la configuracion

	self->loggerProgramaBESO =log_create("logProgramaBeso.log", "ProgramaBeso", 1, LOG_LEVEL_DEBUG); //Creo el archivo Log


	//consolaExtraer_programaESO(self, argv[1]);
	consolaConectarConElLOADER(self);

    return EXIT_SUCCESS;
}

void consolaExtraer_programaESO (t_programaBESO* self,char *parametro){

	//Defino una estructura generica que me da informacion del archivo (como el tamaño)
	struct stat atributos_archivo;

	//Abro el archivo. Esto es solo para probar (porque no tenemos la conexion de sockets)
	FILE *archivo = fopen(parametro, "r" );

	//Leo el archivo y guardo sus atributos en data_programa
	stat(parametro, &atributos_archivo);

	//El codigo completo
	char *codigo = (char*) malloc(sizeof(char) * (atributos_archivo.st_size) + 1); //Reservo + 1 por el \0

	if( archivo == 0 ){
		perror("El archivo no pudo abrirse");
		exit(-1);
	} else {
		fread(codigo, atributos_archivo.st_size, 1, archivo);

		self->codigo = codigo;
		self->codigo[atributos_archivo.st_size] = '\0';

		if( atributos_archivo.st_size == 0 ){
			log_info(self->loggerProgramaBESO, "El archivo no contiene codigo. Cerrando Programa");
			printf("Programa: El archivo no contiene codigo. Cerrando Programa\n");
			exit(-1);
		}
	}

}

t_programaBESO*  consolaCargarConfiguracion(char* config_file){

	t_programaBESO* self = malloc(sizeof(t_programaBESO));
	t_config* config = config_create(config_file);

	self->puertoLoader = config_get_int_value(config, "PUERTO_KERNEL");
	self->ipLoader = string_duplicate(config_get_string_value(config, "IP_KERNEL"));

	config_destroy(config);
	return self;
}



void verificar_argumentosProgramaBeso(int argc, char* argv[]){
	if( argc < 2 ){
		printf("Modo de empleo: ./ProgramaBeso consolaConfig.cfg\n");
		perror("La consola Beso no recibio las configuraciones");
		exit (EXIT_FAILURE);
	}
}
