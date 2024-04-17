//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "ficheros.h"
#define TAMBUFFER 1500

int main(int argc,char const *argv[]){
    if(argc!=3){
        fprintf(stderr,"Error: La sintaxis es incorrecta. Sintaxis: ./leer <nombre_dispositivo> <ninodo>");
        return FALLO;
    }
    
    if(bmount(argv[1])==FALLO){
        return FALLO;
    }

    int ninodo=atoi(argv[2]);
    char buffer[TAMBUFFER];
    memset(buffer,0,TAMBUFFER);
    int offset=0;
    int leidos=mi_read_f(ninodo,buffer,offset,TAMBUFFER);
    int total=0;
    while(leidos>0){
        total+=leidos;
        write(1,buffer,leidos);
        offset+=TAMBUFFER;
        memset(buffer,0,TAMBUFFER);
        leidos=mi_read_f(ninodo,buffer,offset,TAMBUFFER);
    }
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    fprintf(stderr,"\ntotal_leidos: %d\ntamEnBytesLog: %d\n",total,inodo.tamEnBytesLog);

    if(bumount()==FALLO){
        return FALLO;
    }
}