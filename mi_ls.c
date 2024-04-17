//Hecho por Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá
#include "directorios.h"

int main(int argc,char const *argv[]){
    if(argc<3 || argc>4){
        fprintf(stderr,"Error: La sintaxis es incorrecta. Sintaxis: ./mi_ls <disco> </ruta> o ./mi_ls -l <disco> </ruta>\n");
        return FALLO;
    }

    const char *camino;
    if(argc==3){
        if(bmount(argv[1])==FALLO){
            return FALLO;
        }
        camino=argv[2];
    }else{
        if(bmount(argv[2])==FALLO){
            return FALLO;
        }
        camino=argv[3];
    }
    
    char buffer[TAMBUFFER];
    int nentradas;
    if(camino[strlen(camino)-1]!='/'){
        nentradas=mi_dir(camino,buffer,'f');
    }else{
        nentradas=mi_dir(camino,buffer,'d');
    }
    if(nentradas<0){
        mostrar_error_buscar_entrada(nentradas);
        return FALLO;
    }
    printf("Total: %d\n",nentradas);
    if(nentradas!=0){
        printf("Tipo\tPermisos\tmTime\t\tTamaño\tNombre\n");
        printf("---------------------------------------------------------------------------------------------\n");
        printf("%s\n",buffer);
    }
    if(bumount()==FALLO){
        return FALLO;
    }
}