//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc!=3){
        fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./mi_rmdir disco /ruta"RESET);
        return FALLO;
    }

    if(argv[2][strlen(argv[2])-1]!='/'){
        fprintf(stderr,RED"Error: No es un directorio valido"RESET);
        return FALLO;
    }
    
    if(bmount(argv[1])==FALLO){
        return FALLO;
    }

    if(mi_unlink(argv[2])<0){
        return FALLO;
    }

    if(bumount()==FALLO){
        return FALLO;
    }
}