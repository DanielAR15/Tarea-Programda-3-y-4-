/* Este archivo tiene la logica del proceso con rank 0. Su trabajo es recibir los mensajes de los demas procesos (clientes) y mandarlos para donde corresponda:*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include "protocolo.h"


static void escribir_bitacora(FILE *log, const char *texto) {
    time_t ahora = time(NULL);
    char *fecha = ctime(&ahora);

    fecha[strlen(fecha) - 1] = '\0';

    fprintf(log, "[%s] %s\n", fecha, texto);
    fflush(log); 

    printf("[COORDINADOR] %s\n", texto);
    fflush(stdout);
}

void ejecutar_coordinador(int num_procesos) {
    Mensaje msg;
    MPI_Status estado;
    char linea_log[700];

    FILE *log = fopen("bitacora.txt", "a");
    if (log == NULL) {
        printf("No se pudo abrir el archivo de bitacora, sigo sin guardar log.\n");
    }


    int total_mensajes_directos = 0;
    int total_mensajes_difusion = 0;

    if (log != NULL) {
        escribir_bitacora(log, "Coordinador iniciado, esperando mensajes...");
    }
    printf("[COORDINADOR] Listo. Numero de procesos en total: %d\n", num_procesos);

    int clientes_activos = num_procesos - 1;

    while (clientes_activos > 0) {

        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);

        limpiar_mensaje(&msg);
        MPI_Recv(&msg, sizeof(Mensaje), MPI_BYTE, estado.MPI_SOURCE,
                 estado.MPI_TAG, MPI_COMM_WORLD, &estado);

        if (msg.tipo == TIPO_DIRECTO) {
            total_mensajes_directos++;

            sprintf(linea_log, "DIRECTO de %s (rank %d) para rank %d: %s",
                    msg.nombre_origen, msg.rank_origen, msg.rank_destino, msg.texto);
            if (log != NULL) escribir_bitacora(log, linea_log);

            MPI_Send(&msg, sizeof(Mensaje), MPI_BYTE, msg.rank_destino,
                     TAG_DIRECTO, MPI_COMM_WORLD);

        } else if (msg.tipo == TIPO_DIFUSION) {
            total_mensajes_difusion++;

            sprintf(linea_log, "DIFUSION de %s (rank %d): %s",
                    msg.nombre_origen, msg.rank_origen, msg.texto);
            if (log != NULL) escribir_bitacora(log, linea_log);

            for (int destino = 1; destino < num_procesos; destino++) {
                if (destino != msg.rank_origen) {
                    MPI_Send(&msg, sizeof(Mensaje), MPI_BYTE, destino,
                             TAG_DIFUSION, MPI_COMM_WORLD);
                }
            }

        } else if (msg.tipo == TIPO_SALIR) {
            sprintf(linea_log, "%s (rank %d) se desconecto.",
                    msg.nombre_origen, msg.rank_origen);
            if (log != NULL) escribir_bitacora(log, linea_log);

            clientes_activos--;
        }
    }

    if (log != NULL) {
        char resumen[200];
        sprintf(resumen, "Coordinador terminando. Total directos: %d, Total difusiones: %d",
                total_mensajes_directos, total_mensajes_difusion);
        escribir_bitacora(log, resumen);
        fclose(log);
    }

    printf("[COORDINADOR] Todos los clientes se desconectaron. Cerrando.\n");
}
