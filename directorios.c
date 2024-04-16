#include "directorios.h"

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    if(camino[0]!='/'){
        return FALLO;
    }
    char aux[strlen(camino)];
    strcpy(aux,camino);
    char *token=strtok(aux,"/");

    char *resto=strchr(camino+1,'/');
    if(resto==NULL){
        strcpy(inicial,camino+1); //Copiar camino sin el primer '/'
        strcpy(final,"");
        strcpy(tipo,"f");
    }else{
        strcpy(final,resto);
        strcpy(inicial,token);
        strcpy(tipo,"d");
    }
    return EXITO;
}

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

    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }

    if(strcmp(camino_parcial,"/")==0){
        *p_inodo=SB.posInodoRaiz;
        *p_entrada=0;
        return EXITO;
    }

    if(extraer_camino(camino_parcial,inicial,final,&tipo)==FALLO){
        return ERROR_CAMINO_INCORRECTO;
    }

    fprintf(stderr,GRAY"[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n"RESET, inicial, final, reservar);

    if(leer_inodo(*p_inodo_dir,&inodo_dir)==FALLO){
        return FALLO;
    }

    if((inodo_dir.permisos&4)!=4){
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
                    return ERROR_PERMISO_LECTURA;
                }else{
                    strcpy(entrada.nombre,inicial);
                    if(tipo=='d'){
                        if(strcmp(final,"/")==0){
                            entrada.ninodo=reservar_inodo('d',permisos);
                            fprintf(stderr,GRAY"[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n"RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
                        }else{
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    }else{
                        entrada.ninodo=reservar_inodo('f',permisos);
                        fprintf(stderr,GRAY"[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n"RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
                    }
                    fprintf(stderr,GRAY"[buscar_entrada()->creada entrada: %s, %d] \n"RESET, inicial, entrada.ninodo);
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
    return EXITO;   
}

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