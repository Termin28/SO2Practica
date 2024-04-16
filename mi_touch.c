//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc!=4){
        fprintf(stderr,"Error: La sintaxis es incorrecta. Sintaxis: ./mi_touch <disco> <permisos> </ruta>\n");
        return FALLO;
    }

    int permisos=atoi(argv[2]);
    if(permisos<0 || permisos>7){
        fprintf(stderr,"Error: Los permisos deben ser un numero entre 0-7\n");
        return FALLO;
    }

    if(argv[3][strlen(argv[3])-1]!='/'){
        if(bmount(argv[1])==FALLO){
            return FALLO;
        }
        int error=mi_creat(argv[3],permisos);
        if(error<0){
            mostrar_error_buscar_entrada(error);
            return FALLO;
        }
        
        if(bumount()==FALLO){
            return FALLO;
        }
    }
    
    
}