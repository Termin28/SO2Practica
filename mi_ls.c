//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc<3 || argc>4){
        fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./mi_ls <disco> </ruta> o ./mi_ls -l <disco> </ruta>\n"RESET);
        return FALLO;
    }

    const char *camino;
    int simple=1;
    if(argc==3){
        if(bmount(argv[1])==FALLO){
            return FALLO;
        }
        camino=argv[2];
    }else{
        if(strcmp(argv[1],"-l")!=0){
            fprintf(stderr,RED"Error: La sintaxis es incorrecta. Sintaxis: ./mi_ls <disco> </ruta> o ./mi_ls -l <disco> </ruta>\n"RESET);
            return FALLO;
        }
        if(bmount(argv[2])==FALLO){
            return FALLO;
        }
        simple=0;
        camino=argv[3];
    }
    
    char buffer[TAMBUFFER];
    int nentradas;
    if(simple){
        nentradas=mi_dir(camino,buffer,'d');
        if(nentradas<0){
            return FALLO;
        }
        printf("Total: %d\n",nentradas);
        char *tok=strtok(buffer,"\n");
        while(tok!=NULL){
            char *name=strrchr(tok, '\t');
            name=name+1;
            char *aux=strchr(name,' ');
            aux[0]='\0';
            printf("%s ",name);
            tok=strtok(NULL,"\n");
        }
        printf("\n");
    }else{
        if(camino[strlen(camino)-1]!='/'){
            nentradas=mi_dir(camino,buffer,'f');
            if(nentradas<0){
                return FALLO;
            }
            printf("Tipo\tModo\tmTime\t\t\tTamaño\t\tNombre\n");
            printf("---------------------------------------------------------------------------------------------\n");
            printf("%s\n",buffer);
        }else{
            nentradas=mi_dir(camino,buffer,'d');
            if(nentradas<0){
                return FALLO;
            }
            printf("Total: %d\n",nentradas);
            if(nentradas!=0){
                printf("Tipo\tModo\tmTime\t\t\tTamaño\t\tNombre\n");
                printf("---------------------------------------------------------------------------------------------\n");
                printf("%s\n",buffer);
            }
        }
    }
    
    
    if(bumount()==FALLO){
        return FALLO;
    }
}
