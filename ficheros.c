//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "ficheros.h"
#define FAILURE -1
/**
 * Función que escribe el contenido de un buffer de nbytes en un fichero/directorio
 * Recibe: Nº inodo al que escribir el fichero, buffer con el contenido a escribir,
 * posicion de escritura incial respecto al inodo (en bytes), Nº bytes a escribir
 * Devuelve: Nº bytes escritos. En caso de error devuelve -1
*/
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    //Declaraciones
    unsigned int primerBL, ultimoBL;
    int desp1, desp2, nbfisico;
    int bytesescritos = 0;
    int auxByteEscritos = 0;
    char unsigned buf_bloque[BLOCKSIZE];
    struct inodo inodo;

    //Leer el inodo.
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "mi_write_f(): Error leer_inodo() \n");
        return FALLO;
    }

    //Comprobamos que el inodo tenga los permisos para escribir
    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, "No hay permisos de escritura\n");
        return FALLO;
    }

    //Asignaciones de las variables.
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //Obtencion del numero de bloque fisico
    mi_waitSem();
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
    if (nbfisico == FALLO)
    {
        fprintf(stderr, "mi_write_f(): Error al traducir_bloque_inodo()\n");
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();

    //Leemos el bloque fisico
    if (bread(nbfisico, buf_bloque) == FALLO)
    {
        fprintf(stderr, "mi_write_f(): Error al leer el bloque fisico\n");
        return FALLO;
    }

    //Caso en el que lo que queremos escribir cabe en un bloque fisico
    if (primerBL == ultimoBL)
    {
        memcpy(buf_bloque + desp1, buf_original, nbytes);

        //Escribimos el bloque fisico en el disco
        auxByteEscritos = bwrite(nbfisico, buf_bloque);
        if (auxByteEscritos == FALLO)
        {
            fprintf(stderr, "Error mi_write_f(): bwrite()\n");
            return FALLO;
        }
        bytesescritos += nbytes;
    }

    //Caso en el que la escritura ocupa mas de un bloque fisico
    else if (primerBL < ultimoBL)
    {
        //Parte 1: Primero bloque escrito parcialmente
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        //Escribimos el bloque fisico en el disco
        auxByteEscritos = bwrite(nbfisico, buf_bloque);
        if (auxByteEscritos == FALLO)
        {
            fprintf(stderr, "Error mi_write_f(): bwrite()\n");
            return FALLO;
        }
        bytesescritos += auxByteEscritos - desp1;

        //Parte 2: Bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            //Obtenemos los bloques intermedios
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            if (nbfisico == FALLO)
            {
                fprintf(stderr, "Error mi_write_f(): traducir_bloque_inodo()\n");
                mi_signalSem();
                return FALLO;
            }
            mi_signalSem();

            //Escribimos los bloques intermedios
            auxByteEscritos = bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
            if (auxByteEscritos == FALLO)
            {
                fprintf(stderr, "Error mi_write_f(): bwrite()\n");
                return FALLO;
            }
            bytesescritos += auxByteEscritos;
        }

        //Parte 3: Último bloque escrito parcialmente
        //Obtenemos el bloque fisico
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (nbfisico == FALLO)
        {
            fprintf(stderr, "Error mi_write_f(): traducir_bloque_inodo()\n");
            mi_signalSem();
            return FALLO;
        }
        mi_signalSem();
        //Leemos el bloque fisico
        if (bread(nbfisico, buf_bloque) == FALLO)
        {
            fprintf(stderr, "Error mi_write_f(): bread()\n");
            return FALLO;
        }

        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        auxByteEscritos = bwrite(nbfisico, buf_bloque);
        if (auxByteEscritos == FALLO)
        {
            fprintf(stderr, "Error mi_write_f(): bwrite()\n");
            return FALLO;
        }

        bytesescritos += desp2 + 1;
    }

    //Leer el inodo actualizado.
    mi_waitSem(); //Semaforos
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "Error leer_inodo(): mi_write_f() \n");
        mi_signalSem();
        return FALLO;
    }

    //Actualizar la metainformación
    //Comprobación si lo que hemos escrito es mas grande que el fichero
    if (inodo.tamEnBytesLog < (nbytes + offset))
    {
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "Error escribir_inodo(): mi_write_f() \n");
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem(); //Semaforos

    //Comprobar que no haya errores de escritura y que se haya escrito todo bien.
    if (nbytes == bytesescritos)
    {
#if DEBUGGER
        fprintf(stderr, "\tmi_write_f: BIEN\n");
        fprintf(stderr, "\tmi_read_f(): nbfisico = %i\n", nbfisico);
#endif
        return bytesescritos;
    }
    else
    {
#if DEBUGGER
        fprintf(stderr, "mi_write_f: MAL\n\tnbytes:%i\n\tbytesescritos:%i\n", nbytes, bytesescritos);
#endif
        return nbytes;
    }
}

/**
 * Función que lee información de un fichero/directorio y lo almacena en un buffer de memoria
 * Recibe: Nº inodo que leer, buffer donde se almacenara la información leida,
 * posicion de lectura incial respecto al inodo (en bytes), Nº bytes a leer
 * Devuelve: Nº Bytes leidos. En caso de error devuelve -1
*/
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    //Declaraciones
    unsigned int primerBL, ultimoBL;
    int desp1, desp2, nbfisico;
    int bytesleidos = 0;
    int auxByteLeidos = 0;
    char unsigned buf_bloque[BLOCKSIZE];
    struct inodo inodo;

    //Leer el inodo.
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "Error in mi_read_f(): leer_inodo()\n");
        return bytesleidos;
    }

    //Comprobamos que el inodo tenga los permisos para leer
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, "Error in mi_read_f(): No hay permisos de lectura!\n");
        return bytesleidos;
    }

    if (offset >= inodo.tamEnBytesLog)
    {
        // No podemos leer nada
        return bytesleidos;
    }

    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    { // pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
        // leemos sólo los bytes que podemos desde el offset hasta EOF
    }

    //Asignaciones de las variables.
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //Obtencion del numero de bloque fisico
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
    //Caso el cual lo que queremos leer cabe en un bloque fisico
    if (primerBL == ultimoBL)
    {
        if (nbfisico != FALLO)
        {
            //Leemos el bloque fisico del disco
            auxByteLeidos = bread(nbfisico, buf_bloque);
            if (auxByteLeidos == FALLO)
            {
                fprintf(stderr, "Error mi_read_f(): bread()\n");
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        bytesleidos = nbytes;
    }
    //Caso en el que la lectura ocupa mas de un bloque fisico
    else if (primerBL < ultimoBL)
    {
        //Parte 1: Primero bloque leido parcialmente
        if (nbfisico != FALLO)
        {
            //Leemos el bloque fisico del disco
            auxByteLeidos = bread(nbfisico, buf_bloque);
            if (auxByteLeidos == FALLO)
            {
                fprintf(stderr, "Error mi_read_f(): bread()\n");
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }

        bytesleidos = BLOCKSIZE - desp1;

        //Parte 2: Bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            //Obtenemos los bloques intermedios
            nbfisico = traducir_bloque_inodo(ninodo, i, 0);
            if (nbfisico != FALLO)
            {
                //Leemos el bloque fisico del disco
                auxByteLeidos = bread(nbfisico, buf_bloque);
                if (auxByteLeidos == FALLO)
                {
                    fprintf(stderr, "Error mi_read_f(): bread()\n");
                    return FALLO;
                }
                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            bytesleidos += BLOCKSIZE;
        }

        //Parte 3: Último bloque leido parcialmente
        //Obtenemos el bloque fisico
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        //Parte 1: Primero bloque leido parcialmente
        if (nbfisico != FALLO)
        {
            //Leemos el bloque fisico del disco
            auxByteLeidos = bread(nbfisico, buf_bloque);
            if (auxByteLeidos == FALLO)
            {
                fprintf(stderr, "Error mi_read_f(): bread()\n");
                return FALLO;
            }
            //Calculamos el byte lógico del último bloque hasta donde hemos de leer
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
        }
        bytesleidos += desp2 + 1;
    }

    //Leer el inodo actualizado.
    mi_waitSem(); //Semaforos
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "Error leer_inodo(): mi_read_f()\n");
        return FALLO;
    }

    //Actualizar la metainformación
    inodo.atime = time(NULL);

    //Escribimos inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "Error escribir_inodo(): mi_read_f()\n");
        return FALLO;
    }
    mi_signalSem(); //Semaforos

    //Comprobar que no haya errores de escritura y que se haya escrito todo bien.
    if (nbytes == bytesleidos)
    {
#if DEBUGGER
        fprintf(stderr, "\tmi_read_f: BIEN\n");
#endif
        return bytesleidos;
    }
    else
    {
#if DEBUGGER
        fprintf(stderr, "mi_read_f(): MAL\n\tnbytes:%i\n\tbytesleidos:%i\n", nbytes, bytesleidos);
#endif
        return FALLO;
    }
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
    mi_waitSem();
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    inodo.permisos=permisos;
    inodo.ctime=time(NULL);
    if(escribir_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    mi_signalSem();
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