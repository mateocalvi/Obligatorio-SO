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
    int shmfd_mostrador = shm_open(SHM_MOSTRADOR, O_RDWR, 0);
    if (shmfd_mostrador == -1) exit(1);

    // Mapear memoria compartida al espacio de direcciones
    int *mem_mostrador = mmap(NULL, BUFFERSIZE_MOSTRADOR, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_mostrador, 0);
    if (mem_mostrador == MAP_FAILED) exit(1);

    // Abrir semáforos existentes
    sem_t *sem_mesada = sem_open(SEM_MESADA, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);
    sem_t *sem_barrera = sem_open(SEM_BARRERA, 0);

    if (sem_mesada == SEM_FAILED || sem_mutex == SEM_FAILED || sem_barrera == SEM_FAILED) exit(1);

    // Bucle principal del cocinero
    while (1) {
        // Leer estado actual con exclusión mutua
        sem_wait(sem_mutex);
        int flag_fin = mem_mostrador[2];
        int platos = mem_mostrador[0];
        int pedidos_restantes = mem_mostrador[1];
        sem_post(sem_mutex);

        // Verificar condiciones de terminación
        if (flag_fin || pedidos_restantes == 0) break;

        // Si no hay platos en el mostrador, hacer platos
        if (platos == 0) {
            for (int i = 0; i < MAX_PLATOS_MOSTRADOR; i++) {
                // Verificar si debemos continuar
                sem_wait(sem_mutex);
                if (mem_mostrador[2] || mem_mostrador[1] == 0) {
                    sem_post(sem_mutex);
                    break;
                }
                // Incrementar contador de platos
                mem_mostrador[0]++;
                sem_post(sem_mutex);

                // Simular tiempo de preparación
                usleep(100000);
                // Señalizar que hay un plato disponible
                sem_post(sem_mesada);
                printf("Cocinero %d hizo un plato. Platos: %d\n", getpid(), mem_mostrador[0]);
            }
        } else {
            // Esperar si ya hay platos disponibles
            usleep(100000);
        }
    }

    // Liberar recursos
    sem_post(sem_barrera);
    munmap(mem_mostrador, BUFFERSIZE_MOSTRADOR);
    close(shmfd_mostrador);
    sem_close(sem_mesada);
    sem_close(sem_mutex);
    sem_close(sem_barrera);

    return 0;
}
