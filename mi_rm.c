#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc!=3){
        fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./mi_rm disco /ruta"RESET);
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