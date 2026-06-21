

#ifndef COMUNICACION_H
#define COMUNICACION_H

#include <pthread.h>
#include "protocolo.h"

#define MAX_MENSAJES_GUARDADOS 200

typedef struct {
    int mi_rank;
    int num_procesos;
    char mi_nombre[TAM_NOMBRE];

    /* lista de mensajes recibidos, bien simple: un arreglo y un contador */
    Mensaje mensajes_recibidos[MAX_MENSAJES_GUARDADOS];
    int cantidad_mensajes;

    /* mutex para que el hilo de red y el hilo de la interfaz no
     * choquen al leer/escribir la lista al mismo tiempo */
    pthread_mutex_t candado_lista;

    /* bandera para avisarle al hilo de red que ya nos vamos a salir */
    int debe_terminar;

    pthread_t hilo_red;
} ClienteEstado;

/* Inicializa la estructura del cliente (rank, nombre, etc) */
void Inicializar_Cliente(ClienteEstado *cliente, int rank, int num_procesos);

/* Arranca el hilo que escucha mensajes con MPI_Recv en un ciclo */
void iniciar_hilo_red(ClienteEstado *cliente);

/* Le dice al hilo de red que pare y espera que termine (pthread_join) */
void detener_hilo_red(ClienteEstado *cliente);

/* Envia un mensaje directo a otro rank */
void envio_directo(ClienteEstado *cliente, int rank_destino, const char *texto);

/* Envia un mensaje de difusion (a todos) */
void enviar_difusion(ClienteEstado *cliente, const char *texto);

/* Avisa al coordinador que este cliente se va a desconectar */
void informar_salida(ClienteEstado *cliente);

/* Saca el siguiente mensaje nuevo de la lista (si hay).
 * Devuelve 1 si encontro un mensaje y lo copio en 'destino', 0 si no hay nada nuevo. */
int obtener_siguiente_mensaje(ClienteEstado *cliente, Mensaje *destino);

#endif
