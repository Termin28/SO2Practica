#include "verificacion.h"

int main(int argc,char **argv){
    if(argc!=3){
        fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulación>\n"RESET);
        return FALLO;
    }

    if(bmount(argv[1])==FALLO){
        return FALLO;
    }
    struct STAT stat;
    if(mi_stat(argv[2],&stat)<0){
        return FALLO;
    }
    int numentradas=stat.tamEnBytesLog/sizeof(struct entrada);
    if(numentradas!=NUMPROCESOS){
        return FALLO;
    }

    char nombre[TAMFILA]="informe.txt";
    sprintf(nombre,"%s%s",argv[2],"informe.txt");
    if(mi_creat(nombre,7)<0){
        return FALLO;
    }
    struct entrada entradas[numentradas];
    if(mi_read(argv[2],&entradas,0,sizeof(entradas))<0){
        return FALLO;
    }
    for(int i=0;i<numentradas;i++){
        char *proceso=strchr(entradas[i].nombre,'_');
        pid_t pid=atoi(proceso+1); // +1 para quitar el "_"

        struct INFORMACION informacion;
        informacion.pid=pid;
        informacion.nEscrituras=0;
    }
}