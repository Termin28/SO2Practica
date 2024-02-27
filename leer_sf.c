#include "ficheros_basico.h"

int main(int argc,char **argv){
    if(argc!=2){
        fprintf(stderr,"Error: La sintaxis es incorrecta. Sintaxis: ./leer_sf <nombre_dispositivo>");
    }
    if(bmount(argv[1])==FALLO){
        return FALLO;
    }
    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n",SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n",SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n",SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n",SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n",SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n",SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n",SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n",SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n",SB.cantBloquesLibres);
    printf("cantInodosLibres = %d\n",SB.cantInodosLibres);
    printf("totBloques = %d\n",SB.totBloques);
    printf("totInodos = %d\n",SB.totInodos);

    printf("\nsizeof struct superbloque = %ld\n",sizeof(struct superbloque));
    printf("sizeof struct inodo = %ld\n",sizeof(struct inodo));

    printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    struct inodo inodo[BLOCKSIZE/INODOSIZE];
    int contador=0;
    for(int i=SB.posPrimerBloqueAI;i<=SB.posUltimoBloqueAI;i++){
        if(bread(i,inodo)==FALLO){
            return FALLO;
        }
        for(int j=0;j<BLOCKSIZE/INODOSIZE;j++){
            if(inodo[j].tipo=='l'){
                contador++;
                if(contador==SB.totInodos){
                    printf("-1\n");
                }else{
                    printf("%d ",contador);
                }
            }
            
        }
    }


}