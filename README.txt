-Practica hecha por: Alexandre Hierro Pedrosa, Carlos Larruscain Monar y Jaume Ribas Gayá

-Sintaxis de los ficheros:
    escribir.c: ./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>
    leer.c: ./leer <nombre_dispositivo> <ninodo>
    truncar.c: ./truncar <nombre_dispositivo> <ninodo> <nbytes>
    permitir.c: ./permitir <nombre_dispositivo> <ninodo> <permisos>
    leer_sf.c: ./leer_sf <nombre_dispositivo>
    mi_mkfs.c: ./mi_mkfs <nombre_dispositivo> <nbloques>

    mi_touch.c: ./mi_touch <disco> <permisos> </ruta>
    mi_mkdir.c: ./mi_mkdir <disco> <permisos> </ruta>
    mi_ls.c: ./mi_ls <disco> </ruta> o ./mi_ls -l <disco> </ruta>
    mi_chmod.c: ./mi_chmod <disco> <permisos> </ruta>
    mi_stat.c: ./mi_stat <disco> </ruta>
    mi_escribir.c: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>
    mi_cat.c: ./mi_cat <disco> </ruta_fichero>
    mi_link.c: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>
    mi_rm.c: ./mi_rm <disco> </ruta>
    mi_rmdir.c: ./mi_rmdir <disco> </ruta>

-Observaciones:
    Se han realizado varias mejoras opcioanes, los mensajes de debug se pueden mostrar o ocultar
    poniendo el valor de DEBUGNX a 0(para ocultarlos) o a 1(para mostrarlos) donde X representa el nivel donde se 
    añadió el comentario. 

-Mejoras:
    -mi_ls: Se ha implementado las info de las entradas, se ha implementado la opción -l, los ficheros y directorios tienen colores diferentes, mi_ls funciona con ficheros.
    -mi_write y mi_read: Se ha implementado una cola FIFO
    -Se han creado los programas mi_touch y mi_rmdir