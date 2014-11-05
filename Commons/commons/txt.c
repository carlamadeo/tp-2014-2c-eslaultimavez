/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#include "string.h"
#include "txt.h"

#include <stdio.h>


FILE* txt_open_for_append(char* path) {
	return fopen(path, "a");
}

void txt_write_in_file(FILE* file, char* bytes) {
	fprintf(file, "%s", bytes);
	fflush(file);
}




void txt_write_in_stdout(char* string, char * level )
{

	if( string_starts_with( level, "ERROR" ) ) {
		printf("%s%s%s", ANSI_COLOR_RED, string, ANSI_COLOR_RESET);
	}else if(  string_starts_with( level, "INFO" ) ) {
		printf("%s%s%s", ANSI_COLOR_YELLOW , string, ANSI_COLOR_RESET);
	}else if(  string_starts_with( level, "WARNING" ) ) {
		printf("%s%s%s", ANSI_COLOR_MAGENTA, string, ANSI_COLOR_RESET);
	}else if(  string_starts_with( level, "DEBUG" ) ) {
		printf("%s%s%s", ANSI_COLOR_BLUE , string, ANSI_COLOR_RESET);
	}else if(  string_starts_with( level, "TRACE" ) ) {
		printf("%s%s%s", ANSI_COLOR_GREEN, string, ANSI_COLOR_RESET);
	}else{
		printf("%s", string);
	}
	fflush(stdout);
}

void txt_close_file(FILE* file) {
	fclose(file);
}
