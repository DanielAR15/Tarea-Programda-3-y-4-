## Requisitos

```bash
sudo apt install build-essential libopenmpi-dev openmpi-bin libgtk-3-dev
```

## Compilar

```bash
make
```

Genera el ejecutable `cliente`. Para limpiar: `make clean`.

## Ejecutar

**Con interfaz grafica (GUI):**
```bash
mpirun -np 4 ./cliente
```

**Con linea de comandos (CLI):**
```bash
mpirun -np 4 ./cliente --cli
```

El rank 0 siempre es el coordinador; los demas ranks son clientes. `N` en
`-np N` debe ser al menos 2.

> Nota: si lanzas varios clientes CLI con un solo `mpirun`, todos comparten
> el mismo `stdin` de la terminal. Para probar mensajes entre usuarios sin
> ese problema, usa la GUI (cada ventana es independiente).

## Comandos del CLI

| Comando | Que hace |
|---|---|
| `enviar <rank> <texto>` | Mensaje directo a ese rank |
| `difundir <texto>` | Mensaje a todos los demas |
| `salir` | Cierra el cliente |
| `ayuda` | Lista de comandos |

## Como funciona

- Cada cliente tiene un nombre fijo segun su rank (`usuario1`, `usuario2`, etc).
- Todo mensaje pasa primero por el coordinador (rank 0), quien decide si lo
  reenvia a un solo cliente o a todos.
- El mensaje se manda como un solo `struct Mensaje` de tamano fijo
  (`src/protocolo.h`), usando `MPI_Send`/`MPI_Recv` con `MPI_BYTE`.
- Cada cliente usa dos hilos: uno para la interfaz (CLI o GUI) y otro
  dedicado solo a recibir mensajes con `MPI_Recv`, asi la interfaz nunca se
  congela esperando un mensaje.

## Estructura

```
cliente-mensajeria/
├── Makefile
├── README.md
└── src/
    ├── main.c            -> arranca MPI y decide el rol de cada proceso
    ├── protocolo.h        -> struct Mensaje y tags
    ├── coordinador.c/.h   -> logica del rank 0
    ├── comunicacion.c/.h  -> hilo de red y envio de mensajes
    ├── cliente_cli.c/.h   -> interfaz de linea de comandos
    └── cliente_gui.c/.h   -> interfaz grafica (GTK 3)
```
## Referencia

```
Claude.ai Se hizo uso de la inteligencia artificial con el fin de poder avanzar 
