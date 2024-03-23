
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
    #if DEBUGN3
        int prev=posbyte;
    #endif
    posbyte=posbyte%BLOCKSIZE;


    unsigned char mascara = 128; // 10000000
    mascara >>= posbit;          // desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);

    #if DEBUGN3
        printf(GRAY"[leer_bit(%d)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n"RESET,nbloque,prev,posbyte,posbit,nbloqueMB,nbloqueabs);
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

int obtener_nRangoBL(struct inodo *inodo,unsigned int nblogico,unsigned int *ptr){
    if(nblogico<DIRECTOS){
        *ptr=inodo->punterosDirectos[nblogico];
        return 0;
    }else if(nblogico<INDIRECTOS0){
        *ptr=inodo->punterosIndirectos[0];
        return 1;
    }else if(nblogico<INDIRECTOS1){
        *ptr=inodo->punterosIndirectos[1];
        return 2;
    }else if(nblogico<INDIRECTOS2){
        *ptr=inodo->punterosIndirectos[2];
        return 3;
    }else{
        *ptr=0;
        fprintf(stderr,"Bloque logico fuera de rango");
        return FALLO;
    }
}

int obtener_indice(unsigned int nblogico,int nivel_punteros){
    if(nblogico<DIRECTOS){
        return nblogico;
    }else if(nblogico<INDIRECTOS0){
        return nblogico-DIRECTOS;
    }else if(nblogico<INDIRECTOS1){
        if(nivel_punteros==2){
            return (nblogico-INDIRECTOS0)/NPUNTEROS;
        }else if(nivel_punteros==1){
            return (nblogico-INDIRECTOS0)%NPUNTEROS;
        }
    }else if(nblogico<INDIRECTOS2){
        if(nivel_punteros==3){
            return (nblogico-INDIRECTOS1)/(NPUNTEROS*NPUNTEROS);
        }else if(nivel_punteros==2){
            return ((nblogico-INDIRECTOS1)%(NPUNTEROS*NPUNTEROS))/NPUNTEROS;
        }else if(nivel_punteros==1){
            return ((nblogico-INDIRECTOS1)%(NPUNTEROS*NPUNTEROS))%NPUNTEROS;
        }
    }
    return FALLO;
}

int traducir_bloque_inodo(struct inodo *inodo, unsigned int nblogico, unsigned char reservar){
    unsigned int ptr=0;
    unsigned int ptr_ant=0;
    int nRangoBL=obtener_nRangoBL(inodo,nblogico,&ptr);
    if(nRangoBL==FALLO){
        return FALLO;
    }
    int nivel_punteros=nRangoBL;
    int indice;
    unsigned int buffer[NPUNTEROS];
    while(nivel_punteros>0){
        if(ptr==0){
            if(reservar==0){
                return -1;
            }
            ptr=reservar_bloque();
            inodo->numBloquesOcupados++;
            inodo->ctime=time(NULL);
            if(nivel_punteros==nRangoBL){
                inodo->punterosIndirectos[nRangoBL-1]=ptr;
                #if DEBUGN4
                    printf(GRAY"[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %d (reservado BF %d para punteros_nivel%d)]\n"RESET,nRangoBL-1,ptr,ptr,nivel_punteros);
                #endif
            }else{
                buffer[indice]=ptr;
                #if DEBUGN4
                    printf(GRAY"[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para punteros_nivel%d)]\n"RESET,nivel_punteros,indice,ptr,ptr,nivel_punteros);
                #endif
                if(bwrite(ptr_ant,buffer)==FALLO){
                    return FALLO;
                }
            }
            memset(buffer,0,BLOCKSIZE);
        }else{
            if(bread(ptr,buffer)==FALLO){
                return FALLO;
            }
        }
        indice=obtener_indice(nblogico,nivel_punteros);
        if(indice==FALLO){
            return FALLO;
        }
        ptr_ant=ptr;
        ptr=buffer[indice];
        nivel_punteros--;
    }

    if(ptr==0){
        if(reservar==0){
            return FALLO;
        }
        ptr=reservar_bloque();
        inodo->numBloquesOcupados++;
        inodo->ctime=time(NULL);
        if(nRangoBL==0){
            inodo->punterosDirectos[nblogico]=ptr;
            printf(GRAY"[traducir_bloque_inodo()→ inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n"RESET,nblogico,ptr,ptr,nblogico);
        }else{
            buffer[indice]=ptr;
            printf(GRAY"[traducir_bloque_inodo()→ punteros_nivel1 [%d] = %d (reservado BF %d para BL %d)]\n"RESET,indice,ptr,ptr,nblogico);
            if(bwrite(ptr_ant,buffer)==FALLO){
                return FALLO;
            }
        }
    }
    return ptr;
}

int liberar_inodo(unsigned int ninodo){
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    int liberados=liberar_bloques_inodo(0,&inodo);
    if(liberados==FALLO){
        return FALLO;
    }
    inodo.numBloquesOcupados-=liberados;
    inodo.tipo='l';
    inodo.tamEnBytesLog=0;

    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }
    int aux=SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre=ninodo;
    inodo.punterosDirectos[0]=aux;
    SB.cantInodosLibres++;

    inodo.ctime=time(NULL);

    if(bwrite(posSB,&SB)==FALLO){
        return FALLO;
    }

    if(escribir_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    return ninodo;
}

int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo){
    int liberados=0;
    if(inodo->tamEnBytesLog==0){
        return liberados;
    }

    unsigned int ultimoBL;
    if(inodo->tamEnBytesLog%BLOCKSIZE==0){
        ultimoBL=inodo->tamEnBytesLog/BLOCKSIZE-1;
    }else{
        ultimoBL=inodo->tamEnBytesLog/BLOCKSIZE;
    }

    unsigned int nivel_punteros,indice,ptr=0;
    int nRangoBL;
    unsigned char bufAux_punteros[BLOCKSIZE]; //1024 bytes
    unsigned int bloque_punteros [3][NPUNTEROS];
    int ptr_nivel[3];
    int indices[3];
    memset (bufAux_punteros, 0, BLOCKSIZE);
    for(int nBL=primerBL;nBL<=ultimoBL;nBL++){
        nRangoBL=obtener_nRangoBL(inodo,nBL,&ptr);
        if(nRangoBL==FALLO){
            return FALLO;
        }
        nivel_punteros=nRangoBL;

        while(ptr>0&&nivel_punteros>0){
            indice=obtener_indice(nBL,nivel_punteros);
            if(indice==0 || nBL==primerBL){
                if(bread(ptr,bloque_punteros[nivel_punteros-1])==FALLO){
                    return FALLO;
                }
            }
            ptr_nivel[nivel_punteros-1]=ptr;
            indices[nivel_punteros-1]=indice;
            ptr=bloque_punteros[nivel_punteros-1][indice];
            nivel_punteros--;
        }
        if(ptr>0){
            if(liberar_bloque(ptr)==FALLO){
                return FALLO;
            }
            #if DEBUGN6
                printf(GRAY"[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n"RESET,ptr,nBL);
            #endif
            liberados++;
            if(nRangoBL==0){
                inodo->punterosDirectos[nBL]=0;
            }else{
                nivel_punteros=1;
                while(nivel_punteros<=nRangoBL){
                    indice=indices[nivel_punteros-1];
                    bloque_punteros[nivel_punteros-1][indice]=0;
                    ptr=ptr_nivel[nivel_punteros-1];
                    if(memcmp(bloque_punteros[nivel_punteros-1],bufAux_punteros,BLOCKSIZE)==0){
                        if(liberar_bloque(ptr)==FALLO){
                            return FALLO;
                        }
                        #if DEBUGN6
                            printf(GRAY"[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n"RESET,ptr,nivel_punteros,nBL);
                        #endif
                        liberados++;
                        if(nivel_punteros==nRangoBL){
                            inodo->punterosIndirectos[nRangoBL-1]=FALLO;
                        }
                        nivel_punteros++;
                    }else{
                        if(bwrite(ptr,bloque_punteros[nivel_punteros-1])==FALLO){
                            return FALLO;
                        }
                        nivel_punteros=nRangoBL+1;
                    }
                }
            }
        }
    }
    return liberados;
}