#include "ficheros.h"

int main(int argc,char **argv){
    if(argc!=4){
        return FALLO;
    }
    bmount(argv[1]);
    
    bumount();
    return EXITO;
}