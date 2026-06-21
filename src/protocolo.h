#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#define TAM_NOMBRE 32
#define TAM_TEXTO  500

#define TAG_DIRECTO  1
#define TAG_DIFUSION 2
#define TAG_SALIR    3
#define TAG_REGISTRO 4

#define TIPO_DIRECTO  0
#define TIPO_DIFUSION 1
#define TIPO_SALIR    2

typedef struct {
    int  tipo;                 
    int  rank_origen;          
    int  rank_destino;         
    char nombre_origen[TAM_NOMBRE];
    char texto[TAM_TEXTO];
} Mensaje;

static inline void limpiar_mensaje(Mensaje *m) {
    m->tipo = 0;
    m->rank_origen = 0;
    m->rank_destino = 0;
    for (int i = 0; i < TAM_NOMBRE; i++) {
        m->nombre_origen[i] = '\0';
    }
    for (int i = 0; i < TAM_TEXTO; i++) {
        m->texto[i] = '\0';
    }
}

#endif
