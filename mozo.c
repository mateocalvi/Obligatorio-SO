// Librerías estándar para I/O, memoria, semáforos y memoria compartida
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "constantes.h"

// Nombres de las memorias compartidas
#define SHM_MOSTRADOR "/shm_mostrador"
#define SHM_HELADERA  "/shm_heladera"
// Tamaños de los buffers para memoria compartida
#define BUFFERSIZE_MOSTRADOR (3 * sizeof(int))
#define BUFFERSIZE_HELADERA  (1 * sizeof(int))

int main() {
    // Abrir memorias compartidas existentes
    int shmfd_mostrador = shm_open(SHM_MOSTRADOR, O_RDWR, 0);
    int shmfd_heladera = shm_open(SHM_HELADERA, O_RDWR, 0);

    if (shmfd_mostrador == -1 || shmfd_heladera == -1) exit(1);

    // Mapear memoria compartida al espacio de direcciones del proceso
    int *mem_Mostrador = mmap(NULL, BUFFERSIZE_MOSTRADOR, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_mostrador, 0);
    int *mem_Heladera  = mmap(NULL, BUFFERSIZE_HELADERA, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_heladera, 0);

    if (mem_Mostrador == MAP_FAILED || mem_Heladera == MAP_FAILED) exit(1);

    // Abrir semáforos existentes
    sem_t *sem_mesada = sem_open(SEM_MESADA, 0);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);
    sem_t *sem_barrera = sem_open(SEM_BARRERA, 0);

    if (sem_mesada == SEM_FAILED || sem_heladera == SEM_FAILED || sem_mutex == SEM_FAILED || sem_barrera == SEM_FAILED)
        exit(1);

    // Inicializar semilla para números aleatorios
    srand(getpid());

    // Bucle principal del mozo
    while (1) {
        // Leer estado actual con exclusión mutua
        sem_wait(sem_mutex);
        int flag_fin = mem_Mostrador[2];
        int pedidos_restantes = mem_Mostrador[1];
        sem_post(sem_mutex);

        // Verificar condiciones de terminación
        if (flag_fin || pedidos_restantes == 0) break;

        // Decidir aleatoriamente si retirar plato o postre
        int retirarPlato = rand() % 2;

        // Si decide retirar plato
        if (retirarPlato) {
            // Intentar adquirir semáforo de mesada sin bloquear
            if (sem_trywait(sem_mesada) == 0) {
                sem_wait(sem_mutex);
                // Verificar que hay platos y pedidos pendientes
                if (mem_Mostrador[0] > 0 && mem_Mostrador[1] > 0) {
                    mem_Mostrador[0]--;
                    mem_Mostrador[1]--;
                    int restantes = mem_Mostrador[1];
                    printf("Mozo %d entregó un plato. Pedidos restantes: %d\n", getpid(), restantes);
                }
                sem_post(sem_mutex);
                usleep(100000);
            } else {
                // No hay platos disponibles, continuar
            }
        } else {
            // Si decide retirar postre
            if (sem_trywait(sem_heladera) == 0) {
                sem_wait(sem_mutex);
                // Verificar que hay postres y pedidos pendientes
                if (mem_Heladera[0] > 0 && mem_Mostrador[1] > 0) {
                    mem_Heladera[0]--;
                    mem_Mostrador[1]--;
                    int restantes = mem_Mostrador[1];
                    printf("Mozo %d entregó un postre. Pedidos restantes: %d\n", getpid(), restantes);
                }
                sem_post(sem_mutex);
                usleep(100000);
            } else {
                // No hay postres disponibles, continuar
            }
        }
    }

    // Liberar recursos
    sem_post(sem_barrera);
    munmap(mem_Mostrador, BUFFERSIZE_MOSTRADOR);
    munmap(mem_Heladera, BUFFERSIZE_HELADERA);
    close(shmfd_mostrador);
    close(shmfd_heladera);
    sem_close(sem_mesada);
    sem_close(sem_heladera);
    sem_close(sem_mutex);
    sem_close(sem_barrera);

    return 0;
}
