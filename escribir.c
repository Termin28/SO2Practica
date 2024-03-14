#include "ficheros.h"
int main(int argc,char const *argv[]){
    int noffset=5;
    int offset[]={9000,209000,30725000,409605000,480000000};
    if(argc!=4){
        fprintf(stderr,RED"Sintaxis: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n"RESET);
        fprintf(stderr,RED"Offsets: %d, %d, %d, %d, %d\n"RESET,offset[0],offset[1],offset[2],offset[3],offset[4]);
        fprintf(stderr,RED"Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n"RESET);
        return FALLO;
    }
    
    if(bmount(argv[1])==FALLO){
        return FALLO;
    }
    int len=strlen(argv[2]);
    printf("longitud texto: %d\n",len);
    char buffer[len];
    strcpy(buffer,argv[2]);
    int diferentes_inodos=atoi(argv[3]);

    if(diferentes_inodos==0){
        struct STAT stat;
        int ninodo=reservar_inodo('f',6);
        for(int i=0;i<noffset;i++){
            printf("\nNº inodo reservado: %d\n",ninodo);
            printf("offset: %d\n",offset[i]);
            int escritos=mi_write_f(ninodo,buffer,offset[i],len);
            printf("Bytes escritos: %d\n",escritos);
            if(mi_stat_f(ninodo,&stat)==FALLO){
                return FALLO;
            }
            printf("stat.tamEnBytesLog=%d\n",stat.tamEnBytesLog);
            printf("stat.numBloquesOcupados=%d\n",stat.numBloquesOcupados);
        }
    }else if(diferentes_inodos==1){
        struct STAT stat;
        int ninodo;
        for(int i=0;i<noffset;i++){
            ninodo=reservar_inodo('f',6);
            printf("\nNº inodo reservado: %d\n",ninodo);
            printf("offset: %d\n",offset[i]);
            int escritos=mi_write_f(ninodo,buffer,offset[i],len);
            printf("Bytes escritos: %d\n",escritos);
            if(mi_stat_f(ninodo,&stat)==FALLO){
                return FALLO;
            }
            printf("stat.tamEnBytesLog=%d\n",stat.tamEnBytesLog);
            printf("stat.numBloquesOcupados=%d\n",stat.numBloquesOcupados);
        }
    }
    
    if(bumount()==FALLO){
        return FALLO;
    }
}