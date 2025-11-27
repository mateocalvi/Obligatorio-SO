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
    int pedidosTotales;

    // Solicitar cantidad de pedidos al usuario
    printf("SISTEMA DE GESTION DE ALBONDIGA EMBRUJADA\n");
    printf("Ingrese la cantidad de pedidos a entregar: ");
    scanf("%d", &pedidosTotales);

    // Limpiar recursos anteriores si existen
    shm_unlink(SHM_MOSTRADOR);
    shm_unlink(SHM_HELADERA);
    sem_unlink(SEM_MESADA);
    sem_unlink(SEM_HELADERA);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_BARRERA);

    // Crear memorias compartidas
    int shmfd_m = shm_open(SHM_MOSTRADOR, O_CREAT | O_RDWR, 0666);
    int shmfd_h = shm_open(SHM_HELADERA, O_CREAT | O_RDWR, 0666);

    // Establecer tamaño de las memorias compartidas
    ftruncate(shmfd_m, BUFFERSIZE_MOSTRADOR);
    ftruncate(shmfd_h, BUFFERSIZE_HELADERA);

    // Mapear memoria compartida al espacio de direcciones
    int *memMostrador = mmap(0, BUFFERSIZE_MOSTRADOR, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_m, 0);
    int *memHeladera  = mmap(0, BUFFERSIZE_HELADERA, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_h, 0);

    // Inicializar variables compartidas
    memMostrador[0] = 0; // platos en mostrador
    memMostrador[1] = pedidosTotales; // pedidos restantes (global)
    memMostrador[2] = 0; // bandera de fin
    memHeladera[0]  = 0; // postres en heladera

    // Crear semáforos
    sem_t *semmesada = sem_open(SEM_MESADA, O_CREAT, 0666, 0);
    sem_t *semheladera = sem_open(SEM_HELADERA, O_CREAT, 0666, 0);
    sem_t *semmutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t *sembarrera = sem_open(SEM_BARRERA, O_CREAT, 0666, 0);

    if (semmesada == SEM_FAILED || semheladera == SEM_FAILED || semmutex == SEM_FAILED || sembarrera == SEM_FAILED) {
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
        sem_wait(semmutex);
        int restantes = memMostrador[1];
        sem_post(semmutex);

        if (restantes <= 0) break;
        usleep(100000);
    }

    // Señalizar terminación a todos los procesos
    sem_wait(semmutex);
    memMostrador[2] = 1;
    sem_post(semmutex);

    // Esperar a que todos los procesos terminen
    for (int i = 0; i < TOTAL_PROCESOS; i++) {
        sem_wait(sembarrera);
    }

    printf("Todos los pedidos han sido entregados!\n");

    // Esperar a que todos los procesos hijos terminen
    for (int i = 0; i < TOTAL_PROCESOS; i++) {
        wait(NULL);
    }

    // Liberar memoria compartida
    munmap(memMostrador, BUFFERSIZE_MOSTRADOR);
    munmap(memHeladera, BUFFERSIZE_HELADERA);
    close(shmfd_m);
    close(shmfd_h);

    // Eliminar memorias compartidas
    shm_unlink(SHM_MOSTRADOR);
    shm_unlink(SHM_HELADERA);

    // Cerrar semáforos
    sem_close(semmesada);
    sem_close(semheladera);
    sem_close(semmutex);
    sem_close(sembarrera);

    // Eliminar semáforos
    sem_unlink(SEM_MESADA);
    sem_unlink(SEM_HELADERA);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_BARRERA);

    return 0;
}
