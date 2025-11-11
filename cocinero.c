#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

const int CONTADORES = 5;
const int BUFFER_SIZE = sizeof(int) * CONTADORES;
const char *SHM_NAME = "/shared_mem";
const char *SEM_MESADA = "/sem_MESADA";
const char *SEM_MUTEX = "/sem_MUTEX";
const int MAX_PLATOS_MOSTRADOR = 27;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    int *mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    sem_t *sem_mesada = sem_open(SEM_MESADA, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);

    srand(getpid());

    while (1) {
        // Simular tiempo de preparación
        sleep(rand() % 3 + 1);

        // Esperar espacio disponible en mostrador
        sem_wait(sem_mesada);

        // Acceder a sección crítica
        sem_wait(sem_mutex);
        
        if (mem[0] < MAX_PLATOS_MOSTRADOR) {
            mem[0]++;
            printf("Cocinero %d: generó un plato. Platos en mostrador: %d\n", getpid(), mem[0]);
        }
        
        sem_post(sem_mutex);
    }

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_mesada);
    sem_close(sem_mutex);
    return 0;
}
