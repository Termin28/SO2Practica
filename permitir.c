//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "ficheros.h"

int main(int argc,char **argv){
    if(argc!=4){
        fprintf(stderr,RED"Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>"RESET);
        return FALLO;
    }
    if(bmount(argv[1])==FALLO){
        return FALLO;
    }
    int ninodo=atoi(argv[2]);
    int permisos=atoi(argv[3]);
    if(mi_chmod_f(ninodo,permisos)==FALLO){
        return FALLO;
    }
    if(bumount()==FALLO){
        return FALLO;
    }
}