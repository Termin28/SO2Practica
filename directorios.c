//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "directorios.h"

int indicecache=0;
static struct UltimaEntrada UltimasEntradas[CACHE_SIZE];

/**
 * Función para separar en 2 una cadena de caracteres.
 * Recibe: El camino que hay que separar, un puntero por referencia donde se almacenara la parte inicial (la parte ubicada entre los dos primeros '/'),
 * un puntero donde se almacena la parte final (el resto del camino después del segundo '/' incluido), un puntero al tipo de elemento que es
 * el elemento almacenado en inicial ('f' fichero, 'd' directorio).
 * Devuelve: 0 Si todo va bien. En caso de error devuelve -1.
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    if(camino[0]!='/'){
        return FALLO;
    }
    char *resto=strchr(camino+1,'/');
    if(resto==NULL){
        strcpy(inicial,camino+1); 
        strcpy(final,"");
        strcpy(tipo,"f");
    }else{
        strcpy(final,resto);
        strncpy(inicial,camino+1,strlen(camino+1) - strlen(resto)); //N= Tamaño del camino(sin la primera '/') - tamaño del resto 
        strcpy(tipo,"d");
    }
    return EXITO;
}

/**
 * Función para buscar una entrada determinada entre las entrada del inodo correspondiente a su inodo padre.
 * Recibe: Dirección del directorio de la entrada, puntero al inodo del directorio padre, numero del inodo al que esta asociado
 * la entrada buscada, el numero de la entrada dentro de p_inodo_dir que lo contiene, un valor para indicar si hay que reservar el inodo,
 * permisos del inodo.
 * Devuelve: 0 si todo va bien. Un valor menor a 0 si hay un error.
*/
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    struct superbloque SB;
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo;
    int num_entrada_inodo;
    memset(inicial,0,sizeof(entrada.nombre));
    memset(final,0,strlen(camino_parcial));

    if(strcmp(camino_parcial,"/")==0){
        if(bread(posSB,&SB)==FALLO){
            return FALLO;
        }
        *p_inodo=SB.posInodoRaiz;
        *p_entrada=0;
        return EXITO;
    }
    if(extraer_camino(camino_parcial,inicial,final,&tipo)==FALLO){
        return ERROR_CAMINO_INCORRECTO;
    }
    #if DEBUGN7
        fprintf(stderr,GRAY"[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n"RESET, inicial, final, reservar);
    #endif
    if(leer_inodo(*p_inodo_dir,&inodo_dir)==FALLO){
        return FALLO;
    }
    if((inodo_dir.permisos&4)!=4){
        #if DEBUGN7
            fprintf(stderr,"[buscar_entrada()->El inodo %d no tiene permisos de lectura]\n",*p_inodo_dir);
        #endif
        return ERROR_PERMISO_LECTURA;
    }

    memset(entrada.nombre,0,sizeof(entrada.nombre));
    cant_entradas_inodo=inodo_dir.tamEnBytesLog/sizeof(struct entrada);
    num_entrada_inodo=0;
    if(cant_entradas_inodo>0){
        if(mi_read_f(*p_inodo_dir,&entrada,num_entrada_inodo*sizeof(struct entrada),sizeof(struct entrada))==FALLO){
            return FALLO;
        }
        while((num_entrada_inodo<cant_entradas_inodo)&&(strcmp(inicial,entrada.nombre)!=0)){
            num_entrada_inodo++;
            memset(entrada.nombre,0,sizeof(entrada.nombre));
            if(mi_read_f(*p_inodo_dir,&entrada,num_entrada_inodo*sizeof(struct entrada),sizeof(struct entrada))==FALLO){
                return FALLO;
            }
        }
    }
    if((strcmp(inicial,entrada.nombre)!=0)&&(num_entrada_inodo==cant_entradas_inodo)){
        switch(reservar){
            case 0:
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1:
                if(inodo_dir.tipo=='f'){
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }
                if((inodo_dir.permisos&2)!=2){
                    return ERROR_PERMISO_ESCRITURA;
                }else{
                    strcpy(entrada.nombre,inicial);
                    if(tipo=='d'){
                        if(strcmp(final,"/")==0){
                            entrada.ninodo=reservar_inodo('d',permisos);
                            #if DEBUGN7
                                fprintf(stderr,GRAY"[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n"RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
                            #endif
                        }else{
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    }else{
                        entrada.ninodo=reservar_inodo('f',permisos);
                        #if DEBUGN7
                            fprintf(stderr,GRAY"[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n"RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
                        #endif
                    }
                    #if DEBUGN7
                        fprintf(stderr,GRAY"[buscar_entrada()->creada entrada: %s, %d] \n"RESET, inicial, entrada.ninodo);
                    #endif
                    if(mi_write_f(*p_inodo_dir,&entrada,num_entrada_inodo*sizeof(struct entrada),sizeof(struct entrada))==FALLO){
                        if(entrada.ninodo!=FALLO){
                            liberar_inodo(entrada.ninodo);
                        }
                        return FALLO;
                    }
                }
        }
    }
    if((strcmp(final,"/")==0)||(strcmp(final,"")==0)){
        if((num_entrada_inodo<cant_entradas_inodo)&&(reservar==1)){
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo=entrada.ninodo;
        *p_entrada=num_entrada_inodo;
        return EXITO;
    }else{
        *p_inodo_dir=entrada.ninodo;
        return buscar_entrada(final,p_inodo_dir,p_inodo,p_entrada,reservar,permisos);
    }
}

/**
 * Función para mostrar un mensaje de error por pantalla
 * Recibe: El valor del error
*/
void mostrar_error_buscar_entrada(int error){
    switch(error){
        case ERROR_CAMINO_INCORRECTO:
            fprintf(stderr,RED"Error: Camino incorrecto.\n"RESET);
            break;
        case ERROR_NO_EXISTE_ENTRADA_CONSULTA:
            fprintf(stderr,RED"Error: No existe el archivo o el directorio.\n"RESET);
            break;
        case ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO:
            fprintf(stderr,RED"Error: No existe algún directorio intermedio.\n"RESET);
            break;
        case ERROR_ENTRADA_YA_EXISTENTE:
            fprintf(stderr,RED"Error: El archivo ya existe.\n"RESET);
            break;
        case ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO:
            fprintf(stderr,RED"Error: No es un directorio.\n"RESET);
            break;
        case ERROR_PERMISO_ESCRITURA:
            fprintf(stderr,RED"Error: Permiso denegado de escritura.\n"RESET);
            break;
        case ERROR_PERMISO_LECTURA:
            fprintf(stderr,RED"Error: Permiso denegado de lectura.\n"RESET);
            break;
    }
}

/**
 * Función que crea un fichero o directorio y su entrada de directorio
 * Recibe: El camino donde crear el fichero/directorio, los permisos del fichero/directorio
 * Devuelve 0 si todo va bien. Un valor menor a 0 si da error. 
*/
int mi_creat(const char *camino, unsigned char permisos){
    mi_waitSem();
    unsigned int p_inodo_dir=0;
    unsigned int p_inodo=0;
    unsigned int p_entrada=0;
    int error=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,1,permisos);
    if(error<0){
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }
    mi_signalSem();
    return EXITO;
}

/**
 * Función para poner el contenido de un directorio en un buffer de memoria.
 * Recibe: El camino del directorio, el buffer de memoria, el tipo de elemento (fichero/directorio);
 * Devuelve: El numero de entradas. En caso de error devuelve un valor menor a 0
*/
int mi_dir(const char *camino, char *buffer, char tipo){
    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }
    unsigned int p_inodo_dir=SB.posInodoRaiz;
    unsigned int p_inodo=SB.posInodoRaiz;
    unsigned int p_entrada=0;
    int error=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,4);
    if(error<0){
        mostrar_error_buscar_entrada(error);
        return error;
    }
    struct inodo inodo;
    if(leer_inodo(p_inodo,&inodo)==FALLO){
        return FALLO;
    }

    if((inodo.permisos&4)!=4){
        return FALLO;
    }

    char tmp[TAMFILA];
    char tam[10];
    struct entrada entrada;
    int nentradas=inodo.tamEnBytesLog/sizeof(struct entrada);
    if(tipo=='d'){
        struct entrada entradas[BLOCKSIZE/sizeof(struct entrada)];
        memset(entradas,0,sizeof(struct entrada));
        int leidos=0;
        leidos=mi_read_f(p_inodo,entradas,0,BLOCKSIZE);
        if(leidos==FALLO){
            return FALLO;
        }
        for(int i=0;i<nentradas;i++){
            if(leer_inodo(entradas[i%(BLOCKSIZE/sizeof(struct entrada))].ninodo,&inodo)==FALLO){
                return FALLO;
            }
            if(inodo.tipo=='d'){
                strcat(buffer, "d");
            }else{
                strcat(buffer, "f"); 
            }
            strcat(buffer,"\t");

            if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
            if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
            if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
            strcat(buffer,"\t");

            struct tm *tm; //ver info: struct tm
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
            strcat(buffer, tmp);
            strcat(buffer,"\t");

            sprintf(tam,"%d",inodo.tamEnBytesLog);
            strcat(buffer,tam);
            strcat(buffer,"\t");

            if(inodo.tipo=='d'){
                strcat(buffer,ORANGE);
            }else{
                strcat(buffer,BLUE);
            }
            
            strcat(buffer,entradas[i%(BLOCKSIZE/sizeof(struct entrada))].nombre);
            while((strlen(buffer)%TAMFILA)!=0){
                strcat(buffer," ");
            }
            strcat(buffer, RESET);
            strcat(buffer, "\n");

            if (leidos % (BLOCKSIZE / sizeof(struct entrada)) == 0){
                leidos += mi_read_f(p_inodo, entradas, leidos, BLOCKSIZE);
            }
        }
    }else{
        if(mi_read_f(p_inodo_dir, &entrada, sizeof(entrada)*p_entrada, sizeof(struct entrada))==FALLO){
            return FALLO;
        }

        if(leer_inodo(entrada.ninodo,&inodo)==FALLO){
            return FALLO;
        }

        strcat(buffer, "f");  
        strcat(buffer,"\t");

        if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
        if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
        if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
        strcat(buffer,"\t");

        struct tm *tm; //ver info: struct tm
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer,"\t");

        sprintf(tam,"%d",inodo.tamEnBytesLog);
        strcat(buffer,tam);
        strcat(buffer,"\t");

        strcat(buffer,BLUE);
        strcat(buffer,entrada.nombre);
        while((strlen(buffer)%TAMFILA)!=0){
            strcat(buffer," ");
        }
        strcat(buffer, RESET);
        strcat(buffer, "\n");
    }
    return nentradas;
}

/**
 * Función que cambia los permisos de un fichero o directorio
 * Recibe: El camino del fichero/directorio, los permisos que poner
 * Devuelve: 0 si todo va bien. Un valor menor a 0 si da error
*/
int mi_chmod(const char *camino, unsigned char permisos){
    unsigned int p_inodo_dir=0;
    unsigned int p_inodo=0;
    unsigned int p_entrada=0;
    int error=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,permisos);
    if(error<0){
        return error;
    }
    if(mi_chmod_f(p_inodo,permisos)==FALLO){
        return FALLO;
    }
    return EXITO;
}

/**
 * Función que muestra la información acerca de un inodo de un fichero o de un directorio
 * Recibe: El camino del fichero/directorio, recibe un puntero del struct STAT donde almacenar la información
 * Devuelve: p_inodo di todo va bien. En caso de error devuelve menor a 0
*/
int mi_stat(const char *camino, struct STAT *p_stat){
    unsigned int p_inodo_dir=0;
    unsigned int p_inodo=0;
    unsigned int p_entrada=0;
    int error=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,p_stat->permisos);
    if(error<0){
        return error;
    }

    if(mi_stat_f(p_inodo,p_stat)==FALLO){
        return FALLO;
    }
    return p_inodo;
}

/**
 * Función para escribir el contenido de un buffer en un fichero
 * Recibe: El camino dle fichero al que escribir el contenido, el buffer que contiene el contenido a escribir,
 * el offset a partir del que escribir, NºBytes a escribir.
 * Devuelve:Nº de Bytes escritos, en caso de error devuelve un valor menor a 0.
*/
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir=0;
    unsigned int p_inodo=0;
    unsigned int p_entrada=0;
    int existe=0;
    indicecache=indicecache%3;
    for(int i=0;i<CACHE_SIZE;i++){
        if(strcmp(UltimasEntradas[i].camino, camino) == 0){
            #if DEBUGN9==2
                fprintf(stderr,BLUE"Utilizamos cache[%d]: %s\n"RESET,i,camino);
            #endif
            #if DEBUGN9
                fprintf(stderr,BLUE"[mi_write() → Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n"RESET);
            #endif
            p_inodo=UltimasEntradas[i].p_inodo;
            existe=1;
            break;
        }
    }
    if(!existe){
        int error=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,4);
        if(error<0){
            fprintf(stderr,RED"\nHola\n"RESET);
            mostrar_error_buscar_entrada(error);
            return error;
        }
        strcpy(UltimasEntradas[indicecache].camino,camino);
        UltimasEntradas[indicecache].p_inodo=p_inodo;
        #if DEBUGN9==2
            fprintf(stderr,ORANGE"Reemplazamos la cache[%d]: %s\n"RESET,indicecache,camino);
        #endif
        #if DEBUGN9
            fprintf(stderr,ORANGE"[mi_write() → Actualizamos la caché de escritura]\n"RESET);
        #endif
        indicecache++;
    }
    
    int escritos=mi_write_f(p_inodo, buf, offset, nbytes);
    if(escritos<0){
        return FALLO;
    }
    return escritos;
}

/**
 * Función que lee los Nº Bytes de un fichero y lo almacena en un buffer.
 * Recibe: El camino donde leer el fichero, buffer donde escribir el contenido leido,
 * offset a partir del que leer, Nº Bytes a leer.
 * Devuelve: NºBytes leido, valor menor a 0 en caso de error.
*/
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir=0;
    unsigned int p_inodo=0;
    unsigned int p_entrada=0;
    indicecache=indicecache%3;
    int existe=1;
    for(int i=0;i<CACHE_SIZE;i++){
        if(strcmp(UltimasEntradas[i].camino, camino) == 0){
            p_inodo=UltimasEntradas[i].p_inodo;
            existe=0;
            #if DEBUGN9
                fprintf(stderr,BLUE"\n[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n"RESET);
            #endif
            break;
        }
    }
    if(existe){
        int error=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,4);
        if(error<0){
            return error;
        }
        strcpy(UltimasEntradas[indicecache].camino,camino);
        UltimasEntradas[indicecache].p_inodo=p_inodo;
        #if DEBUGN9
            fprintf(stderr,ORANGE"\n[mi_read() → Actualizamos la cache de lectura]\n"RESET);
        #endif
    }
    int leidos=mi_read_f(p_inodo,buf,offset,nbytes);
    if(leidos<0){
        return FALLO;
    }
    return leidos;
}

int mi_link(const char *camino1, const char *camino2){
    mi_waitSem();
    unsigned int p_inodo_dir1=0;
    unsigned int p_inodo1=0;
    unsigned int p_entrada1=0;
    int error=buscar_entrada(camino1,&p_inodo_dir1,&p_inodo1,&p_entrada1,0,4);
    if(error<0){
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }
    struct inodo inodo;
    if(leer_inodo(p_inodo1,&inodo)==FALLO){
        mi_signalSem();
        return FALLO;
    }
    if(inodo.tipo!='f'){
        //FICHERO
        mi_signalSem();
        return FALLO;
    }
    if((inodo.permisos&4)!=4){
        mi_signalSem();
        return FALLO;
    }

    unsigned int p_inodo_dir2=0;
    unsigned int p_inodo2=0;
    unsigned int p_entrada2=0;
    error=buscar_entrada(camino2,&p_inodo_dir2,&p_inodo2,&p_entrada2,1,6);
    if(error<0){
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }
    struct entrada entrada2;
    if(mi_read_f(p_inodo_dir2,&entrada2,sizeof(struct entrada)*p_entrada2,sizeof(struct entrada))==FALLO){
        mi_signalSem();
        return FALLO;
    }
    entrada2.ninodo=p_inodo1;
    if(mi_write_f(p_inodo_dir2,&entrada2,sizeof(struct entrada)*p_entrada2,sizeof(struct entrada))==FALLO){
        mi_signalSem();
        return FALLO;
    }
    if(liberar_inodo(p_inodo2)==FALLO){
        mi_signalSem();
        return FALLO;
    }
    inodo.nlinks++;
    inodo.ctime=time(NULL);
    if(escribir_inodo(p_inodo1,&inodo)==FALLO){
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    return EXITO;
}

int mi_unlink(const char *camino){
    mi_waitSem();
    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        mi_signalSem();
        return FALLO;
    }

    unsigned int p_inodo_dir=SB.posInodoRaiz;
    unsigned int p_inodo=SB.posInodoRaiz;
    unsigned int p_entrada=0;
    int error=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,4);
    if(error<0){
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }
    if(SB.posInodoRaiz==p_inodo){
        mi_signalSem();
        return FALLO;
    }
    struct inodo inodo;
    if(leer_inodo(p_inodo,&inodo)==FALLO){
        mi_signalSem();
        return FALLO;
    }
    if(inodo.tamEnBytesLog>0 && inodo.tipo=='d'){
        fprintf(stderr,RED"El directorio %s no esta vacio\n"RESET,camino);
        mi_signalSem();
        return FALLO;
    }

    struct inodo inodo_dir;
    if(leer_inodo(p_inodo_dir,&inodo_dir)==FALLO){
        mi_signalSem();
        return FALLO;
    }
    int nentradas=inodo_dir.tamEnBytesLog/sizeof(struct entrada);
    if(p_entrada==(nentradas-1)){
        if(mi_truncar_f(p_inodo_dir,sizeof(struct entrada)*(nentradas-1))==FALLO){
            mi_signalSem();
            return FALLO;
        }
    }else{
        struct entrada entrada;
        if(mi_read_f(p_inodo_dir,&entrada,sizeof(struct entrada)*(nentradas-1),sizeof(struct entrada))==FALLO){
            mi_signalSem();
            return FALLO;
        }
        if(mi_write_f(p_inodo_dir,&entrada,sizeof(struct entrada)*(p_entrada),sizeof(struct entrada))==FALLO){
            mi_signalSem();
            return FALLO;
        }
        if(mi_truncar_f(p_inodo_dir,sizeof(struct entrada)*(nentradas-1))==FALLO){
            mi_signalSem();
            return FALLO;
        }
    }
    inodo.nlinks--;
    if(inodo.nlinks==0){
        if(liberar_inodo(p_inodo)==FALLO){
            mi_signalSem();
            return FALLO;
        }
    }else{
        inodo.ctime=time(NULL);
        if(escribir_inodo(p_inodo,&inodo)==FALLO){
            mi_signalSem();
            return FALLO;
        }
    }
    mi_signalSem();
    return EXITO;
}