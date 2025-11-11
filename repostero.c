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
    
    sem_t *sem_heladera = sem_open(SEM_HELADERA, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);
    sem_t *sem_barrera = sem_open(SEM_BARRERA, 0);

    if (sem_heladera == SEM_FAILED || sem_mutex == SEM_FAILED || sem_barrera == SEM_FAILED) {
        exit(1);
    }

    while (1) {
        // Verificar bandera de terminación
        sem_wait(sem_mutex);
        int debe_terminar = mem[3];
        int postres_actuales = mem[1];
        int pedidos_restantes = mem[2];
        sem_post(sem_mutex);

        if (debe_terminar || pedidos_restantes <= 0) {
            break;
        }

        if (postres_actuales == 0) {
            // Llenar la heladera completa
            for (int i = 0; i < MAX_POSTRES_HELADERA; i++) {
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
                mem[1]++;
                printf(MAGENTA "🍰 Repostero" RESET " → generó un postre. " 
                       NEGRITA "Postres en heladera: %d\n" RESET, mem[1]);
                sem_post(sem_mutex);
                
                sem_post(sem_heladera);
            }
            
            printf(MAGENTA "⏸️  Repostero" RESET " → heladera llena, esperando a que se vacíe...\n");
        }
        
        sleep(1);
    }

    // Notificar terminación a través de la barrera
    sem_post(sem_barrera);

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_heladera);
    sem_close(sem_mutex);
    sem_close(sem_barrera);
    return 0;
}
