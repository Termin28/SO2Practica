
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

int escribir_bit(unsigned int nbloque, unsigned int bit){
    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }
    int posbyte=nbloque/8;
    int posbit=nbloque%8;
    int nbloqueMB=posbyte/BLOCKSIZE;

    int nbloqueabs=SB.posPrimerBloqueMB+nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    if(bread(nbloqueabs,bufferMB)==FALLO){
        return FALLO;
    }

    unsigned char mascara=128;
    mascara>>=posbit;
    if(bit==1){
        bufferMB[posbyte]|=mascara;
    }else{
        bufferMB[posbyte]&=~mascara;
    }
    if(bwrite(nbloqueabs,bufferMB)==FALLO){
        return FALLO;
    }
    return EXITO;
}

char leer_bit(unsigned int nbloque){
    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }
    int posbyte=nbloque/8;
    int posbit=nbloque%8;
    int nbloqueMB=posbyte/BLOCKSIZE;

    int nbloqueabs=SB.posPrimerBloqueMB+nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    if(bread(nbloqueabs,bufferMB)==FALLO){
        return FALLO;
    }
    int prev=posbyte;
    posbyte=posbyte%BLOCKSIZE;


    unsigned char mascara = 128; // 10000000
    mascara >>= posbit;          // desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);

    #if DEBUG
        printf(ANSI_COLOR_GRIS"[leer_bit(%d)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n"ANSI_COLOR_RESET,nbloque,prev,posbyte,posbit,nbloqueMB,nbloqueabs);
    #endif
    return mascara;
}

int reservar_bloque(){
    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }

    if(SB.cantBloquesLibres==0){
        return FALLO;
    }

    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    memset(bufferAux, 255, BLOCKSIZE);

    int nbloqueMB=-1;
    int found=0;
    while(found==0){
        nbloqueMB++;
        if(bread(nbloqueMB + SB.posPrimerBloqueMB , bufferMB)==FALLO){
            return FALLO;
        }
        found=memcmp(bufferAux,bufferMB,BLOCKSIZE);
    }

    int posbyte=0;
    while(bufferMB[posbyte]==255){
        posbyte++;
    }

    unsigned char mascara = 128; // 10000000
    int posbit = 0;
    while (bufferMB[posbyte] & mascara) { // operador AND para bits
        bufferMB[posbyte] <<= 1;          // desplazamiento de bits a la izquierda
        posbit++;
    }
    int nbloque = (nbloqueMB * BLOCKSIZE + posbyte) * 8 + posbit;
    if(escribir_bit(nbloque,1)==FALLO){
        return FALLO;
    }

    memset(bufferAux,0,BLOCKSIZE);
    if(bwrite(nbloque,bufferAux)==FALLO){
        return FALLO;
    }

    SB.cantBloquesLibres--;
    if(bwrite(posSB,&SB)==FALLO){
        return FALLO;
    }

    return nbloque;
}

int liberar_bloque(unsigned int nbloque){
    if(escribir_bit(nbloque,0)==FALLO){
        return FALLO;
    }

    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }
    SB.cantBloquesLibres++;
    if(bwrite(posSB,&SB)==FALLO){
        return FALLO;
    }
    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo){
    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }

    int bloqueinodo=SB.posPrimerBloqueAI+(ninodo/(BLOCKSIZE/INODOSIZE));

    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    if(bread(bloqueinodo,&inodos)==FALLO){
        return FALLO;
    }

    inodos[ninodo%(BLOCKSIZE/INODOSIZE)]=*inodo;
    if(bwrite(bloqueinodo,inodos)==FALLO){
        return FALLO;
    }
    return EXITO;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }

    int bloqueinodo=SB.posPrimerBloqueAI+(ninodo/(BLOCKSIZE/INODOSIZE));

    struct inodo inodos[BLOCKSIZE/INODOSIZE];

    if(bread(bloqueinodo,&inodos)==FALLO){
        return FALLO;
    }

    *inodo=inodos[ninodo%(BLOCKSIZE/INODOSIZE)];
    return EXITO;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos){
    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }

    if(SB.cantInodosLibres==0){
        return FALLO;
    }

    struct inodo inodo;
    int posInodoReservado=SB.posPrimerInodoLibre;
    
    if(leer_inodo(posInodoReservado,&inodo)==FALLO){
        return FALLO;
    }
    
    SB.posPrimerInodoLibre=inodo.punterosDirectos[0];
    inodo.tipo=tipo;
    inodo.permisos=permisos;
    inodo.nlinks=1;
    inodo.tamEnBytesLog=0;
    inodo.atime=time(NULL);
    inodo.ctime=time(NULL);
    inodo.mtime=time(NULL);
    inodo.numBloquesOcupados=0;
    memset(inodo.punterosDirectos,0,sizeof(inodo.punterosDirectos));
    memset(inodo.punterosIndirectos,0,sizeof(inodo.punterosIndirectos));
    
    if(escribir_inodo(posInodoReservado,&inodo)==FALLO){
        return FALLO;
    }
    
    SB.cantInodosLibres--;
    
    if(bwrite(posSB,&SB)==FALLO){
        return FALLO;
    }
    
    return posInodoReservado;
}
