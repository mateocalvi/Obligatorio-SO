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

const int MAX_PROD_POSTRES = 6;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    int *mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, 0);

    while (1) {
        sem_wait(sem_heladera); // Esperar espacio disponible
        while (mem[3] >= MAX_PROD_POSTRES) {
            sleep(1);
        }
        mem[3]++;       // prodpostres
        mem[1]++;       // postres
        mem[4] = 2;     // estado_bandeja = postres
        printf("Repostero: generó un postre. Postres en heladera: %d\n", mem[1]);
        sleep(rand() % 3 + 1);
        sem_post(sem_heladera); // Señalar producción
    }

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_heladera);
    return 0;
}
