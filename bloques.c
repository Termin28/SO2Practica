//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "bloques.h"

static int descriptor=0;

/**
 * Función que monta el dispositivo virtual
 * Recibe: El nombre del dispositivo
 * Devuelve: El filedescriptor. En caso de error devuelve -1. 
*/
int bmount(const char *camino){
    umask(000);
    descriptor=open(camino,O_RDWR | O_CREAT,0666);
    if(descriptor==-1){
        perror("Error");
        return FALLO;
    }
    return descriptor;
}

/**
 * Función que desmonta el dispositivo virtual
 * Devuelve: 0 si no hay error. En caso de error devuelve -1
*/
int bumount(){
    if(close(descriptor)==-1){
        perror("Error");
        return FALLO;
    }
    return EXITO;
}

/**
 * Función que escribe un bloque en el dispositivo virtual
 * Recibe: El bloque fisico donde escribir, buffer con el contenido que escribir.
 * Devuelve: Nº Bytes escritos. En caso de error devuelve -1.
*/
int bwrite(unsigned int nbloque, const void *buf){
    if(lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET)==-1){
        perror("Error");
        return FALLO;
    }
    int bytes=write(descriptor,buf,BLOCKSIZE);
    if(bytes==-1){
        perror("Error");
        return FALLO;
    }
    return bytes;
}

/**
 * Función que lee un bloque del dispositivo virtual
 * Recibe: El bloque fisico que hay que leer, buffer de memoria donde se volcará lo leido
 * Devuelve: Nº Bytes leidos. En caso de error devuelve -1 
*/
int bread(unsigned int nbloque, void *buf){
    if(lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET)==-1){
        perror("Error");
        return FALLO;
    }
    int bytes=read(descriptor,buf,BLOCKSIZE);
    if(bytes==-1){
        perror("Error");
    }
    return bytes;
}