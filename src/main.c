/*Punto de entrada de toda la aplicacion*/

#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include "coordinador.h"
#include "cliente_cli.h"
#include "cliente_gui.h"

int main(int argc, char **argv) {
    int proporcionado;
    int rank, num_procesos;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &proporcionado);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos);

    if (num_procesos < 2) {
        if (rank == 0) {
            printf("Hace falta minimo 2 procesos (1 coordinador + 1 cliente).\n");
            printf("Ejemplo: mpirun -np 3 ./cliente\n");
        }
        MPI_Finalize();
        return 0;
    }

    int usar_cli = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--cli") == 0) {
            usar_cli = 1;
        }
    }

    if (rank == 0) {
        ejecutar_coordinador(num_procesos);
        
    } else {
        if (usar_cli == 1) {
            ejecutar_cliente_cli(rank, num_procesos);
        } else {
            ejecutar_cliente_gui(rank, num_procesos, argc, argv);
        }
    }

    MPI_Finalize();
    return 0;
}
