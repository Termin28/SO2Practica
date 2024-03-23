//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "ficheros.h"

int main(int argc,char **argv){
    if(argc!=4){
        fprintf(stderr,"Error: La sintaxis es incorrecta. Sintaxis: ./truncar <nombre_dispositivo> <ninodo> <nbytes>");
        return FALLO;
    }
    if(bmount(argv[1])==FALLO){
        return FALLO;
    }
    int nbytes=atoi(argv[3]);
    int ninodo=atoi(argv[2]);
    if(nbytes==0){
        if(liberar_inodo(ninodo)==FALLO){
            return FALLO;
        }
    }else{
        if(mi_truncar_f(ninodo,nbytes)==FALLO){
            return FALLO;
        }
    }
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    printf("\nDATOS INODO %d:\n",ninodo);
    printf("tipo=%c\n",inodo.tipo);
    printf("permisos=%d\n",inodo.permisos);

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("atime: %s\n",atime);
    printf("ctime: %s\n",ctime);
    printf("mtime: %s\n",mtime);
    printf("nlinks=%d\n",inodo.nlinks);
    printf("tamEnBytesLog=%d\n",inodo.tamEnBytesLog);
    printf("numBloquesOcupados=%d\n",inodo.numBloquesOcupados);

    if(bumount()==FALLO){
        return FALLO;
    }
}