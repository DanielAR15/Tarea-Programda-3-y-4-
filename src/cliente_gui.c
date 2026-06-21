/*
 * Version con interfaz grafica (GTK3) del cliente. La ventana tiene:
 *   - Un area de texto donde se muestran los mensajes recibidos.
 *   - Un campo para escribir a quien va el mensaje (numero de rank, o se puede dejar vacio si se va a usar difusion).
 *   - Un checkbox para marcar si el mensaje es de difusion.
 *   - Un campo de texto para escribir el mensaje.
 *   - Un boton de enviar.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <mpi.h>
#include "comunicacion.h"


typedef struct {
    ClienteEstado *cliente;

    GtkWidget *ventana;
    GtkWidget *area_mensajes;       
    GtkTextBuffer *buffer_mensajes;
    GtkWidget *entrada_destino;     
    GtkWidget *checkbox_difusion;  
    GtkWidget *entrada_texto;     
    GtkWidget *boton_enviar;
} DatosGui;

static void agregar_linea_chat(DatosGui *datos, const char *linea) {
    GtkTextIter fin;
    gtk_text_buffer_get_end_iter(datos->buffer_mensajes, &fin);
    gtk_text_buffer_insert(datos->buffer_mensajes, &fin, linea, -1);
    gtk_text_buffer_insert(datos->buffer_mensajes, &fin, "\n", -1);
}


static gboolean revisar_mensajes_nuevos(gpointer puntero_datos) {
    DatosGui *datos = (DatosGui *) puntero_datos;
    Mensaje msg;
    char linea[700];

    while (obtener_siguiente_mensaje(datos->cliente, &msg) == 1) {
        if (msg.tipo == TIPO_DIRECTO) {
            sprintf(linea, "%s (directo): %s", msg.nombre_origen, msg.texto);
        } else {
            sprintf(linea, "%s (difusion): %s", msg.nombre_origen, msg.texto);
        }
        agregar_linea_chat(datos, linea);
    }

    return TRUE;
}


static void al_hacer_click_enviar(GtkWidget *boton, gpointer puntero_datos) {
    DatosGui *datos = (DatosGui *) puntero_datos;

    const char *texto_destino = gtk_entry_get_text(GTK_ENTRY(datos->entrada_destino));
    const char *texto_mensaje = gtk_entry_get_text(GTK_ENTRY(datos->entrada_texto));
    gboolean es_difusion = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(datos->checkbox_difusion));

    if (strlen(texto_mensaje) == 0) {
        agregar_linea_chat(datos, "(no se envio nada, el mensaje esta vacio)");
        return;
    }

    if (es_difusion == TRUE) {
        enviar_difusion(datos->cliente, texto_mensaje);
        agregar_linea_chat(datos, "(yo, difusion): enviado a todos");

    } else {
   
        int rank_destino = atoi(texto_destino);

        if (rank_destino <= 0 || rank_destino >= datos->cliente->num_procesos) {
            agregar_linea_chat(datos, "(rank destino invalido, revisa el numero)");
            return;
        }
        if (rank_destino == datos->cliente->mi_rank) {
            agregar_linea_chat(datos, "(no te puedes enviar un mensaje a ti mismo)");
            return;
        }

        envio_directo(datos->cliente, rank_destino, texto_mensaje);
        agregar_linea_chat(datos, "(yo, directo): enviado");
    }

    gtk_entry_set_text(GTK_ENTRY(datos->entrada_texto), "");
}

static void al_cerrar_ventana(GtkWidget *ventana, gpointer puntero_datos) {
    DatosGui *datos = (DatosGui *) puntero_datos;

    informar_salida(datos->cliente);
    detener_hilo_red(datos->cliente);

    gtk_main_quit();
}

void ejecutar_cliente_gui(int rank, int num_procesos, int argc, char **argv) {
    ClienteEstado cliente;
    Inicializar_Cliente(&cliente, rank, num_procesos);
    iniciar_hilo_red(&cliente);

    gtk_init(&argc, &argv);

    DatosGui datos;
    datos.cliente = &cliente;

    datos.ventana = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    char titulo[100];
    sprintf(titulo, "Cliente de mensajeria - usuario%d (rank %d)", rank, rank);
    gtk_window_set_title(GTK_WINDOW(datos.ventana), titulo);
    gtk_window_set_default_size(GTK_WINDOW(datos.ventana), 450, 400);
    gtk_container_set_border_width(GTK_CONTAINER(datos.ventana), 10);

    GtkWidget *caja_principal = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(datos.ventana), caja_principal);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);

    datos.area_mensajes = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(datos.area_mensajes), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(datos.area_mensajes), GTK_WRAP_WORD);
    datos.buffer_mensajes = gtk_text_view_get_buffer(GTK_TEXT_VIEW(datos.area_mensajes));

    gtk_container_add(GTK_CONTAINER(scroll), datos.area_mensajes);
    gtk_box_pack_start(GTK_BOX(caja_principal), scroll, TRUE, TRUE, 0);

    GtkWidget *fila_destino = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

    GtkWidget *etiqueta_destino = gtk_label_new("Rank destino:");
    gtk_box_pack_start(GTK_BOX(fila_destino), etiqueta_destino, FALSE, FALSE, 0);

    datos.entrada_destino = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(datos.entrada_destino), "ej: 2");
    gtk_box_pack_start(GTK_BOX(fila_destino), datos.entrada_destino, FALSE, FALSE, 0);

    datos.checkbox_difusion = gtk_check_button_new_with_label("Enviar a todos (difusion)");
    gtk_box_pack_start(GTK_BOX(fila_destino), datos.checkbox_difusion, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(caja_principal), fila_destino, FALSE, FALSE, 0);

    /* fila con: campo de mensaje + boton enviar */
    GtkWidget *fila_mensaje = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

    datos.entrada_texto = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(datos.entrada_texto), "Escribe tu mensaje aqui...");
    gtk_widget_set_hexpand(datos.entrada_texto, TRUE);
    gtk_box_pack_start(GTK_BOX(fila_mensaje), datos.entrada_texto, TRUE, TRUE, 0);

    datos.boton_enviar = gtk_button_new_with_label("Enviar");
    gtk_box_pack_start(GTK_BOX(fila_mensaje), datos.boton_enviar, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(caja_principal), fila_mensaje, FALSE, FALSE, 0);

    /* conectamos las senales (eventos) a las funciones de arriba */
    g_signal_connect(datos.boton_enviar, "clicked",
                      G_CALLBACK(al_hacer_click_enviar), &datos);

    /* tambien permitimos enviar con la tecla Enter en el campo de texto */
    g_signal_connect(datos.entrada_texto, "activate",
                      G_CALLBACK(al_hacer_click_enviar), &datos);

    g_signal_connect(datos.ventana, "destroy",
                      G_CALLBACK(al_cerrar_ventana), &datos);

    /* cada 200 milisegundos revisamos si llegaron mensajes nuevos */
    g_timeout_add(200, revisar_mensajes_nuevos, &datos);

    gtk_widget_show_all(datos.ventana);

    agregar_linea_chat(&datos, "(sistema) Bienvenido. Ya estas conectado al chat.");

    gtk_main();
}
