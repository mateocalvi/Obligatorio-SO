// Librerías estándar para I/O, memoria, semáforos y memoria compartida
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include "constantes.h"

// Nombre de la memoria compartida del mostrador
#define SHM_MOSTRADOR "/shm_mostrador"
// Tamaño del buffer del mostrador (platos, pedidos, bandera_fin)
#define BUFFERSIZE_MOSTRADOR (3 * sizeof(int))

int main() {
    // Abrir memoria compartida del mostrador
    int shmfd_m = shm_open(SHM_MOSTRADOR, O_RDWR, 0);
    if (shmfd_m == -1) exit(1);

    // Mapear memoria compartida al espacio de direcciones
    int *memMostrador = mmap(NULL, BUFFERSIZE_MOSTRADOR, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_m, 0);
    if (memMostrador == MAP_FAILED) exit(1);

    // Abrir semáforos existentes
    sem_t *semmesada = sem_open(SEM_MESADA, 0);
    sem_t *semmutex = sem_open(SEM_MUTEX, 0);
    sem_t *sembarrera = sem_open(SEM_BARRERA, 0);

    if (semmesada == SEM_FAILED || semmutex == SEM_FAILED || sembarrera == SEM_FAILED) exit(1);

    // Bucle principal del cocinero
    while (1) {
        // Leer estado actual con exclusión mutua
        sem_wait(semmutex);
        int flag_fin = memMostrador[2];
        int platos = memMostrador[0];
        int pedidos_restantes = memMostrador[1];
        sem_post(semmutex);

        // Verificar condiciones de terminación
        if (flag_fin || pedidos_restantes == 0) break;

        // Si no hay platos en el mostrador, hacer platos
        if (platos == 0) {
            for (int i = 0; i < MAX_PLATOS_MOSTRADOR; i++) {
                // Verificar si debemos continuar
                sem_wait(semmutex);
                if (memMostrador[2] || memMostrador[1] == 0) {
                    sem_post(semmutex);
                    break;
                }
                // Incrementar contador de platos
                memMostrador[0]++;
                sem_post(semmutex);

                // Simular tiempo de preparación
                usleep(100000);
                // Señalizar que hay un plato disponible
                sem_post(semmesada);
                printf("Cocinero %d hizo un plato. Platos: %d\n", getpid(), memMostrador[0]);
            }
        } else {
            // Esperar si ya hay platos disponibles
            usleep(100000);
        }
    }

    // Liberar recursos
    sem_post(sembarrera);
    munmap(memMostrador, BUFFERSIZE_MOSTRADOR);
    close(shmfd_m);
    sem_close(semmesada);
    sem_close(semmutex);
    sem_close(sembarrera);

    return 0;
}
