#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

const int CONTADORES = 5;
const int BUFFER_SIZE = sizeof(int) * CONTADORES;
const char *SHM_NAME = "/shared_mem";
const char *SEM_HELADERA = "/sem_HELADERA";
const char *SEM_MUTEX = "/sem_MUTEX";
const int MAX_POSTRES_HELADERA = 25;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    int *mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    sem_t *sem_heladera = sem_open(SEM_HELADERA, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);

    srand(getpid());

    while (1) {
        // Simular tiempo de preparación
        sleep(rand() % 3 + 1);

        // Esperar espacio disponible en heladera
        sem_wait(sem_heladera);

        // Acceder a sección crítica
        sem_wait(sem_mutex);
        
        if (mem[1] < MAX_POSTRES_HELADERA) {
            mem[1]++;
            printf("Repostero: generó un postre. Postres en heladera: %d\n", mem[1]);
        }
        
        sem_post(sem_mutex);
    }

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_heladera);
    sem_close(sem_mutex);
    return 0;
}
