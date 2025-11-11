#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>

const int CONTADORES = 3;
const int BUFFER_SIZE = sizeof(int) * CONTADORES;
const char *SHM_NAME = "/shared_mem";
const char *SEM_MESADA = "/sem_MESADA";
const char *SEM_MUTEX = "/sem_MUTEX";
const int MAX_PLATOS_MOSTRADOR = 27;

int terminar = 0;

void manejar_senal(int sig) {
    terminar = 1;
}

int main() {
    signal(SIGTERM, manejar_senal);
    
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

    while (!terminar) {
        // Verificar si debo llenar la mesada
        sem_wait(sem_mutex);
        int platos_actuales = mem[0];
        int pedidos_restantes = mem[2];
        sem_post(sem_mutex);

        if (pedidos_restantes <= 0) {
            break;
        }

        if (platos_actuales == 0) {
            // Llenar la mesada completa
            for (int i = 0; i < MAX_PLATOS_MOSTRADOR && !terminar; i++) {
                sleep(1); // Simular tiempo de preparación
                
                sem_wait(sem_mutex);
                if (mem[2] > 0) {  // Solo producir si quedan pedidos
                    mem[0]++;
                    printf("Cocinero %d: generó un plato. Platos en mostrador: %d\n", getpid(), mem[0]);
                    sem_post(sem_mutex);
                    sem_post(sem_mesada); // Señalar que hay un plato disponible
                } else {
                    sem_post(sem_mutex);
                    break;
                }
            }
            
            printf("Cocinero %d: mesada llena, esperando a que se vacíe...\n", getpid());
        }
        
        sleep(1); // Esperar antes de verificar nuevamente
    }

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_mesada);
    sem_close(sem_mutex);
    return 0;
}
