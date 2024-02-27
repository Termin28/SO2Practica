
#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){
    int tam=(nbloques/8)/BLOCKSIZE;
    if((nbloques/8)%BLOCKSIZE!=0){
        tam++;
    }
    return tam;
}

int tamAI(unsigned int ninodos){
    int tam=(ninodos*INODOSIZE)/BLOCKSIZE;
    if((ninodos*INODOSIZE)%BLOCKSIZE!=0){
        tam++;
    }
    return tam;
}

int initSB(unsigned int nbloques, unsigned int ninodos){
    struct superbloque SB;
    SB.posPrimerBloqueMB=posSB+tamSB;
    SB.posUltimoBloqueMB=SB.posPrimerBloqueMB + tamMB(nbloques) -1;
    SB.posPrimerBloqueAI=SB.posUltimoBloqueMB+1;
    SB.posUltimoBloqueAI=SB.posPrimerBloqueAI+tamAI(ninodos)-1;
    SB.posPrimerBloqueDatos=SB.posUltimoBloqueAI+1;
    SB.posUltimoBloqueDatos=nbloques-1;
    SB.posInodoRaiz=0;
    SB.posPrimerInodoLibre=0;
    SB.cantBloquesLibres=nbloques;
    SB.cantInodosLibres=ninodos;
    SB.totBloques=nbloques;
    SB.totInodos=ninodos;
    if(bwrite(posSB,&SB)==FALLO){
        return FALLO;
    }
    return EXITO;
}

int initMB(){
    struct superbloque SB;

    char bufferMB[BLOCKSIZE];
    memset(bufferMB,255,BLOCKSIZE);

    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }

    int tam=tamSB+tamMB(SB.totBloques)+tamAI(SB.totInodos);
    int bytes=tam/8;
    int ocupados=bytes/BLOCKSIZE;
    if(bytes%BLOCKSIZE!=0){
        ocupados++;
    }

    int posbloqueMB=SB.posPrimerBloqueMB;
    for(int i=0;i<ocupados-1;i++){
        if(bwrite(posbloqueMB,bufferMB)==FALLO){
            return FALLO;
        }
        posbloqueMB++;
    }

    while(bytes>=BLOCKSIZE){
        bytes-=BLOCKSIZE;
    }

    memset(bufferMB,0,BLOCKSIZE);
    for(int i=0;i<bytes;i++){
        bufferMB[i]=255;
    }
    int resto=tam%8;
    bufferMB[bytes]=255;
    bufferMB[bytes]<<=(8-resto);
    if(bwrite(posbloqueMB,bufferMB)==FALLO){
        return FALLO;
    }
    SB.cantBloquesLibres=SB.cantBloquesLibres-tam;
    if(bwrite(posSB,&SB)==FALLO){
        return FALLO;
    }
    return EXITO;
}

int initAI(){
    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int contInodos=SB.posPrimerInodoLibre+1;
    for(int i=SB.posPrimerBloqueAI;i<=SB.posUltimoBloqueAI;i++){
        if(bread(i,inodos)==FALLO){
            return FALLO;
        }
        for(int j=0;j<BLOCKSIZE/INODOSIZE;j++){
            inodos[j].tipo='l';
            if(contInodos<SB.totInodos){
                inodos[j].punterosDirectos[0]=contInodos;
                contInodos++;
            }else{
                inodos[j].punterosDirectos[0]=UINT_MAX;
                break;
            }
        }
        if(bwrite(i,&inodos)==FALLO){
            return FALLO;
        }
    }
    return EXITO;
}