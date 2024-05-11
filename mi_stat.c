//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc!=3){
        fprintf(stderr,"Error: La sintaxis es incorrecta. Sintaxis: ./mi_stat <disco> </ruta>\n");
        return FALLO;
    }

    if(bmount(argv[1])==FALLO){
        return FALLO;
    }

    struct STAT stat;
    int error=mi_stat(argv[2],&stat);
    if(error<0){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    ts = localtime(&stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    printf("Nº Inodo: %d\n",error);
    printf("tipo: %c\n",stat.tipo);
    printf("permisos: %d\n",stat.permisos);
    printf("atime: %s\n",atime);
    printf("ctime: %s\n",ctime);
    printf("mtime: %s\n",mtime);
    printf("nlinks: %d\n",stat.nlinks);
    printf("tamEnBytesLog: %d\n",stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n",stat.numBloquesOcupados);

    if(bumount()==FALLO){
        return FALLO;
    }
}