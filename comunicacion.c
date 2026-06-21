/*Implementacion del hilo de red y de las funciones para enviar mensajes*/


#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include "comunicacion.h"

void Inicializar_Cliente(ClienteEstado *cliente, int rank, int num_procesos) {
    cliente->mi_rank = rank;
    cliente->num_procesos = num_procesos;
    sprintf(cliente->mi_nombre, "usuario%d", rank);
    cliente->cantidad_mensajes = 0;
    cliente->debe_terminar = 0;
    pthread_mutex_init(&cliente->candado_lista, NULL);
}

static void *hilo_red(void *argumento) {
    ClienteEstado *cliente = (ClienteEstado *) argumento;
    Mensaje msg;
    MPI_Status estado;

    while (1) {
        limpiar_mensaje(&msg);
        MPI_Recv(&msg, sizeof(Mensaje), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG,
                 MPI_COMM_WORLD, &estado);

        if (cliente->debe_terminar == 1) {
            break;
        }

        pthread_mutex_lock(&cliente->candado_lista);

        if (cliente->cantidad_mensajes < MAX_MENSAJES_GUARDADOS) {
            cliente->mensajes_recibidos[cliente->cantidad_mensajes] = msg;
            cliente->cantidad_mensajes++;
        } else {

            printf("[%s] Aviso: ya no hay espacio para mas mensajes guardados.\n",
                   cliente->mi_nombre);
        }

        pthread_mutex_unlock(&cliente->candado_lista);
    }

    return NULL;
}

void iniciar_hilo_red(ClienteEstado *cliente) {
    pthread_create(&cliente->hilo_red, NULL, hilo_red, cliente);
}

void detener_hilo_red(ClienteEstado *cliente) {
    cliente->debe_terminar = 1;

    Mensaje msg;
    limpiar_mensaje(&msg);
    msg.tipo = TIPO_SALIR;
    MPI_Send(&msg, sizeof(Mensaje), MPI_BYTE, cliente->mi_rank, 99, MPI_COMM_WORLD);

    pthread_join(cliente->hilo_red, NULL);
}

void envio_directo(ClienteEstado *cliente, int rank_destino, const char *texto) {
    Mensaje msg;
    limpiar_mensaje(&msg);

    msg.tipo = TIPO_DIRECTO;
    msg.rank_origen = cliente->mi_rank;
    msg.rank_destino = rank_destino;
    strncpy(msg.nombre_origen, cliente->mi_nombre, TAM_NOMBRE - 1);
    strncpy(msg.texto, texto, TAM_TEXTO - 1);

    MPI_Send(&msg, sizeof(Mensaje), MPI_BYTE, 0, TAG_DIRECTO, MPI_COMM_WORLD);
}

void enviar_difusion(ClienteEstado *cliente, const char *texto) {
    Mensaje msg;
    limpiar_mensaje(&msg);

    msg.tipo = TIPO_DIFUSION;
    msg.rank_origen = cliente->mi_rank;
    msg.rank_destino = -1; /* no aplica en difusion */
    strncpy(msg.nombre_origen, cliente->mi_nombre, TAM_NOMBRE - 1);
    strncpy(msg.texto, texto, TAM_TEXTO - 1);

    MPI_Send(&msg, sizeof(Mensaje), MPI_BYTE, 0, TAG_DIFUSION, MPI_COMM_WORLD);
}

void informar_salida(ClienteEstado *cliente) {
    Mensaje msg;
    limpiar_mensaje(&msg);

    msg.tipo = TIPO_SALIR;
    msg.rank_origen = cliente->mi_rank;
    strncpy(msg.nombre_origen, cliente->mi_nombre, TAM_NOMBRE - 1);

    MPI_Send(&msg, sizeof(Mensaje), MPI_BYTE, 0, TAG_SALIR, MPI_COMM_WORLD);
}

int obtener_siguiente_mensaje(ClienteEstado *cliente, Mensaje *destino) {
    int hay_mensaje = 0;

    pthread_mutex_lock(&cliente->candado_lista);

    if (cliente->cantidad_mensajes > 0) {

        *destino = cliente->mensajes_recibidos[0];

        for (int i = 1; i < cliente->cantidad_mensajes; i++) {
            cliente->mensajes_recibidos[i - 1] = cliente->mensajes_recibidos[i];
        }
        cliente->cantidad_mensajes--;

        hay_mensaje = 1;
    }

    pthread_mutex_unlock(&cliente->candado_lista);

    return hay_mensaje;
}
