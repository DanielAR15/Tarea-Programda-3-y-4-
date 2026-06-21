/*Version de linea de comandos del cliente. Sirve para probar la app sin necesidad de la interfaz grafica, y tambien para sacar metricas de tiempo con MPI_Wtime.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "comunicacion.h"

static void mostrar_nuevos_mensajes(ClienteEstado *cliente) {
    Mensaje msg;

    while (obtener_siguiente_mensaje(cliente, &msg) == 1) {
        if (msg.tipo == TIPO_DIRECTO) {
            printf("\n[%s -> mi] %s\n", msg.nombre_origen, msg.texto);
        } else if (msg.tipo == TIPO_DIFUSION) {
            printf("\n[DIFUSION de %s] %s\n", msg.nombre_origen, msg.texto);
        }
        printf("> "); 
        fflush(stdout);
    }
}

void ejecutar_cliente_cli(int rank, int num_procesos) {
    ClienteEstado cliente;
    char linea[600];

    Inicializar_Cliente(&cliente, rank, num_procesos);
    iniciar_hilo_red(&cliente);

    printf("Hola, soy %s (rank %d). Escribe 'ayuda' para ver los comandos.\n",
           cliente.mi_nombre, rank);
    printf("> ");
    fflush(stdout);

    int seguir = 1;
    while (seguir == 1) {

        mostrar_nuevos_mensajes(&cliente);

        if (fgets(linea, sizeof(linea), stdin) == NULL) {
            break;
        }

        int len = strlen(linea);
        if (len > 0 && linea[len - 1] == '\n') {
            linea[len - 1] = '\0';
        }

        if (strcmp(linea, "ayuda") == 0) {
            printf("Comandos:\n");
            printf("  enviar <rank> <texto>\n");
            printf("  difundir <texto>\n");
            printf("  salir\n");

        } else if (strcmp(linea, "salir") == 0) {
            seguir = 0;

        } else if (strncmp(linea, "enviar ", 7) == 0) {
            
            int rank_destino;
            char texto[TAM_TEXTO];

     
            int posicion = 0;
            if (sscanf(linea + 7, "%d %n", &rank_destino, &posicion) >= 1) {
                strcpy(texto, linea + 7 + posicion);

                if (rank_destino <= 0 || rank_destino >= num_procesos) {
                    printf("Ese rank no existe. Debe ser entre 1 y %d.\n", num_procesos - 1);
                } else if (rank_destino == rank) {
                    printf("No te puedes enviar un mensaje a ti mismo.\n");
                } else {
                    envio_directo(&cliente, rank_destino, texto);
                    printf("Mensaje enviado a usuario%d.\n", rank_destino);
              
                    printf("(enviado en el segundo %.3f desde que arranco MPI)\n",
                           MPI_Wtime());
                }
            } else {
                printf("Formato incorrecto. Usa: enviar <rank> <texto>\n");
            }

        } else if (strncmp(linea, "difundir ", 9) == 0) {
            char *texto = linea + 9;
            enviar_difusion(&cliente, texto);
            printf("Mensaje de difusion enviado a todos.\n");

        } else if (strlen(linea) == 0) {
       
        } else {
            printf("Comando no reconocido. Escribe 'ayuda' para ver opciones.\n");
        }

        if (seguir == 1) {
            printf("> ");
            fflush(stdout);
        }
    }

    informar_salida(&cliente);
    detener_hilo_red(&cliente);

    printf("Adios, %s.\n", cliente.mi_nombre);
}
