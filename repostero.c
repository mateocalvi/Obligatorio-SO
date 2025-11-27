// Librerías estándar para I/O, memoria, semáforos y memoria compartida
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include "constantes.h"

// Nombres de las memorias compartidas
#define SHM_MOSTRADOR "/shm_mostrador"
#define SHM_HELADERA  "/shm_heladera"
// Tamaños de los buffers para memoria compartida
#define BUFFERSIZE_HELADERA (1 * sizeof(int))
#define BUFFERSIZE_MOSTRADOR (3 * sizeof(int))

int main() {
    // Abrir memorias compartidas existentes
    int shmfd_m = shm_open(SHM_MOSTRADOR, O_RDWR, 0);
    int shmfd_h = shm_open(SHM_HELADERA, O_RDWR, 0);

    if (shmfd_m == -1 || shmfd_h == -1) exit(1);

    // Mapear memoria compartida al espacio de direcciones del proceso
    int *memMostrador = mmap(NULL, BUFFERSIZE_MOSTRADOR, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_m, 0);
    int *memHeladera  = mmap(NULL, BUFFERSIZE_HELADERA, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_h, 0);

    if (memMostrador == MAP_FAILED || memHeladera == MAP_FAILED) exit(1);

    // Abrir semáforos existentes
    sem_t *semheladera = sem_open(SEM_HELADERA, 0);
    sem_t *semmutex = sem_open(SEM_MUTEX, 0);
    sem_t *sembarrera = sem_open(SEM_BARRERA, 0);

    if (semheladera == SEM_FAILED || semmutex == SEM_FAILED || sembarrera == SEM_FAILED) exit(1);

    // Bucle principal del repostero
    while (1) {
        // Leer estado actual con exclusión mutua
        sem_wait(semmutex);
        int flag_fin = memMostrador[2];
        int postres = memHeladera[0];
        int pedidos_restantes = memMostrador[1];
        sem_post(semmutex);

        // Verificar condiciones de terminación
        if (flag_fin || pedidos_restantes == 0) break;

        // Si no hay postres en la heladera, hacer postres
        if (postres == 0) {
            for (int i = 0; i < MAX_POSTRES_HELADERA; i++) {
                // Verificar si debemos continuar
                sem_wait(semmutex);
                if (memMostrador[2] || memMostrador[1] == 0) {
                    sem_post(semmutex);
                    break;
                }
                // Incrementar contador de postres
                memHeladera[0]++;
                sem_post(semmutex);

                // Simular tiempo de preparación
                usleep(100000);
                // Señalizar que hay un postre disponible
                sem_post(semheladera);
                printf("Repostero %d hizo un postre. Postres: %d\n", getpid(), memHeladera[0]);
            }
        } else {
            // Esperar si ya hay postres disponibles
            usleep(100000);
        }
    }

    // Liberar recursos
    sem_post(sembarrera);
    munmap(memMostrador, BUFFERSIZE_MOSTRADOR);
    munmap(memHeladera, BUFFERSIZE_HELADERA);
    close(shmfd_m);
    close(shmfd_h);
    sem_close(semheladera);
    sem_close(semmutex);
    sem_close(sembarrera);

    return 0;
}
