#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include "constantes.h"

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (shm_fd == -1) {
        exit(1);
    }
    
    int *mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mem == MAP_FAILED) {
        exit(1);
    }
    
    sem_t *sem_mesada = sem_open(SEM_MESADA, 0);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);
    sem_t *sem_barrera = sem_open(SEM_BARRERA, 0);

    if (sem_mesada == SEM_FAILED || sem_heladera == SEM_FAILED || 
        sem_mutex == SEM_FAILED || sem_barrera == SEM_FAILED) {
        exit(1);
    }

    srand(getpid());

    while (1) {
        // Verificar bandera de terminación y pedidos restantes
        sem_wait(sem_mutex);
        int debe_terminar = mem[3];
        int pedidos_restantes = mem[2];
        sem_post(sem_mutex);

        if (debe_terminar || pedidos_restantes <= 0) {
            break;
        }

        // Decidir qué retirar (alternar entre platos y postres)
        int retirar_plato = rand() % 2;

        if (retirar_plato) {
            // Intentar retirar plato
            if (sem_trywait(sem_mesada) == 0) {
                sem_wait(sem_mutex);
                if (mem[0] > 0 && mem[2] > 0) {
                    mem[0]--;
                    mem[2]--;
                    int restantes = mem[2];
                    printf(VERDE "🍽️  Mozo %d" RESET " → retiró un " AZUL "plato" RESET ". " 
                           "Mostrador: " NEGRITA "%d" RESET " | "
                           AMARILLO "Pedidos restantes: %d\n" RESET, 
                           getpid(), mem[0], restantes);
                    sem_post(sem_mutex);
                    
                    // Simular tiempo de entrega
                    sleep(rand() % 2 + 1);
                } else {
                    sem_post(sem_mutex);
                    sem_post(sem_mesada);
                }
            } else {
                usleep(100000);
            }
        } else {
            // Intentar retirar postre
            if (sem_trywait(sem_heladera) == 0) {
                sem_wait(sem_mutex);
                if (mem[1] > 0 && mem[2] > 0) {
                    mem[1]--;
                    mem[2]--;
                    int restantes = mem[2];
                    printf(VERDE "🍽️  Mozo %d" RESET " → retiró un " MAGENTA "postre" RESET ". " 
                           "Heladera: " NEGRITA "%d" RESET " | "
                           AMARILLO "Pedidos restantes: %d\n" RESET, 
                           getpid(), mem[1], restantes);
                    sem_post(sem_mutex);
                    
                    // Simular tiempo de entrega
                    sleep(rand() % 2 + 1);
                } else {
                    sem_post(sem_mutex);
                    sem_post(sem_heladera);
                }
            } else {
                usleep(100000);
            }
        }
    }

    // Notificar terminación a través de la barrera
    sem_post(sem_barrera);

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_mesada);
    sem_close(sem_heladera);
    sem_close(sem_mutex);
    sem_close(sem_barrera);
    return 0;
}
