#include "ficheros.h"
int main(int argc,char const *argv[]){
    if(argc!=4){
        return FALLO;
    }
    int noffset=5;
    int offset[]={9000,209000,30725000,409605000,480000000};
    
    if(bmount(argv[1])==FALLO){
        return FALLO;
    }
    
    bumount();
}