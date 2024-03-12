
#include "ficheros.h"
#define TAMBUFFER 1500

int main(int argc,char const *argv[]){
    if(argc!=3){
        return FALLO;
    }

    if(bmount(argv[1])==FALLO){
        return FALLO;
    }

    bumount();
    return EXITO;
}