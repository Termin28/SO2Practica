//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "ficheros_basico.h"

int main(int argc, char **argv){
    //Comprobar sintaxis
    if(argc!=3){
        fprintf(stderr,"Error: La sintaxis es incorrecta. Sintaxis: ./mi_mkfs <nombre_dispositivo> <nbloques>");
        return FALLO;
    }

    if(bmount(argv[1])==FALLO){
        return FALLO;
    }

    unsigned char buffer[BLOCKSIZE];
    memset(buffer,0,BLOCKSIZE);
    int nbloques=atoi(argv[2]);
    int ninodos=nbloques/4;
    
    //Inicializar a 0 el dispositivo virtual
    for(int i=0;i<nbloques;i++){
        if(bwrite(i,buffer)==FALLO){
            return FALLO;
        }
    }

    initSB(nbloques,ninodos);
    initMB();
    initAI();
    reservar_inodo('d', 7);

    if(bumount()==FALLO){
        return FALLO;
    }
}