//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc!=3){
        fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./mi_cat <disco> </ruta_fichero>"RESET);
        return FALLO;
    }

    if(argv[2][strlen(argv[2])-1]=='/'){
        fprintf(stderr,RED"No es un fichero"RESET);
        return FALLO;
    }

    if(bmount(argv[1])==FALLO){
        return FALLO;
    }
    int tambuffer=BLOCKSIZE*4;
    char buffer[tambuffer];
    memset(buffer,0,tambuffer);
    int offset=0;
    int leidos=0;
    int aux=mi_read(argv[2],buffer,offset,tambuffer);
    if(aux<0){
        mostrar_error_buscar_entrada(aux);
        aux=0;
    }
    
    while(aux>0){
        leidos+=aux;
        write(1,buffer,aux);
        memset(buffer,0,sizeof(buffer));
        offset+=tambuffer;
        aux=mi_read(argv[2],buffer,offset,tambuffer);
        if(aux<0){
            mostrar_error_buscar_entrada(aux);
            aux=0;
        }
    }
    
    printf("\nTotal_leidos: %d\n",leidos);
    if(bumount()==FALLO){
        return FALLO;
    }
}