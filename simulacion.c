#include "simulacion.h"

int acabados=0;

void reaper(){
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
        acabados++;
    }
}

int main(int argc,char **argv){
    if(argc!=2){
        fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./simulacion <disco>\n"RESET);
        return FALLO;
    }
    if(bmount(argv[1])==FALLO){
        return FALLO;
    }
    char tmp[14];
    time_t tiempo;
    time(&tiempo);
    struct tm *tm;
    tm = localtime(&tiempo);
    sprintf(tmp, "%d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
    char simul[8]="/simul_";
    char *camino=malloc(strlen(simul)+strlen(tmp)+1);
    strcpy(camino,simul);
    strcat(camino,tmp);
    strcat(camino,"/");
    if(mi_creat(camino,6)<0){
        bumount();
        return FALLO;
    }
    signal(SIGCHLD, reaper);
    pid_t pid;
    for(int i=1;i<=NUMPROCESOS;i++){
        pid=fork();
        if(pid==0){
            if(bmount(argv[1])==FALLO){
                return FALLO;
            }
            char *aux=malloc(sizeof(camino)+sizeof(pid_t));
            strcpy(aux,simul);
            strcat(aux,tmp);
            char *temp=malloc(sizeof(pid_t));
            sprintf(temp,"%d",getpid());
            strcat(aux,temp);
            strcat(aux,"/");
            if(mi_creat(aux,6)<0){
                bumount();
                return FALLO;
            }
            char fichero[11]="prueba.dat";
            strcat(aux,fichero);
            if(mi_creat(aux,6)<0){
                bumount();
                return FALLO;
            }
            srand(time(NULL) + getpid());
            for(int j=1;j<=NUMESCRITURAS;j++){
                struct REGISTRO registro;
                registro.fecha=time(NULL);
                registro.pid=getpid();
                registro.nEscritura=j;
                registro.nRegistro=rand()%REGMAX;
                if(mi_write(aux,&registro,registro.nRegistro*sizeof(struct REGISTRO),sizeof(struct REGISTRO))<0){
                    return FALLO;
                }
                #if DEBUGN12
                    fprintf(stderr, "[simulación.c → Escritura %d en %s\n", j, aux);
                #endif
                usleep(50000);
            }
            fprintf(stderr, "Proceso %d: Completadas %d escrituras en %s\n", i, 
                    NUMESCRITURAS, aux);
            if(bumount()==FALLO){
                return FALLO;
            }
            exit(0);
        }
        usleep(150000);
    }
    while(acabados<NUMPROCESOS){
        pause();
    }
    if(bumount()==FALLO){
        return FALLO;
    }
    exit(0);
}