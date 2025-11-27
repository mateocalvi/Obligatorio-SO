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
    int shmfd_mostrador = shm_open(SHM_MOSTRADOR, O_RDWR, 0);
    int shmfd_heladera = shm_open(SHM_HELADERA, O_RDWR, 0);

    if (shmfd_mostrador == -1 || shmfd_heladera == -1) exit(1);

    // Mapear memoria compartida al espacio de direcciones del proceso
    int *mem_mostrador = mmap(NULL, BUFFERSIZE_MOSTRADOR, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_mostrador, 0);
    int *mem_heladera  = mmap(NULL, BUFFERSIZE_HELADERA, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_heladera, 0);

    if (mem_mostrador == MAP_FAILED || mem_heladera == MAP_FAILED) exit(1);

    // Abrir semáforos existentes
    sem_t *sem_heladera = sem_open(SEM_HELADERA, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);
    sem_t *sem_barrera = sem_open(SEM_BARRERA, 0);

    if (sem_heladera == SEM_FAILED || sem_mutex == SEM_FAILED || sem_barrera == SEM_FAILED) exit(1);

    // Bucle principal del repostero
    while (1) {
        // Leer estado actual con exclusión mutua
        sem_wait(sem_mutex);
        int flag_fin = mem_mostrador[2];
        int postres = mem_heladera[0];
        int pedidos_restantes = mem_mostrador[1];
        sem_post(sem_mutex);

        // Verificar condiciones de terminación
        if (flag_fin || pedidos_restantes == 0) break;

        // Si no hay postres en la heladera, hacer postres
        if (postres == 0) {
            for (int i = 0; i < MAX_POSTRES_HELADERA; i++) {
                // Verificar si debemos continuar
                sem_wait(sem_mutex);
                // Si hay que terminar o se acabaron los pedidos, salir
                if (mem_mostrador[2] || mem_mostrador[1] == 0) {
                    sem_post(sem_mutex);
                    break;
                }
                // Incrementar contador de postres
                mem_heladera[0]++;
                sem_post(sem_mutex);

                // Simular tiempo de preparación
                usleep(100000);
                // Señalizar que hay un postre disponible
                sem_post(sem_heladera);
                printf("Repostero %d hizo un postre. Postres: %d\n", getpid(), mem_heladera[0]);
            }
        } else {
            // Esperar si ya hay postres disponibles
            usleep(100000);
        }
    }

    // Liberar recursos
    sem_post(sem_barrera);
    munmap(mem_mostrador, BUFFERSIZE_MOSTRADOR);
    munmap(mem_heladera, BUFFERSIZE_HELADERA);
    close(shmfd_mostrador);
    close(shmfd_heladera);
    sem_close(sem_heladera);
    sem_close(sem_mutex);
    sem_close(sem_barrera);

    return 0;
}
