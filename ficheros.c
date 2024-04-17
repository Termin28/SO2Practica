//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "ficheros.h"

/**
 * Función que escribe el contenido de un buffer de nbytes en un fichero/directorio
 * Recibe: Nº inodo al que escribir el fichero, buffer con el contenido a escribir,
 * posicion de escritura incial respecto al inodo (en bytes), Nº bytes a escribir
 * Devuelve: Nº bytes escritos. En caso de error devuelve -1
*/
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }

    if ((inodo.permisos & 2) != 2) {
       fprintf(stderr, RED "No hay permisos de escritura\n" RESET);
       return FALLO;
    }
    int primerBL=offset/BLOCKSIZE;
    int ultimoBL=(offset+nbytes-1)/BLOCKSIZE;
    int desp1=offset%BLOCKSIZE;
    int desp2=(offset+nbytes-1)%BLOCKSIZE;

    int nbfisico=traducir_bloque_inodo(&inodo,primerBL,1);
    if(escribir_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }

    if(nbfisico==FALLO){
        return FALLO;
    }

    unsigned char buf_bloque[BLOCKSIZE];
    if(bread(nbfisico,buf_bloque)==FALLO){
        return FALLO;
    }

    int escritos=0;
    int aux;
    if(primerBL==ultimoBL){
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if(bwrite(nbfisico,buf_bloque)==FALLO){
            return FALLO;
        }
        escritos=nbytes;
    }else if(primerBL<ultimoBL){
        memcpy(buf_bloque+desp1,buf_original,BLOCKSIZE-desp1);
        aux=bwrite(nbfisico,buf_bloque);
        if(aux==FALLO){
            return FALLO;
        }
        escritos=aux-desp1;
        for(int i=primerBL+1;i<ultimoBL;i++){
            nbfisico=traducir_bloque_inodo(&inodo,i,1);

            if(escribir_inodo(ninodo,&inodo)==FALLO){
                return FALLO;
            }
            if(nbfisico==FALLO){
                return FALLO;
            }
            aux=bwrite(nbfisico,buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
            if(aux==FALLO){
                return FALLO;
            }
            escritos+=aux;
        }

        nbfisico=traducir_bloque_inodo(&inodo,ultimoBL,1);
        if(escribir_inodo(ninodo,&inodo)==FALLO){
            return FALLO;
        }
        if(nbfisico==FALLO){
            return FALLO;
        }
        if(bread(nbfisico,buf_bloque)==FALLO){
            return FALLO;
        }

        memcpy(buf_bloque,buf_original + (nbytes - (desp2 + 1)), desp2 + 1);
        if(bwrite(nbfisico,buf_bloque)==FALLO){
            return FALLO;
        }
        escritos+=desp2+1;
    }
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }

    if(inodo.tamEnBytesLog<(nbytes+offset)){
        inodo.tamEnBytesLog=nbytes+offset;
        inodo.ctime=time(NULL);
    }
    inodo.mtime=time(NULL);
    if(escribir_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }

    if(nbytes==escritos){
        return nbytes;
    }
    return FALLO;
}

/**
 * Función que lee información de un fichero/directorio y lo almacena en un buffer de memoria
 * Recibe: Nº inodo que leer, buffer donde se almacenara la información leida,
 * posicion de lectura incial respecto al inodo (en bytes), Nº bytes a leer
 * Devuelve: Nº Bytes leidos. En caso de error devuelve -1
*/
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    
    if ((inodo.permisos & 4) != 4){
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
        return FALLO;
    }

    int leidos=0;
    if(offset>=inodo.tamEnBytesLog){
        return leidos;
    }

    if((offset+nbytes)>=inodo.tamEnBytesLog){
        nbytes=inodo.tamEnBytesLog-offset;
    }

    int primerBL=offset/BLOCKSIZE;
    int ultimoBL=(offset+nbytes-1)/BLOCKSIZE;
    int desp1=offset%BLOCKSIZE;
    int desp2=(offset+nbytes-1)%BLOCKSIZE;

    unsigned char buf_bloque[BLOCKSIZE];

    int nbfisico=traducir_bloque_inodo(&inodo,primerBL,0);
    if(primerBL==ultimoBL){
        if(nbfisico!=FALLO){   
            if(bread(nbfisico,buf_bloque)==FALLO){
                return FALLO;
            }
            memcpy(buf_original,buf_bloque + desp1,nbytes);
        }
        leidos=nbytes;
    }else if(primerBL<ultimoBL){
        if(nbfisico!=FALLO){ 
            if(bread(nbfisico,buf_bloque)==FALLO){
                    return FALLO;
            }
            memcpy(buf_original,buf_bloque + desp1,BLOCKSIZE-desp1);
        }
        leidos=BLOCKSIZE-desp1;

        for(int i=primerBL+1;i<ultimoBL;i++){
            nbfisico=traducir_bloque_inodo(&inodo,i,0);
            if(nbfisico!=FALLO){
                if(bread(nbfisico,buf_bloque)==FALLO){
                    return FALLO;
                }
                memcpy(buf_original+(BLOCKSIZE-desp1)+(i-primerBL-1)*BLOCKSIZE,buf_bloque,BLOCKSIZE);
            }
            leidos+=BLOCKSIZE;
        }

        nbfisico=traducir_bloque_inodo(&inodo,ultimoBL,0);
        if(nbfisico!=FALLO){
            if(bread(nbfisico,buf_bloque)==FALLO){
                return FALLO;
            }
            memcpy(buf_original+(nbytes-(desp2-1)),buf_bloque,desp2+1);
        }
        leidos+=desp2+1;
    }

    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }

    inodo.atime=time(NULL);

    if(escribir_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }

    if(nbytes==leidos){
        return leidos;
    }
    return FALLO;
}

/**
 * Función que devuelve la metainformacion de unf fichero/directorio
 * Recibe: Nº inodo, puntero pasado por referencia de variable struct STAT
 * Devuelve: 0 si todo va bien. En caso de error devuelve -1
*/
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    p_stat->atime=inodo.atime;
    p_stat->ctime=inodo.ctime;
    p_stat->mtime=inodo.mtime;
    p_stat->nlinks=inodo.nlinks;
    p_stat->numBloquesOcupados=inodo.numBloquesOcupados;
    p_stat->permisos=inodo.permisos;
    p_stat->tamEnBytesLog=inodo.tamEnBytesLog;
    p_stat->tipo=inodo.tipo;

    return EXITO;
}

/**
 * Función que cambia los permisos de un fichero/directorio
 * Recibe: Nº Inodo, valor de los nuevos permisos
 * Devuelve: 0 si todo va bien. En caso de error devuelve -1
*/
int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    inodo.permisos=permisos;
    inodo.ctime=time(NULL);
    if(escribir_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    return EXITO;
}

/**
 * Función que trunca un fichero/directorio a los bytes indicados como nbytes, liberando los bloques necesarios
 * Recibe: Nº inodo del fichero, Nº de bytes a partir del que truncar
 * Devuelve: Bloques liberados. En caso de error devuelve -1
*/
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }

    int primerBL;
    if(nbytes%BLOCKSIZE==0){
        primerBL=nbytes/BLOCKSIZE;
    }else{
        primerBL=nbytes/BLOCKSIZE+1;
    }
    int liberados=liberar_bloques_inodo(primerBL,&inodo);
    if(liberados==FALLO){
        return FALLO;
    }
    inodo.tamEnBytesLog=nbytes;
    inodo.numBloquesOcupados-=liberados;
    inodo.mtime=time(NULL);
    inodo.ctime=time(NULL);
    if(escribir_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    return liberados;
}