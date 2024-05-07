#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc!=3){
        fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./mi_rm disco /ruta"RESET);
        return FALLO;
    }

    if(argv[2][strlen(argv[2])-1]=='/'){
        fprintf(stderr,RED"Error: No es un fichero valido"RESET);
        return FALLO;
    }
    
    if(bmount(argv[1])==FALLO){
        return FALLO;
    }

    unsigned int p_inodo_dir=0;
    unsigned int p_inodo=0;
    unsigned int p_entrada = 0;
    int error=buscar_entrada(argv[2], &p_inodo_dir, &p_inodo, &p_entrada,0,4);
    if(error<0){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    struct inodo inodo;
    struct inodo aux;
    if(leer_inodo(p_inodo, &inodo)==FALLO){
        return FALLO;
    }
    int nentradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    struct entrada entrada;
    for (int i = 0; i < nentradas; i++){
        if(mi_read_f(p_inodo,&entrada,sizeof(entrada)*i,sizeof(entrada))==FALLO){
            return FALLO;
        }
        if(leer_inodo(entrada.ninodo, &aux)==FALLO){
            return FALLO;
        }
        if (inodo.tipo == 'd'){
            mi_unlink(entrada.nombre);
        }
    }
    if(bumount()==FALLO){
        return FALLO;
    }
}