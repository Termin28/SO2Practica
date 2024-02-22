#include "ficheros_basico.h"

int main(int argc, char **argv){
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
    for(int i=0;i<nbloques;i++){
        if(bwrite(i,buffer)==FALLO){
            return FALLO;
        }
    }
    initSB(nbloques,ninodos);
    initMB();
    initAI();

    if(bumount()==FALLO){
        return FALLO;
    }
}