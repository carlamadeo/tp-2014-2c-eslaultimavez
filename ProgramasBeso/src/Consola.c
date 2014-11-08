/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Consola.h"

/**
  * @paramsc Es la cantidad de parametros que recibe main
  * @paramv Son los parametros. Se guardan en un vector [param1, param2, .., paramN]
*/
int main( int argc, char **argv ) {
	if( argc < 2 ){
		perror("No se cargo un programa eso para ejecutar.");
		return EXIT_FAILURE;
	}

	logger = log_create(PATH_LOG, "PROGRAMA", 0, LOG_LEVEL_DEBUG); //Creo el archivo Log

	t_buffer_ConsolaPrograma datos_del_programa = extraer_programa(argv[1]);

	// Cargo la configuracion
	cargar_configuracion();

	consolaConectarConElLOADER(datos_del_programa);

    return EXIT_SUCCESS;
}

t_buffer_ConsolaPrograma consolaExtraer_programaESO(char *parametro){
	t_buffer_ConsolaPrograma buffer_programa;

	//Defino una estructura generica que me da informacion del archivo (como el tamaño)
	struct stat atributos_archivo;

	//Abro el archivo. Esto es solo para probar (porque no tenemos la conexion de sockets)
	FILE *archivo = fopen( parametro, "r" );

	//Leo el archivo y guardo sus atributos en data_programa
	stat(parametro, &atributos_archivo);

	//Declaro la linea en donde se van a guardar temporalmente cada linea del archivo
	//char *line = malloc( 512 * sizeof(char) );

	//El codigo completo
	char *codigo = (char*) malloc(sizeof(char) * (atributos_archivo.st_size) + 1); //Reservo + 1 por el \0

	if( archivo == 0 ){
		perror("El archivo no pudo abrirse");
		exit(-1);
	} else {
		fread(codigo, atributos_archivo.st_size, 1, archivo);

		buffer_programa.codigo = codigo;
		buffer_programa.codigo[atributos_archivo.st_size] = '\0';

		if( atributos_archivo.st_size == 0 ){
			log_info(logger, "El archivo no contiene codigo. Cerrando Programa");
			printf("El archivo no contiene codigo. Cerrando Programa\n");
			exit(-1);
		}
	}

	return buffer_programa;
}

void consolaCargarConfiguracion(){
	t_config* config = config_create(getenv("ANSISOP_CONFIG"));//apunta al archivo configuracion
	if (!config_has_property(config, "IP")) {//Averigua si hay "IP" en archivo logs
		 log_error(logger, "Falta el IP donde se encuentra ejecutando el Hilo KERNEL.");//Carga en log el error
		 perror("Falta el IP donde se encuentra ejecutando el Hilo KERNEL.");
	 } else{
		 info_conexion_Loader.ip = config_get_string_value(config, "IP"); 		//saca IP de config
		 log_debug(logger, "IP = %s", info_conexion_Loader.ip);
	 }
	if (!config_has_property(config, "Puerto")) {
		 log_error(logger, "Falta el Puerto TCP donde se encuentra escuchando el Hilo KERNEL."); //Carga en log el error
		 perror("Falta el Puerto TCP donde se encuentra escuchando el Hilo KERNEL.");
	 } else{
		 info_conexion_Loader.port = config_get_int_value(config, "Puerto");
		 log_debug(logger, "Puerto = %d", info_conexion_Loader.port); //Carga en log el puertogo
	 }
}



