//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc!=4){
        fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>"RESET);
        return FALLO;
    }
    if(argv[2][strlen(argv[2])-1]=='/'){
        return FALLO;
    }
    
    if(argv[3][strlen(argv[3])-1]=='/'){
        return FALLO;
    }
    if(bmount(argv[1])==FALLO){
        return FALLO;
    }

    if(mi_link(argv[2],argv[3])<0){
        return FALLO;
    }
    
    if(bumount()==FALLO){
        return FALLO;
    }
}