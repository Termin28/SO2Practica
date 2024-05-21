//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "bloques.h"

static sem_t *mutex;
static int descriptor=0;
static unsigned int inside_sc = 0;

/**
 * Función que monta el dispositivo virtual
 * Recibe: El nombre del dispositivo
 * Devuelve: El filedescriptor. En caso de error devuelve -1. 
*/
int bmount(const char *camino){
    if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem(); 
        if (mutex == SEM_FAILED) {
            return -1;
        }
    }
    umask(000);
    if(descriptor>0){
        close(descriptor);
    }
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
    descriptor=close(descriptor);
    if(descriptor==-1){
        perror("Error");
        return FALLO;
    }
    deleteSem(); 
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

void mi_waitSem() {
   if (!inside_sc) { // inside_sc==0, no se ha hecho ya un wait
       waitSem(mutex);
   }
   inside_sc++;
}


void mi_signalSem() {
   inside_sc--;
   if (!inside_sc) {
       signalSem(mutex);
   }
}
