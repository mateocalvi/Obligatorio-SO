// Librerías estándar para I/O, memoria, semáforos y procesos
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include "constantes.h"

// Nombres de las memorias compartidas
#define SHM_MOSTRADOR "/shm_mostrador"
#define SHM_HELADERA  "/shm_heladera"
// Tamaños de los buffers para memoria compartida
#define BUFFERSIZE_MOSTRADOR (3 * sizeof(int)) // platos, pedidos_restantes, bandera_fin
#define BUFFERSIZE_HELADERA  (1 * sizeof(int)) // postres

int main() {
    int pedidos_totales;

    // Solicitar cantidad de pedidos al usuario
    printf("SISTEMA DE GESTION DE ALBONDIGA EMBRUJADA\n");
    printf("Ingrese la cantidad de pedidos a entregar: ");
    scanf("%d", &pedidos_totales);

    // Limpiar recursos anteriores si existen
    shm_unlink(SHM_MOSTRADOR);
    shm_unlink(SHM_HELADERA);
    sem_unlink(SEM_MESADA);
    sem_unlink(SEM_HELADERA);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_BARRERA);

    // Crear memorias compartidas
    int shmfd_mostrador = shm_open(SHM_MOSTRADOR, O_CREAT | O_RDWR, 0666);
    int shmfd_heladera = shm_open(SHM_HELADERA, O_CREAT | O_RDWR, 0666);

    // Establecer tamaño de las memorias compartidas
    ftruncate(shmfd_mostrador, BUFFERSIZE_MOSTRADOR);
    ftruncate(shmfd_heladera, BUFFERSIZE_HELADERA);

    // Mapear memoria compartida al espacio de direcciones
    int *mem_mostrador = mmap(0, BUFFERSIZE_MOSTRADOR, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_mostrador, 0);
    int *mem_heladera  = mmap(0, BUFFERSIZE_HELADERA, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_heladera, 0);

    // Inicializar variables compartidas
    mem_mostrador[0] = 0; // platos en mostrador
    mem_mostrador[1] = pedidos_totales; // pedidos restantes (global)
    mem_mostrador[2] = 0; // bandera de fin
    mem_heladera[0]  = 0; // postres en heladera

    // Crear semáforos
    sem_t *sem_mesada = sem_open(SEM_MESADA, O_CREAT, 0666, 0);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, O_CREAT, 0666, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t *sem_barrera = sem_open(SEM_BARRERA, O_CREAT, 0666, 0);

    if (sem_mesada == SEM_FAILED || sem_heladera == SEM_FAILED || sem_mutex == SEM_FAILED || sem_barrera == SEM_FAILED) {
        perror("Error abriendo semaforos");
        exit(1);
    }

    printf("Iniciando procesos...\n");

    // Crear procesos cocineros
    for (int i = 0; i < NUM_COCINEROS; i++) {
        if (fork() == 0) {
            execl("./cocinero", "./cocinero", NULL);
            perror("execl cocinero");
            exit(1);
        }
    }

    // Crear procesos reposteros
    for (int i = 0; i < NUM_REPOSTEROS; i++) {
        if (fork() == 0) {
            execl("./repostero", "./repostero", NULL);
            perror("execl repostero");
            exit(1);
        }
    }

    // Crear procesos mozos
    for (int i = 0; i < NUM_MOZOS; i++) {
        if (fork() == 0) {
            execl("./mozo", "./mozo", NULL);
            perror("execl mozo");
            exit(1);
        }
    }

    // Esperar a que se completen todos los pedidos
    while (1) {
        sem_wait(sem_mutex);
        int restantes = mem_mostrador[1];
        sem_post(sem_mutex);

        if (restantes <= 0) break;
        usleep(100000);
    }

    // Señalizar terminación a todos los procesos
    sem_wait(sem_mutex);
    mem_mostrador[2] = 1;
    sem_post(sem_mutex);

    // Esperar a que todos los procesos terminen
    for (int i = 0; i < TOTAL_PROCESOS; i++) {
        sem_wait(sem_barrera);
    }

    printf("Todos los pedidos han sido entregados!\n");

    // Esperar a que todos los procesos hijos terminen
    for (int i = 0; i < TOTAL_PROCESOS; i++) {
        wait(NULL);
    }

    // Liberar memoria compartida
    munmap(mem_mostrador, BUFFERSIZE_MOSTRADOR);
    munmap(mem_heladera, BUFFERSIZE_HELADERA);
    close(shmfd_mostrador);
    close(shmfd_heladera);

    // Eliminar memorias compartidas
    shm_unlink(SHM_MOSTRADOR);
    shm_unlink(SHM_HELADERA);

    // Cerrar semáforos
    sem_close(sem_mesada);
    sem_close(sem_heladera);
    sem_close(sem_mutex);
    sem_close(sem_barrera);

    // Eliminar semáforos
    sem_unlink(SEM_MESADA);
    sem_unlink(SEM_HELADERA);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_BARRERA);

    return 0;
}
