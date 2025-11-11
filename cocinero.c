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
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);

    if (sem_mesada == SEM_FAILED || sem_mutex == SEM_FAILED) {
        exit(1);
    }

    while (1) {
        // Verificar bandera de terminación
        sem_wait(sem_mutex);
        int debe_terminar = mem[3];
        int platos_actuales = mem[0];
        int pedidos_restantes = mem[2];
        sem_post(sem_mutex);

        if (debe_terminar || pedidos_restantes <= 0) {
            break;
        }

        if (platos_actuales == 0) {
            // Llenar la mesada completa
            for (int i = 0; i < MAX_PLATOS_MOSTRADOR; i++) {
                // Verificar bandera antes de cada producción
                sem_wait(sem_mutex);
                debe_terminar = mem[3];
                int seguir = (mem[2] > 0 && !debe_terminar);
                sem_post(sem_mutex);
                
                if (!seguir) {
                    break;
                }
                
                sleep(1); // Simular tiempo de preparación
                
                sem_wait(sem_mutex);
                mem[0]++;
                printf(AZUL "🍳 Cocinero %d" RESET " → generó un plato. " 
                       NEGRITA "Platos en mostrador: %d\n" RESET, getpid(), mem[0]);
                sem_post(sem_mutex);
                
                sem_post(sem_mesada);
            }
            
            printf(AZUL "⏸️  Cocinero %d" RESET " → mesada llena, esperando a que se vacíe...\n", getpid());
        }
        
        sleep(1);
    }

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_mesada);
    sem_close(sem_mutex);
    return 0;
}
