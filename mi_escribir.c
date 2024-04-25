#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc!=5){
        fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>"RESET);
        return FALLO;
    }

    if(argv[2][strlen(argv[2])-1]=='/'){
        fprintf(stderr,RED"No es un fichero"RESET);
        return FALLO;
    }

    if(bmount(argv[1])==FALLO){
        return FALLO;
    }
    printf("Longitud texto: %ld\n",strlen(argv[3]));
    int offset=atoi(argv[4]);
    int escritos=mi_write(argv[2],argv[3],offset,strlen(argv[3]));
    if(escritos<0){
        mostrar_error_buscar_entrada(escritos);
        escritos=0;
    }
    printf("Bytes escritos: %d\n",escritos);
    if(bumount()==FALLO){
        return FALLO;
    }
}