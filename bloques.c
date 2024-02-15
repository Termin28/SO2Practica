#include "bloques.h"

static int descriptor=0;

int bmount(const char *camino){
    descriptor=open(camino,O_RDWR | O_CREAT,0666);
    if(descriptor==-1){
        perror("Error");
        return FALLO;
    }
    return descriptor;
}

int bumount(){
    if(close(descriptor)==-1){
        perror("Error");
        return FALLO;
    }
    return EXITO;
}

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