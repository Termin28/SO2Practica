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

    char *nombre=malloc(strlen(argv[2])+strlen("informe.txt"));
    sprintf(nombre,"%s%s",argv[2],"informe.txt");
    if(mi_creat(nombre,7)<0){
        return FALLO;
    }
    struct entrada entradas[numentradas];
    if(mi_read(argv[2],&entradas,0,sizeof(entradas))<0){
        return FALLO;
    }
    int escritos=0;
    for(int i=0;i<numentradas;i++){
        char *proceso=strchr(entradas[i].nombre,'_');
        pid_t pid=atoi(proceso+1); // +1 para quitar el "_"

        struct INFORMACION informacion;
        informacion.pid=pid;
        informacion.nEscrituras=0;

        int cant_registros_buffer_escrituras = 256; 
        struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        int offset=0;
        char *camino=malloc(strlen(argv[2])+strlen(entradas[i].nombre)+strlen("prueba.dat")+1);
        sprintf(camino,"%s%s%s",argv[2],entradas[i].nombre,"/prueba.dat");
        while (mi_read(camino, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0){
            int validadas=0;
            while(validadas<cant_registros_buffer_escrituras){
                if(buffer_escrituras[validadas].pid==pid){
                    if(informacion.nEscrituras==0){
                        informacion.MayorPosicion=buffer_escrituras[validadas];
                        informacion.MenorPosicion=buffer_escrituras[validadas];
                        informacion.PrimeraEscritura=buffer_escrituras[validadas];
                        informacion.UltimaEscritura=buffer_escrituras[validadas];
                        informacion.nEscrituras++;
                    }else{
                        /*
                        if(difftime(buffer_escrituras[validadas].fecha,informacion.PrimeraEscritura.fecha)>=0 && 
                            buffer_escrituras[validadas].nEscritura<informacion.PrimeraEscritura.nEscritura){
                                informacion.PrimeraEscritura=buffer_escrituras[validadas];
                        }*/
                        if(buffer_escrituras[validadas].nEscritura<informacion.PrimeraEscritura.nEscritura){
                            informacion.PrimeraEscritura=buffer_escrituras[validadas];
                        }else if(buffer_escrituras[validadas].nEscritura>informacion.UltimaEscritura.nEscritura){
                            informacion.UltimaEscritura=buffer_escrituras[validadas];
                        }
                        if(buffer_escrituras[validadas].nRegistro<informacion.MenorPosicion.nRegistro){
                            informacion.MenorPosicion=buffer_escrituras[validadas];
                        }else if(buffer_escrituras[validadas].nRegistro>informacion.MayorPosicion.nRegistro){
                            informacion.MayorPosicion=buffer_escrituras[validadas];
                        }
                        informacion.nEscrituras++;
                    }
                }
                validadas++;
            }
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
            offset+=sizeof(buffer_escrituras);
        }
        #if DEBUGN13
            fprintf(stderr,GRAY"[%d) %d escrituras validadas en %s]\n"RESET,i,informacion.nEscrituras,camino);
        #endif
        char primero[TAMFILA];
        char ultimo[TAMFILA];
        char mayor[TAMFILA];
        char menor[TAMFILA];
        struct tm *tm;
        tm=localtime(&informacion.PrimeraEscritura.fecha);
        strftime(primero,sizeof(primero),"%a %d-%m-%Y %H:%M:%S",tm);
        tm=localtime(&informacion.UltimaEscritura.fecha);
        strftime(ultimo,sizeof(ultimo),"%a %d-%m-%Y %H:%M:%S",tm);
        tm=localtime(&informacion.MayorPosicion.fecha);
        strftime(mayor,sizeof(mayor),"%a %d-%m-%Y %H:%M:%S",tm);
        tm=localtime(&informacion.MenorPosicion.fecha);
        strftime(menor,sizeof(menor),"%a %d-%m-%Y %H:%M:%S",tm);

        char buffer[BLOCKSIZE];
        memset(buffer,0,BLOCKSIZE);

        sprintf(buffer,"PID: %d\nNumero de escrituras: %d\n"
        "Primera Escritura\t%d\t%d\t%s"
        "\nUltima Escritura\t%d\t%d\t%s"
        "\nMenor Posicion\t%d\t%d\t%s"
        "\nMayor Posicion\t%d\t%d\t%s\n\n",pid,informacion.nEscrituras,
        informacion.PrimeraEscritura.nEscritura,informacion.PrimeraEscritura.nRegistro,primero,
        informacion.UltimaEscritura.nEscritura,informacion.UltimaEscritura.nRegistro,ultimo,
        informacion.MenorPosicion.nEscritura,informacion.MenorPosicion.nRegistro,menor,
        informacion.MayorPosicion.nEscritura,informacion.MayorPosicion.nRegistro,mayor);

        if((escritos+=mi_write(nombre,&buffer,escritos,strlen(buffer)))<0){
            if(bumount()){
                return FALLO;
            }
        }
        free(camino);
    }
    if(bumount()==FALLO){
        return FALLO;
    }
}