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
    /*
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
    */
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int bloque=reservar_bloque();
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }
    printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB SB.cantBloquesLibres = %d\n",bloque,SB.cantBloquesLibres);
    bloque=liberar_bloque(bloque);
    if(bread(posSB,&SB)==FALLO){
        return FALLO;
    }
    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %d\n",SB.cantBloquesLibres);

    printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    printf("posSB: %d → leer_bit(%d) = %d\n",posSB,posSB,leer_bit(posSB));
    printf("SB.posPrimerBloqueMB: %d → leer_bit(%d) = %d\n",SB.posPrimerBloqueMB,SB.posPrimerBloqueMB,leer_bit(SB.posPrimerBloqueMB));
    printf("SB.posUltimoBloqueMB: %d → leer_bit(%d) = %d\n",SB.posUltimoBloqueMB,SB.posUltimoBloqueMB,leer_bit(SB.posUltimoBloqueMB));
    printf("SB.posPrimerBloqueAI: %d → leer_bit(%d) = %d\n",SB.posPrimerBloqueAI,SB.posPrimerBloqueAI,leer_bit(SB.posPrimerBloqueAI));
    printf("SB.posUltimoBloqueAI: %d → leer_bit(%d) = %d\n",SB.posUltimoBloqueAI,SB.posUltimoBloqueAI,leer_bit(SB.posUltimoBloqueAI));
    printf("SB.posPrimerBloqueDatos: %d → leer_bit(%d) = %d\n",SB.posPrimerBloqueDatos,SB.posPrimerBloqueDatos,leer_bit(SB.posPrimerBloqueDatos));
    printf("SB.posUltimoBloqueDatos: %d → leer_bit(%d) = %d\n",SB.posUltimoBloqueDatos,SB.posUltimoBloqueDatos,leer_bit(SB.posUltimoBloqueDatos));

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];


    struct inodo inodo;
    int ninodo=0;
    leer_inodo(ninodo,&inodo);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    printf("\nDATOS DEL DIRECTORIO RAIZ\n");
    printf("tipo: %c\n",inodo.tipo);
    printf("permisos: %d\n",inodo.permisos);
    printf("atime: %s\n",atime);
    printf("ctime: %s\n",ctime);
    printf("mtime: %s\n",mtime);
    printf("nlinks: %d\n",inodo.nlinks);
    printf("tamEnBytesLog: %d\n",inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n",inodo.numBloquesOcupados);
}