//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc!=4){
        fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./mi_mkdir <disco> <permisos> </ruta>\n"RESET);
        return FALLO;
    }

    int permisos=atoi(argv[2]);
    if(permisos<0 || permisos>7){
        fprintf(stderr,RED"Error: modo invalido <<%d>>\n"RESET,permisos);
        return FALLO;
    }

    if(argv[3][strlen(argv[3])-1]=='/'){
        if(bmount(argv[1])==FALLO){
            return FALLO;
        }
        int error=mi_creat(argv[3],permisos);
        if(error<0){
            return FALLO;
        }
        
        if(bumount()==FALLO){
            return FALLO;
        }
    }   
}